/* MH1 network loader (docs/xbox.md "Network loader").
 *
 * Installed once as E:\Games\MH1\default.xbe, so Evolution X lists it as
 * "Monster Hunter". On each start it:
 *   1. sends the last game log (E:\Games\MH1\mh1log.txt) to the laptop,
 *   2. downloads the newest game build (mh1.xbe) when the laptop has a new one,
 *   3. starts the game.
 * When the game exits it starts this loader again (launch data "MH1X"); then
 * the loader waits for a new build before starting the game again, so the
 * laptop side (tools/xbox_serve.py) can test builds without anyone at the pad.
 * The laptop is found at the address in E:\Games\MH1\loader.txt
 * ("192.168.1.166:8765" when the file is missing). Only our own build files
 * travel this way; there is no login.
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

#define DIR      "E:\\Games\\MH1\\"
#define GAME     DIR "mh1.xbe"
#define GAME_VER DIR "mh1.ver"
#define GAME_LOG DIR "mh1log.txt"
#define HOSTFILE DIR "loader.txt"

extern struct netif *g_pnetif;

static char host[64] = "192.168.1.166";
static int port = 8765;

static void read_host(void)
{
    char line[96], *colon;
    FILE *f = fopen(HOSTFILE, "r");
    if (!f)
        return;
    if (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n ")] = 0;
        if ((colon = strchr(line, ':')) != NULL) {
            *colon = 0;
            port = atoi(colon + 1);
        }
        if (line[0])
            snprintf(host, sizeof host, "%s", line);
    }
    fclose(f);
}

/* connect with a 3 s limit; -1 when the laptop does not answer */
static int dial(void)
{
    struct sockaddr_in a;
    struct timeval tv = { 3, 0 };
    fd_set w;
    int s = lwip_socket(AF_INET, SOCK_STREAM, 0), err = 0;
    socklen_t el = sizeof err;

    if (s < 0)
        return -1;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    a.sin_addr.s_addr = inet_addr(host);
    lwip_fcntl(s, F_SETFL, O_NONBLOCK);
    if (lwip_connect(s, (struct sockaddr *)&a, sizeof a) < 0 && errno != EINPROGRESS) {
        lwip_close(s);
        return -1;
    }
    FD_ZERO(&w);
    FD_SET(s, &w);
    if (lwip_select(s + 1, NULL, &w, NULL, &tv) <= 0 ||
        lwip_getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &el) < 0 || err) {
        lwip_close(s);
        return -1;
    }
    lwip_fcntl(s, F_SETFL, 0);
    tv.tv_sec = 10;
    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    return s;
}

static int send_all(int s, const void *p, int n)
{
    const char *c = p;
    while (n > 0) {
        int k = lwip_send(s, c, n, 0);
        if (k <= 0)
            return -1;
        c += k;
        n -= k;
    }
    return 0;
}

/* one HTTP/1.0 request; returns the body (malloc'd, NUL-terminated) or NULL
 * on any error or a status other than 200 */
static char *http(const char *method, const char *path, const void *body, int nbody, int *nout)
{
    char req[256], *buf = NULL, *p;
    int s, cap = 0, n = 0, k;

    if ((s = dial()) < 0)
        return NULL;
    snprintf(req, sizeof req, "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\n\r\n",
             method, path, host, nbody);
    if (send_all(s, req, strlen(req)) < 0 || (nbody && send_all(s, body, nbody) < 0))
        goto fail;
    for (;;) {
        if (cap - n < 65536) {
            cap = cap ? cap * 2 : 1 << 20;
            if (!(p = realloc(buf, cap + 1)))
                goto fail;
            buf = p;
        }
        if ((k = lwip_recv(s, buf + n, cap - n, 0)) < 0)
            goto fail;
        if (k == 0)
            break;
        n += k;
    }
    lwip_close(s);
    s = -1;
    if (!buf || n < 12 || strncmp(buf + 9, "200", 3))
        goto fail;
    buf[n] = 0;
    if (!(p = strstr(buf, "\r\n\r\n")))
        goto fail;
    p += 4;
    if ((k = strstr(buf, "Content-Length:") ? atoi(strstr(buf, "Content-Length:") + 15) : -1) >= 0 &&
        k != n - (p - buf))
        goto fail;   /* cut short */
    *nout = n - (p - buf);
    memmove(buf, p, *nout + 1);
    return buf;
fail:
    if (s >= 0)
        lwip_close(s);
    free(buf);
    return NULL;
}

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

static int write_file(const char *path, const void *p, int n)
{
    FILE *f = fopen(path, "wb");
    int ok;
    if (!f)
        return -1;
    ok = fwrite(p, 1, n, f) == (size_t)n;
    return fclose(f) == 0 && ok ? 0 : -1;
}

static void launch_game(void)
{
    FILE *f = fopen(GAME, "rb");
    if (!f) {
        debugPrint("\nNo game build at %s yet, and the laptop did not send one.\n", GAME);
        debugPrint("Back to the dashboard in 30 s.\n");
        Sleep(30000);
        XLaunchXBE(NULL);
    }
    fclose(f);
    debugPrint("Starting the game.\n");
    XLaunchXBE(GAME);
}

int main(void)
{
    unsigned long ldt;
    const unsigned char *ld;
    int after_game, n, waited = 0;
    char *log, *ver, *have, *xbe;

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");
    after_game = XGetLaunchInfo(&ldt, &ld) == 0 && ldt == LDT_TITLE && ld && !memcmp(ld, "MH1X", 4);
    read_host();
    debugPrint("MH1 network loader\n");
    if (nxNetInit(NULL) != 0) {
        debugPrint("No network.\n");
        launch_game();
    }
    debugPrint("Xbox %s, laptop %s:%d\n", ip4addr_ntoa(netif_ip4_addr(g_pnetif)), host, port);

    if ((log = read_file(GAME_LOG, &n)) != NULL) {
        char *r = http("POST", "/log", log, n, &n);
        debugPrint(r ? "Sent the last game log.\n" : "Could not send the last game log.\n");
        if (r)
            remove(GAME_LOG);
        free(r);
        free(log);
    }

    for (;;) {
        ver = http("GET", "/version", NULL, 0, &n);
        if (!ver) {
            debugPrint("The laptop does not answer.\n");
            if (after_game) {
                debugPrint("Back to the dashboard.\n");
                Sleep(5000);
                XLaunchXBE(NULL);
            }
            launch_game();
        }
        have = read_file(GAME_VER, &n);
        if (!have || strcmp(have, ver)) {
            debugPrint("Downloading build %s ...\n", ver);
            if ((xbe = http("GET", "/mh1.xbe", NULL, 0, &n)) != NULL &&
                write_file(GAME, xbe, n) == 0 && write_file(GAME_VER, ver, strlen(ver)) == 0) {
                debugPrint("Got %d bytes.\n", n);
                http("POST", "/note", "downloaded", 10, &n);
                launch_game();
            }
            debugPrint("Download failed.\n");
            free(xbe);
            Sleep(5000);
        } else if (!after_game) {
            launch_game();
        } else {
            if (!waited++)
                debugPrint("Game exited. Waiting for a new build (or press the Xbox's power button).\n");
            Sleep(3000);
        }
        free(have);
        free(ver);
    }
}
