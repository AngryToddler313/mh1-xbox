/* lb_bz60 - lobby UI/client 0x005C26E0-0x005C2754: server_select_sub_00 (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"

void server_select_sub_00(u8 *arg0) {
    Ncm_mmbb_spr_load();
    Ncm_mmbb_spr_create();
    F(u8, arg0, PNET_G(2)) = (u8) (F(u8, arg0, PNET_G(2)) + 1);
    F(s8, arg0, PNET_G(8)) = 0;
#ifdef MHG_PS2
    Lbc_set_prim(0, 0, 0, 0);
    fade_set(2);
    SetSceneTitle(2);    /* G: its own title index, no connect time */
    SetHelpLineMsg(0);
#else
    Lbc_set_prim(0, 0, 0);
    fade_set(2);
    cnLbc_DispNetConnectTime();
    SetSceneTitle(0, 2);
    SetHelpLineMsg(0, 3);
#endif
}
