/* shell10 - game.bin 0x00633B50-0x00634040. A dropped object (model 55 of the
 * effect model set) that lasts up to 9000 frames; on contact it spawns
 * effect 14 and a shell09. */
#include "g_alias.h"
#include "shell.h"
#include "game.h"
#include "pl.h"
#include "prim.h"
#include "clay.h"
#include "fl.h"

typedef struct EFT_MDLW {
    u8 flag;            /* 0x00 */
    u8 _pad01[0x0F];
    s32 mat;            /* 0x10 passed to Material_set_sub */
    u8 _pad14[0x1C];
    CLAY *clay;         /* 0x30 */
} EFT_MDLW;

extern struct {
    EFT_MDLW *p;
    u8 _pad04[0x10];
} eft_mdlw;
extern s32 shell10_body_tbl[1];

int Pl_master_ck(PLW *);
#ifdef MHG_PS2
int Pl_Skill_ck(PLW *, int);
#endif
s16 get_prim2(void);
PRIM *get_prim_ptr2(s16);
void release_prim2(s16);
void flvecCopy(void *, void *);
void eft14_set(VEC3 *, int, f32);
#ifdef MHG_PS2
void Shell09_set(VEC3 *, int, int, int);
#else
void Shell09_set(VEC3 *, int, int);
#endif
void flmatMakeScale(FLMAT *, f32, f32, f32);
void Material_set_sub(s32, CLAY *);

static void shell10_move(SHLW *sh);
static void shell10_i(SHLW *sh);
static void shell10_m(SHLW *sh);
static void shell10_d(SHLW *sh);
static void shell10_e(SHLW *sh);
static void shell10_trans(PRIM *pr);

#if !defined(G_PART) || G_PART == 3
void Shell10_set(f32 *pos, int arg, int stg, PLW *pl) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->type = 10;
        sh->arg = arg;
        sh->move = shell10_move;
        sh->x7A = 0;
        sh->owner = pl;
        sh->pos2.x = pos[0];
        sh->pos2.y = pos[1];
        sh->pos2.z = pos[2];
        sh->stg = stg;
        sh->x09 = 1;
#ifdef MHG_PS2 /* G: kinds other than 0 float higher; x06 is the hunter's skill 0x53 (passed on to the burst) */
        if (sh->arg == 0) {
            sh->pos2.y += 50.0f;
        } else {
            sh->pos2.y += 75.0f;
        }
        sh->x06 = 0;
        if (pl != 0) {
            if (Pl_master_ck(pl) != 0) {
                game_w.shl10_num++;
                sh->x07 = 1;
            }
            if (Pl_Skill_ck(pl, 0x53) == 1) {
                sh->x06 = 1;
            }
        }
#else
        sh->pos2.y += 50.0f;
        if (pl != 0 && Pl_master_ck(pl) != 0) {
            game_w.shl10_num++;
            sh->x07 = 1;
        }
#endif
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
static void shell10_move(SHLW *sh) {
    switch (sh->mode) {
    case 0:
        shell10_i(sh);
        break;
    case 1:
        shell10_m(sh);
        break;
    case 2:
        shell10_d(sh);
        break;
    case 3:
        shell10_d(sh);
        break;
    case 4:
        shell10_d(sh);
        break;
    case 5:
        shell10_d(sh);
        break;
    case 7:
        shell10_d(sh);
        break;
    case 6:
        shell10_e(sh);
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
static void shell10_i(SHLW *sh) {
    sh->mode++;
    sh->be_flag = 1;
    sh->trans = 0;
    sh->x08 = 0xFF;
    sh->xB = 0;
    sh->x1E = 0;
    sh->x9C = 0;
    sh->xA0 = 0;
    sh->x88 = 0;
    sh->x8C = shell10_body_tbl[sh->arg];
    sh->char0 = 0;
    shell_flag_set(sh, 0x100);
    sh->prim_no = get_prim2();
    if (sh->prim_no != -1) {
        sh->prim = get_prim_ptr2(sh->prim_no);
        sh->prim->owner = sh;
        sh->prim->trans = shell10_trans;
        flvecCopy(&sh->prim->pos, &sh->pos2);
        add_prim(ot1, sh->prim, 0x20, 0);
        return;
    }
    shell10_e(sh);
}
#endif

#if !defined(G_PART) || G_PART == 4
static void shell10_m(SHLW *sh) {
    if (++sh->char0 > 9000) {
        sh->xB = 0;
        sh->mode++;
        return;
    }
    if (sh->xB4 != 0) {
        sh->mode++;
#ifdef MHG_PS2 /* G: kinds other than 0 burst bigger into poison (shell09 kind 0x16); x06 goes along */
        if (sh->arg == 0) {
            if (game_w.stage == sh->stg) {
                eft14_set(&sh->pos2, 0, 2.0f);
            }
            Shell09_set(&sh->pos2, 2, sh->stg, sh->x06);
        } else {
            if (game_w.stage == sh->stg) {
                eft14_set(&sh->pos2, 0, 3.0f);
            }
            Shell09_set(&sh->pos2, 0x16, sh->stg, sh->x06);
        }
#else
        if (game_w.stage == sh->stg) {
            eft14_set(&sh->pos2, 0, 2.0f);
        }
        Shell09_set(&sh->pos2, 2, sh->stg);
#endif
    }
    if (sh->prim != 0) {
        flvecCopy(&sh->prim->pos, &sh->pos2);
        add_prim(ot1, sh->prim, 0x20, 0);
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
static void shell10_d(SHLW *sh) {
    sh->xB = 0;
    sh->mode = 6;
    sh->be_flag = 0;
    if (sh->prim != 0) {
        release_prim2(sh->prim_no);
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
static void shell10_e(SHLW *sh) {
    if (sh->x07 != 0) {
        game_w.shl10_num--;
    }
    push_shell_work(sh);
}
#endif

#if !defined(G_PART)
static void disp_sub(CLAY *cl, FLMAT *mat, s32 m) {
    flSetRenderState(0x1A, (u32)mat);
    if (cl != 0 && cl->handle != -1) {
        Material_set_sub(m, cl);
        clay_attr_set(cl->attr);
        flExecuteClay(cl->handle, 0);
    }
    clay_attr_reset();
}
#endif

#if !defined(G_PART) || G_PART == 5
static void shell10_trans(PRIM *pr) {
    FLMAT mat;
    SHLW *sh = pr->owner;
    EFT_MDLW *mw;
    s32 m;

    if (game_w.stage == sh->stg) {
        mw = eft_mdlw.p;
        if (mw != 0 && mw->flag != 0) {
            m = mw->mat;
#ifdef MHG_PS2 /* G: kinds other than 0 are drawn bigger */
            if (sh->arg == 0) {
                flmatMakeScale(&mat, 3.0f, 3.0f, 3.0f);
            } else {
                flmatMakeScale(&mat, 4.5f, 4.5f, 4.5f);
            }
#else
            flmatMakeScale(&mat, 3.0f, 3.0f, 3.0f);
#endif
            flmatSetTrans(&mat, sh->pos2.x, sh->pos2.y, sh->pos2.z);
            disp_sub(&mw->clay[55], &mat, m);
        }
    }
}
#endif
