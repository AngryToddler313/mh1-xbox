/* lb_url - one translation unit 0x005E8890-0x005E9960: the lobby browser's URL helpers (copy / compare, scheme,
 * path, "//" "./" "../" folding into an absolute URL, bad-header list, file extension, %XX encoding) and the
 * Shift-JIS <-> EUC kanji conversion. The file-static helpers must stay in this TU: their callers keep values in
 * caller-saved registers across the calls (MWCC knows the static callee's register use), e.g. KanjiSjis2EucEx,
 * bs_url_to_absolute_url, BsUrlParse. Assembled from agent F's runs (lb_m12, lb_z135, lb_z19, lb_ap01, lb_z106,
 * lb_z20, lb_gdr12, lb_gap01, lb_z21, lb_gap02, lb_ap02) plus new C (agent B, 12 Oct 2026). */
#include "lobby_f.h"
u32 strlen();
int strncmp();
char *strncpy();
void *memmove();
extern char BsCacheCurrentBaseUrlstr[];
extern char BsCacheTmpUrlstr[];
extern char *BadHeaderList[2];
extern char UrlSchemeList[];
extern char lit_719_00666248[];
extern char lit_720_00666258[];
extern char lit_928_00666260[];

int BsUrlCompare_SS(char *a, char *b);
char *BsUrlCopy_SS(char *dst, char *src);
void BsUrlSet(char **p, char *url);
void BsUrlBaseSet(char *url);
void BsUrlBaseClear(void);
static char *bs_url_end(char *p);
static char *bs_url_last_slash(char *base, char *p);
static char *bs_url_extension(char *base, char *p);
static char *bs_url_path(char *p, char *end);
static char *bs_url_slash(char *p, char *end);
static char *bs_url_slash_slash(char *p, char *end);
static char *bs_url_dot_slash(char *p, char *end);
static char *bs_url_dot_dot_slash(char *p, char *end);
static char *bs_url_to_absolute_url(char *base, char *url, int dir);
char *BsUrlParse(char *rel, char *url);
static void _my_tolower(char *p);
int BsUrlBadHeaderGet(char *url);
static int bs_url_cmp_list(char **list, char *s);
int BsUrlSchemeGet(char *url);
int BsUrlFileExtensionGet(char *url);
int BsUrlEncode(char *dst, u8 *src);
static int sjis2euc_sub(u32 v);
static int euc2sjis_sub(u32 v);
void KanjiSjis2EucEx(u8 *s);
void KanjiEuc2SjisEx(u8 *s);

int BsUrlCompare_SS(char *a, char *b) {
    return strncmp(a, b, 0x100);
}

char *BsUrlCopy_SS(char *dst, char *src) {
    char *r;

    r = strncpy(dst, src, 0xFF);
    dst[0xFF] = 0;
    return r;
}

void BsUrlSet(char **p, char *url) {
    *p = url;
}

void BsUrlBaseSet(char *url) {
    BsUrlBaseClear();
    BsUrlCopy_SS(BsCacheCurrentBaseUrlstr, url);
}

void BsUrlBaseClear(void) {
    memset(BsCacheCurrentBaseUrlstr, 0, 0x100);
}

static char *bs_url_end(char *p) {
    while (*p != 0 && *p != '?' && *p != '#') {
        p++;
    }
    return p;
}

static char *bs_url_last_slash(char *base, char *p) {
    p--;
    while (*p != '/') {
        if (p <= base) {
            return 0;
        }
        p--;
    }
    return p;
}

static char *bs_url_extension(char *base, char *p) {
    while (*--p != '/') {
        if (p <= base) {
            return 0;
        }
        if (*p == '.') {
            return p;
        }
    }
    return 0;
}

static char *bs_url_path(char *p, char *end) {
    int st = 0;

    switch ((u8)BsUrlSchemeGet(p)) {
    case 0:
        break;
    case 1:
        return p + 6;
    default:
        for (;;) {
            switch (st) {
            case 0:
                if (p + 1 >= end) {
                    return 0;
                }
                if (p[0] == '/' && p[1] == '/') {
                    st++;
                    p++;
                }
                break;
            case 1:
                if (p >= end) {
                    return 0;
                }
                if (*p == '/') {
                    return p;
                }
                break;
            }
            p++;
        }
    }
    return 0;
}

static char *bs_url_slash(char *p, char *end) {
    while (p < end) {
        if (*p == '/') {
            return p;
        }
        p++;
    }
    return 0;
}

static char *bs_url_slash_slash(char *p, char *end) {
    while (p + 1 < end) {
        if (p[0] == '/' && p[1] == '/') {
            return p;
        }
        p++;
    }
    return 0;
}

static char *bs_url_dot_slash(char *p, char *end) {
    while (p + 2 < end) {
        if (p[0] == '/' && p[1] == '.' && p[2] == '/') {
            return p;
        }
        p++;
    }
    return 0;
}

static char *bs_url_dot_dot_slash(char *p, char *end) {
    while (p + 3 < end) {
        if (p[0] == '/' && p[1] == '.' && p[2] == '.' && p[3] == '/') {
            return p;
        }
        p++;
    }
    return 0;
}

static char *bs_url_to_absolute_url(char *base, char *url, int dir) {
    int sch;
    char *e;
    char *q;
    int n;

    sch = BsUrlSchemeGet(url);
    if ((u8)sch == 0) {
        if (*base != 0) {
            e = bs_url_end(base);
            if (*url == '/') {
                q = bs_url_path(base, e);
            } else {
                q = bs_url_last_slash(base, e);
            }
            if (q == 0) {
                return 0;
            }
            n = q - base + 1;
            memmove(url + n, url, strlen(url) + 1);
            memcpy(url, base, n);
            sch = BsUrlSchemeGet(url);
        } else {
            return 0;
        }
    }
    n = strlen(url);
    e = bs_url_end(url);
    if (bs_url_extension(url, e) == 0 && *e == 0 && dir != 0) {
        if (n >= 0xFF) {
            return 0;
        }
        if (e[-1] != '/') {
            e[0] = '/';
            e[1] = 0;
        }
    }
    if ((u8)sch != 1) {
        n = strlen(url);
        e = bs_url_end(url);
        if (bs_url_slash(bs_url_slash_slash(url, e) + 2, e) == 0) {
            if (n >= 0xFF) {
                return 0;
            }
            if (e[-1] != '/') {
                e[0] = '/';
                e[1] = 0;
            }
        }
    }
    return url;
}

/* rel: the relative URL, then (same variable) the path part of the absolute one */
char *BsUrlParse(char *rel, char *url) {
    char *e;
    char *p;

    if (BsUrlBadHeaderGet(url) != -1) {
        return 0;
    }
    if (rel == 0 || *rel == 0) {
        rel = lit_719_00666248;
    }
    p = bs_url_to_absolute_url(rel, BsCacheCurrentBaseUrlstr, 1);
    if (p == 0) {
        return 0;
    }
    url = bs_url_to_absolute_url(p, url, 0);
    if (url == 0) {
        return 0;
    }
    e = bs_url_end(url);
    rel = bs_url_path(url, e);
    if (rel == 0) {
        return 0;
    }
    while ((p = bs_url_slash_slash(rel, e)) != 0) {
        memmove(p, p + 1, strlen(p + 1) + 1);
        e = bs_url_end(rel);
    }
    switch ((u8)BsUrlSchemeGet(url)) {
    case 2:
        while ((p = bs_url_dot_dot_slash(rel, e)) != 0) {
            if (p != rel) {
                break;
            }
            memmove(p, p + 3, strlen(p + 3) + 1);
            e = bs_url_end(rel);
        }
        while ((p = bs_url_dot_dot_slash(rel, e)) != 0) {
            e = bs_url_last_slash(rel, p - 1);
            if (e == 0) {
                return 0;
            }
            memmove(e, p + 3, strlen(p + 3) + 1);
            e = bs_url_end(rel);
        }
        break;
    default:
        while ((p = bs_url_dot_dot_slash(rel, e)) != 0) {
            if (p != rel) {
                e = bs_url_last_slash(rel, p - 1);
                if (e == 0) {
                    return 0;
                }
                memmove(e, p + 3, strlen(p + 3) + 1);
            } else {
                rel += strlen(lit_720_00666258);
            }
            e = bs_url_end(rel);
        }
        rel = bs_url_path(url, e);
        break;
    }
    while ((e = bs_url_dot_slash(rel, e)) != 0) {
        memmove(e, e + 2, strlen(e + 2) + 1);
        e = bs_url_end(rel);
    }
    return url;
}

static void _my_tolower(char *p) {
    while (*p != 0) {
        if (*p >= 'A' && *p <= 'Z') {
            *p += 0x20;
        }
        p++;
    }
}

/* index of the first "bad header" prefix the url starts with, -1 if none */
int BsUrlBadHeaderGet(char *url) {
    int i;
    char **p;
    char *cur;

    i = 0;
    p = BadHeaderList;
    BsUrlCopy_SS(BsCacheTmpUrlstr, url);
    _my_tolower(BsCacheTmpUrlstr);
    cur = BadHeaderList[0];
    if (cur != 0) {
        do {
            if (strncmp(BsCacheTmpUrlstr, *p, strlen(cur)) == 0) {
                break;
            }
            p += 1;
            cur = *p;
            i += 1;
        } while (cur != 0);
    }
    if (*p != 0) {
        return i;
    }
    i = -1;
    return i;
}

static int bs_url_cmp_list(char **list, char *s) {
    int i;
    char *a;
    a = *list;
    i = 0;
    if (a != 0) {
        for (;;) {
            if (strncmp(s, *list, strlen(a)) != 0) {
                list += 1;
                a = *list;
                i += 1;
                if (a != 0) {
                    continue;
                }
            }
            break;
        }
    }
    if (*list != 0) {
        return i;
    }
    i = -1;
    return i;
}

int BsUrlSchemeGet(char *url) {
    return (u8)(bs_url_cmp_list((char **)UrlSchemeList, url) + 1);
}

int BsUrlFileExtensionGet(char *url) {
    char *e;
    char *x;

    e = bs_url_end(url);
    x = bs_url_extension(url, e);
    if (x == 0) {
        return 0;
    }
    if (x[1] != 0 && x[2] != 0 && x[3] != 0 && x + 4 == e) {
        if ((x[1] == 'P' || x[1] == 'p') && (x[2] == 'N' || x[2] == 'n') && (x[3] == 'G' || x[3] == 'g')) {
            return 2;
        }
        if ((x[1] == 'B' || x[1] == 'b') && (x[2] == 'M' || x[2] == 'm') && (x[3] == 'P' || x[3] == 'p')) {
            return 3;
        }
        if ((x[1] == 'H' || x[1] == 'h') && (x[2] == 'T' || x[2] == 't') && (x[3] == 'M' || x[3] == 'm')) {
            return 1;
        }
    }
    if (x[1] != 0 && x[2] != 0 && x[3] != 0 && x[4] != 0 && x + 5 == e) {
        if ((x[1] == 'H' || x[1] == 'h') && (x[2] == 'T' || x[2] == 't') && (x[3] == 'M' || x[3] == 'm')
            && (x[4] == 'L' || x[4] == 'l')) {
            return 1;
        }
    }
    if (x[1] != 0 && x[2] != 0 && x[3] != 0 && x[4] != 0 && x[5] != 0 && x[6] != 0 && x + 7 == e) {
        if ((x[1] == 'M' || x[1] == 'm') && (x[2] == 'M' || x[2] == 'm') && (x[3] == 'B' || x[3] == 'b')
            && (x[4] == 'B' || x[4] == 'b') && (x[5] == 'I' || x[5] == 'i') && (x[6] == 'N' || x[6] == 'n')) {
            return 4;
        }
    }
    return 0;
}

int BsUrlEncode(char *dst, u8 *src) {
    int i;
    i = 0;
    if (*src != 0) {
        do {
            *dst++ = 0x25;
            i += 3;
            *dst++ = lit_928_00666260[(*src & 0xF0) >> 4];
            *dst++ = lit_928_00666260[*src++ & 0xF];
        } while (*src != 0);
    }
    *dst = 0;
    return i;
}

static int sjis2euc_sub(u32 v) {
    u32 hi;
    u32 lo;
    int a;
    int b;
    int base;
    hi = (v >> 8) & 0xFF;
    lo = v & 0xFF;
    if (hi < 0xA0U) {
        base = 0x71;
    } else {
        base = 0xB1;
    }
    b = ((hi - base) * 2) + 1;
    if (lo >= 0x9EU) {
        a = lo - 0x7E;
        b += 1;
    } else if (lo > 0x7FU) {
        a = lo - 0x20;
    } else {
        a = lo - 0x1F;
    }
    a = a | 0x80;
    b = b | 0x80;
    if (a == 0xA0) {
        a = 0xFE;
        b -= 1;
    }
    return (b << 8) | a;
}

static int euc2sjis_sub(u32 v) {
    u32 hi;
    u32 lo;
    u32 w;

    w = v & 0xFFFF7F7F;
    hi = (w >> 8) & 0xFF;
    lo = w & 0xFF;
    lo += (hi & 1) ? 0x1F : 0x7D;
    if (lo >= 0x7F) {
        lo++;
    }
    hi = ((hi - 0x21) >> 1) + 0x81;
    if (hi > 0x9F) {
        hi += 0x40;
    }
    return (hi << 8) | lo;
}

void KanjiSjis2EucEx(u8 *s) {
    int len;
    int i;
    int j;
    u8 c;
    int w;

    len = strlen(s);
    i = 0;
    j = 0;
    for (; i < len; i++) {
        c = s[i];
        if (c < 0x80) {
            s[j++] = c;
        } else if (c >= 0xA0 && c < 0xE0) {
            s[j++] = 0x5F;
        } else {
            w = (c << 8) | s[i + 1];
            i++;
            w = sjis2euc_sub(w);
            s[j] = w >> 8;
            s[j + 1] = w;
            j += 2;
        }
    }
    s[j] = 0;
}

void KanjiEuc2SjisEx(u8 *s) {
    int len;
    int i;
    int j;
    int c;
    int w;

    len = strlen(s);
    i = 0;
    j = 0;
    for (; i < len; i++) {
        c = s[i];
        if (c < 0x80) {
            s[j++] = c;
        } else {
            c = (c << 8) | s[i + 1];
            i++;
            if (c > 0x8EA0 && c < 0x8EFF) {
                s[j++] = c;
            } else {
                w = euc2sjis_sub(c);
                s[j] = w >> 8;
                s[j + 1] = w;
                j += 2;
            }
        }
    }
    s[j] = 0;
}
