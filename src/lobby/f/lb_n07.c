/* lb_n07 - lobby senders 0x005CA9C0-0x005CAAE4: Lb_check_existF, Lb_guild_check_requireF. Whole file in lb_n.c. */
#include "g_alias.h"
#include "lobby_f.h"













extern u8 D_3E55F0[], D_3E5FF0[], D_3E69F0[], D_3E73F0[], D_3E7DF0[], D_3E87F0[], D_3E91F0[];


#if !defined(G_PART) || G_PART == 1
int Lb_check_existF(void) {
    if (Quest_clear_bit_ck(GV(0x67, 3004)) == 1) {
        return 0;
    }
    if (Quest_clear_bit_ck(GV(0x68, 3005)) == 1) {
        return 0;
    }
    if (Quest_clear_bit_ck(GV(0x69, 3006)) == 1) {
        return 0;
    }
    if (Quest_clear_bit_ck(GV(0x6A, 3007)) != 1) {
        return 1;
    }
    return 0;
}
#endif

#if !defined(G_PART)
int Lb_guild_check_requireF(void) {
    if (*(u8 *)0x3C733B < 0x14) {
        return 0;
    }
    if (Quest_clear_bit_ck(0x6B) == 0) {
        return 0;
    }
    if (Event_flag_ck(0x4E) == 0) {
        return 0;
    }
    if (*(u8 *)0x3C741C < 0x32) {
        return 0;
    }
    if (*(u8 *)0x3C741D >= 0x32) {
        return 1;
    }
    return 0;
}
#endif
