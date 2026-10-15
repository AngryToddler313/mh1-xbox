/* MH1 network loader (docs/xbox.md "Network loader").
 *
 * Installed once as E:\Games\MH1\default.xbe, so Evolution X lists it as
 * "Monster Hunter". On each start it:
 *   1. sends the last game log (E:\Games\MH1\mh1log.txt) to the laptop,
 *   2. downloads the newest game build (mh1.xbe) when the laptop has a new one,
 *   3. starts the game.
 * When the game exits it starts this loader again (launch data "MH1X"); then
 * the loader only sends the log and goes back to the dashboard. (A first
 * version polled the laptop every 3 s for a new build; the Xbox's network
 * froze after 5 connections, ping included, cause unknown. So each start
 * makes at most 3 connections, and the next start comes from the pad.)
 * The laptop is found at the address in E:\Games\MH1\loader.txt
 * ("192.168.1.166:8765" when the file is missing). Only our own build files
 * travel this way; there is no login.
 * When the laptop runs tools/xbox_serve.py --data FOLDER, /version ends in a "DATA" line and
 * the loader first installs that game folder (install.c: E:\Games\MHG or the MH1 data),
 * resuming where it stopped; a finished G folder is then started.
 */
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <nxdk/mount.h>
#include <nxdk/net.h>
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../xnet_http.h"
#include "install.h"

extern struct netif *g_pnetif;

#define DIR      "E:\\Games\\MH1\\"
#define GAME     DIR "mh1.xbe"
#define GAME_VER DIR "mh1.ver"
#define GAME_LOG DIR "mh1log.txt"
#define HOSTFILE DIR "loader.txt"
#define TEST_INPUT DIR "data\\input.txt"
#define TEST_ENV   DIR "data\\env.txt"

static char *read_file(const char *path, int *n)
{
    FILE *f = fopen(path, "rb");
    char *b;
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    *n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if ((b = malloc(*n + 1)) != NULL) {
        *n = fread(b, 1, *n, f);
        b[*n] = 0;
    }
    fclose(f);
    return b;
}

/* one WriteFile for the whole file: pdclib's fwrite hands the kernel 1 KB at a time, which
 * took ~10 s for the 3.5 MB game in xemu (and the screen sat at "Downloading" all along;
 * on 12/13 Oct the owner saw the box "hang" right there, docs/xbox.md "Loader download froze") */
static int write_file(const char *path, const void *p, int n)
{
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD done = 0;
    BOOL ok;
    if (h == INVALID_HANDLE_VALUE)
        return -1;
    ok = WriteFile(h, p, (DWORD)n, &done, NULL);
    return CloseHandle(h) && ok && done == (DWORD)n ? 0 : -1;
}

/* the NIC must be stopped before XLaunchXBE: a program started right after
 * one that left it running hangs in nxNetInit (seen 11 Oct 2026) */
static int net_up;

static void launch(const char *xbe)
{
    if (net_up)
        nxNetShutdown();
    XLaunchXBE(xbe);
}

static void launch_game(void)
{
    FILE *f = fopen(GAME, "rb");
    if (!f) {
        debugPrint("\nNo game build at %s yet, and the laptop did not send one.\n", GAME);
        debugPrint("Back to the dashboard in 30 s.\n");
        Sleep(30000);
        launch(NULL);
    }
    fclose(f);
    debugPrint("Starting the game.\n");
    launch(GAME);
}

/* a test run's scripted input and RT_* switches (tools/xbox_serve.py --input/--env):
 * "MH1TEST <input bytes> <env bytes>" then both; a part of length 0 removes the file,
 * so no test script lingers on the disk for a normal start */
static void fetch_test(void)
{
    int n, ni, ne;
    char *b = xh_http("GET", "/test", NULL, 0, &n), *p;
    if (!b)
        return;
    if (sscanf(b, "MH1TEST %d %d", &ni, &ne) == 2 && (p = strchr(b, '\n')) != NULL &&
        ni >= 0 && ne >= 0 && p + 1 + ni + ne == b + n) {
        p++;
        if (ni)
            write_file(TEST_INPUT, p, ni);
        else
            remove(TEST_INPUT);
        if (ne)
            write_file(TEST_ENV, p + ni, ne);
        else
            remove(TEST_ENV);
        if (ni || ne)
            debugPrint("Test run: input %d bytes, env %d bytes.\n", ni, ne);
    }
    free(b);
}

int main(void)
{
    unsigned long ldt;
    const unsigned char *ld;
    int after_game, n;
    char *log, *ver, *have, *xbe, *data, game[64];

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");
    after_game = XGetLaunchInfo(&ldt, &ld) == 0 && ldt == LDT_TITLE && ld && !memcmp(ld, "MH1X", 4);
    xh_read_host(HOSTFILE);
    debugPrint("MH1 network loader\n");
    if (nxNetInit(NULL) != 0) {
        debugPrint("No network.\n");
        launch_game();
    }
    net_up = 1;
    debugPrint("Xbox %s, laptop %s:%d\n", ip4addr_ntoa(netif_ip4_addr(g_pnetif)), xh_host, xh_port);

    if ((log = read_file(GAME_LOG, &n)) != NULL) {
        char *r = xh_http("POST", "/log", log, n, &n);
        debugPrint(r ? "Sent the last game log.\n" : "Could not send the last game log.\n");
        if (r)
            remove(GAME_LOG);
        free(r);
        free(log);
    }

    if (after_game) {
        debugPrint("Game exited. Back to the dashboard.\n");
        Sleep(3000);
        launch(NULL);
    }
    if (!(ver = xh_http("GET", "/version", NULL, 0, &n))) {
        debugPrint("The laptop does not answer.\n");
        launch_game();
    }
    data = strstr(ver, "\nDATA");
    ver[strcspn(ver, "\n")] = 0;
    if (data) {
        int r = install_data(game, sizeof game);
        if (r < 0) {
            debugPrint("Back to the dashboard in 60 s.\n");
            Sleep(60000);
            launch(NULL);
        }
        if (r > 0 && !strcmp(ver, "none")) {      /* G's folder done and no build to test: start G */
            debugPrint("Starting %s in 5 s.\n", game);
            Sleep(5000);
            launch(game);
        }
    }
    have = read_file(GAME_VER, &n);
    if (strcmp(ver, "none") && (!have || strcmp(have, ver))) {
        DWORD t0 = GetTickCount(), t1;
        debugPrint("Downloading build %s ...\n", ver);
        if ((xbe = xh_http("GET", "/mh1.xbe", NULL, 0, &n)) != NULL) {
            t1 = GetTickCount();
            debugPrint("Got %d bytes in %lu ms. Writing it ...\n", n, (unsigned long)(t1 - t0));
            if (write_file(GAME, xbe, n) == 0 && write_file(GAME_VER, ver, strlen(ver)) == 0)
                debugPrint("Written in %lu ms.\n", (unsigned long)(GetTickCount() - t1));
            else
                debugPrint("Could not write %s; starting the copy on the disk.\n", GAME);
            free(xbe);
        } else
            debugPrint("Download failed; starting the copy on the disk.\n");
    }
    fetch_test();
    launch_game();
}
