/* lb_tu_browser - one translation unit 0x005E5F00-0x005E6E80: bs_request_queue_add, bs_request_queue_free_node, bs_request_cache_clear, bs_route_queue_add, bs_route_queue_forward, bs_route_queue_back, bs_route_queue_free_reverse, bs_route_queue_free_after, bs_route_commit, bs_page_status_flag_set, bs_page_status_flag_get, bs_cache_queue_check, bs_cache_queue_get_last, bs_cache_queue_free_node, bs_cache_queue_get, bs_source_cache_clear, bs_image_cache_clear, BsCacheInitialize, BsCacheCleanup, BsRouteForward, BsRouteForwardCheck, BsRouteBack, BsRouteBackCheck, bs_route_current_page_status, BsRouteReload, BsRouteReloadCheck, BsRouteCurrent, BsRequestHtmlGetCached, BsRequestHtmlPost. Built by tools/lbtu.py from the per-run files; functions that are
   not C yet stay original bytes (asm stubs, build/raw/*.inc). */
#include "lobby_f.h"
extern BSNODE *BcRoute_cur;
extern BSNODE BcImage_head;
void BsUrlCopy_SS();
int BsUrlCompare_SS();
BSNODE *bs_route_queue_add();
BSNODE *bs_route_queue_free_after();
static BSNODE *bs_route_queue_forward(BSNODE *head, BSNODE *p);
static BSNODE *bs_route_queue_back(BSNODE *head, BSNODE *p);
BSNODE *bs_cache_queue_get_last();
void bs_cache_queue_image_free_all();
void bs_request_queue_free_node();
void flReleaseTextureHandle_NOWAITDMA();
void flReleasePaletteHandle_NOWAITDMA();
void bs_cache_queue_free_node();
typedef struct PAIR2 { u8 a, b; } PAIR2;
extern BSNODE BcSource_head;
extern BSNODE BcRoute_head;
extern BSNODE BcRequest_head;
extern u8 BcCurrentPage[0x14];
extern u8 BsCachePost[0x1000];
BSNODE *bs_request_queue_add(BSNODE *head, char *url);
static void bs_page_status_flag_set();
static u8 *bs_route_current_page_status();
char *BsRouteCurrent();
int BsUrlFileExtensionGet();
BSNODE *bs_request_queue_add(BSNODE *head, char *url) {
    BSNODE *p;
    BSNODE *n;
    p = head;
    for (;;) {
        n = p->next;
        if (n == 0) {
            return 0;
        }
        p = n;
        if (n->used == 0) {
            BsUrlCopy_SS(n->url, url);
            n->used = 1;
            return n;
        }
    }
}

void bs_request_queue_free_node(BSNODE *head, BSNODE *node) {
    BSNODE *last;
    BSNODE *p;
    BSNODE *n;
    p = head;
    last = 0;
    for (;;) {
        n = p->next;
        if (n == 0) {
            break;
        }
        if (n == node) {
            last = p;
        }
        p = n;
    }
    if (last != 0) {
        n = node->next;
        if (n != 0) {
            last->next = n;
            p->next = node;
            node->next = 0;
        }
        memset(node->url, 0, 0x100);
        node->used = 0;
        memset(&node->x108, 0, 2);
    }
}

void bs_request_cache_clear(BSNODE *arg0) {
    bs_request_queue_free_node(&BcRequest_head, arg0);
    F(s8, arg0, 0x10C) = 0;
    F(s8, arg0, 0x10D) = 0;
    F(s32, arg0, 0x110) = 0;
    F(s32, arg0, 0x114) = 0;
    F(s32, arg0, 0x11C) = 0;
    F(s32, arg0, 0x120) = 0;
    F(s8, arg0, 0x125) = 0;
    F(s8, arg0, 0x124) = 0;
}

BSNODE *bs_route_queue_add(BSNODE *arg0, char *arg1) {
    BSNODE *temp_v0;
    BSNODE *var_s0;
    BSNODE *var_v1;

    var_v1 = 0;
    var_s0 = arg0;
loop_1:
    temp_v0 = var_s0->next;
    if (temp_v0 != 0) {
        var_v1 = var_s0;
        var_s0 = temp_v0;
        goto loop_1;
    }
    var_s0->next = arg0->next;
    arg0->next = var_s0;
    var_v1->next = 0;
    BsUrlCopy_SS(var_s0->url, arg1);
    var_s0->used = 1;
    return var_s0;
}

static BSNODE *bs_route_queue_forward(BSNODE *head, BSNODE *p) {
    BSNODE *n;
    if (head->next == p) {
        return p;
    }
    for (;;) {
        n = head->next;
        if (n == p) {
            return head;
        }
        head = n;
    }
}

static BSNODE *bs_route_queue_back(BSNODE *head, BSNODE *p) {
    BSNODE *n;
    n = p->next;
    if (n == 0) {
        return p;
    }
    if (n->used != 0) {
        return n;
    }
    return p;
}

/* original bytes: build/raw/bs_route_queue_free_reverse.inc (config/c_rawfuncs.txt); near-match C is in lb_am.c */
#ifdef __MWERKS__   /* PS2 only; the PC build takes the near-match C */
asm BSNODE *bs_route_queue_free_reverse()
{
#include "bs_route_queue_free_reverse.inc"
}
#endif

BSNODE *bs_route_queue_free_after(BSNODE *arg0, BSNODE *arg1) {
    BSNODE *var_a1;

    var_a1 = arg1;
loop_1:
    if (arg0->next != var_a1) {
        var_a1 = bs_route_queue_free_reverse(arg0, var_a1);
        goto loop_1;
    }
    return var_a1;
}

void bs_route_commit(BSNODE *head, BSNODE *req, void *c) {
    s8 k;
    k = req->x10C;
    switch (k) {
    case 4:
        BcRoute_cur = bs_route_queue_forward(head, BcRoute_cur);
        return;
    case 5:
        BcRoute_cur = bs_route_queue_back(head, BcRoute_cur);
        return;
    case 6:
        *(PAIR2 *)&BcRoute_cur->x108 = *(PAIR2 *)&req->x108;
        return;
    default:
        BcRoute_cur = bs_route_queue_free_after(head, BcRoute_cur);
        BcRoute_cur = bs_route_queue_add(head, c);
        *(PAIR2 *)&BcRoute_cur->x108 = *(PAIR2 *)&req->x108;
        return;
    }
}

static void bs_page_status_flag_set(u8 *p, int bit, int on) {
    u8 m;
    if (on != 0) {
        p[1] = p[1] | bit;
    } else {
        m = bit;
        p[1] &= ~m;
    }
}

static s32 bs_page_status_flag_get(u8 *arg0, s32 arg1) {
    return F(u8, arg0, 1) & (arg1 & 0xFFFF) & 0xFF;
}

BSNODE *bs_cache_queue_check(BSNODE *head, char *url) {
    BSNODE *p;
    BSNODE *prev;
    BSNODE *found;
    BSNODE *fprev;
    BSNODE *n;
    p = head;
    found = 0;
    fprev = 0;
    for (;;) {
        n = p->next;
        if (n != 0) {
            prev = p;
            p = n;
            if (n->used != 0 && BsUrlCompare_SS(n->url, url) == 0) {
                found = n;
                fprev = prev;
            } else {
                continue;
            }
        }
        break;
    }
    if (found != 0) {
        if (found != head->next) {
            fprev->next = found->next;
            found->next = head->next;
            head->next = found;
        }
        return found;
    }
    return 0;
}

BSNODE *bs_cache_queue_get_last(arg0)
BSNODE *arg0;
{
    BSNODE *var_a0;
    BSNODE *var_v0;

    var_a0 = arg0;
    var_v0 = 0;
    if (var_a0->next->used == 0) {
        return 0;
    }
loop_2:
    var_a0 = var_a0->next;
    if (var_a0 != 0) {
        if (var_a0->used != 0) {
            var_v0 = var_a0;
        }
        goto loop_2;
    }
    return var_v0;
}

void bs_cache_queue_free_node(BSNODE *head, BSNODE *node) {
    BSNODE *last;
    BSNODE *p;
    BSNODE *n;
    p = head;
    last = 0;
    for (;;) {
        n = p->next;
        if (n == 0) {
            break;
        }
        if (n == node) {
            last = p;
        }
        p = n;
    }
    if (last != 0) {
        n = node->next;
        if (n != 0) {
            last->next = n;
            p->next = node;
            node->next = 0;
        }
        memset(node->url, 0, 0x100);
        node->used = 0;
        memset(&node->x108, 0, 2);
    }
}

BSNODE *bs_cache_queue_get(BSNODE *head, char *url) {
    BSNODE *s0;
    BSNODE *p;
    BSNODE *prev;

    p = head;
    for (;;) {
        s0 = p->next;
        if (s0 == 0) {
            return 0;
        }
        prev = p;
        p = s0;
        if (s0->used == 0) {
            if (prev != head) {
                prev->next = s0->next;
                s0->next = head->next;
                head->next = s0;
            }
            BsUrlCopy_SS(s0->url, url);
            s0->used = 1;
            return s0;
        }
    }
}

void bs_source_cache_clear(BSNODE *arg0) {
    bs_cache_queue_free_node(&BcSource_head, arg0);
    memset((void *)F(s32, arg0, 0x10C), 0, 0x8000);
    F(s32, arg0, 0x110) = 0;
}

void bs_image_cache_clear(BSNODE *n) {
    u32 h;
    bs_cache_queue_free_node(&BcImage_head, n);
    if (n->tex != 0) {
        flReleaseTextureHandle_NOWAITDMA(n->tex & 0xFFFF);
        h = (n->tex & 0xFFFF0000) >> 0x10;
        if (h != 0) {
            flReleasePaletteHandle_NOWAITDMA(h);
        }
    }
    n->tex = 0;
    *(s32 *)&n->x10C = 0;
    n->x110 = 0;
    n->x118 = 0;
    n->x11A = 0;
}

typedef struct BCSRC { struct BCSRC *next; u8 _pad04[0x108]; u8 *buf; u8 _pad110[4]; } BCSRC;    /* 0x114 */
typedef struct BCIMG { struct BCIMG *next; u8 _pad04[0x118]; } BCIMG;                          /* 0x11C */
typedef struct BCRT { struct BCRT *next; u8 _pad04[0x108]; } BCRT;                             /* 0x10C */
extern BCSRC *BcSource;
extern BCIMG *BcImage;
extern BCRT *BcRoute;
extern BSNODE *BcRequest;
extern u8 *BcImageBuf[1];
extern BCSRC BsCacheSource[3];
extern BCIMG BsCacheImage[32];
extern BCRT BsCacheRoute[32];
extern BSNODE BsCacheRequest[20];
extern u8 BsCacheImageBuf[0x4000];
extern u8 BsCacheSourceBuf[0x18000];
void BsUrlBaseClear();
void BsRequestPostClear();
void BsCacheInitialize(void) {
    int i;

    BcSource = BsCacheSource;
    BcImage = BsCacheImage;
    BcRoute = BsCacheRoute;
    BcRequest = BsCacheRequest;
    BcImageBuf[0] = BsCacheImageBuf;
    memset(BcSource, 0, sizeof(BsCacheSource));
    BcSource_head.next = (BSNODE *)BcSource;
    BcSource[0].next = &BcSource[1];
    BcSource[1].next = &BcSource[2];
    BcSource[0].buf = BsCacheSourceBuf;
    BcSource[1].buf = BsCacheSourceBuf + 0x8000;
    BcSource[2].buf = BsCacheSourceBuf + 0x10000;
    memset(BcImage, 0, sizeof(BsCacheImage));
    BcImage_head.next = (BSNODE *)BcImage;
    for (i = 0; i < 31; i++) {
        BcImage[i].next = &BcImage[i + 1];
    }
    memset(BcRoute, 0, sizeof(BsCacheRoute));
    BcRoute_head.next = (BSNODE *)BcRoute;
    BcRoute_cur = (BSNODE *)BcRoute;
    for (i = 0; i < 31; i++) {
        BcRoute[i].next = &BcRoute[i + 1];
    }
    memset(BsCacheRequest, 0, sizeof(BsCacheRequest));
    BcRequest_head.next = BsCacheRequest;
    for (i = 0; i < 19; i++) {
        BsCacheRequest[i].next = &BsCacheRequest[i + 1];
    }
    memset(BcCurrentPage, 0, 0x14);
    BsUrlBaseClear();
    BsRequestPostClear();
}

void BsCacheCleanup(void) {
    bs_cache_queue_image_free_all(&BcImage_head);
}

char *BsRouteForward(void) {
    BSNODE *n;
    BSNODE *r;
    n = bs_route_queue_forward(&BcRoute_head, BcRoute_cur);
    r = bs_request_queue_add(&BcRequest_head, n->url);
    r->used = 4;
    if (n == BcRoute_cur) {
        r->x10C = 6;
        r->x10D = 1;
        r->x110 = BcRoute_cur->url;
        *(PAIR2 *)&r->x108 = *(PAIR2 *)&n->x108;
        BcCurrentPage[0] = 5;
    } else {
        r->x10C = 4;
        r->x10D = 1;
        r->x110 = BcRoute_cur->url;
        *(PAIR2 *)&r->x108 = *(PAIR2 *)&n->x108;
        BcCurrentPage[0] = 3;
    }
    return n->url;
}

int BsRouteForwardCheck(void) {
    BSNODE *c = BcRoute_cur;
    return c != bs_route_queue_forward(&BcRoute_head, c);
}

char *BsRouteBack(void) {
    BSNODE *n;
    BSNODE *r;
    n = bs_route_queue_back(&BcRoute_head, BcRoute_cur);
    r = bs_request_queue_add(&BcRequest_head, n->url);
    r->used = 4;
    if (n == BcRoute_cur) {
        r->x10C = 6;
        r->x10D = 1;
        r->x110 = BcRoute_cur->url;
        *(PAIR2 *)&r->x108 = *(PAIR2 *)&n->x108;
        BcCurrentPage[0] = 5;
    } else {
        r->x10C = 5;
        r->x10D = 1;
        r->x110 = n->url;
        *(PAIR2 *)&r->x108 = *(PAIR2 *)&n->x108;
        BcCurrentPage[0] = 4;
    }
    return n->url;
}

int BsRouteBackCheck(void) {
    BSNODE *c = BcRoute_cur;
    return c != bs_route_queue_back(&BcRoute_head, c);
}

static u8 *bs_route_current_page_status() {
    return &BcRoute_cur->x108;
}

char *BsRouteReload(void) {
    BSNODE *r;
    u8 c;
    r = bs_request_queue_add(&BcRequest_head, BcRoute_cur->url);
    r->used = 4;
    r->x10C = 6;
    r->x10D = 1;
    r->x110 = BcRoute_cur->url;
    c = BcCurrentPage[0x10] + 1;
    BcCurrentPage[0x10] = c;
    r->x108 = c;
    r->x109 = bs_route_current_page_status(r)[1];
    BcCurrentPage[0] = 5;
    return BcRoute_cur->url;
}

s32 BsRouteReloadCheck(void) {
    if (bs_page_status_flag_get(bs_route_current_page_status(), 1) != 0) {
        return 0;
    }
    bs_page_status_flag_get(bs_route_current_page_status(), 2);
    return 1;
}

char *BsRouteCurrent(void) {
    return BcRoute_cur->url;
}

void BsRequestHtmlGetCached(char **a) {
    u8 c;
    BSNODE *r;
    r = bs_request_queue_add(&BcRequest_head, *a);
    r->used = 4;
    r->x10C = 1;
    r->x10D = 1;
    r->x110 = BsRouteCurrent();
    c = BcCurrentPage[0x10] + 1;
    BcCurrentPage[0x10] = c;
    r->x108 = c;
    r->x109 = 0;
    BcCurrentPage[0] = 1;
}

void BsRequestHtmlPost(char **a) {
    u8 c;
    BSNODE *r;
    char *cur;
    r = bs_request_queue_add(&BcRequest_head, *a);
    r->used = 4;
    r->x10C = 3;
    r->x10D = 1;
    cur = BsRouteCurrent();
    r->x110 = cur;
    c = BcCurrentPage[0x10] + 1;
    BcCurrentPage[0x10] = c;
    r->x108 = c;
    r->x109 = 0;
    bs_page_status_flag_set(&r->x108, 1, 1);
    BcCurrentPage[0] = 2;
}

/* original bytes: build/raw/BsRequestPostAdd.inc (config/c_rawfuncs.txt) */
u32 strlen();
char *strncpy();
void KanjiSjis2EucEx();
int BsUrlEncode();
int BsRequestPostAdd(char *name, char *val) {
    char buf[0x100];
    int n;
    u32 nl;
    u32 vl;

    n = strlen(BsCachePost);
    nl = strlen(name);
    vl = strlen(val);
    if ((int)(n + nl + vl + 2) > 0xFFF) {
        return -1;
    }
    if (vl >= 0x100 || nl >= 0x100) {
        return -1;
    }
    if (n != 0) {
        BsCachePost[n++] = '&';
    }
    if (bs_page_status_flag_get(bs_route_current_page_status(), 4) == 0) {
        memset(buf, 0, 0x100);
        strncpy(buf, name, 0x100);
        KanjiSjis2EucEx(buf);
        nl = strlen(buf);
        memcpy(BsCachePost + n, buf, nl);
        n += nl;
        BsCachePost[n++] = '=';
        memset(buf, 0, 0x100);
        strncpy(buf, val, 0x100);
        KanjiSjis2EucEx(buf);
        n += BsUrlEncode(BsCachePost + n, buf);
    } else {
        memcpy(BsCachePost + n, name, nl);
        n += nl;
        BsCachePost[n++] = '=';
        n += BsUrlEncode(BsCachePost + n, val);
    }
    return n;
}

void BsRequestPostClear(void) {
    memset(BsCachePost, 0, 0x1000);
}
int BsRequestImage(char **a) {
    int v;
    BSNODE *r;
    u8 *st;
    r = bs_request_queue_add(&BcRequest_head, *a);
    r->used = 4;
    r->x10C = 1;
    r->x10D = 2;
    r->x110 = BsRouteCurrent();
    st = bs_route_current_page_status();
    *(PAIR2 *)&r->x108 = *(PAIR2 *)st;
    v = BsUrlFileExtensionGet(*a);
    if (!(v & 0xFF)) {
        r->used = 6;
        r->x124 = 2;
        r->x125 = 0;
        return v;
    }
    return v;
}

/* original bytes: build/raw/bs_cache_request_html_core.inc (config/c_rawfuncs.txt) */
BSNODE *bs_cache_request_html_core(char *url, int mode, u8 *st) {
    BSNODE *n;

    switch ((u8)BsUrlSchemeGet(url)) {
    case 4:
        if (mode == 1) {
            mode = 2;
        }
        break;
    }
    n = bs_cache_queue_check(&BcSource_head, url);
    switch (mode) {
    case 4:
    case 5:
        if (n == 0 && bs_page_status_flag_get(st, 1) != 0) {
            return 0;
        }
    case 1:
        if (n != 0) {
            n->used = 2;
            return n;
        }
        break;
    case 6:
        if (bs_page_status_flag_get(st, 1) != 0) {
            return 0;
        }
        break;
    case 0:
    case 2:
    case 3:
        break;
    }
    if (n == 0) {
        n = bs_cache_queue_get(&BcSource_head, url);
        if (n == 0) {
            bs_source_cache_clear(bs_cache_queue_get_last(&BcSource_head));
            n = bs_cache_queue_get(&BcSource_head, url);
        }
    }
    n->used = 3;
    return n;
}

/* original bytes: build/raw/bs_cache_request_image_core.inc (config/c_rawfuncs.txt) */
BSNODE *bs_cache_request_image_core(char *url, int mode, u8 *st) {
    BSNODE *n;

    n = bs_cache_queue_check(&BcImage_head, url);
    if (n != 0) {
        if (n->used == 5) {
            if (BcCurrentPage[0] == 5 && n->x108 != *st) {
                bs_image_cache_clear(n);
                return 0;
            }
            n->used = 2;
            return n;
        }
        return 0;
    }
    if (n == 0) {
        n = bs_cache_queue_get(&BcImage_head, url);
        if (n == 0) {
            bs_image_cache_clear(bs_cache_queue_get_last(&BcImage_head));
            n = bs_cache_queue_get(&BcImage_head, url);
        }
    }
    n->used = 3;
    return n;
}

void bs_cache_queue_image_free_all(BSNODE *arg0) {
    BSNODE *var_a0;

    var_a0 = bs_cache_queue_get_last();
    if (var_a0 != 0) {
        do {
            bs_image_cache_clear(var_a0);
            var_a0 = bs_cache_queue_get_last(arg0);
        } while (var_a0 != 0);
    }
}

/* original bytes: build/raw/bs_cache_request_create_texture.inc (config/c_rawfuncs.txt) */
typedef struct BCIMGN {
    struct BCIMGN *next;    /* 0x000 */
    char url[0x100];        /* 0x004 */
    s32 used;               /* 0x104 */
    u8 page;                /* 0x108 page counter of the page that loaded it */
    u8 _pad109[3];
    u8 *data;               /* 0x10C file data */
    s32 size;               /* 0x110 */
    s32 tex;                /* 0x114 texture | palette handle << 16 */
    s16 w, h;               /* 0x118 */
} BCIMGN;
int flCreateTextureFromPNG_mem_err();
int flCreateTextureFromBMP_mem_err();
int bs_cache_request_create_texture(BCIMGN *n) {
    int r;
    BCIMGN *last;

    for (;;) {
        r = -3;
        switch ((u8)BsUrlFileExtensionGet(n->url)) {
        case 2:
            r = flCreateTextureFromPNG_mem_err(n->data, n->size, 3, &n->w, &n->h);
            break;
        case 3:
            r = flCreateTextureFromBMP_mem_err(n->data, 3, &n->w, &n->h);
            break;
        }
        switch (r) {
        case -1:
            last = (BCIMGN *)bs_cache_queue_get_last(&BcImage_head);
            if (last == 0) {
                n->used = 6;
                n->tex = 0;
                return 6;
            }
            if (last->page == *bs_route_current_page_status()) {
                n->used = 6;
                n->tex = 0;
                return 6;
            }
            bs_image_cache_clear((BSNODE *)last);
            continue;
        case -2:
        case -3:
            n->used = 6;
            n->tex = 0;
            return 6;
        }
        n->tex = r;
        return 0;
    }
}

/* original bytes: build/raw/bs_request_check_task_http.inc (config/c_rawfuncs.txt) */
#ifdef __MWERKS__   /* PS2 only; the PC build takes the near-match C */
asm int bs_request_check_task_http()
{
#include "bs_request_check_task_http.inc"
}
#endif

/* original bytes: build/raw/bs_request_set_task_core.inc (config/c_rawfuncs.txt) */
typedef struct BSTASK {
    u8 x00;         /* 0x00 1 = set */
    u8 x01;
    u8 x02;
    u8 _pad03;
    char *x04;      /* 0x04 url */
    char *x08;      /* 0x08 proxy */
    char *x0C;      /* 0x0C proxy (https) */
    u8 *x10;        /* 0x10 receive buffer */
    s32 x14;
    char *url;      /* 0x18 only for http / https urls (scheme 2, 3) */
    s32 x1C;
    s32 x20;
    s32 x24;
    s32 x28;
    u8 x2C;         /* 0x2C 2 = post */
    u8 _pad2D[3];
    u8 *x30;        /* 0x30 post data */
    s8 x34;
    s8 x35;
    u8 _pad36[0x10];
    s8 buf;         /* 0x46 image buffer index */
} BSTASK;
extern s8 BsProxyUseFlag;
extern char BsProxyUrlstr[];
extern char BsProxyUrlstr_S[];
BSTASK *HttpTaskPull(void);
void bs_request_set_task_core(BSTASK *t, char *a, s8 b, char *c, char *d, s8 e, u8 *f, s32 g, char *url, s32 h, s32 i,
                              s32 j, s32 k, s8 post, u8 *l, s8 m) {
    t->x00 = 1;
    t->x04 = a;
    t->x01 = b;
    t->x08 = c;
    t->x0C = d;
    t->x02 = e;
    t->x10 = f;
    t->x14 = g;
    if (url == 0) {
        t->url = 0;
    } else {
        switch ((u8)BsUrlSchemeGet(url)) {
        case 2:
        case 3:
            t->url = url;
            break;
        case 1:
        case 4:
        case 5:
        default:
            t->url = 0;
            break;
        }
    }
    t->x1C = h;
    t->x20 = i;
    t->x24 = j;
    t->x28 = k;
    if (post) {
        t->x2C = 2;
        t->x30 = l;
    } else {
        t->x2C = 1;
        t->x30 = 0;
    }
    t->x34 = m;
    t->x35 = 0;
}

/* original bytes: build/raw/bs_request_set_task_html.inc (config/c_rawfuncs.txt) */
void bs_request_set_task_html(BSNODE *r) {
    char url[0x100];
    BSNODE *c;
    BSTASK *t;

    BsUrlCopy_SS(url, r->url);
    if (BsUrlParse(BsRouteCurrent(), url) == 0) {
        r->used = 6;
        r->x124 = 2;
        r->x125 = 0;
        r->tex = 0;
        return;
    }
    c = bs_cache_request_html_core(url, r->x10C, &r->x108);
    if (c == 0) {
        r->used = 6;
        r->x124 = 8;
        r->x125 = 0;
        return;
    }
    r->tex = (u32)c;
    switch (c->used) {
    case 2:
        r->used = 5;
        *(BSTASK **)&r->x118 = 0;
        break;
    case 3:
        r->used = 3;
        t = HttpTaskPull();
        *(BSTASK **)&r->x118 = t;
        bs_request_set_task_core(t, c->url, BsProxyUseFlag, BsProxyUrlstr, BsProxyUrlstr_S,
                                 r->x10C == 6 || r->x10C == 2, *(u8 **)&c->x10C, 0x8000, r->x110, 0, 0, 0, 0,
                                 r->x10C == 3, BsCachePost, 1);
        break;
    case 6:
        break;
    }
}

/* original bytes: build/raw/bs_request_set_task_image.inc (config/c_rawfuncs.txt) */
void bs_request_set_task_image(BSNODE *r) {
    char url[0x100];
    BSNODE *c;
    BSTASK *t;

    BsUrlCopy_SS(url, r->url);
    if (BsUrlParse(BsRouteCurrent(), url) == 0) {
        r->used = 6;
        r->x124 = 2;
        r->x125 = 0;
        r->tex = 0;
        return;
    }
    c = bs_cache_request_image_core(url, r->x10C, &r->x108);
    if (c == 0) {
        return;
    }
    r->tex = (u32)c;
    switch (c->used) {
    case 2:
        r->used = 5;
        *(BSTASK **)&r->x118 = 0;
        break;
    case 3:
        r->used = 3;
        t = HttpTaskPull();
        *(BSTASK **)&r->x118 = t;
        bs_request_set_task_core(t, c->url, BsProxyUseFlag, BsProxyUrlstr, BsProxyUrlstr_S, BcCurrentPage[0] == 5,
                                 BcImageBuf[t->buf], 0x4000, r->x110, 0, 0, 0, 0, 0, 0, 0);
        break;
    case 6:
        break;
    }
}

/* original bytes: build/raw/BsRequestTask.inc (config/c_rawfuncs.txt) */
#ifdef __MWERKS__   /* PS2 only; the PC build takes the near-match C */
asm int BsRequestTask()
{
#include "BsRequestTask.inc"
}
#endif

/* original bytes: build/raw/BsRequestCheck.inc (config/c_rawfuncs.txt) */
#ifdef __MWERKS__   /* PS2 only; the PC build takes the near-match C */
asm int BsRequestCheck()
{
#include "BsRequestCheck.inc"
}
#endif

void BsRequestCancelHtml(void) {
    switch (BcCurrentPage[0]) {
    case 2:
    case 1:
        BcRoute_cur = bs_route_queue_back(&BcRoute_head, BcRoute_cur);
        BcRoute_cur = bs_route_queue_free_after(&BcRoute_head, BcRoute_cur);
        break;
    case 3:
        BcRoute_cur = bs_route_queue_back(&BcRoute_head, BcRoute_cur);
        break;
    case 4:
        BcRoute_cur = bs_route_queue_forward(&BcRoute_head, BcRoute_cur);
        break;
    case 0:
    case 5:
        break;
    }
}

void BsRequestCancelAll(void) {
    BSNODE *n;
    n = &BcRequest_head;
    for (;;) {
        n = n->next;
        if (n == 0) {
            return;
        }
        if (n->used == 0) {
            return;
        }
        switch (n->used) {
        case 6:
            break;
        case 5:
        case 4:
            n->used = 6;
            n->x124 = 7;
            n->x125 = 0;
            break;
        case 3:
            *(u8 *)(*(int *)((u8 *)n + 0x118) + 0x35) = 1;
            break;
        }
    }
}


extern char *bs_strtbl_html[];
extern char *bs_strtbl_toolmenu[];
extern char *bs_strtbl_titlebar[2];
extern char *bs_strtbl_mmbb_dialog[];
extern char *bs_strtbl_mmbb_ng_msg[];
extern char *bs_strtbl_cap_dialog[];
extern char *bs_strtbl_err_dialog[2];
char *BsStrtblGet(int kind, int idx) {
    switch (kind) {
    case 2:
        return bs_strtbl_html[idx];
    case 3:
        return bs_strtbl_toolmenu[idx];
    case 4:
        return bs_strtbl_titlebar[idx];
    case 5:
        return bs_strtbl_mmbb_dialog[idx];
    case 6:
        return bs_strtbl_mmbb_ng_msg[idx];
    case 7:
        return bs_strtbl_cap_dialog[idx];
    case 8:
        return bs_strtbl_err_dialog[idx];
    }
    return 0;
}

