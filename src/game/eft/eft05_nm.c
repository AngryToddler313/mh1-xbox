/* NONMATCHING: eft05_t (0x00543200), not built. Control flow and stack
 * layout match; the spline point arithmetic is scheduled differently
 * (75 instructions differ). Tried every order of the three spline terms
 * and of each multiply's operands. The rest matches, built from eft05.c
 * and eft05b.c. */
/* eft05 - game.bin 0x00542D40-0x00543718. A weapon's slash trail. Each
 * frame the weapon tip's matrix (joint 14 or 18, plus an offset per arg) is
 * pushed into a six-entry history while the player stays in the attack
 * animation, and eft05_t draws a skinned ribbon through it. */
#include "eft.h"
#include "game.h"
#include "pl.h"
#include "prim.h"
#include "clay.h"
#include "fl.h"

typedef struct EFT05_WORK {
    u8 _pad00[0x10];
    FLMAT hist[6];      /* 0x10 newest first */
} EFT05_WORK;

typedef struct EFT_MDLW {
    u8 flag;            /* 0x00 */
    u8 _pad01[0x0F];
    void *mat;          /* 0x10 material table */
    u8 _pad14[0x10];
    u8 *skin;           /* 0x24 skin matrix list (count-ish at +0xC2) */
    u8 _pad28[8];
    CLAY *clay;         /* 0x30 */
} EFT_MDLW;

extern f32 eft05_ofs[][3];         /* G: 4 rows (args 0-3) */
extern EFT_MDLW *eft_mdlw[5];
extern f32 mat_calc[6][2];
extern f32 sprine_calc[6][3];
extern u8 Eft_blood_rgb[];

u8 Pl_stg_ck(PLW *);
void release_prim(s16);
FLMAT *get_joint_wmat(PLW *, int);
void flmatCopy(FLMAT *, FLMAT *);
void flmatGetTrans(f32 *, FLMAT *);
void flvecApplyMat33_2(f32 *, FLMAT *);
void flmatBlend(FLMAT *, FLMAT *, FLMAT *, f32, f32);
void flSetSkinTrans(void *);
void SetFilterMode(int);
void Material_set_sub(void *, CLAY *);
void Eft_rendope_set(int);

static void eft05_move(EFTW *ew);
static void eft05_i(EFTW *ew);
static void eft05_m(EFTW *ew);
static void eft05_d(EFTW *ew);
static void eft05_e(EFTW *ew);
static void eft05_t(PRIM *pr);

static void eft05_move(EFTW *ew) {
    switch (ew->mode) {
    case 0:
        eft05_i(ew);
        break;
    case 1:
        eft05_m(ew);
        break;
    case 2:
        eft05_d(ew);
        break;
    case 3:
        eft05_e(ew);
        break;
    }
}

#ifdef VERSION_MHG
/* G (game 0x56BBC0-0x56CAE4, agent A, 16 Oct 2026; written from the asm, not byte-checked: the G build keeps these as
 * raw bytes): an effect draws up to two trails (the Dual Swords' blades). Per arg: trail count eft05_g_tbl[4 + arg]
 * (1, 1, 1, 2, 2) and clay eft05_g_tbl[arg] (0 blood, 1 white blade, 2 the red demon-mode trail). The history is
 * interleaved: entry j of trail i at work + j * 0x80 + i * 0x40; prims at +0x300, prim numbers at +0x308.
 * Arg 1 / 2 = the right / left blade, 3 = both blades in demon mode (pl_init sets it up for the whole quest with
 * timer -1, from the second prim pool), 4 = both blades (no caller found). For a Dual Swords weapon the tip comes
 * from per-weapon tables (offset + row scale of the hand matrix, ds_type_index), else from eft05_ofs as in MH1. */
typedef struct EFT05G_WORK {
    FLMAT hist[6][2];   /* 0x000 newest first; [trail] */
    PRIM *prim[2];      /* 0x300 */
    s16 prim_no[2];     /* 0x308 */
} EFT05G_WORK;

extern s16 eft05_g_tbl[10];             /* G main 0x2EC728: clay per arg [4], trail count per arg [6] */
extern s16 ds_type_ranges[];            /* G main 0x253F50: Dual Swords weapon types {first, last} .., -1 */
extern f32 eft05_ds_ofs[2][0x44];       /* G game 0x67A0B0 / 0x67A1C0: blade tip offset {x,y,z} per Dual Swords [blade] */
extern f32 eft05_ds_scl[2][0x44];       /* G game 0x67A2D0 / 0x67A3E0: hand matrix row scales [blade] (0x110 apart) */
int get_prim2(void);
PRIM *get_prim_ptr2(s16);
void release_prim2(s16);
void RotateZ(FLMAT *, f32);
f32 flvecCalcLength(f32 *);
int frame_check2(void *w, int n, f32 f);  /* the definition's order (this file is PC only, no rt_abi adaptor) */
#define EFT05_N(arg) (eft05_g_tbl[4 + (arg)])

/* G main 0x101F10: the Dual Swords weapon's index into the blade tables, -1 for other classes */
s16 ds_type_index(PLW *pl) {
    s16 i, n = 0;
    u8 type = pl->work34C;
    if (pl->kind == 6) {
        for (i = 0; ds_type_ranges[i * 2] != -1; i++) {
            if (type >= ds_type_ranges[i * 2] && type <= ds_type_ranges[i * 2 + 1]) {
                return n + type - ds_type_ranges[i * 2];
            }
            n += ds_type_ranges[i * 2 + 1] - ds_type_ranges[i * 2] + 1;
        }
    }
    return -1;
}

/* game 0x56C880 / 0x56C660: the tip offset and the hand matrix of trail no */
static int eft05_left(EFTW *ew, int no) {
    return !(ew->arg == 1 || (ew->arg == 3 && no == 0));
}
static void eft05_ofs_get(f32 *v, EFTW *ew, PLW *pl, int no) {
    s16 k = ds_type_index(pl);
    f32 *s;
    if (k != -1) {
        s = &eft05_ds_ofs[eft05_left(ew, no)][k * 3];
    } else {
        s = eft05_ofs[ew->arg];
    }
    v[0] = s[0];
    v[1] = s[1];
    v[2] = s[2];
}
static void eft05_mat_get(FLMAT *m, EFTW *ew, PLW *pl, int no) {
    s16 k = ds_type_index(pl);
    f32 *s;
    int r;
    if (k != -1) {
        if (!eft05_left(ew, no)) {
            s = &eft05_ds_scl[0][k * 3];
            flmatCopy(m, get_joint_wmat(pl, 0xE));
        } else {
            s = &eft05_ds_scl[1][k * 3];
            flmatCopy(m, get_joint_wmat(pl, 0x12));
            RotateZ(m, 3.1415927f);
        }
        for (r = 0; r < 3; r++) {
            (*m)[r][0] *= s[r];
            (*m)[r][1] *= s[r];
            (*m)[r][2] *= s[r];
        }
    } else if (pl->work615 != 0) {
        flmatCopy(m, get_joint_wmat(pl, 0xE));
    } else {
        flmatCopy(m, get_joint_wmat(pl, 0x12));
    }
}
static void eft05_tip(FLMAT *m, EFTW *ew, PLW *pl, int no) {
    f32 v[3];
    eft05_ofs_get(v, ew, pl, no);
    eft05_mat_get(m, ew, pl, no);
    flvecApplyMat33_2(v, m);
    (*m)[3][0] += v[0];
    (*m)[3][1] += v[1];
    (*m)[3][2] += v[2];
}

static void eft05_i(EFTW *ew) {
    FLMAT m;
    EFT05G_WORK *w = ew->work;
    PLW *pl = (PLW *)ew->owner;
    s16 i, j, n;

    ew->mode++;
    ew->mode2 = 0;
    ew->be_flag = 1;
    ew->work14 = 0;
    n = EFT05_N(ew->arg);
    for (i = 0; i < n; i++) {
        w->prim_no[i] = ew->prim2 == 1 ? get_prim2() : get_prim();
        if (w->prim_no[i] != -1) {
            w->prim[i] = ew->prim2 == 1 ? get_prim_ptr2(w->prim_no[i]) : get_prim_ptr(w->prim_no[i]);
            w->prim[i]->owner = ew;
            w->prim[i]->no = i;
            w->prim[i]->trans = eft05_t;
        } else {
            w->prim[i] = 0;
        }
        eft05_tip(&m, ew, pl, i);
        for (j = 0; j < 6; j++) {
            flmatCopy(&w->hist[j][i], &m);
        }
    }
    eft05_m(ew);
}

static void eft05_m(EFTW *ew) {
    EFT05G_WORK *w = ew->work;
    PLW *pl = (PLW *)ew->owner;
    s16 i, j, n;
    int draw;
    u16 c;

    if (pl->be_flag == 0) {
        ew->be_flag = 0;
        ew->mode++;
        return;
    }
    if (ew->timer != -1) {
        if (pl->char0 != ew->u0A.ang || --ew->timer <= 0) {
            ew->be_flag = 0;
            ew->mode++;
            return;
        }
    }
    n = EFT05_N(ew->arg);
    for (i = 0; i < n; i++) {
        for (j = 5; j > 0; j--) {
            flmatCopy(&w->hist[j][i], &w->hist[j - 1][i]);
        }
        eft05_tip(&w->hist[0][i], ew, pl, i);
        draw = 1;
        if (ew->timer == -1) {      /* the demon-mode trails: only while the mode is on (and while it ends) */
            c = pl->char0;
            draw = 0;
            if ((((u8 *)pl)[0x4D5] & 0x40) || (c == 0x3FB && frame_check2(pl, 0, 22.0f) == 0)) {
                if (c != 0x3FA || frame_check2(pl, 0, 32.0f) != 0) {
                    if ((u16)(c - 0x3C) >= 2 || (pl->flag14 != 2 && pl->flag14 != 6)) {
                        draw = 1;
                    }
                }
            }
        }
        if (draw && ew->owner->x01 != 0 && Pl_stg_ck(pl) != 0 && w->prim[i] != 0) {
            flmatGetTrans(w->prim[i]->pos, &w->hist[0][i]);
            add_prim(ot0, w->prim[i], 0x40, 0);
        }
    }
}

static void eft05_d(EFTW *ew) {
    EFT05G_WORK *w = ew->work;
    s16 i, n;

    ew->mode++;
    n = EFT05_N(ew->arg);
    for (i = 0; i < n; i++) {
        if (w->prim[i] != 0) {
            if (ew->prim2 == 1) {
                release_prim2(w->prim_no[i]);
            } else {
                release_prim(w->prim_no[i]);
            }
        }
    }
}

static void eft05_e(EFTW *ew) {
    push_eft_work(ew);
}

static void eft05_t(PRIM *pr) {
    FLMAT m, a, b, c;
    EFTW *ew = pr->owner;
    EFT05G_WORK *w = ew->work;
    EFT_MDLW *mw = eft_mdlw[4];
    PLW *pl = (PLW *)ew->owner;
    u16 flag = 0;
    u8 *skin;
    s32 n, j, k, no = pr->no;
    f32 *mc, *sc, len;
    CLAY *cl;
    u8 r, g, bl;

    if (mw == 0 || pl->x01 == 0 || mw->flag == 0) {
        return;
    }
    skin = mw->skin;
    n = *(s16 *)(skin + 0xC2);
    eft05_tip(&m, ew, pl, no);
    flmatCopy((FLMAT *)skin, &m);
    flmatCopy((FLMAT *)(skin + 0x190), &m);
    skin += 0x320;
    n -= 2;
    len = flvecCalcLength(m[2]);        /* G: the blade's length scale (row 2), kept on the blended rows */
    for (j = 0; j < 4; j++) {
        flmatCopy(&a, &w->hist[j][no]);
        flmatCopy(&b, &w->hist[j + 1][no]);
        flmatCopy(&c, &w->hist[j + 2][no]);
        for (k = 0, mc = mat_calc[0], sc = sprine_calc[0]; k < 6; k++, mc += 2, sc += 3) {
            flmatBlend(&m, &a, &b, mc[0], mc[1]);
            m[2][0] *= len;
            m[2][1] *= len;
            m[2][2] *= len;
            flmatSetTrans(&m,
                          (a[3][0] + b[3][0]) / 2.0f * sc[0] + sc[2] * (b[3][0] + c[3][0]) / 2.0f + b[3][0] * sc[1],
                          (a[3][1] + b[3][1]) / 2.0f * sc[0] + sc[2] * (b[3][1] + c[3][1]) / 2.0f + b[3][1] * sc[1],
                          (a[3][2] + b[3][2]) / 2.0f * sc[0] + sc[2] * (b[3][2] + c[3][2]) / 2.0f + b[3][2] * sc[1]);
            flmatCopy((FLMAT *)skin, &m);
            if (--n <= 0) {
                goto done;
            }
            skin += 0x190;
        }
    }
done:
    flSetSkinTrans(mw->skin);
    SetFilterMode(1);
    cl = &mw->clay[eft05_g_tbl[ew->arg]];
    switch (ew->arg) {
    case 0:
        r = Eft_blood_rgb[0];
        g = Eft_blood_rgb[1];
        bl = Eft_blood_rgb[2];
        break;
    case 1:
    case 2:
        r = g = bl = 0xFF;
        flag |= 2;
        break;
    default:            /* the demon mode's red */
        r = 0xFF;
        g = bl = 0x33;
        flag |= 2;
        break;
    }
    flSetRenderState(0x6C, 0);
    flSetRenderState(0x67, (r << 16 | 0xFF000000) | (g << 8) | bl);
    if (cl != 0 && cl->handle != -1) {
        Material_set_sub(mw->mat, cl);
        clay_attr_set(cl->attr);
        Eft_rendope_set(flag);
        flExecuteClay(cl->handle, 0);
    }
    clay_attr_reset();
    flSetRenderState(0x6C, 1);
}

/* G only (game 0x567F70-0x568950; written from the asm, PC only): the Dual Swords' demon-mode glow, a red flicker at
 * each hand (eft_mdlw[0] clay 59, colour 0x33FF3333), drawn on every other frame while the demon mode is on, with
 * the trails' conditions. Created by pl_init for the quest (eft_ds_aura_set, game 0x5688E0: effect type 3, second
 * prim pool). Elements of 0x18 bytes: {s16 blade, s16 prim_no, PRIM *prim, f32 pos[3], s16 blink}; the weapon's
 * blade-table index at work+0x30. Glow points per arg: eft_ds_aura_n (G main 0x2EC6F8: 2). */
typedef struct EFTDS_PT {
    s16 blade;          /* 0x00 0 right hand (joint 0xE), 1 left (0x12) */
    s16 prim_no;        /* 0x02 */
    PRIM *prim;         /* 0x04 */
    f32 pos[3];         /* 0x08 */
    s16 blink;          /* 0x14 */
    s16 _pad16;
} EFTDS_PT;

extern s16 eft_ds_aura_n[4];            /* G main 0x2EC6F8 */
extern f32 eft_ds_aura_ofs[2][0x44];    /* G game 0x679200 / 0x679310 ({x,y,z} per weapon, 0x110 apart) */
extern f32 eft_ds_aura_scl[2][0x44];    /* G game 0x679420 / 0x679530 */
void RotateX(FLMAT *, f32);
void flmatSetTrans(FLMAT *, f32, f32, f32);

static void eft_ds_aura_t(PRIM *pr);

/* game 0x5685B0: the glow's frame: the hand, scaled per weapon and turned (mode 1); mode 0 the bare hand */
static void eft_ds_aura_mat(FLMAT *m, EFTW *ew, PLW *pl, EFTDS_PT *e, int mode) {
    s16 k = *(s16 *)((u8 *)ew->work + 0x30);
    f32 *s;
    int r;
    if (mode == 0) {
        flmatCopy(m, get_joint_wmat(pl, e->blade == 0 ? 0xE : 0x12));
        return;
    }
    if (e->blade == 0) {
        s = &eft_ds_aura_scl[0][k * 3];
        flmatCopy(m, get_joint_wmat(pl, 0xE));
    } else {
        s = &eft_ds_aura_scl[1][k * 3];
        flmatCopy(m, get_joint_wmat(pl, 0x12));
        RotateZ(m, 3.1415927f);
    }
    RotateX(m, 1.5707964f);
    for (r = 0; r < 3; r++) {
        (*m)[r][0] *= s[r];
        (*m)[r][1] *= s[r];
        (*m)[r][2] *= s[r];
    }
}

/* game 0x5687C0: the glow's offset from the hand */
static void eft_ds_aura_ofs_get(f32 *v, EFTW *ew, EFTDS_PT *e) {
    s16 k = *(s16 *)((u8 *)ew->work + 0x30);
    if (e->blade == 0) {
        v[0] = 10.0f + eft_ds_aura_ofs[0][k * 3];
        v[1] = eft_ds_aura_ofs[0][k * 3 + 1] - 1.0f;
        v[2] = eft_ds_aura_ofs[0][k * 3 + 2];
    } else {
        v[0] = -eft_ds_aura_ofs[1][k * 3] - 10.0f;
        v[1] = eft_ds_aura_ofs[1][k * 3 + 1];
        v[2] = eft_ds_aura_ofs[1][k * 3 + 2];
    }
}

static void eft_ds_aura_e(EFTW *ew) {
    push_eft_work(ew);
}

static void eft_ds_aura_i(EFTW *ew) {
    EFTDS_PT *e = ew->work;
    PLW *pl = (PLW *)ew->owner;
    s16 i, n, k;

    ew->mode++;
    ew->mode2 = 0;
    ew->stg = 0;
    ew->be_flag = 1;
    ew->work14 = 0;
    ew->timer = 0;
    n = eft_ds_aura_n[ew->arg];
    if (pl->be_flag == 0 || (k = ds_type_index(pl), *(s16 *)((u8 *)ew->work + 0x30) = k, k == -1)) {
        ew->be_flag = 0;
        eft_ds_aura_e(ew);
        return;
    }
    for (i = 0; i < n; i++, e++) {
        e->blade = i;
        e->blink = i & 1;
        e->prim_no = get_prim2();
        if (e->prim_no != -1) {
            e->prim = get_prim_ptr2(e->prim_no);
            e->prim->owner = ew;
            e->prim->no = i;
            e->prim->trans = eft_ds_aura_t;
        } else {
            e->prim = 0;
        }
    }
}

static void eft_ds_aura_m(EFTW *ew) {
    EFTDS_PT *e = ew->work;
    PLW *pl = (PLW *)ew->owner;
    FLMAT m;
    f32 v[3];
    s16 i, n;
    u16 c;

    if (pl->be_flag == 0) {
        ew->be_flag = 0;
        ew->mode++;
        return;
    }
    if (pl->x01 == 0 || Pl_stg_ck(pl) == 0 || pl->flag14 == 2 || pl->flag14 == 6) {
        return;
    }
    c = pl->char0;
    if (!(((u8 *)pl)[0x4D5] & 0x40) && !(c == 0x3FB && frame_check2(pl, 0, 22.0f) == 0)) {
        return;
    }
    if (c == 0x3FA && frame_check2(pl, 0, 32.0f) == 0) {
        return;
    }
    if (c == 0x3C || c == 0x3D) {
        return;
    }
    n = eft_ds_aura_n[ew->arg];
    for (i = 0; i < n; i++, e++) {
        e->blink ^= 1;
        eft_ds_aura_ofs_get(v, ew, e);
        eft_ds_aura_mat(&m, ew, pl, e, 1);     /* (the G code leaves this mode argument unset here; 1 as in the draw) */
        flvecApplyMat33_2(v, &m);
        e->pos[0] = m[3][0] + v[0];
        e->pos[1] = m[3][1] + v[1];
        e->pos[2] = m[3][2] + v[2];
        if ((e->blink & 1) && e->prim != 0) {
            e->prim->pos[0] = e->pos[0];
            e->prim->pos[1] = e->pos[1];
            e->prim->pos[2] = e->pos[2];
            add_prim(ot0, e->prim, 0x40, 0);
        }
    }
}

static void eft_ds_aura_d(EFTW *ew) {
    EFTDS_PT *e = ew->work;
    s16 i, n;

    ew->mode++;
    n = eft_ds_aura_n[ew->arg];
    for (i = 0; i < n; i++, e++) {
        if (e->prim != 0) {
            release_prim2(e->prim_no);
        }
    }
}

static void eft_ds_aura_move(EFTW *ew) {
    switch (ew->mode) {
    case 0:
        eft_ds_aura_i(ew);
        break;
    case 1:
        eft_ds_aura_m(ew);
        break;
    case 2:
        eft_ds_aura_d(ew);
        break;
    case 3:
        eft_ds_aura_e(ew);
        break;
    }
}

static void eft_ds_aura_t(PRIM *pr) {
    EFTW *ew = pr->owner;
    PLW *pl = (PLW *)ew->owner;
    EFT_MDLW *mw = eft_mdlw[0];
    EFTDS_PT *e = (EFTDS_PT *)ew->work + pr->no;
    FLMAT m;
    CLAY *cl;

    if (mw == 0 || pl->x01 == 0 || mw->flag == 0) {
        return;
    }
    eft_ds_aura_mat(&m, ew, pl, e, 1);
    flmatSetTrans(&m, e->pos[0], e->pos[1], e->pos[2]);
    flSetRenderState(0x1A, (u32)&m);
    cl = (CLAY *)((u8 *)mw->clay + 0x2044);    /* clay 59 */
    flSetRenderState(0x6C, 0);
    flSetRenderState(0x67, 0x33FF3333);
    if (cl != 0 && cl->handle != -1) {
        Material_set_sub(mw->mat, cl);
        clay_attr_set(cl->attr);
        Eft_rendope_set(2);
        flExecuteClay(cl->handle, 0);
    }
    clay_attr_reset();
    flSetRenderState(0x6C, 0);
}

void eft_ds_aura_set(PLW *pl, int arg) {
    EFTW *ew = pull_eft_work(1);

    if (ew != 0) {
        ew->type = 3;
        ew->move = eft_ds_aura_move;
        ew->work14 = 0;
        ew->arg = arg;
        ew->owner = (EMW *)pl;
        ew->x1E = pl->id;
        ew->prim2 = 1;
    }
}

/* G (game 0x56CA10): arg 3 / 4 use the second prim pool (they outlive area changes); no blade trail of arg 0-2
   while the demon mode's own trails show (PLW+0x4D5 bit 0x40) */
void Eft05_set(PLW *pl, int timer, int arg) {
    EFTW *ew;
    int two = (s16)arg == 3 || (s16)arg == 4;

    if (!two && (((u8 *)pl)[0x4D5] & 0x40)) {
        return;
    }
    if (Pl_stg_ck(pl) != 0) {
        ew = pull_eft_work(2);
        if (ew != 0) {
            ew->type = 5;
            ew->move = eft05_move;
            ew->work14 = 0;
            ew->owner = (EMW *)pl;
            ew->x1E = pl->id;
            ew->timer = timer;
            ew->u0A.ang = pl->char0;
            ew->arg = arg;
            if (two) {
                ew->prim2 = 1;
            }
        }
    }
}
#else
static void eft05_i(EFTW *ew) {
    f32 v[3];
    FLMAT m;
    EFT05_WORK *w = ew->work;
    PLW *pl = (PLW *)ew->owner;
    s32 i;

    ew->mode++;
    ew->mode2 = 0;
    ew->be_flag = 1;
    ew->work14 = 0;
    ew->prim_no = get_prim();
    if (ew->prim_no != -1) {
        ew->prim = get_prim_ptr(ew->prim_no);
        ew->prim->owner = ew;
        ew->prim->trans = eft05_t;
    } else {
        ew->prim = 0;
    }
    v[0] = eft05_ofs[ew->arg][0];
    v[1] = eft05_ofs[ew->arg][1];
    v[2] = eft05_ofs[ew->arg][2];
    if (pl->work615 != 0) {
        flmatCopy(&m, get_joint_wmat(pl, 0xE));
    } else {
        flmatCopy(&m, get_joint_wmat(pl, 0x12));
    }
    flvecApplyMat33_2(v, &m);
    m[3][0] += v[0];
    m[3][1] += v[1];
    m[3][2] += v[2];
    for (i = 0; i < 6; i++) {
        flmatCopy(&w->hist[i], &m);
    }
    eft05_m(ew);
}

static void eft05_m(EFTW *ew) {
    f32 v[3];
    FLMAT m;
    EFT05_WORK *w = ew->work;
    PLW *pl = (PLW *)ew->owner;
    s32 i;

    if (pl->char0 != ew->u0A.ang || --ew->timer <= 0) {
        ew->be_flag = 0;
        ew->mode++;
    }
    for (i = 5; i > 0; i--) {
        flmatCopy(&w->hist[i], &w->hist[i - 1]);
    }
    v[0] = eft05_ofs[ew->arg][0];
    v[1] = eft05_ofs[ew->arg][1];
    v[2] = eft05_ofs[ew->arg][2];
    if (pl->work615 != 0) {
        flmatCopy(&m, get_joint_wmat(pl, 0xE));
    } else {
        flmatCopy(&m, get_joint_wmat(pl, 0x12));
    }
    flvecApplyMat33_2(v, &m);
    m[3][0] += v[0];
    m[3][1] += v[1];
    m[3][2] += v[2];
    flmatCopy(&w->hist[0], &m);
    flmatGetTrans(ew->pos, &w->hist[0]);
    if (ew->prim != 0 && ew->owner->x01 != 0) {
        ew->prim->pos[0] = ew->pos[0];
        ew->prim->pos[1] = ew->pos[1];
        ew->prim->pos[2] = ew->pos[2];
        add_prim(ot0, ew->prim, 0x40, 0);
    }
}

static void eft05_d(EFTW *ew) {
    ew->mode++;
    if (ew->prim != 0) {
        release_prim(ew->prim_no);
    }
}

static void eft05_e(EFTW *ew) {
    push_eft_work(ew);
}

static void eft05_t(PRIM *pr) {
    f32 v[3];
    FLMAT m;
    FLMAT a;
    FLMAT b;
    FLMAT c;
    EFTW *ew = pr->owner;
    EFT05_WORK *w;
    EFT_MDLW *mw = eft_mdlw[4];
    u16 flag = 0;
    PLW *pl;
    u8 *skin;
    s32 n;
    s32 j, k;
    f32 *mc;
    f32 *sc;
    CLAY *cl;
    void *mats;
    u8 r, g, bl;

    w = ew->work;
    pl = (PLW *)ew->owner;
    if (mw == 0 || pl->x01 == 0) {
        return;
    }
    if (mw->flag == 0) {
        return;
    }
    skin = mw->skin;
    n = *(s16 *)(skin + 0xC2);
    v[0] = eft05_ofs[ew->arg][0];
    v[1] = eft05_ofs[ew->arg][1];
    v[2] = eft05_ofs[ew->arg][2];
    if (pl->work615 != 0) {
        flmatCopy(&m, get_joint_wmat(pl, 0xE));
    } else {
        flmatCopy(&m, get_joint_wmat(pl, 0x12));
    }
    flvecApplyMat33_2(v, &m);
    m[3][0] += v[0];
    m[3][1] += v[1];
    m[3][2] += v[2];
    flmatCopy((FLMAT *)skin, &m);
    flmatCopy((FLMAT *)(skin + 0x190), &m);
    skin += 0x320;
    n -= 2;
    for (j = 0; j < 4; j++) {
        flmatCopy(&a, &w->hist[j]);
        flmatCopy(&b, &w->hist[j + 1]);
        flmatCopy(&c, &w->hist[j + 2]);
        for (k = 0, mc = mat_calc[0], sc = sprine_calc[0]; k < 6; k++, mc += 2, sc += 3) {
            flmatBlend(&m, &a, &b, mc[0], mc[1]);
            flmatSetTrans(&m,
                          (a[3][0] + b[3][0]) / 2.0f * sc[0] + sc[2] * (b[3][0] + c[3][0]) / 2.0f + b[3][0] * sc[1],
                          (a[3][1] + b[3][1]) / 2.0f * sc[0] + sc[2] * (b[3][1] + c[3][1]) / 2.0f + b[3][1] * sc[1],
                          (a[3][2] + b[3][2]) / 2.0f * sc[0] + sc[2] * (b[3][2] + c[3][2]) / 2.0f + b[3][2] * sc[1]);
            flmatCopy((FLMAT *)skin, &m);
            if (--n <= 0) {
                goto done;
            }
            skin += 0x190;
        }
    }
done:
    flSetSkinTrans(mw->skin);
    SetFilterMode(1);
    cl = &mw->clay[ew->arg];
    mats = mw->mat;
    if (ew->arg == 0) {
        r = Eft_blood_rgb[0];
        g = Eft_blood_rgb[1];
        bl = Eft_blood_rgb[2];
    } else {
        r = 0xFF;
        g = r;
        bl = r;
        flag |= 2;
    }
    flSetRenderState(0x6C, 0);
    flSetRenderState(0x67, (r << 16 | 0xFF000000) | (g << 8) | bl);
    if (cl != 0 && cl->handle != -1) {
        Material_set_sub(mats, cl);
        clay_attr_set(cl->attr);
        Eft_rendope_set(flag);
        flExecuteClay(cl->handle, 0);
    }
    clay_attr_reset();
    flSetRenderState(0x6C, 1);
}

void Eft05_set(PLW *pl, int timer, int arg) {
    EFTW *ew;

    if (Pl_stg_ck(pl) != 0) {
        ew = pull_eft_work(2);
        if (ew != 0) {
            ew->type = 5;
            ew->move = eft05_move;
            ew->work14 = 0;
            ew->owner = (EMW *)pl;
            ew->x1E = pl->id;
            ew->timer = timer;
            ew->u0A.ang = pl->char0;
            ew->arg = arg;
        }
    }
}
#endif
