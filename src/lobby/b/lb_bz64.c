/* lb_bz64 - lobby UI/client 0x005C4E20-0x005C4E60: lobby_visual, lobby_to_quest (first drafted by tools/lbauto.py). */
#include "lobby_f.h"

#if !defined(G_PART)
s32 lobby_visual(void) {
    Visual_main();
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
s32 lobby_to_quest(void) {
    all_reset();
    return 1;
}
#endif
