/*
 * kbm_xbox.c - a USB keyboard and mouse on the Xbox, for the bug reporter (pick.c).
 *
 * nxdk's USB host stack (lib/usb) has a HID class driver that its nxdk_usb.lib leaves out (NXDK_USB_ENABLE_HID);
 * tools/build_xbox.py compiles its two files (hid_core.c, hid_driver.c) into the game with the on-screen debug
 * messages switched off. kbm_init registers the driver before SDL's joystick start runs the USB stack, so a keyboard
 * or mouse plugged in at power-on (on a pad port through an adapter, behind a hub, or as an extra interface of a
 * composite pad, docs/xbox.md "Keyboard and mouse on the Xbox") is probed like the pads. Only boot-protocol
 * interfaces are used (subclass 1: protocol 1 keyboard, 2 mouse): their reports have a fixed layout (keyboard: modifier
 * byte, reserved byte, six key codes; mouse: buttons, dx, dy), so no report descriptor has to be parsed.
 * The report callback runs in the USB interrupt's DPC: it only copies into a ring (keyboard) or adds up (mouse); the
 * game thread decodes key presses in kbm_key. Not tried on a real keyboard yet (nothing in xemu or on hardware).
 */
#include "kbm_xbox.h"
#include <usbh_lib.h>
#include <usbh_hid.h>
#include <string.h>

#define MAXDEV 4
static struct {
    HID_DEV_T *hdev;
    int proto;                  /* 1 keyboard, 2 mouse */
    int started;
} dev[MAXDEV];

/* keyboard reports from the DPC: a ring of 8-byte reports */
#define KRING 64
static volatile unsigned char kring[KRING][8];
static volatile unsigned khead, ktail;
static unsigned char klast[8];
/* decoded presses waiting for kbm_key */
static int kq[32][2], kqn;
/* mouse: added up in the DPC, taken in kbm_mouse */
static volatile long mdx, mdy;
static volatile int mbuttons;
static int inited;

static void on_report(HID_DEV_T *hdev, uint16_t ep, int status, uint8_t *data, uint32_t len)
{
    int i;
    (void)ep;
    if (status || !data || !len)
        return;
    for (i = 0; i < MAXDEV; i++)
        if (dev[i].hdev == hdev)
            break;
    if (i == MAXDEV)
        return;
    if (dev[i].proto == 1 && len >= 8) {
        if (khead - ktail < KRING) {
            memcpy((void *)kring[khead % KRING], data, 8);
            khead++;
        }
    } else if (dev[i].proto == 2 && len >= 3) {
        mbuttons = data[0];
        mdx += (signed char)data[1];
        mdy += (signed char)data[2];
    }
}

static void on_connect(HID_DEV_T *hdev, int param)
{
    int i;
    (void)param;
    if (hdev->bSubClassCode != 1 || (hdev->bProtocolCode != 1 && hdev->bProtocolCode != 2))
        return;                 /* not a boot keyboard / mouse */
    for (i = 0; i < MAXDEV; i++)
        if (!dev[i].hdev) {
            dev[i].hdev = hdev;
            dev[i].proto = hdev->bProtocolCode;
            dev[i].started = 0;   /* the reads start in the game thread (service) */
            return;
        }
}

static void on_disconnect(HID_DEV_T *hdev, int param)
{
    int i;
    (void)param;
    for (i = 0; i < MAXDEV; i++)
        if (dev[i].hdev == hdev)
            dev[i].hdev = NULL;
}

void kbm_init(void)
{
    if (inited)
        return;
    inited = 1;
    usbh_core_init();           /* (SDL's joystick start calls it again: a no-op the second time) */
    usbh_hid_init();
    usbh_install_hid_conn_callback(on_connect, on_disconnect);
}

/* a newly connected device: boot protocol, no idle repeat, start its interrupt reads */
static void service(void)
{
    int i;
    for (i = 0; i < MAXDEV; i++)
        if (dev[i].hdev && !dev[i].started) {
            dev[i].started = 1;
            usbh_hid_set_protocol(dev[i].hdev, 0);
            usbh_hid_set_idle(dev[i].hdev, 0, 0);
            usbh_hid_start_int_read(dev[i].hdev, 0, on_report);
        }
}

static int have(int proto)
{
    int i;
    for (i = 0; i < MAXDEV; i++)
        if (dev[i].hdev && dev[i].proto == proto)
            return 1;
    return 0;
}

int kbm_have_keyboard(void) { return have(1); }
int kbm_have_mouse(void) { return have(2); }

int kbm_key(int *usage, int *mods)
{
    if (!inited)
        return 0;
    service();
    while (ktail != khead && kqn < 32) {        /* each new report: the key codes that were not down before */
        unsigned char r[8];
        int i, k;
        memcpy(r, (const void *)kring[ktail % KRING], 8);
        ktail++;
        for (i = 2; i < 8; i++) {
            int was = 0;
            if (r[i] < 4)               /* 0 none, 1-3 errors / roll-over */
                continue;
            for (k = 2; k < 8; k++)
                if (klast[k] == r[i])
                    was = 1;
            if (!was && kqn < 32) {
                kq[kqn][0] = r[i];
                kq[kqn][1] = r[0];
                kqn++;
            }
        }
        memcpy(klast, r, 8);
    }
    if (!kqn)
        return 0;
    *usage = kq[0][0];
    *mods = kq[0][1];
    memmove(kq, kq + 1, sizeof kq[0] * (size_t)--kqn);
    return 1;
}

char kbm_ascii(int u, int mods)
{
    static const char lo[] = "1234567890\n\x1b\b\t -=[]\\#;'`,./";
    static const char hi[] = "!@#$%^&*()\n\x1b\b\t _+{}|~:\"~<>?";
    int sh = (mods & KBM_SHIFT) != 0;
    if (u >= 4 && u <= 0x1D)            /* a-z */
        return (char)((sh ? 'A' : 'a') + u - 4);
    if (u >= 0x1E && u <= 0x38) {
        char c = (sh ? hi : lo)[u - 0x1E];
        return c >= ' ' ? c : 0;        /* Enter, Esc, Backspace, Tab: the caller's */
    }
    return 0;
}

int kbm_mouse(int *dx, int *dy, int *buttons)
{
    if (!inited)
        return 0;
    service();
    if (!have(2))
        return 0;
    *dx = (int)mdx;                     /* (a report landing between these lines is counted next time) */
    mdx -= *dx;
    *dy = (int)mdy;
    mdy -= *dy;
    *buttons = mbuttons;
    return 1;
}
