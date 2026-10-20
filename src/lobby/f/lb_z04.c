/* lb_z04 - auto-drafted 0x005D7E40-0x005D7F40: Lb_get_lb_rank, Lb_cursorUD (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"

#if !defined(G_PART) || G_PART == 1
s32 Lb_get_lb_rank(void) {
    u8 temp_v0;

    temp_v0 = *(u8 *)GV(0x3C733B, 0x32CE33);
    if (temp_v0 < 5) {
        return 0;
    }
    if (temp_v0 < GV(9, 13)) {
        return 1;
    }
    if (temp_v0 < GV(0xD, 21)) {
        return 2;
    }
    if (temp_v0 < GV(0x11, 27)) {
        return 3;
    }
    return 4;
}
#endif

#if !defined(G_PART)
s32 Lb_cursorUD(s32 arg0, s32 arg1) {
    s32 temp_v1;
    s32 var_s1;

    var_s1 = arg0;
    temp_v1 = Get_sw2(0) & 0xFFFF;
    if (temp_v1 & 0x2000) {
        cnWrap_SoundRequest(1);
        if (var_s1 == 0) {
            var_s1 = arg1 - 1;
        } else {
            var_s1 -= 1;
        }
    } else if (temp_v1 & 0x1000) {
        cnWrap_SoundRequest(1);
        var_s1 += 1;
        if (var_s1 >= arg1) {
            var_s1 = 0;
        }
    }
    return var_s1;
}
#endif
