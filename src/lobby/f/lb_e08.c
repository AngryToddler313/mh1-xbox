/* lb_e08 - lobby members/cockpit/icons 0x005CD090-0x005CD0E8: Lb_check_pl_load. Whole file in lb_e.c. */
#include "g_alias.h"
#include "lobby_f.h"













int Lb_check_pl_load(int id) {
    s8 i = id;
    if (player_work[i].be_flag != 0 && ((s8 *)(i + (int)cw))[GV(0x2BFE, CW_OFS(0x2BFE))] == 0) {
        return 0;
    }
    return 1;
}
