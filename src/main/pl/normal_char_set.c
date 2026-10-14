/* normal_char_set / to_normal - SLPM_654.95 0x0014F0D0-0x0014F32C: the player's idle character (motion set)
 * by state and stage environment. The `case 0: default:` label pair makes the original reload the constant 1
 * in the default case (a lone `default:` reuses the register of the `case 1` comparison). */
#include "pl.h"
#include "game.h"


s16 Stage_env_ck(u8);
void pl_chr_set();   /* called without a prototype: args are promoted */
int Pl_master_ck(PLW *);
void net_send_pl(PLW *, int, int);

#if !defined(G_PART) || G_PART == 1
void normal_char_set(PLW *pl, int a, int b) {
    u16 c0, c1;

    if (pl->flag604 != 0) {
        c0 = 0x18;
        c1 = 0x7C;
    } else if (pl->flag12 != 0) {
        c0 = 0x3E9;
        c1 = 0x44D;
    } else if (pl->work882 < 0x4C) {
        c0 = 0x193;
        c1 = 0x1F7;
    } else {
        switch (Stage_env_ck(pl->stg)) {
        case 0:
        default:
            c0 = 1;
            c1 = 0x65;
            break;
        case 1:
            c0 = 0x15;
            c1 = 0x79;
            break;
        case 2:
            c0 = 0x1AF;
            c1 = 0x213;
            break;
        }
    }
    pl_chr_set(pl, c0, a, b, 0);
    pl_chr_set(pl, c1, a, b, 1);
}
#endif

/* 0x14F1E0 */
#if !defined(G_PART)
void to_normal(PLW *pl, int blend, int tm) {
    if (pl->flag604 != 0) {
        pl->char0 = 0x18;
        pl->char1 = 0x7C;
    } else if (pl->flag12 != 0) {
        pl->char0 = 0x3E9;
        pl->char1 = 0x44D;
    } else if (pl->work882 < 0x4C) {
        pl->char0 = 0x193;
        pl->char1 = 0x1F7;
    } else {
        switch (Stage_env_ck(pl->stg)) {
        case 0:
        default:
            pl->char0 = 1;
            pl->char1 = 0x65;
            break;
        case 1:
            pl->char0 = 0x15;
            pl->char1 = 0x79;
            break;
        case 2:
            pl->char0 = 0x1AF;
            pl->char1 = 0x213;
            break;
        }
    }
    pl->blend0 = blend / 2;
    pl->blend1 = blend / 2;
    pl->act_tm0 = tm;
    pl->act_tm1 = tm;
    pl->flag14 = 0;
    pl->flag15 = 0;
    if (Pl_master_ck(pl) == 1) {
        net_send_pl(pl, 1, 0);
    }
}
#endif
