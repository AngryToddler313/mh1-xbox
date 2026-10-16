/* Game data install through the network loader (docs/xbox.md "Installing game data through
 * the loader"). Evolution X's FTP server stalls on big files and can't resume, so the
 * loader fetches the game folder from tools/xbox_serve.py --data instead:
 *   GET /manifest?game=mhg   "MH1DATA <folder> <chunk> <files>", then per file
 *                            "F <size> <crc32> <path>" and "C <crc32 of each chunk> ..."
 *   GET /data/<path>?game=mhg with "Range: bytes=N-": the file from N on.
 * <folder> must be this loader's game (MH1 / MHG); each file goes to the loader's own folder
 * (E:\Games\MHG\<path>), never over the loader (default.xbe). What is already on the disk is checked chunk
 * by chunk against the manifest's checksums and kept up to the first bad or missing chunk;
 * the download starts there. Each chunk is checked in memory before it is written (one
 * WriteFile per 4 MB chunk; pdclib's fwrite would go 1 KB at a time). A dropped or stalled
 * connection (10 s without data) is retried from the last good chunk, 20 times in a row
 * without progress (about 2 minutes with the laptop away) before it gives up. When a file is
 * complete it is read back and checked once more, then noted in <folder>\mh1data.txt
 * ("<size> <crc32> <path>" lines), so the next start skips it without reading it. */
#include <hal/debug.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "install.h"
#include "../xnet_http.h"

extern int nextRow, nextCol;    /* nxdk's debugPrint cursor: the progress line is rewritten in place */

#define MAXFILES 8

typedef struct {
    unsigned long size, crc, nchunk;
    unsigned long *chunk;       /* crc32 of each chunk */
    char path[64];              /* "data/AFS00.AFS" */
} DataFile;

static unsigned long crc_tab[256];

static void crc_init(void)
{
    unsigned long c;
    int i, k;
    for (i = 0; i < 256; i++) {
        for (c = i, k = 0; k < 8; k++)
            c = c & 1 ? 0xEDB88320UL ^ (c >> 1) : c >> 1;
        crc_tab[i] = c;
    }
}

static unsigned long crc32_of(const unsigned char *p, unsigned long n)
{
    unsigned long c = 0xFFFFFFFFUL;
    while (n--)
        c = crc_tab[(c ^ *p++) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFUL;
}

/* folder names and paths from the laptop: letters, digits, _ . - and / only, no "..", so
 * nothing outside E:\Games\<folder> can be written */
static int safe_name(const char *s, int slash)
{
    if (!*s || *s == '/' || *s == '.' || strstr(s, ".."))
        return 0;
    for (; *s; s++)
        if (!((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') ||
              *s == '_' || *s == '.' || *s == '-' || (slash && *s == '/')))
            return 0;
    return 1;
}

typedef struct {
    HANDLE h;
    DataFile *f;
    unsigned long done;         /* bytes checked and written */
    unsigned char *buf;         /* one chunk */
    unsigned long fill, chunk;
    int bad;                    /* a chunk with the wrong checksum came in */
    DWORD t0;
    unsigned long from;
    int row;
} Fetch;

/* speed in tenths of MB/s (nxdk's printf has no %f, docs/xbox.md "Floats in Xbox logs") */
static int on_line;              /* the cursor sits on a progress line */

static void progress(const char *what, const char *path, unsigned long done, unsigned long size, unsigned long tenths)
{
    int row = nextRow, col = nextCol;
    if (tenths)
        debugPrint("%s %-16s %4lu / %4lu MB  %lu.%lu MB/s      \n", what, path, done >> 20, size >> 20,
                   tenths / 10, tenths % 10);
    else
        debugPrint("%s %-16s %4lu / %4lu MB                \n", what, path, done >> 20, size >> 20);
    nextRow = row;               /* the next update overwrites this line */
    nextCol = col;
    on_line = 1;
}

static void progress_done(void)
{
    if (on_line)
        debugPrint("\n");
    on_line = 0;
}

static int on_data(void *ctx, const char *p, int n)
{
    Fetch *x = ctx;
    while (n > 0) {
        unsigned long want = x->f->size - x->done < x->chunk ? x->f->size - x->done : x->chunk;
        unsigned long k = want - x->fill < (unsigned long)n ? want - x->fill : (unsigned long)n;
        DWORD wr;
        memcpy(x->buf + x->fill, p, k);
        x->fill += k;
        p += k;
        n -= k;
        if (x->fill == want) {
            if (crc32_of(x->buf, want) != x->f->chunk[x->done / x->chunk]) {
                x->bad = 1;
                return 1;
            }
            if (!WriteFile(x->h, x->buf, want, &wr, NULL) || wr != want)
                return 1;
            x->done += want;
            x->fill = 0;
            {
                DWORD ms = GetTickCount() - x->t0;
                progress("Copying ", x->f->path, x->done, x->f->size,
                         ms ? (unsigned long)((unsigned long long)(x->done - x->from) * 10000 / 1048576 / ms) : 0);
            }
        }
    }
    return 0;
}

/* bytes at the start of the file that match the manifest, in whole chunks (the whole file
 * when it is complete) */
static unsigned long check_disk(const char *full, DataFile *f, unsigned char *buf, unsigned long chunk, int show)
{
    HANDLE h = CreateFileA(full, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    unsigned long done = 0, i;
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    for (i = 0; i < f->nchunk; i++) {
        unsigned long want = f->size - done < chunk ? f->size - done : chunk;
        DWORD rd = 0;
        if (!ReadFile(h, buf, want, &rd, NULL) || rd != want || crc32_of(buf, want) != f->chunk[i])
            break;
        done += want;
        if (show && (i & 7) == 7)
            progress("Checking", f->path, done, f->size, 0);
    }
    CloseHandle(h);
    return done;
}

static int fetch_file(const char *dir, const char *game, DataFile *f, unsigned char *buf, unsigned long chunk)
{
    char full[280], url[112], *s;
    Fetch x;
    int round, fails;
    unsigned long done;

    snprintf(full, sizeof full, "%s%s", dir, f->path);
    for (s = full; *s; s++)
        if (*s == '/') {
            *s = 0;
            CreateDirectoryA(full, NULL);       /* E:\Games\MHG\data; fails harmlessly when it exists */
            *s = '\\';
        }
    snprintf(url, sizeof url, "/data/%s?game=%s", f->path, game);
    for (round = 0; round < 3; round++) {
        progress("Checking", f->path, 0, f->size, 0);
        done = check_disk(full, f, buf, chunk, 1);
        if (done == f->size) {
            HANDLE h = CreateFileA(full, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (h != INVALID_HANDLE_VALUE) {    /* cut off anything past the end (an older, longer file) */
                SetFilePointer(h, (LONG)f->size, NULL, FILE_BEGIN);
                SetEndOfFile(h);
                CloseHandle(h);
            }
            progress("OK      ", f->path, done, f->size, 0);
            progress_done();
            return 0;
        }
        for (fails = 0; done < f->size && fails < 20;) {
            long r;
            memset(&x, 0, sizeof x);
            x.h = CreateFileA(full, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (x.h == INVALID_HANDLE_VALUE) {
                progress_done();
                debugPrint("Cannot write %s\n", full);
                return -1;
            }
            SetFilePointer(x.h, (LONG)done, NULL, FILE_BEGIN);
            x.f = f;
            x.done = x.from = done;
            x.buf = buf;
            x.chunk = chunk;
            x.t0 = GetTickCount();
            r = xh_get_range(url, done, on_data, &x);
            CloseHandle(x.h);
            if (x.done > done)
                fails = 0;
            else
                fails++;
            done = x.done;
            if (r < 0 && done < f->size) {
                progress_done();
                debugPrint(x.bad ? "  bad checksum at %lu MB, again from there\n"
                                 : "  connection lost at %lu MB, resuming in 3 s\n", done >> 20);
                Sleep(3000);
            }
        }
        if (done < f->size)
            break;
    }
    progress_done();
    debugPrint("%s: could not complete it (%lu of %lu MB).\n", f->path, done >> 20, f->size >> 20);
    return -1;
}

static int parse_manifest(char *m, char *folder, unsigned long *chunk, DataFile *fl, int *nf)
{
    char *line = m, *next;
    int n = 0;
    if (sscanf(m, "MH1DATA %15s %lu %*d", folder, chunk) != 2 || !safe_name(folder, 0) ||
        *chunk < 65536 || *chunk > (16UL << 20))
        return -1;
    for (; line && *line; line = next) {
        if ((next = strchr(line, '\n')) != NULL)
            *next++ = 0;
        if (line[0] == 'F' && line[1] == ' ') {
            DataFile *f = &fl[n];
            if (n == MAXFILES || sscanf(line, "F %lu %lx %63s", &f->size, &f->crc, f->path) != 3 ||
                !safe_name(f->path, 1))
                return -1;
            /* never over the loader itself (both games' default.xbe is the loader) */
            if (!strcmp(f->path, "default.xbe"))
                return -1;
            f->nchunk = (f->size + *chunk - 1) / *chunk;
            if (!(f->chunk = calloc(f->nchunk + 1, sizeof *f->chunk)))
                return -1;
            n++;
        } else if (line[0] == 'C' && line[1] == ' ' && n) {
            DataFile *f = &fl[n - 1];
            char *p = line + 2, *e;
            unsigned long i;
            for (i = 0; i < f->nchunk; i++, p = e) {
                f->chunk[i] = strtoul(p, &e, 16);
                if (e == p)
                    return -1;
            }
        }
    }
    *nf = n;
    return n ? 0 : -1;
}

int install_data(const char *dir, const char *game)
{
    char folder[16], want[8], rec_path[240], url[48], *m, *rec;
    unsigned long chunk;
    unsigned char *buf;
    DataFile fl[MAXFILES];
    int nf = 0, n, i, failed = 0;
    DWORD t0 = GetTickCount();

    memset(fl, 0, sizeof fl);
    crc_init();
    snprintf(url, sizeof url, "/manifest?game=%s", game);
    if (!(m = xh_http("GET", url, NULL, 0, &n))) {
        debugPrint("The laptop has no data list.\n");
        return -1;
    }
    if (parse_manifest(m, folder, &chunk, fl, &nf) < 0) {
        debugPrint("The data list makes no sense; nothing installed.\n");
        free(m);
        return -1;
    }
    free(m);
    for (i = 0; i < 3 && game[i]; i++)            /* "mhg" -> "MHG" */
        want[i] = game[i] >= 'a' && game[i] <= 'z' ? game[i] - 32 : game[i];
    want[i] = 0;
    if (strcmp(folder, want)) {
        debugPrint("The laptop's data is for %s, not %s; nothing installed.\n", folder, want);
        for (i = 0; i < nf; i++)
            free(fl[i].chunk);
        return -1;
    }
    snprintf(rec_path, sizeof rec_path, "%smh1data.txt", dir);
    debugPrint("Installing game data to %s (%d files)\n", dir, nf);
    if (!(buf = malloc(chunk))) {
        debugPrint("Out of memory.\n");
        return -1;
    }
    {   /* the list of files already checked */
        FILE *f = fopen(rec_path, "rb");
        long len = 0;
        rec = NULL;
        if (f) {
            fseek(f, 0, SEEK_END);
            len = ftell(f);
            fseek(f, 0, SEEK_SET);
            if ((rec = malloc(len + 1)) != NULL)
                rec[fread(rec, 1, len, f)] = 0;
            fclose(f);
        }
        if (!rec && (rec = malloc(1)) != NULL)
            rec[0] = 0;
    }
    {   /* the new list: only this manifest's files that are complete (old or stale lines drop out) */
        static char okl[MAXFILES * 96];
        okl[0] = 0;
        for (i = 0; i < nf && rec; i++) {
            char line[96];
            HANDLE h;
            DWORD wr;
            snprintf(line, sizeof line, "%lu %08lx %s\n", fl[i].size, fl[i].crc, fl[i].path);
            if (strstr(rec, line))
                debugPrint("OK       %-16s (checked before)\n", fl[i].path);
            else if (fetch_file(dir, game, &fl[i], buf, chunk) < 0) {
                failed = 1;
                break;
            }
            strcat(okl, line);              /* note it: one write of the whole list */
            h = CreateFileA(rec_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (h != INVALID_HANDLE_VALUE) {
                WriteFile(h, okl, strlen(okl), &wr, NULL);
                CloseHandle(h);
            }
        }
    }
    free(rec);
    free(buf);
    for (i = 0; i < nf; i++)
        free(fl[i].chunk);
    if (failed) {
        debugPrint("Install not finished. Start the game again to go on from here.\n");
        return -1;
    }
    debugPrint("Game data complete in %s (%lu s).\n", dir, (unsigned long)(GetTickCount() - t0) / 1000);
    return 0;
}
