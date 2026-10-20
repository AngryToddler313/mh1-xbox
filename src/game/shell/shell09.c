/* shell09 - game.bin 0x006331C0-0x00633B48. Blasts/shockwaves; some types
 * also spawn a second shell09 for the wind-noise effect ("kaze oto"). */
#include "g_alias.h"
#include "shell.h"
#include "game.h"
#include "pl.h"
#include "prim.h"

extern u8 shell09_tbl[4];
extern s32 shell09_body_tbl[];
extern s16 atck_data_no[20];

void flvecCopy(void *, void *);
void atck_data_set_shl(SHLW *, int, u8 *);
void pl_atck_data_set_shl(SHLW *, void *, int, u8 *);
void se_req2(int, int, int, VEC3 *, int, int);
u16 ran_suu(int);
void release_prim(s16);
#ifdef MHG_PS2
void game_func_00666260(SHLW *sh);
void game_func_00666350(SHLW *sh);     /* G: mode 3 runs shell09_m again for the poison cloud (arg 19), else shell09_d */
#endif

#if defined(VERSION_MHG) && !defined(__MWERKS__)
/* defined in the G block */
static void shell09_set_com(SHLW *sh);
#endif
static void shell09_move(SHLW *sh);
static void shell09_kaze_oto_set(SHLW *sh);
static void shell09_se_req(SHLW *sh);
static void shell09_i(SHLW *sh);
static void shell09_m(SHLW *sh);
static void shell09_d(SHLW *sh);
static void shell09_e(SHLW *sh);

#if !defined(G_PART) || G_PART == 4
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_set_com(SHLW *sh) {
    sh->type = 9;
    sh->move = shell09_move;
    switch (sh->arg) {
    case 0:
    case 2:
    case 7:
    case 8:
    case 0xE:
    case 0xF:
    case 0x10:
#ifdef MHG_PS2
    case 20:            /* G: the big bombs */
    case 22:
#endif
        shell09_kaze_oto_set(sh);
        break;
    }
#ifdef MHG_PS2
    sh->x06 = 0;
#endif
}
#endif
#endif

#if !defined(G_PART) || G_PART == 5
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
#ifdef MHG_PS2
void Shell09_set(VEC3 *pos, int arg, int stg, int skill) {     /* G: x06 = the bomber skill flag */
#else
void Shell09_set(VEC3 *pos, int arg, int stg) {
#endif
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        flvecCopy(&sh->pos2, pos);
        sh->em_no = 0;
        sh->x7A = 0;
        sh->stg = stg;
        shell09_set_com(sh);
#ifdef MHG_PS2
        sh->x06 = skill;
#endif
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 6
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
void Shell09_set_pl(PLW *pl, VEC3 *pos, int arg) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        flvecCopy(&sh->pos2, pos);
#ifdef MHG_PS2
        if (arg != 2 && arg != 22) {
#else
        if (arg != 2) {
#endif
            sh->em_no = pl->id;
            sh->x7A = ((EMW *)pl)->x10;
            sh->owner = pl;
            sh->xC8 = ((EMW *)pl)->ang[1];
            sh->stg = pl->stg;
        } else {
            sh->em_no = 0;
            sh->x7A = 0;
            sh->stg = game_w.stage;
        }
        shell09_set_com(sh);
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 1
void Shell09_set_em(EMW *em, VEC3 *pos, int arg) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        sh->em_no = em->id;
        sh->x7A = em->x10;
        sh->owner = em;
        sh->xC8 = em->ang[1];
        flvecCopy(&sh->pos2, pos);
        sh->stg = em->stg;
        shell09_set_com(sh);
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void Shell09_set_pl2(PLW *pl, VEC3 *pos, int arg, int stg) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        flvecCopy(&sh->pos2, pos);
        sh->em_no = pl->id;
        sh->x7A = ((EMW *)pl)->x10;
        sh->owner = pl;
        sh->xC8 = ((EMW *)pl)->ang[1];
        sh->stg = stg;
        shell09_set_com(sh);
    }
}
#endif

#if !defined(G_PART) || G_PART == 7
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_kaze_oto_set(SHLW *sh) {
    SHLW *nw;
    int arg;

    switch (sh->arg) {
    case 0:
    case 7:
    case 8:
    case 0xE:
    case 0xF:
    case 0x10:
        arg = 9;
        break;
    case 1:
        arg = 3;
        break;
    case 2:
        arg = 10;
        break;
    case 4:
        arg = 5;
        break;
#ifdef MHG_PS2
    case 20:            /* G: the big bombs have their own wind noise */
        arg = 21;
        break;
    case 22:
        arg = 23;
        break;
#endif
    default:
        return;
    }
    if ((nw = pull_shell_work(0)) != 0) {
        nw->type = 9;
        nw->arg = arg;
        nw->move = shell09_move;
        nw->em_no = sh->em_no;
        nw->x7A = sh->x7A;
        nw->owner = sh->owner;
        nw->xC8 = sh->xC8;
        flvecCopy(&nw->pos2, &sh->pos2);
        nw->stg = sh->stg;
#ifdef MHG_PS2
        nw->x06 = 0;
#endif
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 2
static void shell09_move(SHLW *sh) {
    switch (sh->mode) {
    case 0:
        shell09_i(sh);
        break;
    case 1:
        shell09_m(sh);
        break;
    case 2:
        shell09_d(sh);
        break;
    case 3:
        GV(shell09_d, game_func_00666260)(sh);
        break;
    case 4:
        shell09_d(sh);
        break;
    case 5:
        shell09_d(sh);
        break;
    case 7:
        shell09_d(sh);
        break;
    case 6:
        shell09_e(sh);
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 8
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_se_req(SHLW *sh) {
    if (sh->stg == game_w.stage) {
        switch (sh->arg) {
        case 0:
        case 0x10:
#ifdef MHG_PS2
        case 20:
#endif
            se_req2(1, 0x4D, 0, &sh->pos2, 1, 0);
            break;
        case 2:
#ifdef MHG_PS2
        case 22:
#endif
            se_req2(1, 0x4F, 0, &sh->pos2, 1, 0);
            break;
        case 7:
            if (ran_suu(1) & 1) {
                se_req2(1, 0x4E, 0, &sh->pos2, 1, 0);
            }
            break;
        case 8:
        case 0xE:
        case 0xF:
            se_req2(1, 0x4E, 0, &sh->pos2, 1, 0);
            break;
        }
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 9
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_i(SHLW *sh) {
    sh->mode++;
    sh->be_flag = 1;
    sh->trans = 0;
    shell09_se_req(sh);
    switch (sh->arg) {
    case 0:
    case 2:
    case 0x10:
#ifdef MHG_PS2
    case 20:
    case 22:
#endif
        shell_flag_set(sh, 4);
        atck_data_set_shl(sh, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    case 6:
        shell_flag_set(sh, 0);
        atck_data_set_shl(sh, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    case 1:
    case 4:
#ifdef MHG_PS2
    case 18:
#endif
        shell_flag_set(sh, 0x200);
        pl_atck_data_set_shl(sh, sh->owner, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    case 7:
    case 8:
    case 0xE:
    case 0xF:
        shell_flag_set(sh, 4);
        pl_atck_data_set_shl(sh, sh->owner, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    case 9:
    case 0xA:
#ifdef MHG_PS2
    case 21:
    case 23:
#endif
        shell_flag_set(sh, 0x400);
        atck_data_set_shl(sh, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    case 0xD:
        shell_flag_set(sh, 0x400);
        pl_atck_data_set_shl(sh, sh->owner, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    default:
        shell_flag_set(sh, 0);
        pl_atck_data_set_shl(sh, sh->owner, GV(atck_data_no[sh->arg], sh->arg + 1), shell09_tbl);
        break;
    }
#ifdef MHG_PS2
    game_func_00666350(sh);     /* G: the bomber skill raises the blast power */
    sh->x88 = shell09_body_tbl[sh->body];
#else
    sh->x88 = shell09_body_tbl[sh->arg];
#endif
    sh->x8C = 0;
    sh->char0 = 0;
    sh->prim = 0;
}
#endif
#endif

#if !defined(G_PART)
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_m(SHLW *sh) {
#ifdef MHG_PS2      /* G: kind 18 also sets its wind noise; the poison cloud (19) lasts 80 ticks */
    s16 limit;

    if ((sh->arg == 1 || sh->arg == 4 || sh->arg == 18) && sh->x61 <= 0) {
        shell09_kaze_oto_set(sh);
        sh->x61 = 0;
        sh->mode = 2;
        return;
    }
    if (sh->arg != 19) {
        limit = 40;
    } else {
        limit = 80;
    }
    if (++sh->char0 > limit) {
        sh->x61 = 0;
        sh->mode = 2;
        return;
    }
#else
    if ((sh->arg == 1 || sh->arg == 4) && sh->x61 <= 0) {
        shell09_kaze_oto_set(sh);
        sh->x61 = 0;
        sh->mode++;
        return;
    }
    if (++sh->char0 > 40) {
        sh->x61 = 0;
        sh->mode++;
    }
#endif
    if (sh->prim != 0) {
        flvecCopy(&sh->prim->pos, &sh->pos2);
        add_prim(ot1, sh->prim, 0x20, 0);
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 3
#if !defined(VERSION_MHG) || defined(__MWERKS__)    /* MH1; G on the PC: the G block at the end of the file */
static void shell09_d(SHLW *sh) {
#ifdef MHG_PS2
    if ((sh->arg == 1 || sh->arg == 4 || sh->arg == 18) && sh->mode == 3) {     /* G: kind 18 too */
#else
    if ((sh->arg == 1 || sh->arg == 4) && sh->mode == 3) {
#endif
        shell09_kaze_oto_set(sh);
    }
    sh->xB = 0;
    sh->mode = 6;
    sh->be_flag = 0;
    if (sh->prim != 0) {
        release_prim(sh->prim_no);
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 3
static void shell09_e(SHLW *sh) {
    push_shell_work(sh);
}
#endif

#if defined(VERSION_MHG) && !defined(__MWERKS__)
/* PS2 Monster Hunter G's shell09 (blast areas) for the PC (agent A, 15 Oct 2026; written from G game.bin
 * 0x665920-0x666440, not byte-checked). G drops MH1's atck_data_no table (attack data number = arg + 1; on the G PC
 * the MH1 table was an empty work area, so every blast used attack data 0), takes the hit body from the attack
 * data (body 0x6A) and adds kinds 18-23 (19: the poison smoke cloud, 80 ticks; 20/22: big bombs with their own wind
 * noise kinds 21/23). Shell09_set gets a 4th argument, the bomber skill flag (x06): game 0x666350 raises the
 * attack power of blast kinds 0/2/20/22 by half. */
static void shell09_set_com(SHLW *sh) {
    sh->type = 9;
    sh->move = shell09_move;
    switch (sh->arg) {
    case 0: case 2: case 7: case 8: case 14: case 15: case 16: case 20: case 22:
        shell09_kaze_oto_set(sh);
        break;
    }
    sh->x06 = 0;
}

void Shell09_set_g(VEC3 *pos, int arg, int stg, int skill) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        flvecCopy(&sh->pos2, pos);
        sh->em_no = 0;
        sh->x7A = 0;
        sh->stg = stg;
        shell09_set_com(sh);
        sh->x06 = skill;
    }
}

void Shell09_set(VEC3 *pos, int arg, int stg) {      /* the MH1 3-argument callers (no skill) */
    Shell09_set_g(pos, arg, stg, 0);
}

void Shell09_set_pl(PLW *pl, VEC3 *pos, int arg) {
    SHLW *sh;

    if ((sh = pull_shell_work(0)) != 0) {
        sh->arg = arg;
        flvecCopy(&sh->pos2, pos);
        if (arg != 2 && arg != 22) {
            sh->em_no = pl->id;
            sh->x7A = ((EMW *)pl)->x10;
            sh->owner = pl;
            sh->xC8 = ((EMW *)pl)->ang[1];
            sh->stg = pl->stg;
        } else {
            sh->em_no = 0;
            sh->x7A = 0;
            sh->stg = game_w.stage;
        }
        shell09_set_com(sh);
    }
}

static void shell09_kaze_oto_set(SHLW *sh) {
    SHLW *nw;
    int arg;

    switch (sh->arg) {
    case 0: case 7: case 8: case 14: case 15: case 16:
        arg = 9;
        break;
    case 1:
        arg = 3;
        break;
    case 2:
        arg = 10;
        break;
    case 4:
        arg = 5;
        break;
    case 20:
        arg = 21;
        break;
    case 22:
        arg = 23;
        break;
    default:
        return;
    }
    if ((nw = pull_shell_work(0)) != 0) {
        nw->type = 9;
        nw->arg = arg;
        nw->move = shell09_move;
        nw->em_no = sh->em_no;
        nw->x7A = sh->x7A;
        nw->owner = sh->owner;
        nw->xC8 = sh->xC8;
        flvecCopy(&nw->pos2, &sh->pos2);
        nw->stg = sh->stg;
        nw->x06 = 0;
    }
}

static void shell09_se_req(SHLW *sh) {
    if (sh->stg == game_w.stage) {
        switch (sh->arg) {
        case 0: case 16: case 20:
            se_req2(1, 0x4D, 0, &sh->pos2, 1, 0);
            break;
        case 2: case 22:
            se_req2(1, 0x4F, 0, &sh->pos2, 1, 0);
            break;
        case 7:
            if (ran_suu(1) & 1) {
                se_req2(1, 0x4E, 0, &sh->pos2, 1, 0);
            }
            break;
        case 8: case 14: case 15:
            se_req2(1, 0x4E, 0, &sh->pos2, 1, 0);
            break;
        }
    }
}

static void shell09_bomber(SHLW *sh) {      /* G game 0x666350 */
    u32 v;

    if (sh->x06 == 1 && (sh->arg == 0 || sh->arg == 2 || sh->arg == 20 || sh->arg == 22)) {
        v = (u32)(1.5f * (f32)sh->x62) & 0xFFFF;
        sh->x62 = v >= 0x100 ? 0xFF : (u8)v;
    }
}

static void shell09_i(SHLW *sh) {
    static const u8 how[24] = {     /* G's jump table (0x6DAFB0): 0 flag 4 / 1 flag 0 / 2 flag 0x400 with the plain
                                     * attack data, 3 flag 0x200 / 4 flag 4 / 5 flag 0x400 / 6 flag 0 with the player's */
        0, 3, 0, 6, 3, 6, 1, 4, 4, 2, 2, 6, 6, 5, 4, 4, 0, 6, 3, 6, 0, 2, 0, 2 };
    static const u16 flag[7] = { 4, 0, 0x400, 0x200, 4, 0x400, 0 };
    int h = sh->arg < 24 ? how[sh->arg] : 6;

    sh->mode++;
    sh->be_flag = 1;
    sh->trans = 0;
    shell09_se_req(sh);
    shell_flag_set(sh, flag[h]);
    if (h < 3) {
        atck_data_set_shl(sh, sh->arg + 1, shell09_tbl);
    } else {
        pl_atck_data_set_shl(sh, sh->owner, sh->arg + 1, shell09_tbl);
    }
    shell09_bomber(sh);
    sh->x88 = shell09_body_tbl[sh->body];
    sh->x8C = 0;
    sh->char0 = 0;
    sh->prim = 0;
}

static void shell09_m(SHLW *sh) {
    if ((sh->arg == 1 || sh->arg == 4 || sh->arg == 18) && sh->x61 <= 0) {
        shell09_kaze_oto_set(sh);
        sh->x61 = 0;
        sh->mode = 2;
        return;
    }
    if (sh->arg != 19) {
        limit = 40;
    } else {
        limit = 80;
    }
    if (++sh->char0 > limit) {
        sh->x61 = 0;
        sh->mode = 2;
        return;
    }
    if (sh->prim != 0) {
        flvecCopy(&sh->prim->pos, &sh->pos2);
        add_prim(ot1, sh->prim, 0x20, 0);
    }
}

static void shell09_d(SHLW *sh) {
    if ((sh->arg == 1 || sh->arg == 4 || sh->arg == 18) && sh->mode == 3) {
        shell09_kaze_oto_set(sh);
    }
    sh->xB = 0;
    sh->mode = 6;
    sh->be_flag = 0;
    if (sh->prim != 0) {
        release_prim(sh->prim_no);
    }
}
#endif
