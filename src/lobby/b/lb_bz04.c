/* lb_bz04 - lobby UI/client 0x005B2320-0x005B2328: cnWrap_PushWork (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
void push_set_work();
#ifdef MHG_PS2
void push_em_work();
#endif

void cnWrap_PushWork(void) {
    GV(push_set_work, push_em_work)();
}
