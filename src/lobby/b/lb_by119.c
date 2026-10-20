/* lb_by119 - agent B 0x005B3DA0-0x005B3E1C: Lb_ck_menu (village start menu opened from the walking loop). */
#include "g_alias.h"
#include "lobby_s.h"
extern u8 *lbmw;
void Lb_ck_menu(void) {
    if (lb_sys.x68 != GV(0xF, 0x10)) {
        lb_sys.x68 = GV(0xF, 0x10);
        *(s8 *)GV(0x3F33FE, 0x35ED2E) = 1;
        Lbc_init_network_work();
#ifdef MHG_PS2
        Lbc_set_prim(0, 0, 0, 0);
#else
        Lbc_set_prim(0, 0, 0);
#endif
        lb_menu_init();
        F(s8, lbmw, 5) = 1;
        F(s16, lbmw, 6) = 0;
        se_req(7, 0x11, 0);
    }
}
