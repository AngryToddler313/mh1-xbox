/* set13 - SLPM_654.95 0x001569E0-0x00157080 (first matching run of the
 * file 0x001569E0-0x00158F18; set13_m and set13_trans are still asm, the
 * rest is in set13b.c / set13c.c). Sun glare / lens flare: the alpha
 * (SETW+0x30) starts per stage, on some stages from where the master player
 * stands (inside or outside a building); arg picks the kind. Guessed from
 * sun_pos_tbl and the camera maths in set13_m. */
#include "g_alias.h"
#include "set.h"
#include "game.h"
#include "pl.h"
#include "prim.h"

void flvecCopy(f32 *, f32 *);

void set13_move(SETW *sw);
#ifdef MHG_PS2
void game_func_0064EFF0(SETW *sw);
#endif
void set13_i(SETW *sw);
void set13_m(SETW *sw);
void set13_d(SETW *sw);
void set13_e(SETW *sw);
void set13_trans(PRIM *pr);

#if !defined(G_PART) || G_PART == 1
void Set13_set(int arg) {
#if defined(VERSION_MHG) && !defined(__MWERKS__)
    SETW *sw = pull_set_work(arg == 8 ? 1 : 0);    /* G's sub_main Set13_set (0x4E5F00): kind 8 keeps three timers in
                                                     * a work block (sub_main 0x4E5EC0) */
#else
    SETW *sw = pull_set_work(GV(0, 1));
#endif

    if (sw != 0) {
        sw->type = GV(13, 2);
        sw->arg = arg;
#ifdef MHG_PS2
        sw->move = game_func_0064EFF0;      /* G: the game module has its own copy of the dispatcher */
#else
        sw->move = set13_move;
#endif
    }
}
#endif

#ifdef MHG_PS2
SETW *sub_main_func_004E5EC0(void);
#endif
#if !defined(G_PART) || G_PART == 3
void Set13_set2(int arg, f32 *pos, int n) {
    SETW *sw;

#ifdef MHG_PS2
    if ((sw = sub_main_func_004E5EC0()) != 0) {     /* G: its own pull (sub_main 0x4E5EC0) */
#else
    if ((sw = pull_set_work(0)) != 0) {
#endif
        sw->type = 13;
        sw->arg = arg;
        flvecCopy(sw->pos, pos);
        sw->se1 = n;
        sw->move = set13_move;
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
void set13_move(SETW *sw) {
    switch (sw->mode) {
    case 0:
        set13_i(sw);
        break;
    case 1:
        set13_m(sw);
        break;
    case 2:
        set13_d(sw);
        break;
    case 3:
        set13_e(sw);
        break;
    }
}
#endif

#if !defined(G_PART)
#if defined(VERSION_MHG) && !defined(__MWERKS__)
/* G (sub_main 0x4E6050, PC copy from the asm; G's set13_i is still asm on the PS2): stage 7 as 0x39, 0xF full
 * glare, 0x18 back to the default, 0x1D as 0x2D, new 0x59-0x5B; kind 8 clears its three timers; then set13_m. */
void set13_i(SETW *sw) {
    PLW *pl = &player_work[game_w.master];

    sw->mode++;
    sw->be_flag = 1;
    sw->timer = 0;
    sw->cnt = 0;
    switch (game_w.stage) {
    case 0x00:
    case 0x05:
    case 0x0F:
    case 0x12:
    case 0x16:
    case 0x17:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x46:
    case 0x49:
    case 0x4B:
        sw->speed = 1.0f;
        break;
    case 0x07:
    case 0x39:
        if (pl->pos[2] > 100.0f && pl->pos[2] < 10400.0f) {
            sw->speed = 1.0f;
        } else {
            sw->speed = 0.0f;
        }
        break;
    case 0x22:
        sw->speed = 0.1f;
        break;
    case 0x23:
        sw->speed = 0.3f;
        break;
    case 0x33:
        if (pl->pos[0] < 8750.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x34:
        if (pl->pos[0] > 3000.0f && pl->pos[0] < 15000.0f && pl->pos[2] > 3000.0f && pl->pos[2] < 8000.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x37:
        if (pl->pos[0] > 0.0f && pl->pos[0] < 6200.0f && pl->pos[2] > 0.0f && pl->pos[2] < 9300.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x1D:
    case 0x2D:
        if (game_w.info_stop == 1) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 3400.0f && pl->pos[0] < 17200.0f && pl->pos[2] > 3400.0f &&
                   pl->pos[2] < 16600.0f) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 2800.0f && pl->pos[0] < 17800.0f && pl->pos[2] > 2800.0f &&
                   pl->pos[2] < 17800.0f) {
            sw->se0 = 3;
            sw->speed = 0.0f;
        } else {
            sw->se0 = 0;
            sw->speed = 1.0f;
        }
        break;
    case 0x38:
        if (game_w.info_stop == 1) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 11800.0f && pl->pos[0] < 16600.0f && pl->pos[2] > 2800.0f &&
                   pl->pos[2] < 16600.0f) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 100.0f && pl->pos[0] < 17200.0f && pl->pos[2] > 2200.0f &&
                   pl->pos[2] < 17200.0f) {
            sw->se0 = 3;
            sw->speed = 0.0f;
        } else {
            sw->se0 = 0;
            sw->speed = 1.0f;
        }
        break;
    case 0x59:
        if (pl->pos[0] < 8832.0f || pl->pos[2] > 10675.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x5A:
        if (pl->pos[2] > 10500.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x5B:
        if (pl->pos[0] < 8212.0f || pl->pos[2] < 8318.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    default:
        sw->speed = 0.4f;
        break;
    }
    if (sw->arg == 8) {
        u8 *w = sw->u.work;
        *(s16 *)(w + 0x08) = 0;
        *(s16 *)(w + 0x14) = 0;
        *(s16 *)(w + 0x20) = 0;
    }
    sw->work14 = 0;
    sw->prim_no = get_prim();
    if (sw->prim_no != -1) {
        sw->prim = get_prim_ptr(sw->prim_no);
        sw->prim->owner = sw;
        sw->prim->trans = set13_trans;
        set13_m(sw);
    } else {
        push_set_work(sw);
    }
}
#else
void set13_i(SETW *sw) {
    PLW *pl = &player_work[game_w.master];

    sw->mode++;
    sw->be_flag = 1;
    sw->timer = 0;
    sw->cnt = 0;
    switch (game_w.stage) {
    case 0x00:
    case 0x05:
    case 0x12:
    case 0x16:
    case 0x17:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x46:
    case 0x49:
    case 0x4B:
        sw->speed = 1.0f;
        break;
    case 0x18:
    case 0x22:
        sw->speed = 0.1f;
        break;
    case 0x23:
        sw->speed = 0.3f;
        break;
    case 0x33:
        if (pl->pos[0] < 8750.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x34:
        if (pl->pos[0] > 3000.0f && pl->pos[0] < 15000.0f && pl->pos[2] > 3000.0f && pl->pos[2] < 8000.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x37:
        if (pl->pos[0] > 0.0f && pl->pos[0] < 6200.0f && pl->pos[2] > 0.0f && pl->pos[2] < 9300.0f) {
            sw->speed = 0.0f;
        } else {
            sw->speed = 1.0f;
        }
        break;
    case 0x39:
        if (pl->pos[2] > 100.0f && pl->pos[2] < 10400.0f) {
            sw->speed = 1.0f;
        } else {
            sw->speed = 0.0f;
        }
        break;
    case 0x2D:
        if (game_w.info_stop == 1) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 3400.0f && pl->pos[0] < 17200.0f && pl->pos[2] > 3400.0f &&
                   pl->pos[2] < 16600.0f) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 2800.0f && pl->pos[0] < 17800.0f && pl->pos[2] > 2800.0f &&
                   pl->pos[2] < 17800.0f) {
            sw->se0 = 3;
            sw->speed = 0.0f;
        } else {
            sw->se0 = 0;
            sw->speed = 1.0f;
        }
        break;
    case 0x38:
        if (game_w.info_stop == 1) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 11800.0f && pl->pos[0] < 16600.0f && pl->pos[2] > 2800.0f &&
                   pl->pos[2] < 16600.0f) {
            sw->se0 = 2;
            sw->speed = 1.0f;
        } else if (pl->pos[0] > 100.0f && pl->pos[0] < 17200.0f && pl->pos[2] > 2200.0f &&
                   pl->pos[2] < 17200.0f) {
            sw->se0 = 3;
            sw->speed = 0.0f;
        } else {
            sw->se0 = 0;
            sw->speed = 1.0f;
        }
        break;
    default:
        sw->speed = 0.4f;
        break;
    }
    sw->work14 = 0;
    sw->prim_no = get_prim();
    if (sw->prim_no != -1) {
        sw->prim = get_prim_ptr(sw->prim_no);
        sw->prim->owner = sw;
        sw->prim->trans = set13_trans;
    } else {
        push_set_work(sw);
    }
}
#endif
#endif
