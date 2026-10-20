/* lb_j03 - lobby move handlers 0x005D15B0-0x005D1820: lb_pl_mv063, lb_pl_mv064, lb_pl_mv065. Whole file in lb_j.c. */
#include "g_alias.h"
#include "lobby_f.h"
#ifdef MHG_PS2
void lobby_func_005975E0();
void cnLbc_DispNetConnectTime();   /* the G map's name at 0x5991C0, where G's moves call lb_basic_com_ck */
#endif






void pl_sleeping();











#if !defined(G_PART) || G_PART == 1
void lb_pl_mv063(PLW *pl) {
    u8 s;
    if (PLU8(pl, 0x8EC) != 0) {
        Lb_act_set(pl, 0, 0x1F);
        return;
    }
    s = pl->x05;
    switch (s) {
    case 0:
        pl->x05 = s + 1;
        pl->work39C = 0;
        pl->ang[1] = *(u16 *)&pl->ang_y;
        Lb_pl_chr_set(pl, 0x2A, 0, 0);
        Lb_Pl_basic_flagset(pl, 0, 0, 0);
        Lb_pl_flag_set(pl, 0x1000);
        return;
    case 1:
        if (F(s32, pl, 0x194) == 0) {
            Lb_Pl_act_set2(pl, 0, 0x1F, 0);
        }
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 3
void lb_pl_mv064(PLW *pl) {
    u8 s;
    s = pl->x05;
    switch (s) {
    case 0:
        if (pl->char0 == 0x2A6) {
            pl->x05 = 2;
            return;
        }
        pl->x05 = s + 1;
        pl->work39C = 0;
        Lb_Pl_basic_flagset(pl, 0, 0, 0);
        Lb_pl_chr_set(pl, 0x2A5, 0x14, 0x1E);
        return;
    case 1:
        if (F(s32, pl, 0x194) <= 0) {
            pl->x05 = s + 1;
            Lb_pl_chr_set(pl, 0x2A6, 6, 0);
        }
#ifdef MHG_PS2
        lobby_func_005975E0(pl);   /* G only */
#endif
        break;
    case 2:
        if (Pl_master_ck(pl) == 1) {
            if (pl->sw.pow[0] >= 0x28) {
                Lb_act_set(pl, 0, 0x41);
                return;
            }
            GV(lb_basic_com_ck, cnLbc_DispNetConnectTime)(pl);
        }
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
void lb_pl_mv065(PLW *pl) {
    u8 s;
    s = pl->x05;
    switch (s) {
    case 0:
        pl->x05 = s + 1;
        pl->work39C = 0;
        Lb_Pl_basic_flagset(pl, 0, 0, 0);
        Lb_pl_chr_set(pl, GV(0x2A7, 0x262), GV(6, 4), 0);
        return;
    case 1:
        if (F(s32, pl, 0x194) <= 0) {
            Lb_pl_to_normal(pl, 0, GV(6, 4), 0);
        }
        break;
    }
}
#endif
