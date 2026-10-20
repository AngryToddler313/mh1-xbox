/* lb_z102 - auto-drafted 0x0060DBC0-0x0060DC34: Plaza_disp_chat, Plaza_chatlog_i (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"

#if !defined(G_PART) || G_PART == 1
void Plaza_disp_chat(void) {
    SetFilterMode(0);
    plaza_disp_chat_log_sub(0, 0, *(u8 *)GV(0x39DAD4, 0x2FF2A4));
}
#endif

#if !defined(G_PART) || G_PART == 2
void Plaza_chatlog_i(void) {
    *(s8 *)GV(0x39DAE0, 0x2FF2B7) = 0;
    if (Plaza_get_chat_line_num() < 0xAU) {
        *(s8 *)GV(0x39DAE1, 0x2FF2B8) = 1;
        return;
    }
    *(u8 *)GV(0x39DAE1, 0x2FF2B8) = 0;
}
#endif
