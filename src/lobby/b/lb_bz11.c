/* lb_bz11 - lobby UI/client 0x005B3470-0x005B34C4: lm_friend_list_i (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
extern u8 * pNet;

#ifdef MHG_PS2
extern char *lm_intro_title_tbl_g[];
void lobby_func_0059F6C0();
void lobby_func_0056C480();
/* G: the sub title by the s16 at +0x1A of the pointer at 0x2EEE70, a G-only friend check, and a cleared byte */
s32 lm_friend_list_i(void) {
    Lbc_init_network_work();
    F(s8, pNet, PNET_G(7)) = 3;
    lobby_func_0059F6C0(lm_intro_title_tbl_g[(*(s16 **)0x2EEE70)[13]]);
    SetMessageHaltFlag();
    lobby_func_0056C480();
    *(u8 *)0x2FF2A0 = 0;
    return 0;
}
#else
s32 lm_friend_list_i(void) {
    Lbc_init_network_work();
    F(s8, pNet, 7) = 3;
    SetSceneSubTitle(2, 1, *(s32 *)0x389E78 + 0xDC);
    SetMessageHaltFlag();
    plaza_checkFriend();
    return 0;
}
#endif
