/* xbox_report.c - see xbox_report.h. Used by the network loader (built with XBOX_REPORT_NET: it always has lwIP) and
 * by the game (MH1_ONLINE builds send; the plain build only saves). */
#include "xbox_report.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(XBOX_REPORT_NET) || defined(MH1_ONLINE)
#include "xnet_http.h"
#define HAVE_NET 1
#endif

#define MAXBODY (24 << 20)      /* one POST at most; more reports go next time */

typedef struct { char *p; int n, cap; } buf_t;

static int put(buf_t *b, const void *d, int n)
{
    if (b->n + n > b->cap) {
        int cap = b->cap ? b->cap : 1 << 20;
        char *q;
        while (cap < b->n + n)
            cap *= 2;
        if (!(q = realloc(b->p, (size_t)cap)))
            return -1;
        b->p = q;
        b->cap = cap;
    }
    memcpy(b->p + b->n, d, (size_t)n);
    b->n += n;
    return 0;
}

static long file_size(const char *path)
{
    FILE *f = fopen(path, "rb");
    long n;
    if (!f)
        return -1;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fclose(f);
    return n;
}

/* one report folder into the body; -1 when it does not fit (or a read failed) */
static int add_report(buf_t *b, const char *dir, const char *name)
{
    char pat[400], path[400], head[400];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    int start = b->n, ok = 0, bad = 0;
    snprintf(head, sizeof head, "R %s\n", name);
    put(b, head, (int)strlen(head));
    snprintf(pat, sizeof pat, "%s\\%s\\*", dir, name);
    if ((h = FindFirstFileA(pat, &fd)) == INVALID_HANDLE_VALUE)
        goto fail;
    do {
        long n;
        FILE *f;
        char *d;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        snprintf(path, sizeof path, "%s\\%s\\%s", dir, name, fd.cFileName);
        bad = 1;
        if ((n = file_size(path)) < 0 || b->n + n + 512 > MAXBODY)
            break;
        snprintf(head, sizeof head, "F %ld %s\n", n, fd.cFileName);
        if (put(b, head, (int)strlen(head)) || (d = malloc((size_t)n + 1)) == NULL)
            break;
        f = fopen(path, "rb");
        if (!f || (long)fread(d, 1, (size_t)n, f) != n) {
            if (f)
                fclose(f);
            free(d);
            break;
        }
        fclose(f);
        if (put(b, d, (int)n)) {
            free(d);
            break;
        }
        free(d);
        ok++;
        bad = 0;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    if (!bad && ok && !put(b, "E\n", 2))
        return 0;
fail:
    b->n = start;
    return -1;
}

static int pending(const char *dir, const char *name)
{
    char p[400];
    snprintf(p, sizeof p, "%s\\%s\\sent.txt", dir, name);
    if (GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES)
        return 0;
    snprintf(p, sizeof p, "%s\\%s\\report.json", dir, name);
    return GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES;     /* a finished report */
}

#ifdef HAVE_NET
int xbox_report_send_pending(const char *dir, const char *game)
{
    char pat[400], names[32][64], url[64], head[32];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    buf_t b = { 0 };
    int n = 0, i, got;
    char *r;
    snprintf(pat, sizeof pat, "%s\\*", dir);
    if ((h = FindFirstFileA(pat, &fd)) == INVALID_HANDLE_VALUE)
        return 0;
    put(&b, "MH1REPORTS 00\n", 14);     /* the count is filled in below */
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == '.' || strlen(fd.cFileName) >= 64)
            continue;
        if (!pending(dir, fd.cFileName))
            continue;
        if (add_report(&b, dir, fd.cFileName) == 0)
            snprintf(names[n++], sizeof names[0], "%s", fd.cFileName);
    } while (n < 32 && n < 99 && FindNextFileA(h, &fd));
    FindClose(h);
    if (!n) {
        free(b.p);
        return 0;
    }
    snprintf(head, sizeof head, "MH1REPORTS %02d\n", n);
    memcpy(b.p, head, 14);
    snprintf(url, sizeof url, "/report?game=%s", game);
    r = xh_http("POST", url, b.p, b.n, &got);
    free(b.p);
    if (!r)
        return -1;
    free(r);
    for (i = 0; i < n; i++) {           /* went over: marked, kept on E: */
        FILE *f;
        snprintf(pat, sizeof pat, "%s\\%s\\sent.txt", dir, names[i]);
        if ((f = fopen(pat, "wb")) != NULL) {
            fputs("sent to the laptop\n", f);
            fclose(f);
        }
    }
    return n;
}
#endif

#ifdef MH1_ONLINE
int xbox_net_ready(void);               /* net_cpinet.c: the online build's network is up */
#endif

int xbox_report_send_now(const char *dir)
{
#ifdef MH1_ONLINE
    static int host_read;
    char gdir[300], *s;
    if (!xbox_net_ready())
        return 0;
    snprintf(gdir, sizeof gdir, "%s", dir);
    if ((s = strrchr(gdir, '\\')) != NULL)
        *s = 0;
    if (!host_read) {                   /* the laptop: the game folder's loader.txt, as the loader reads it */
        char lt[340];
        host_read = 1;
        snprintf(lt, sizeof lt, "%s\\loader.txt", gdir);
        xh_read_host(lt);
    }
    s = strrchr(gdir, '\\');
    return xbox_report_send_pending(dir, s && (s[1] | 0x20) == 'm' && (s[2] | 0x20) == 'h' && (s[3] | 0x20) == 'g' && !s[4]
                                    ? "mhg" : "mh1") > 0 ? 1 : -1;
#else
    (void)dir;
    return 0;
#endif
}

/* one whole file in one WriteFile (viewer_write_file: the bug report's pictures and texts). pdclib's fwrite went to the
 * disk a kilobyte at a time: saving a report took over a minute on the owner's Xbox (RELEASE_G GB5 bug 12). */
int xbox_write_file(const char *path, const void *data, size_t n)
{
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD w = 0;
    BOOL ok;
    if (h == INVALID_HANDLE_VALUE)
        return -1;
    ok = WriteFile(h, data, (DWORD)n, &w, NULL);
    CloseHandle(h);
    return ok && w == (DWORD)n ? 0 : -1;
}
