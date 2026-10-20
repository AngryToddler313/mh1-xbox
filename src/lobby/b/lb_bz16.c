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

#if !defined(G_PART) || G_PART == 3
#ifdef MHG_PS2
/* G only */
int lobby_func_0058E300() {
    if (F(u8, lbmw, 5) == 0) {
        return Get_sw2(0);
    }
    return F(u16, lbmw, 6);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 4
#ifdef MHG_PS2
/* G only */
void Skill_set_PL();
void Menu_mix_i();
void lobby_func_0058E690() {
    Skill_set_PL((u8 *)&player_work + (game_w.master * 0xA00));
    Menu_mix_i();
}
#endif
#endif

#if !defined(G_PART) || G_PART == 5
#ifdef MHG_PS2
/* G only */
void Skill_set_PL();
void Menu_data_i();
void lobby_func_0058E720() {
    Skill_set_PL((u8 *)&player_work + (game_w.master * 0xA00));
    Menu_data_i();
}
#endif
#endif

#if !defined(G_PART) || G_PART == 6
#ifdef MHG_PS2
/* G only */
void func_00224630();
void func_0011C620();
void lobby_func_0058E760() {
    func_00224630((u8 *)&player_work + (game_w.master * 0xA00));
    func_0011C620();
}
#endif
#endif

#if !defined(G_PART) || G_PART == 7
#ifdef MHG_PS2
/* G only */
void Skill_set_PL();
void Menu_equipment_i();
void lobby_func_0058E7A0() {
    Skill_set_PL((u8 *)&player_work + (game_w.master * 0xA00));
    Menu_equipment_i();
}
#endif
#endif

#if !defined(G_PART) || G_PART == 8
#ifdef MHG_PS2
/* G only */
void lobby_func_00590DD0() {
    int i;

    for (i = 0; i < 20; i++) {
        ((u8 *)&em_work)[i * 0xA00 + 1150] = 0;
    }
}
#endif
#endif
