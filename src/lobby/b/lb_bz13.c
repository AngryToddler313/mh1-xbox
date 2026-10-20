/* lb_bz13 - lobby UI/client 0x005B3640-0x005B37AC: lm_net_status_i, lm_net_status_mv, lm_net_status_trans, lm_introduction_i (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
extern u8 * pNet;
extern char D_3E5505[];
extern u8 lm_menu2_tbl[8];
extern char statData[];

#if !defined(G_PART) || G_PART == 1
s32 lm_net_status_i(void) {
#ifdef MHG_PS2
    u8 *mini = my_user_mini_data;
#endif
    if (Lbs_InRoomCheck() == 1) {
        return 1;
    }
    Lbc_init_network_work();
#ifdef MHG_PS2
    F(u8, pNet, PNET_G(8)) = mini[2];   /* G: the cursor starts on the own status */
#endif
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
#ifdef MHG_PS2
extern u8 lm_net_status_sel_g;
void lobby_func_00582E30();
/* G: the choice goes through a small-data byte and a G-only sender; seven entries, mirrored into a s16 */
s32 lm_net_status_mv(s32 arg0) {
    s32 v = arg0 & 0xFFFF;

    if (v & 0x20) {
        lm_net_status_sel_g = F(u8, pNet, PNET_G(8));
        lobby_func_00582E30();
        cnWrap_SoundRequest(6);
    } else if (v & 0x40) {
        return 0x40;
    } else {
        F(u8, pNet, PNET_G(8)) = Lb_cursorUD(F(u8, pNet, PNET_G(8)), 7);
        *(s16 *)0x2FF2A2 = F(u8, pNet, PNET_G(8)) + 19;
    }
    return arg0;
}
#else
s32 lm_net_status_mv(s32 arg0) {
    s32 temp_v1;
    u8 temp_a0;
    u8 temp_a1;

    temp_v1 = arg0 & 0xFFFF;
    if (temp_v1 & 0x20) {
        temp_a0 = game_w.master;
        temp_a1 = F(u8, pNet, PNET_G(8));
        *((u8 *)&D_3E5505 + (temp_a0 * 0xA00)) = temp_a1;
        Lbc_SendMiniData(temp_a0, temp_a1);
        cnWrap_SoundRequest(6);
        goto block_7;
    }
    if (temp_v1 & 0x40) {
        return 0x40;
    }
    F(u8, pNet, PNET_G(8)) = Lb_cursorUD(F(u8, pNet, PNET_G(8)), 5);
block_7:
    return arg0;
}
#endif
#endif

#if !defined(G_PART)
void lm_net_status_trans(void) {
    DispFrameList(&statData, (*(s32 *)(lm_menu2_tbl + 4)), F(u8, pNet, PNET_G(8)));
}
#endif

#if !defined(G_PART) || G_PART == 2
#ifdef MHG_PS2
extern char *lm_intro_title_tbl_g[];
void lobby_func_0059F6C0();
void plaza_setMyComment();
/* G: the sub title comes from a string table by the s16 at +0xE of the pointer at 0x2EEE74 */
s32 lm_introduction_i(void) {
    Lbc_init_network_work();
    F(s8, pNet, PNET_G(7)) = 8;
    lobby_func_0059F6C0(lm_intro_title_tbl_g[(*(s16 **)0x2EEE74)[7]]);
    SetMessageHaltFlag();
    plaza_setMyComment(0);
    return 0;
}
#else
s32 lm_introduction_i(void) {
    Lbc_init_network_work();
    F(s8, pNet, 7) = 8;
    SetSceneSubTitle(2, 1, *(s32 *)0x389E7C + 0x70);
    SetMessageHaltFlag();
    return 0;
}
#endif
#endif
