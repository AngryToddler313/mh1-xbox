/* lb_c03 - lobby small helpers 0x005CB360-0x005CB384: Lb_menu_quest_info. Whole file in lb_c.c. */
#include "g_alias.h"
#include "lobby_f.h"













void Lb_menu_quest_info(s32 *p) {
    *(s32 **)GV(0x3C74D4, 0x32D0A4) = p;
    *(s32 *)GV(0x3C7450, 0x32D020) = p[4];
    *(s32 *)GV(0x3C7454, 0x32D024) = p[2];
}
