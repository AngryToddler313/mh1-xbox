/* lb_i03 - lobby move dispatch 0x005D00A0-0x005D0120: lb_pl_mv000. Whole file in lb_i.c. */
#include "g_alias.h"
#include "lobby_f.h"
#ifdef MHG_PS2
void pl_chr_set2();
void cnLbc_DispNetConnectTime();
#endif
extern u8 D_3E4ECC[];






void lb_pl_mv000(PLW *pl) {
    if (PLU8(pl, 0x8EC) != 0) {
        Lb_act_set(pl, 0, 0x55);
        return;
    }
#ifdef MHG_PS2
    /* G: after 240 idle frames, unless the idle motion already runs */
    if (pl->work39C >= 0xF0 && pl->work011 == 1 && pl->char0 != 0x3F) {
        pl_chr_set2(pl, 0x3F, 2, 0);
    }
    cnLbc_DispNetConnectTime(pl);   /* the G map's name at 0x5991C0, where G's moves call lb_basic_com_ck */
#else
    if (pl->work011 == 1 && pl->char0 == 1 && F(s32, pl, 0x194) < 2) {
        Lb_pl_chr_set(pl, 0x3F, 2, 0);
    }
    lb_basic_com_ck(pl);
#endif
}
