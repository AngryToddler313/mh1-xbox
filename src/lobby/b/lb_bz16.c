/* lb_bz16 - lobby UI/client 0x005B3E20-0x005B3ECC: lb_menu_init, Lb_menu_exit, Lb_menu_move (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
extern u8 * lbmw;

#if !defined(G_PART)
void lb_menu_init(void) {
    F(s32, lbmw, 0) = 0;
    *(s8 *)GV(0x39DAD0, 0x2FF2A0) = 1;
    *(s8 *)GV(0x39DAD1, 0x2FF2A1) = 4;
    *(s16 *)GV(0x39DAD2, 0x2FF2A2) = (s16) F(u8, lbmw, 4);
    *(s8 *)GV(0x39DADC, 0x2FF2AC) = 0;
}
#endif

#if !defined(G_PART) || G_PART == 2
void Lb_menu_exit(void) {
    F(s8, lbmw, 5) = 0;
    F(s32, lbmw, 0) = 0;
    *(s8 *)GV(0x39DAD0, 0x2FF2A0) = 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
s32 Lb_menu_move(void) {
    if (F(u8, lbmw, 5) == 0) {
        Lb_menu_exit();
        return 1;
    }
    F(s16, lbmw, 6) = Get_sw2(0);
    return 0;
}
#endif
