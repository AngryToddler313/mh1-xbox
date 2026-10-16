/* lb_z133 - auto-drafted 0x005D78C0-0x005D7950: Lb_reset (first drafted by tools/lbauto.py). */
#include "lobby_f.h"
extern char D_3E5468[];

void Lb_reset(void) {
    u8 temp_v1;

    temp_v1 = game_w.stage;
    if (temp_v1 != 0x4C) {
        if (temp_v1 == 0x4D) {
            goto block_3;
        }
    } else {
block_3:
        Info_Initialization();
    }
#if defined(VERSION_MHG) && !defined(__MWERKS__)
    /* G (lobby 0x59CCA0): offline it stops here. The village's NPCs stay in em_work over a stage change (house,
     * farm ...): G's vs_square_init places them only on the first load of a visit (lb_sys x00), so MH1's
     * push_em_work_all here left the village empty after the hunter came out of his house (owner, G on the Xbox,
     * 16 Oct 2026: "no villagers when the village is reached the normal way"; RELEASE_G GB5 bug 3) */
    if (Online_ck() == 0)
        return;
#endif
    push_em_work_all();
    stage_free();
    clr_move_work();
    ot_init();
    prim_init();
    flCompact();
    *(int *)((u8 *)&D_3E5468 + (game_w.master * 0xA00)) = 0;
}
