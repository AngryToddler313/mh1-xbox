/* set19 - game.bin 0x006263B0-0x00626E64. Up to four stage models placed
 * from a per-stage position table, each with its own texture scroll: one
 * turning slowly, one scrolling on stage 48, and two more with per-stage
 * scroll directions. A per-stage mask table switches each slot off. */
#include "set.h"
#include "game.h"
#include "prim.h"
#include "clay.h"
#include "fl.h"

typedef struct SET_MDLW {
    u8 flag;            /* 0x00 */
    u8 _pad01[0x2F];
    CLAY *clay;         /* 0x30 */
} SET_MDLW;

typedef struct STAGE_WORK {
    u8 _pad00[0x3C];
    SET_MDLW *mdl;      /* 0x3C stage model set */
} STAGE_WORK;

#define ANG2DEG(a) (360.0f * (f32)(a) / 65536.0f)
#define DEG2RAD(d) (2.0f * (3.1415927f * ((d) / 360.0f)))

extern SET_MDLW *set_mdlw;
extern STAGE_WORK stage_work;
extern f32 set19_s11_pos_tbl[];
extern f32 set19_s12_pos_tbl[];
extern f32 set19_s21_pos_tbl[];
extern f32 set19_s28_pos_tbl[];
extern f32 set19_s30_pos_tbl[];
extern f32 set19_s34_pos_tbl[];
extern f32 set19_s48_pos_tbl[];
extern f32 set19_s52_pos_tbl[];
extern f32 set19_s54_pos_tbl[];
extern f32 set19_s57_pos_tbl[];
extern f32 set19_s61_pos_tbl[];
extern f32 set19_s67_pos_tbl[];
extern f32 set19_s73_s75_pos_tbl[];
extern s16 set19_s11_mask_tbl[4];
extern s16 set19_s12_mask_tbl[4];
extern s16 set19_s21_mask_tbl[4];
extern s16 set19_s28_mask_tbl[4];
extern s16 set19_s30_mask_tbl[4];
extern s16 set19_s34_mask_tbl[4];
extern s16 set19_s48_mask_tbl[4];
extern s16 set19_s52_mask_tbl[4];
extern s16 set19_s54_mask_tbl[4];
extern s16 set19_s57_mask_tbl[4];
extern s16 set19_s61_mask_tbl[4];
extern s16 set19_s67_mask_tbl[4];
extern s16 set19_s73_s75_mask_tbl[4];
extern f32 uv_pos[16][2];
extern u8 ot3[];

void flvecCopy(f32 *, f32 *);
f32 flFloor(f32);

static void set19_move(SETW *sw);
static void set19_i(SETW *sw);
static void set19_m(SETW *sw);
static void set19_d(SETW *sw);
static void set19_e(SETW *sw);
static void set19_trans(PRIM *pr);

void Set19_set(void) {
    SETW *sw = pull_set_work(0);

    if (sw != 0) {
        sw->type = 19;
        sw->arg = 0;
        sw->move = set19_move;
    }
}

static void set19_move(SETW *sw) {
    switch (sw->mode) {
    case 0:
        set19_i(sw);
        break;
    case 1:
        set19_m(sw);
        break;
    case 2:
        set19_d(sw);
        break;
    case 3:
        set19_e(sw);
        break;
    }
}

#ifdef VERSION_MHG
/* G (game 0x657580 / 0x657860): new stages 0xF, 0x18 (own table), 0x4A, 0x5B; stage 0x43 and 0x49/0x4A/0x4B read
 * tables G moved to its bss (set19g_s67_pos, set19g_s73_pos: never written, so G draws those pieces at the world
 * origin, as it does for 0xF, 0x18 and 0x5B) [asm read; PCSX2 memory read: all zero in a quest]. The masks of stages
 * 11, 12, 21 sit at other addresses than mhg_map put the MH1 names (set19g_*_mask, config/mhg/g_names.txt). */
extern f32 set19g_s15_pos[];
extern f32 set19g_s24_pos[];
extern f32 set19g_s67_pos[];
extern f32 set19g_s73_pos[];
extern f32 set19g_s91_pos[];
extern s16 set19g_s11_mask[4];
extern s16 set19g_s12_mask[4];
extern s16 set19g_s15_mask[4];
extern s16 set19g_s21_mask[4];
extern s16 set19g_s24_mask[4];
extern s16 set19g_s91_mask[4];
#ifndef __MWERKS__      /* PC: G's bss tables (zero, as on the PS2) */
f32 set19g_s15_pos[6];
f32 set19g_s24_pos[12];
f32 set19g_s67_pos[4];
f32 set19g_s73_pos[4];
f32 set19g_s91_pos[12];
s16 set19g_s24_mask[4];
#endif

static void set19_i(SETW *sw) {
    f32 *p;

    switch (game_w.stage) {
    case 0xB:
        p = set19_s11_pos_tbl;
        break;
    case 0xC:
        p = set19_s12_pos_tbl;
        break;
    case 0xF:
        p = set19g_s15_pos;
        break;
    case 0x15:
        p = set19_s21_pos_tbl;
        break;
    case 0x18:
        p = set19g_s24_pos;
        break;
    case 0x1C:
        p = set19_s28_pos_tbl;
        break;
    case 0x1E:
        p = set19_s30_pos_tbl;
        break;
    case 0x22:
        p = set19_s34_pos_tbl;
        break;
    case 0x30:
        p = set19_s48_pos_tbl;
        break;
    case 0x34:
        p = set19_s52_pos_tbl;
        break;
    case 0x36:
        p = set19_s54_pos_tbl;
        break;
    case 0x39:
        p = set19_s57_pos_tbl;
        break;
    case 0x3D:
        p = set19_s61_pos_tbl;
        break;
    case 0x43:
        p = set19g_s67_pos;
        break;
    case 0x49:
    case 0x4A:
    case 0x4B:
        p = set19g_s73_pos;
        break;
    case 0x5B:
        p = set19g_s91_pos;
        break;
    }
    sw->mode++;
    sw->be_flag = 1;
    sw->pos[0] = *p++;
    sw->pos[1] = *p++;
    sw->pos[2] = *p++;
    sw->timer = 0;
    sw->speed = 0.0f;
    sw->work14 = 0;
    sw->prim = get_prim_ptr(get_prim());
    if (sw->prim != 0) {
        sw->prim->owner = sw;
        sw->prim->trans = set19_trans;
        flvecCopy(sw->prim->pos, sw->pos);
        set19_m(sw);
        return;
    }
    push_set_work(sw);
}
#else
static void set19_i(SETW *sw) {
    f32 *p;

    switch (game_w.stage) {
    case 0xB:
        p = set19_s11_pos_tbl;
        break;
    case 0xC:
        p = set19_s12_pos_tbl;
        break;
    case 0x15:
        p = set19_s21_pos_tbl;
        break;
    case 0x1C:
        p = set19_s28_pos_tbl;
        break;
    case 0x1E:
        p = set19_s30_pos_tbl;
        break;
    case 0x18:
    case 0x22:
        p = set19_s34_pos_tbl;
        break;
    case 0x30:
        p = set19_s48_pos_tbl;
        break;
    case 0x34:
        p = set19_s52_pos_tbl;
        break;
    case 0x36:
        p = set19_s54_pos_tbl;
        break;
    case 0x39:
        p = set19_s57_pos_tbl;
        break;
    case 0x3D:
        p = set19_s61_pos_tbl;
        break;
    case 0x43:
        p = set19_s67_pos_tbl;
        break;
    case 0x49:
    case 0x4B:
        p = set19_s73_s75_pos_tbl;
        break;
    }
    sw->mode++;
    sw->be_flag = 1;
    sw->pos[0] = *p++;
    sw->pos[1] = *p++;
    sw->pos[2] = *p++;
    sw->timer = 0;
    sw->speed = 0.0f;
    sw->work14 = 0;
    sw->prim = get_prim_ptr(get_prim());
    if (sw->prim != 0) {
        sw->prim->owner = sw;
        sw->prim->trans = set19_trans;
        flvecCopy(sw->prim->pos, sw->pos);
        return;
    }
    push_set_work(sw);
}
#endif

static void set19_m(SETW *sw) {
    sw->timer++;
    sw->cnt += 0x5B;
    add_prim(ot3, sw->prim, 8, 1);
}

static void set19_d(SETW *sw) {
    sw->mode++;
    sw->be_flag = 0;
}

static void set19_e(SETW *sw) {
    push_set_work(sw);
}

#ifdef VERSION_MHG
/* G: the slots' matrices are built with flmatInit + flmatSetTrans (rotation first), stage 0x18's slots scroll by
 * (timer & 0x1FF) / 512 and its third slot draws one more clay, stage 0x5B scrolls nothing, 0xF's last slot by
 * (timer & 0x7F) / 128. */
static void set19_trans(PRIM *pr) {
    FLMAT mat;
    FLMAT uv;
    SETW *sw = pr->owner;
    SET_MDLW *mw = set_mdlw;
    STAGE_WORK *stw = &stage_work;
    CLAY *cl;
    s16 no;
    s16 *mask;
    f32 *pos;
    f32 u, v;

    if (mw != 0 && mw->flag != 0) {
        switch (game_w.stage) {
        case 0xB:
            no = 3;
            pos = set19_s11_pos_tbl;
            mask = set19g_s11_mask;
            break;
        case 0xC:
            no = 5;
            pos = set19_s12_pos_tbl;
            mask = set19g_s12_mask;
            break;
        case 0xF:
            no = 6;
            pos = set19g_s15_pos;
            mask = set19g_s15_mask;
            break;
        case 0x15:
            no = 1;
            pos = set19_s21_pos_tbl;
            mask = set19g_s21_mask;
            break;
        case 0x18:
            no = 2;
            pos = set19g_s24_pos;
            mask = set19g_s24_mask;
            break;
        case 0x1C:
            no = 25;
            pos = set19_s28_pos_tbl;
            mask = set19_s28_mask_tbl;
            break;
        case 0x1E:
            no = 3;
            pos = set19_s30_pos_tbl;
            mask = set19_s30_mask_tbl;
            break;
        case 0x22:
            no = 6;
            pos = set19_s34_pos_tbl;
            mask = set19_s34_mask_tbl;
            break;
        case 0x30:
            no = 1;
            pos = set19_s48_pos_tbl;
            mask = set19_s48_mask_tbl;
            break;
        case 0x34:
            no = 0;
            pos = set19_s52_pos_tbl;
            mask = set19_s52_mask_tbl;
            break;
        case 0x36:
            no = 0;
            pos = set19_s54_pos_tbl;
            mask = set19_s54_mask_tbl;
            break;
        case 0x39:
            no = 1;
            pos = set19_s57_pos_tbl;
            mask = set19_s57_mask_tbl;
            break;
        case 0x3D:
            no = 1;
            pos = set19_s61_pos_tbl;
            mask = set19_s61_mask_tbl;
            break;
        case 0x43:
            no = 0;
            pos = set19g_s67_pos;
            mask = set19_s67_mask_tbl;
            break;
        case 0x49:
        case 0x4A:
        case 0x4B:
            mw = stw->mdl;
            no = 3;
            pos = set19g_s73_pos;
            mask = set19_s73_s75_mask_tbl;
            break;
        case 0x5B:
            mw = stw->mdl;
            no = 6;
            pos = set19g_s91_pos;
            mask = set19g_s91_mask;
            break;
        }
        if (*mask++ == 0) {
            flmatInit(&mat);
            cl = &mw->clay[no++];
            switch (game_w.stage) {
            case 0x18:
                flSetRenderState(0x60, 0);
                u = (f32)(sw->timer & 0x1FF) / 512.0f;
                break;
            case 0x5B:
                flSetRenderState(0x60, 0);
                u = 0.0f;
                break;
            default:
                flSetRenderState(0x60, 0x80);
                u = (1.0f / 30.0f) * ANG2DEG(sw->cnt);
                u -= flFloor(u);
                flmatRotY33(&mat, DEG2RAD(ANG2DEG(sw->cnt)));
                break;
            }
            flmatMakeTrans(&uv, u, 0.0f, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            u = *pos++;
            v = *pos++;
            flmatSetTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        flSetRenderState(0x60, 0);
        if (*mask++ == 0) {
            flmatInit(&mat);
            cl = &mw->clay[no++];
            switch (game_w.stage) {
            case 0x18:
                flmatMakeTrans(&uv, (f32)(sw->timer & 0x1FF) / 512.0f, 0.0f, 0.0f);
                flSetRenderState(0x19, (u32)&uv);
                break;
            case 0x30:
                flmatMakeTrans(&uv, 0.0f, (f32)(sw->timer & 0x7F) / 128.0f, 0.0f);
                flSetRenderState(0x19, (u32)&uv);
                break;
            }
            u = *pos++;
            v = *pos++;
            flmatSetTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        if (*mask++ == 0) {
            flmatInit(&mat);
            switch (game_w.stage) {
            case 0x15:
            case 0x22:
            case 0x30:
            case 0x34:
            case 0x36:
            case 0x39:
            case 0x43:
                u = 0.0f;
                v = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            case 0x18:
                flmatMakeTrans(&uv, (f32)(sw->timer & 0x1FF) / 512.0f, 0.0f, 0.0f);
                flSetRenderState(0x19, (u32)&uv);
                cl = &mw->clay[no++];
                u = pos[0];
                v = pos[1];
                flmatSetTrans(&mat, u, v, pos[2]);
                flSetRenderState(0x1A, (u32)&mat);
                clay_attr_set(cl->attr);
                flExecuteClay(cl->handle, 0);
                break;
            case 0x3D:
            case 0x49:
            case 0x4A:
            case 0x4B:
                v = 0.0f;
                u = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            case 0x5B:
                u = 0.0f;
                v = u;
                break;
            default:
                u = uv_pos[sw->timer & 0xF][0];
                v = uv_pos[sw->timer & 0xF][1];
                break;
            }
            flmatMakeTrans(&uv, u, v, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            cl = &mw->clay[no++];
            u = *pos++;
            v = *pos++;
            flmatSetTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        if (*mask == 0) {
            cl = &mw->clay[no];
            flmatInit(&mat);
            switch (game_w.stage) {
            case 0xB:
            case 0xC:
            case 0x1E:
            case 0x1C:
                v = 0.0f;
                u = (f32)(sw->timer & 0x3FF) / 1024.0f;
                break;
            case 0xF:
                v = 0.0f;
                u = (f32)(sw->timer & 0x7F) / 128.0f;
                break;
            case 0x18:
                u = uv_pos[sw->timer & 0xF][0];
                v = uv_pos[sw->timer & 0xF][1];
                break;
            case 0x30:
                v = 0.0f;
                u = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            }
            flmatMakeTrans(&uv, u, v, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            u = *pos++;
            v = *pos++;
            flmatSetTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        clay_attr_reset();
    }
}
#else
static void set19_trans(PRIM *pr) {
    FLMAT mat;
    FLMAT uv;
    SETW *sw = pr->owner;
    SET_MDLW *mw = set_mdlw;
    STAGE_WORK *stw = &stage_work;
    CLAY *cl;
    s16 no;
    f32 *pos;
    s16 *mask;
    f32 u, v;

    if (mw != 0 && mw->flag != 0) {
        switch (game_w.stage) {
        case 0xB:
            no = 3;
            pos = set19_s11_pos_tbl;
            mask = set19_s11_mask_tbl;
            break;
        case 0xC:
            no = 5;
            pos = set19_s12_pos_tbl;
            mask = set19_s12_mask_tbl;
            break;
        case 0x15:
            no = 1;
            pos = set19_s21_pos_tbl;
            mask = set19_s21_mask_tbl;
            break;
        case 0x1C:
            no = 25;
            pos = set19_s28_pos_tbl;
            mask = set19_s28_mask_tbl;
            break;
        case 0x1E:
            no = 3;
            pos = set19_s30_pos_tbl;
            mask = set19_s30_mask_tbl;
            break;
        case 0x18:
        case 0x22:
            no = 6;
            pos = set19_s34_pos_tbl;
            mask = set19_s34_mask_tbl;
            break;
        case 0x30:
            no = 1;
            pos = set19_s48_pos_tbl;
            mask = set19_s48_mask_tbl;
            break;
        case 0x34:
            no = 0;
            pos = set19_s52_pos_tbl;
            mask = set19_s52_mask_tbl;
            break;
        case 0x36:
            no = 0;
            pos = set19_s54_pos_tbl;
            mask = set19_s54_mask_tbl;
            break;
        case 0x39:
            no = 1;
            pos = set19_s57_pos_tbl;
            mask = set19_s57_mask_tbl;
            break;
        case 0x3D:
            no = 1;
            pos = set19_s61_pos_tbl;
            mask = set19_s61_mask_tbl;
            break;
        case 0x43:
            no = 0;
            pos = set19_s67_pos_tbl;
            mask = set19_s67_mask_tbl;
            break;
        case 0x49:
        case 0x4B:
            mw = stw->mdl;
            no = 3;
            pos = set19_s73_s75_pos_tbl;
            mask = set19_s73_s75_mask_tbl;
            break;
        }
        flSetRenderState(0x60, 0x80);
        if (*mask++ == 0) {
            u = (1.0f / 30.0f) * ANG2DEG(sw->cnt);
            u -= flFloor(u);
            flmatMakeTrans(&uv, u, 0.0f, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            cl = &mw->clay[no++];
            u = *pos++;
            v = *pos++;
            flmatMakeTrans(&mat, u, v, *pos++);
            flmatRotY33(&mat, DEG2RAD(ANG2DEG(sw->cnt)));
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        flSetRenderState(0x60, 0);
        if (*mask++ == 0) {
            cl = &mw->clay[no++];
            switch (game_w.stage) {
            case 0x30:
                flmatMakeTrans(&uv, 0.0f, (f32)(sw->timer & 0x7F) / 128.0f, 0.0f);
                flSetRenderState(0x19, (u32)&uv);
                break;
            }
            u = *pos++;
            v = *pos++;
            flmatMakeTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        if (*mask++ == 0) {
            switch (game_w.stage) {
            case 0x15:
            case 0x18:
            case 0x22:
            case 0x30:
            case 0x34:
            case 0x36:
            case 0x39:
            case 0x43:
                u = 0.0f;
                v = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            case 0x3D:
            case 0x49:
            case 0x4B:
                v = 0.0f;
                u = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            default:
                u = uv_pos[sw->timer & 0xF][0];
                v = uv_pos[sw->timer & 0xF][1];
                break;
            }
            flmatMakeTrans(&uv, u, v, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            cl = &mw->clay[no++];
            u = *pos++;
            v = *pos++;
            flmatMakeTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        if (*mask == 0) {
            cl = &mw->clay[no];
            switch (game_w.stage) {
            case 0xB:
            case 0xC:
            case 0x1E:
            case 0x1C:
                v = 0.0f;
                u = (f32)(sw->timer & 0x3FF) / 1024.0f;
                break;
            case 0x30:
                v = 0.0f;
                u = (f32)(int)(u8)sw->timer / 256.0f;
                break;
            }
            flmatMakeTrans(&uv, u, v, 0.0f);
            flSetRenderState(0x19, (u32)&uv);
            u = *pos++;
            v = *pos++;
            flmatMakeTrans(&mat, u, v, *pos++);
            flSetRenderState(0x1A, (u32)&mat);
            clay_attr_set(cl->attr);
            flExecuteClay(cl->handle, 0);
        }
        clay_attr_reset();
    }
}
#endif
