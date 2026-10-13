/*
 * fmt_check.c - run the PC port's format readers (src/pc/fmt) over every
 * entry of an AFS archive and print one line per entry:
 *     index name kind status details
 * Used by tools/mhg_data_check.py to compare MH1 and Monster Hunter G data.
 * Build: gcc -O1 -Isrc/pc/fmt tools/fmt_check.c src/pc/fmt/{afs,melt,amo,apx,ahi,aan,hits,snd}.c -lm
 * Usage: fmt_check ARCHIVE.AFS
 * Prints statistics only (counts, sizes), never file contents.
 */
#include "fmt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ends(const char *s, const char *t)
{
    size_t a = strlen(s), b = strlen(t);
    return a >= b && strcmp(s + a - b, t) == 0;
}

static void amh(const char *name, fmt_blob f)
{
    int n = fmt_link_count(f, 0), i, nv = 0, ns = 0;
    amo_model m;
    ahi_skel s;
    fmt_blob e0 = fmt_link_entry(f, 0, 0), e1 = fmt_link_entry(f, 1, 0);
    if (n < 1 || n > 64 || !e0.p) {
        printf("amh FAIL link count=%d\n", n);
        return;
    }
    if (fmt_amo_load(&m, e0, 0) != 0) {
        printf("amh FAIL amo links=%d\n", n);
        return;
    }
    for (i = 0; i < m.npart; i++) {
        nv += m.part[i].nvert;
        ns += m.part[i].nstrip;
    }
    printf("amh ok links=%d parts=%d verts=%d strips=%d mats=%d", n, m.npart, nv, ns, m.nmat);
    fmt_amo_free(&m);
    if (e1.p) {
        if (fmt_ahi_load(&s, e1, 0) == 0) {
            printf(" bones=%d", s.nbone);
            fmt_ahi_free(&s);
        } else
            printf(" ahi=FAIL");
    }
    printf("\n");
    (void)name;
}

static void tex(fmt_blob f)
{
    apx_image img;
    int n, i, ok = 0, bad = 0;
    long px = 0;
    if (fmt_apx_is_bare(f, 0)) {
        if (fmt_apx_decode(&img, f, 0) == 0) {
            printf("apx ok 1 %dx%d\n", img.w, img.h);
            free(img.rgba);
        } else
            printf("apx FAIL bare\n");
        return;
    }
    n = fmt_link_count(f, 0);
    if (n < 0 || n > 4096) {
        printf("tex FAIL count=%d\n", n);
        return;
    }
    for (i = 0; i < n; i++) {
        fmt_blob e = fmt_link_entry(f, i, 0);
        if (!e.p || !e.n)
            continue;
        if (fmt_apx_decode(&img, e, 0) == 0) {
            ok++;
            px += (long)img.w * img.h;
            free(img.rgba);
        } else
            bad++;
    }
    printf("tex %s images=%d bad=%d pixels=%ld\n", bad ? "FAIL" : "ok", ok, bad, px);
}

static void tbl(fmt_blob f)
{
    int b, s, ok = 0, bad = 0, banks = 0;
    for (b = 0; b < 64; b++) {
        uint32_t cnt;
        if (8 * (size_t)b + 8 > f.n || fmt_u32(f.p + 8 * b, 0) == 0 || fmt_u32(f.p + 8 * b, 0) > 4096)
            break;
        banks++;
        cnt = fmt_u32(f.p + 8 * b, 0);
        for (s = 0; s < (int)cnt; s++) {
            aan_motion m;
            fmt_blob e = fmt_tbl_motion(f, b, s, 0);
            if (!e.p)
                continue;
            if (fmt_aan_load(&m, e, 0) == 0) {
                ok++;
                fmt_aan_free(&m);
            } else
                bad++;
        }
    }
    printf("tbl %s banks=%d motions=%d bad=%d\n", bad || !ok ? "FAIL" : "ok", banks, ok, bad);
}

static void hits(const char *tag, fmt_blob f)
{
    float y;
    if (f.n < 0x30 || memcmp(f.p, "HITS", 4) != 0) {
        int n = fmt_link_count(f, 0), i, h = 0;
        for (i = 0; i < n && n < 16; i++) {
            fmt_blob e = fmt_link_entry(f, i, 0);
            if (e.p && e.n >= 4 && memcmp(e.p, "HITS", 4) == 0)
                h++;
        }
        printf("%s %s nomagic links=%d hits_inside=%d head=%08X\n", tag, h ? "ok" : "FAIL", n, h,
               f.n >= 4 ? fmt_u32(f.p, 0) : 0);
        return;
    }
    printf("%s ok cells=%dx%d probe=%d\n", tag, fmt_s32(f.p + 16, 0), fmt_s32(f.p + 20, 0),
           fmt_hits_ground_y(f, 0, 0, 1e9f, &y, 0));
}

static void snd(const uint8_t *d, size_t n)
{
    snd_pack p;
    int i, ok = 0;
    if (fmt_snd_open(&p, d, n) != 0) {
        printf("snd FAIL open\n");
        return;
    }
    for (i = 0; i < p.nvagi; i++) {
        uint32_t off;
        int rate, ns, lp;
        if (fmt_snd_vag(&p, i, &off, &rate) == 0 && off < p.bd_size) {
            fmt_vag_scan(p.bd + off, p.bd_size - off, &ns, &lp);
            if (ns > 0)
                ok++;
        }
    }
    printf("snd %s prog=%d sset=%d smpl=%d vag=%d vag_ok=%d se=%d\n", ok == p.nvagi ? "ok" : "FAIL",
           p.nprog, p.nsset, p.nsmpl, p.nvagi, ok, p.ntsbd);
}

int main(int argc, char **argv)
{
    fmt_afs a;
    uint32_t i;
    if (argc < 2 || fmt_afs_open(&a, argv[1]) != 0) {
        fprintf(stderr, "usage: fmt_check ARCHIVE.AFS\n");
        return 1;
    }
    for (i = 0; i < a.count; i++) {
        const char *name = a.name[i];
        size_t rn, n = 0;
        uint8_t *raw = fmt_afs_read(&a, (int)i, &rn), *d = NULL;
        fmt_blob f;
        printf("%u %s ", i, name);
        if (!raw) {
            printf("- FAIL read\n");
            continue;
        }
        if (ends(name, ".adx")) {
            adx_info h;
            int r = fmt_adx_header(&h, raw, rn);
            printf("adx %s ch=%d rate=%d samples=%u loop=%d\n", r == 0 ? "ok" : "FAIL", h.ch, h.rate, h.total, h.loop);
        } else if (ends(name, ".sfd")) {
            printf("sfd %s\n", rn > 4 && fmt_u32(raw, 1) == 0x000001BA ? "ok" : "FAIL");
        } else if (ends(name, ".snd") || ends(name, ".snp")) {
            snd(raw, rn);
        } else if (ends(name, ".HTM") || ends(name, ".PNG") || ends(name, ".GIF") || ends(name, ".bin") && rn >= 4 &&
                   memcmp(raw, "MWo3", 4) == 0) {
            printf("raw ok size=%zu\n", rn);
        } else {
            d = fmt_melt(raw, rn, &n, 0);
            if (!d) {
                printf("melt FAIL size=%zu\n", rn);
                free(raw);
                continue;
            }
            f.p = d;
            f.n = n;
            if (ends(name, "_amh.bin"))
                amh(name, f);
            else if (ends(name, "_tex.bin") || ends(name, ".apx"))
                tex(f);
            else if (ends(name, "_tbl.bin"))
                tbl(f);
            else if ((name[0] == 'l' && (name[1] == 'g' || name[1] == 'w')))
                hits(name[1] == 'g' ? (name[2] == 'w' ? "hitsgw" : "hitsg") : name[2] == 'g' ? "hitswg" : "hitsw", f);
            else
                printf("other ok melted=%zu\n", n);
            free(d);
        }
        free(raw);
    }
    fmt_afs_close(&a);
    return 0;
}
