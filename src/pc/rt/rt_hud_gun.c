/*
 * rt_hud_gun.c - the optional "prototype gunner" HUD (docs/pc.md "Prototype gunner HUD", docs/FINDS.md cockpit2.apx).
 *
 * cockpit2.apx (AFS_DATA entry 10 in MH1 JP, filedef_sys slot 3, never loaded by the game) is an early HUD mock-up:
 * a sword-shaped gauge, two name boxes with text baked in and a five-slot wheel with a "4 / 12" counter. This file
 * builds a working HUD from its pieces, drawn by the PC / Xbox 2D layer; the game logic is untouched, everything shown
 * is read from the game's own state (the same fields the original HUD reads, src/main/menu/menu_disp_nm.c):
 *   - the sword: health in the blade's slot (disp_pl_vital: PLW+0x792 max, +0x790 red part, lpPit->x24 green part),
 *     stamina in the gold hilt's slot (PLW+0x748 of +0x882, the original's colours and tired flash);
 *   - bottom right, the ammo wheel: the loaded ammo (PLW+0x88E, Pl_shell_set order) in the slot by the name box, the
 *     next two / previous two loadable ammo types in the other four slots, the ammo name in the box, the pod shows
 *     rounds in the clip (PLW+0x01C) over rounds in reserve (PLW+0x8BC, 0xFF = unlimited);
 *   - bottom left, the item wheel (the same wheel mirrored, the owner's design): the selected pouch item (PLW+0x888,
 *     item_sel_sub order) and its neighbours, its name, the pod shows the count over the stack limit.
 * Liberties: the baked text, icons and digits are covered with black (the slots' and boxes' insides are plain black in
 * the texture), the boxes' cut-off left ends get a cap mirrored from the top box's rounded end, the tabs ("..SH" /
 * "..AG") and the long translucent gauge at the bottom of the texture are not used. The baked yellow pouch icon is
 * kept for crag shot (the owner's reading: an explosive ammo icon); every other slot shows the game's item icon.
 *
 * Shown when hud = prototype gunner (rt_hud_style, PC settings menu / mh1pc.ini) and the hunter is a gunner (job 1 or
 * 5, as disp_cannon tests); everything else keeps the original HUD. rt_2d.c's HUD wrappers call in here.
 */
#include "rt.h"
#include "types.h"
#include "version.h"
#include "pl.h"
#include "menu.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int rt_hud_style;               /* 0 = the original HUD, 1 = the prototype gunner HUD (gfx_opts.c sets it) */

extern u32 mem_tex[];
extern s16 filedef_sys[];
extern u8 Item_data[][ITEM_ROW];
extern char *item_str[];
extern u32 item_col_tbl[];
extern u16 System_timer;
uint8_t *rt_file_load(int idx, size_t *n);
void *flCreateTextureFromApx_mem(void *p, int type);
void SetTextureStage(int);
void reload_tex(int, int);
void SetFilterMode(int);
void flps0008(void *);
void flps0004(void *);
void flfntSetSize(int, int);
void flfntLocate(int, int);
void font_set_palette(int);
void font_print_uf(void *, ...);
int Item_valid_chk(u16 id);
u16 Pl_shell_set(PLW *pl, int slot, int dir);

#define PROTO_SLOT 0x158        /* a mem_tex slot nothing in MH1 / G uses (the HUD's are 0x118-0x11C) */
#define PF(pl, T, o) (*(T *)((u8 *)(pl) + (o)))

typedef struct { s16 x, y, w, h; u32 col; s16 u0, v0, u1, v1; } SPR;     /* flps0008 */
typedef struct { s16 x0, y0, x1, y1; u32 col; } RECT;                     /* flps0004 */

/* ------------------------------------------------------------ the texture */
static u32 proto_handle;

/* cockpit2 into mem_tex[PROTO_SLOT] (once; again if something cleared the slot): filedef_sys[3] is its AFS entry
 * in both versions (MH1 JP 10). A host buffer, not data_load_ptr, so a load in the middle of a quest disturbs nothing. */
static int proto_tex(void)
{
    uint8_t *p;
    size_t n;
    if (proto_handle && mem_tex[PROTO_SLOT] == proto_handle)
        return 1;
    proto_handle = 0;
    if (!(p = rt_file_load(filedef_sys[3], &n)))
        return 0;
    mem_tex[PROTO_SLOT] = (u32)(uintptr_t)flCreateTextureFromApx_mem(p, 0);
    free(p);
    proto_handle = mem_tex[PROTO_SLOT];
    if (getenv("RT_TRACE"))
        fprintf(stderr, "rt_hud_gun: cockpit2 (AFS_DATA %d) -> texture %u\n", filedef_sys[3], (unsigned)proto_handle);
    return proto_handle != 0;
}

static PLW *gunner(void)
{
    PLW *pl;
    if (!rt_hud_style || !lpPit || !(pl = lpPit->pl))
        return NULL;
    if (pl->kind != 1 && pl->kind != 5)
        return NULL;
    return proto_tex() ? pl : NULL;
}

/* ------------------------------------------------------------ drawing helpers */
static void spr(float x0, float y0, float x1, float y1, int tu0, int tv0, int tu1, int tv1, int flip, u32 col)
{
    SPR q;
    q.x = (s16)floorf(x0 + 0.5f);
    q.y = (s16)floorf(y0 + 0.5f);
    q.w = (s16)(floorf(x1 + 0.5f) - q.x);
    q.h = (s16)(floorf(y1 + 0.5f) - q.y);
    q.col = col;
    q.u0 = (s16)(flip ? tu1 : tu0);
    q.u1 = (s16)(flip ? tu0 : tu1);
    q.v0 = (s16)tv0;
    q.v1 = (s16)tv1;
    if (q.w > 0 && q.h > 0)
        flps0008(&q);
}

static void rect(float x0, float y0, float x1, float y1, u32 col)
{
    RECT q;
    q.x0 = (s16)floorf(x0 + 0.5f);
    q.y0 = (s16)floorf(y0 + 0.5f);
    q.x1 = (s16)floorf(x1 + 0.5f);
    q.y1 = (s16)floorf(y1 + 0.5f);
    q.col = col;
    if (q.x1 > q.x0 && q.y1 > q.y0)
        flps0004(&q);
}

/* text: the font's x is in the 640-wide layout (frame x / 0.8), y and the size in frame pixels; centred on cx, cy.
 * Narrowed (down to 60%) when it would not fit in maxw. */
static void text(float cx, float cy, float maxw, int size, int pal, const char *s)
{
    int len = (int)strlen(s), w = size;
    float tw;
    if (!len)
        return;
    tw = len * (w / 2.0f) * 0.8f;
    if (tw > maxw) {
        w = (int)(maxw / (len * 0.4f));
        if (w < size * 6 / 10)
            w = size * 6 / 10;
        tw = len * (w / 2.0f) * 0.8f;
    }
    flfntSetSize(w, size);
    flfntLocate((int)((cx - tw / 2) / 0.8f + 0.5f), (int)(cy - size / 2.0f + 0.5f));
    font_set_palette(pal);
    font_print_uf((void *)s);
}

/* ------------------------------------------------------------ the sword: health and stamina */
#define SW_X 65                 /* screen = texel + (SW_X, SW_Y): the sword (texels 19..255 x 17..45) at 84, 19 */
#define SW_Y 2
#define HP_FULL 150             /* the blade's slot (texels 42..186) holds 150 health, the most a hunter can have */
#define ST_FULL 150             /* the hilt's slot (texels 97..179) holds 150 stamina */

int rt_hud_gun_vital(void)
{
    PLW *pl = gunner();
    int mx, red, cur, st, stmax, a;
    float l, x0;
    u32 col;
    if (!pl)
        return 0;
    SetFilterMode(0);
    /* behind the frame (its slots are holes): the dark slot, then the bars, then the sword over them */
    x0 = 42 + SW_X;
    l = 186 - 42 + 1;
    mx = PF(pl, s16, 0x792);
    rect(x0, 28 + SW_Y, x0 + l, 33 + SW_Y, 0xFF101010);
    rect(x0, 28 + SW_Y, x0 + l * (mx > HP_FULL ? HP_FULL : mx) / HP_FULL, 33 + SW_Y, 0xFF303838);
    if (PF(pl, s16, 0x302) > 0) {
        red = PF(pl, s16, 0x790);
        cur = lpPit->x24;
        if (cur < red)
            rect(x0, 28 + SW_Y, x0 + l * (red > HP_FULL ? HP_FULL : red) / HP_FULL, 33 + SW_Y, 0xFFC01010);
        if (cur > 0)
            rect(x0, 28 + SW_Y, x0 + l * (cur > HP_FULL ? HP_FULL : cur) / HP_FULL, 33 + SW_Y, 0xFF10C010);
    }
    /* stamina: the original's colours (yellow; red flash when tired; pale flash while PLW+0x8CC runs) */
    x0 = 97 + SW_X;
    l = 179 - 97 + 1;
    st = PF(pl, s16, 0x748);
    stmax = PF(pl, s16, 0x882);
    if (st < 0x4C) {
        a = (int)(64.0f * sinf(2.0f * 3.1415927f * (float)((System_timer % 45) * 8) / 360.0f)) + 0x40;
        col = 0xFFFF0000u | ((u32)(a & 0xFF) << 8) | (u32)(a & 0xFF);
    } else if (PF(pl, s16, 0x8CC) == 0) {
        col = 0xFFF0F000u;
    } else {
        float sv = sinf(0.0000958738f * (float)(((System_timer & 0x3F) << 10) & 0xFFFF));
        a = ((int)(18.0f * sv) + 0xE7) & 0xFF;
        col = 0xFF000000u | ((u32)a << 16) | ((u32)a << 8) | (u32)(((int)(40.0f * sv) + 0x9F) & 0xFF);
    }
    rect(x0, 37 + SW_Y, x0 + l, 39 + SW_Y, 0xFF101010);
    rect(x0, 37 + SW_Y, x0 + l * (stmax > ST_FULL ? ST_FULL : stmax) / ST_FULL, 39 + SW_Y, 0xFF303030);
    if (st > 0)
        rect(x0, 37 + SW_Y, x0 + l * (st > ST_FULL ? ST_FULL : st) / ST_FULL, 39 + SW_Y, col);
    SetFilterMode(1);
    reload_tex(1, PROTO_SLOT);
    SetTextureStage(PROTO_SLOT);
    spr(19 + SW_X, 17 + SW_Y, 256 + SW_X, 46 + SW_Y, 19, 17, 256, 46, 0, 0xFFFFFFFF);
    return 1;
}

/* ------------------------------------------------------------ the wheels
 * One widget = [cap][name box][wheel + pod], in local units of texels (u across, v = texel row - 84), drawn at scale
 * WS; the item widget is the same mirrored (and uses the top box, whose rounded end is then on the outside). */
#define WS 0.8f
#define BOX 100                 /* the stretched name box, texels */
#define CAP 12                  /* the rounded end, texels 84..95 of the top box */
#define WU (CAP + BOX + 159)    /* the whole widget: the wheel is texels 82..240 */
#define WX(t) ((t) - 82 + CAP + BOX)   /* wheel texel x -> u */

typedef struct { float ox, oy; int mirror; } WIDGET;

static float sx(const WIDGET *w, float u) { return w->ox + WS * (w->mirror ? WU - u : u); }
static float sy(const WIDGET *w, float v) { return w->oy + WS * v; }

/* a texel rectangle at local u0.., v0.. (size u1-u0, v1-v0), flipped when flip != mirror */
static void put(const WIDGET *w, float u0, float v0, float u1, float v1, int tu0, int tv0, int tu1, int tv1, int flip)
{
    float a = sx(w, u0), b = sx(w, u1);
    if (a > b) {
        float t = a;
        a = b;
        b = t;
    }
    spr(a, sy(w, v0), b, sy(w, v1), tu0, tv0, tu1, tv1, flip ^ w->mirror, 0xFFFFFFFF);
}

static void blank(const WIDGET *w, float u0, float v0, float u1, float v1)
{
    float a = sx(w, u0), b = sx(w, u1);
    rect(a < b ? a : b, sy(w, v0), a < b ? b : a, sy(w, v1), 0xFF010101);
}

/* the empty frame: cap, box, wheel; baked text, icons and digits covered */
static void frame(const WIDGET *w, int top_box)
{
    static const s16 icon_bl[4][4] = { { 96, 117, 116, 129 }, { 124, 90, 143, 105 }, { 123, 142, 143, 158 },
                                       { 177, 142, 199, 157 } };
    int i;
    reload_tex(1, PROTO_SLOT);
    SetTextureStage(PROTO_SLOT);
    put(w, 0, 27, CAP, 53, 84, 79, 96, 106, 1);
    if (top_box)
        put(w, CAP, 27, CAP + BOX, 53, 0, 79, 84, 106, 0);
    else
        put(w, CAP, 27, CAP + BOX, 53, 0, 111, 82, 137, 0);
    put(w, WX(82), 27, WX(241), 80, 82, 111, 241, 164, 0);
    put(w, WX(104), 0, WX(241), 27, 104, 84, 241, 111, 0);
    blank(w, CAP, 32, CAP + BOX + 1, 48);
    for (i = 0; i < 4; i++)
        blank(w, WX(icon_bl[i][0]), icon_bl[i][1] - 84, WX(icon_bl[i][2]), icon_bl[i][3] - 84);
    blank(w, WX(209), 113 - 84, WX(227), 121 - 84);
    blank(w, WX(209), 127 - 84, WX(226), 135 - 84);
}

/* slot centres (texels): by the box (the current one), upper left, upper right, lower left, lower right */
static const s16 slot_c[5][2] = { { 106, 123 }, { 134, 97 }, { 187, 97 }, { 134, 150 }, { 187, 150 } };

static int is_crag(u16 id)      /* crag shot LV1-3: shot types 9-11 (Item_data +8) */
{
    s16 t = *(s16 *)&Item_data[id][8];
    return Item_data[id][1] == 2 && t >= 9 && t <= 11;
}

/* the game's icon for an item (disp_item_icon: Item_data +5 icon, +6 colour; greyed when it cannot be used) */
static void icon(const WIDGET *w, int k, u16 id, int sel)
{
    u8 *d = Item_data[id];
    float cu = WX(slot_c[k][0]), cv = slot_c[k][1] - 84, h = sel ? 11 : 9;
    float a = sx(w, cu - h), b = sx(w, cu + h);
    u32 col;
    int n, tu, tv;
    if (a > b) {
        float t = a;
        a = b;
        b = t;
    }
    if (is_crag(id)) {          /* the mock-up's own yellow pouch icon */
        reload_tex(1, PROTO_SLOT);
        SetTextureStage(PROTO_SLOT);
        spr(a, sy(w, cv - h * 0.8f), b, sy(w, cv + h * 0.8f), 124, 90, 143, 105, 0, sel ? 0xFFFFFFFF : 0xFFA0A0A0);
        return;
    }
    if (d[5] == 0xFF)
        return;
    col = item_col_tbl[Item_valid_chk(id) ? d[6] : 10];
    if (!sel)
        col = (col & 0xFF000000u) | ((col >> 1) & 0x7F7F7F) | 0x202020;
    n = d[5] + 1;
    tu = (n & 7) << 5;
    tv = (n >> 3) << 5;
    reload_tex(1, 0x118);
    SetTextureStage(0x118);
    spr(a, sy(w, cv - h), b, sy(w, cv + h), tu + 1, tv + 1, tu + 0x1F, tv + 0x1F, 0, col);
}

/* the pod: top over bottom (texel centres 218,117 and 217,131); palettes: 0 white, 2 red (the baked "12"),
 * 3 green (the baked names), 5 yellow (nfcol_tbl, main 0x300690) */
static void pod(const WIDGET *w, const char *top, int top_pal, const char *bot, int bot_pal)
{
    float cx = sx(w, WX(218));
    text(cx, sy(w, 117 - 84), 15, 9, top_pal, top);
    text(cx, sy(w, 131 - 84), 15, 9, bot_pal, bot);
}

static void name(const WIDGET *w, const char *s, int pal)
{
    float a = sx(w, CAP + 2), b = sx(w, CAP + BOX - 2);
    text((a + b) / 2, sy(w, 40), a < b ? b - a : a - b, 12, pal, s);
}

/* the pouch neighbours, the way item_sel_sub (f_pl.c) and Pl_shell_set walk the pouch */
static int usable(PLW *pl, int s) { return pl->item[s].id != 0 && Item_data[pl->item[s].id][1] == 1; }
static int item_step(PLW *pl, int s, int dir)
{
    int n;
    for (n = 0; n < 20; n++) {
        s = dir ? (s + 19) % 20 : (s + 1) % 20;
        if (usable(pl, s))
            return s;
    }
    return -1;
}

/* nb[k] (next, next but one, previous, previous but one: slots 1-4) not the current one and not shown already
 * (a pouch with few items wraps round) */
static int fresh(const int *nb, int k, int cur)
{
    int j;
    if (nb[k] == cur)
        return 0;
    for (j = 0; j < k; j++)
        if (nb[j] == nb[k])
            return 0;
    return 1;
}

static void item_widget(PLW *pl)
{
    WIDGET w;
    char buf[16];
    int s = pl->work888, nb[4], k;
    w.ox = 16;
    w.oy = 448 - 24 - 80 * WS;
    w.mirror = 1;
    frame(&w, 1);
    if (s >= 20 || !usable(pl, s))
        return;
    /* next, next but one in the upper slots, previous ones below (the wheel turns as the pad cycles) */
    nb[0] = item_step(pl, s, 0);
    nb[1] = nb[0] >= 0 ? item_step(pl, nb[0], 0) : -1;
    nb[2] = item_step(pl, s, 1);
    nb[3] = nb[2] >= 0 ? item_step(pl, nb[2], 1) : -1;
    for (k = 0; k < 4; k++)
        if (nb[k] >= 0 && fresh(nb, k, s))
            icon(&w, k + 1, pl->item[nb[k]].id, 0);
    icon(&w, 0, pl->item[s].id, 1);
    name(&w, item_str[pl->item[s].id], 3);
    snprintf(buf, sizeof buf, "%d", pl->item[s].num);
    {
        u8 mx = Item_data[pl->item[s].id][3];
        char b2[8];
        snprintf(b2, sizeof b2, mx == 0xFF ? "--" : "%d", mx);
        pod(&w, buf, pl->item[s].num >= mx ? 5 : 0, b2, 2);
    }
}

static void ammo_widget(PLW *pl)
{
    WIDGET w;
    char top[8], bot[8];
    int s = pl->work88E, nb[4], k;
    w.ox = 512 - 16 - WU * WS;
    w.oy = 448 - 24 - 80 * WS;
    w.mirror = 0;
    frame(&w, 0);
    if (s == 0xFF || s >= 20 || pl->item[s].id == 0)
        return;
    nb[0] = Pl_shell_set(pl, s, 0);
    nb[1] = nb[0] != 0xFF ? Pl_shell_set(pl, nb[0], 0) : 0xFF;
    nb[2] = Pl_shell_set(pl, s, 1);
    nb[3] = nb[2] != 0xFF ? Pl_shell_set(pl, nb[2], 1) : 0xFF;
    for (k = 0; k < 4; k++)
        if (nb[k] != 0xFF && fresh(nb, k, s))
            icon(&w, k + 1, pl->item[nb[k]].id, 0);
    icon(&w, 0, pl->item[s].id, 1);
    /* red while a new type is picked but not loaded yet (disp_shell_name's palette 5) */
    name(&w, item_str[pl->item[s].id], pl->x56E && *(s16 *)&Item_data[pl->item[s].id][8] != pl->work56D ? 5 : 3);
    snprintf(top, sizeof top, "%d", pl->work01C);
    if (pl->work8BC == 0xFF)
        snprintf(bot, sizeof bot, "--");
    else
        snprintf(bot, sizeof bot, "%d", pl->work8BC);
    pod(&w, top, 0, bot, 2);
}

/* instead of disp_item (src/main/menu/menu29.c) for a gunner: its early returns and its two timers kept */
int rt_hud_gun_item(void)
{
    PLW *pl = gunner();
    if (!pl)
        return 0;
    if (*(u8 *)pl != 0 && *((u8 *)pl + 1) != 0 && lpPit->x83 == 0) {
        SetFilterMode(1);
        gfx_set_2d_anchor(GFX_A_LEFT);
        item_widget(pl);
        gfx_set_2d_anchor(GFX_A_RIGHT);
        ammo_widget(pl);
        gfx_set_2d_anchor(GFX_A_CENTER);
    }
    if (*(u8 *)pl == 0 || *((u8 *)pl + 1) == 0 || lpPit->x83 != 0)
        return 1;
    if (lpPit->x58 > 0)
        lpPit->x58--;
    if (lpPit->x5C > 0)
        lpPit->x5C--;
    return 1;
}
