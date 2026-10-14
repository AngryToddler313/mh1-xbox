/* USB keyboard manager (f_usbkbdm, SLPM_654.95 main 0x23D370-0x23D870): a small step machine over the Sony
 * usbkb library. usbkbdm_init runs one step per call (irx load, init, info, setting, LED mode, end) through
 * usbkbdm_init_tbl, indexed by localTaskNum; each step bumps the index when it is done or resets it to 0 on
 * failure. getPS2KbData reads the keyboard once connected and packs modifier, LED and up to 7 key codes into
 * pds_kdt (the soft keyboard reads it, sk18.c). */
#include "types.h"

typedef struct USBKBINFO {
    int max_connect;
    int now_connect;
    u8 status[0x88];
} USBKBINFO;

typedef struct USBKBDATA {
    int led;
    int mkey;
    int len;
    u16 keycode[62];
} USBKBDATA;

typedef struct PDSKDT {
    u8 mkey;
    u8 led;
    u8 key[6];
    void *kinfo;    /* key[6] is written past the end, over the low byte of kinfo (as the original does) */
} PDSKDT;

extern s16 is_usbkb;
extern s8 localTaskNum;
extern s8 init_flg;
extern int pds_kinfo;
extern int sync_done_or_exec;
extern int ReadMode;
extern int Arrangement;
extern PDSKDT pds_kdt;
extern USBKBDATA kdat;
extern USBKBINFO info;
extern USBKBDATA bskdat;
extern int (*usbkbdm_init_tbl[])(s8 *);
extern int (*usbkbdm_pnp_init_tbl[])(s8 *);

int sceUsbKbInit(int *);
int sceUsbKbGetInfo(USBKBINFO *);
int sceUsbKbSync(int, int *);
int sceUsbKbSetRepeat(int, int, int);
int sceUsbKbSetCodeType(int, int);
int sceUsbKbSetArrangement(int, int);
int sceUsbKbSetReadMode(int, int);
int sceUsbKbSetLEDMode(int, int);
int sceUsbKbRead(int, USBKBDATA *);
void *memcpy(void *, const void *, int);
int usbKbConnectChk(int port);

#if !defined(G_PART)
int usbkbdm_init(void) {
    return usbkbdm_init_tbl[localTaskNum](&localTaskNum);
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_irxLoad(s8 *t) {
    (*t)++;
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_Init(s8 *t) {
    int r;

    if (init_flg == 0) {
        if (sceUsbKbInit(&r) != 0) {
            *t = 0;
            return 0;
        }
        if (r == 0) {
            *t = 0;
            return 0;
        }
        sync_done_or_exec = 0;
        init_flg = 1;
        (*t)++;
        return 1;
    }
    *t = 0;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_getInfo(s8 *t) {
    int r;

    r = sceUsbKbGetInfo(&info);
    if (r != 0) {
        *t = 0;
        return 0;
    }
    sync_done_or_exec = sceUsbKbSync(0, &r);
    if (sync_done_or_exec == 0 && r == 0) {
        if (info.now_connect == 0) {
            *t = 0;
            return 0;
        }
        (*t)++;
    }
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_setting(s8 *t) {
    int a;
    int b;
    int c;
    int d;

    if (info.status[0] == 0) {
        *t = 0;
        return 0;
    }
    a = sceUsbKbSetRepeat(0, 0, 0);
    b = sceUsbKbSetCodeType(0, 0);
    Arrangement = 1;
    c = sceUsbKbSetArrangement(0, 1);
    ReadMode = 0;
    d = sceUsbKbSetReadMode(0, 0);
    if (a == 0 && b == 0 && c == 0 && d == 0) {
        (*t)++;
    }
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_setLEDMD(s8 *t) {
    int r;

    r = sceUsbKbSetLEDMode(0, 2);
    if (r == 0) {
        sync_done_or_exec = sceUsbKbSync(0, &r);
        if (sync_done_or_exec == 0 && r == 0) {
            (*t)++;
        }
    }
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
int usbkbdm_end(s8 *t) {
    *t = 0;
    is_usbkb = 1;
    return 0;
}
#endif

#if !defined(G_PART)
u8 *getPS2KbData(void) {
    int r;

    if (is_usbkb == 0) {
        usbkbdm_pnp_init_tbl[localTaskNum](&localTaskNum);
        return 0;
    }
    if (usbKbConnectChk(0) == -1) {
        return 0;
    }
    sceUsbKbRead(0, &kdat);
    memcpy(&bskdat, &kdat, 0x88);
    sceUsbKbSync(0, &r);
    memcpy(&bskdat, &bskdat, 0x88);
    if (kdat.len == 0) {
        return 0;
    }
    pds_kdt.mkey = kdat.mkey;
    pds_kdt.led = kdat.led;
    pds_kdt.kinfo = &pds_kinfo;
    pds_kdt.key[0] = kdat.keycode[0];
    pds_kdt.key[1] = kdat.keycode[1];
    pds_kdt.key[2] = kdat.keycode[2];
    pds_kdt.key[3] = kdat.keycode[3];
    pds_kdt.key[4] = kdat.keycode[4];
    pds_kdt.key[5] = kdat.keycode[5];
    pds_kdt.key[6] = kdat.keycode[6];
    return (u8 *)&pds_kdt;
}
#endif

#if !defined(G_PART) || G_PART == 2
int usbKbConnectChk(int port) {
    int r;

    if (sceUsbKbGetInfo(&info) != 0) {
        return -1;
    }
    sceUsbKbSync(0, &r);
    if (r != 0) {
        return -1;
    }
    return info.status[port] == 0 ? -1 : 0;
}
#endif
