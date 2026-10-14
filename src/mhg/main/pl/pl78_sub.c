/* G copy of pl78.c (Pl_view_reset): in G the file is split across main and sub_main, so each part is its own file. */
/* Player code (SLPM_654.95 0x00154E90-0x00154F14): Pl_view_reset, init_move_work, clr_move_work */
#include "pl.h"
#include "game.h"
#include "plf.h"
void init_set_work();
void init_eft_work();
void init_shell_work();
void init_item_work();
void clr_set_work();
void clr_eft_work();
void clr_shell_work();
void clr_item_work();
void clr_used_heap(int, int);
f32 flSqrt(f32);
extern u8 Gun_data[26][0x14];
s16 Pl_item_num_ck(PLW *, int);
void adx_se_set(PLW *, int);
void init_set_work();
void init_eft_work();
void init_shell_work();
void init_item_work();
void clr_set_work();
void clr_eft_work();
void clr_shell_work();
void clr_item_work();
void clr_used_heap(int, int);

void Pl_view_reset(PLW *pl) {
    pl->pch_on = 0;
    pl->x763 = 0;
    pl->x8EE = 0;
}

