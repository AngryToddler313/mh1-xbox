/* lb_c05 - lobby small helpers 0x005CCEE0-0x005CCF5C: Lb_put_msg_type2, Lb_put_msg2. Whole file in lb_c.c. */
#include "g_alias.h"
#include "lobby_f.h"













#if !defined(G_PART) || G_PART == 2
#ifdef MHG_PS2
void func_0012BDD0();
extern char *lb_msg_type2_tbl_g[];
/* G: x, y and a message number (78 messages, table at 0x636CF0) */
void Lb_put_msg_type2(int x, int y, s16 n) {
    if (n < 78) {
        flfntLocate(x, y);
        func_0012BDD0(lb_msg_type2_tbl_g[n]);
    }
}
#else
void Lb_put_msg_type2(s16 *p) {
    flfntLocate(p[0], p[1]);
    font_print(lit_429_00664C38, *(s32 *)(p + 2));
}
#endif
#endif

#if !defined(G_PART) || G_PART == 1
void Lb_put_msg2(int a0, int a1, char *msg) {
    flfntLocate();
    font_print(lit_429_00664C38, msg);
    strlen_sp(msg);
}
#endif
