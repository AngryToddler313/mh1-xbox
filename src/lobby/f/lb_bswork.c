/* lb_bswork - one translation unit 0x005E9960-0x005E9EC8: the browser's draw-work pool (BsWorkInitAll, the static
 * pull / push helpers, BsWorkPull / Push / Move / Trans) and its texture handle list (BsTextureFreeAll / Add / Get /
 * Load). Assembled from agent F's runs lb_gav01, lb_av01, lb_gz01, lb_av02 plus BsTextureAdd (agent B, 12 Oct 2026). */
#include "g_alias.h"
#include "lobby_f.h"



extern BSWK *Bs_work_line_last[9];
extern BSWK *Bs_work_line_head[9];
extern BSWK Bs_work[0x200];
extern BSWK *Bs_work_hit_head;
extern BSWK *Bs_work_hit_last;
extern BSWK *Bs_work_free_head;
extern s32 Bs_tex_handle[0x18];
extern s8 Bs_tex_handle_ptr;
extern s32 data_load_ptr;
void flReleasePaletteHandle();
void flReleaseTextureHandle();
void flCompact();
void load_file_mdl();
int flCreateTextureFromTim2_mem();

/* prototypes of this file */
void BsWorkInitAll(void);
static BSWK *bs_pul_wk(void);
static void bs_psh_wk(BSWK *w);
BSWK *BsWorkPull(int line, int tail);
void BsWorkPush(BSWK *w);
void BsWorkMove(int line);
void BsWorkTrans(int line);
void BsTextureFreeAll(void);
int BsTextureAdd(u32 h);
s32 BsTextureGet(int arg0);
void BsTextureLoad(int a, u8 *p);

#if !defined(G_PART) || G_PART == 1
void BsWorkInitAll(void) {
    int i;
    BSWK **h;
    BSWK **l;
    BSWK *w;
    int n;
    i = 0;
    h = Bs_work_line_head;
    l = Bs_work_line_last;
    do {
        h[0] = 0;
        l[0] = 0;
        i += 3;
        h[1] = 0;
        l[1] = 0;
        h[2] = 0;
        l[2] = 0;
        h += 3;
        l += 3;
    } while (i < 9);
    w = Bs_work;
    Bs_work_hit_head = 0;
    n = 0x200;
    Bs_work_hit_last = 0;
    Bs_work_free_head = Bs_work;
    if (n-- != 0) {
        do {
            memset(w, 0, 0x70);
            if (n != 0) {
                w->next = w + 1;
            }
            w += 1;
        } while (n-- != 0);
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
static BSWK *bs_pul_wk(void) {
    BSWK *n;
    n = Bs_work_free_head;
    if (n != 0) {
        Bs_work_free_head = n->next;
    }
    return n;
}
#endif

#if !defined(G_PART) || G_PART == 1
static void bs_psh_wk(BSWK *w) {
    memset(w, 0, 0x70);
    w->next = Bs_work_free_head;
    Bs_work_free_head = w;
}
#endif

#if !defined(G_PART) || G_PART == 1
BSWK *BsWorkPull(int line, int tail) {
    BSWK *w;
    BSWK *p;
    int off;
    w = bs_pul_wk();
    if (w == 0) {
        return 0;
    }
    w->x03 = line;
    if (tail == 0) {
        p = Bs_work_line_head[line];
        w->next = p;
        Bs_work_line_head[line] = w;
        w->prev = 0;
        if (p == 0) {
            Bs_work_line_last[line] = w;
            return w;
        }
        p->prev = w;
        return w;
    }
    p = Bs_work_line_last[line];
    w->prev = p;
    Bs_work_line_last[line] = w;
    w->next = 0;
    if (p == 0) {
        Bs_work_line_head[line] = w;
        return w;
    }
    p->next = w;
    return w;
}
#endif

#if !defined(G_PART) || G_PART == 1
void BsWorkPush(BSWK *w) {
    BSWK *p;
    BSWK *n;
    p = w->prev;
    if (p == 0) {
        p = w->next;
        Bs_work_line_head[w->x03] = p;
        if (p == 0) {
            Bs_work_line_last[w->x03] = 0;
        } else {
            p->prev = 0;
        }
    } else {
        n = w->next;
        if (n == 0) {
            Bs_work_line_last[w->x03] = p;
            p->next = 0;
        } else {
            p->next = n;
            w->next->prev = w->prev;
        }
    }
    bs_psh_wk(w);
}
#endif

#if !defined(G_PART) || G_PART == 1
void BsWorkMove(int line) {
    BSWK *n;
    BSWK *w;
    w = Bs_work_line_head[line];
    if (w != 0) {
        do {
            n = w->next;
            if (w->x00 != 0) {
                ((void (*)(BSWK *))w->task)(w);
            }
            w = n;
        } while (n != 0);
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void BsWorkTrans(int line) {
    BSWK *n;
    BSWK *w;
    w = Bs_work_line_head[line];
    if (w != 0) {
        do {
            n = w->next;
            if (w->x01 != 0) {
                ((void (*)(BSWK *))w->trans)(w);
            }
            w = n;
        } while (n != 0);
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void BsTextureFreeAll(void) {
    int i;
    s32 *p;
    s32 v;
    u32 h;
    u16 t;
    i = 0;
    p = Bs_tex_handle;
    if (0 < Bs_tex_handle_ptr) {
        do {
            v = *p;
            if (v != 0) {
                h = (u32)(v & 0xFFFF0000) >> 0x10;
                if (h != 0) {
                    flReleasePaletteHandle(h);
                }
                t = *p;
                if (t != 0) {
                    flReleaseTextureHandle(t);
                }
                *p = 0;
            }
            i += 1;
            p += 1;
        } while (i < Bs_tex_handle_ptr);
    }
    Bs_tex_handle_ptr = 0;
    flCompact();
}
#endif

#if !defined(G_PART) || G_PART == 1
int BsTextureAdd(u32 h) {
    if (Bs_tex_handle_ptr == 24) {
        return -1;
    }
    Bs_tex_handle[Bs_tex_handle_ptr] = h;
    Bs_tex_handle_ptr++;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
s32 BsTextureGet(int arg0) {
    int i;

    i = (s8)arg0;
    if (i < 0 || Bs_tex_handle_ptr <= i) {
        return -1;
    }
    return Bs_tex_handle[i];
}
#endif

#if !defined(G_PART) || G_PART == 2
void BsTextureLoad(int a, u8 *p) {
    u8 st;
    switch (a) {
    case 3:
        st = *p;
        switch (st) {
        case 0:
            load_file_mdl(data_load_ptr, GV(0x885, 0xC8A));
            *p = *p + 1;
            break;
        case 1:
            BsTextureAdd(flCreateTextureFromTim2_mem(data_load_ptr, 0));
            *p = *p + 1;
            break;
        case 2:
            *p = st + 1;
            break;
        case 3:
            *p = st + 1;
            break;
        case 4:
            break;
        }
    }
}
#endif
