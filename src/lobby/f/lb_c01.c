/* lb_c01 - lobby small helpers 0x005C8750-0x005C8804: lb_rule_seet_trans_ot2, Lb_Matching, get_questLevelNum. Whole file in lb_c.c. */
#include "g_alias.h"
#include "lobby_f.h"
#include "lbsys_g.h"













#if !defined(G_PART) || G_PART == 1
void lb_rule_seet_trans_ot2(s32 *p) {
    if (SoftKeyboard_alive_check() != 0) {
        font_set_stack_no(p[6]);
        DispSoftkeyboard(1);
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
void Lb_Matching(void) {
    u8 m = CW8(GV(CW_OFS(0x32C5), 0xF751));
    if (m == 1) {
        Lbs_MatchStart(m);
    }
}
#endif

#if !defined(G_PART)
int get_questLevelNum(void) {
    if (Online_ck() == 1) {
        return 6;
    }
    return 5;
}
#endif
