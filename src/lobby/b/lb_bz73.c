/* lb_bz73 - lobby UI/client 0x005B2600-0x005B2628: cpn_PutCNData, cnWrap_BgmStop (first drafted by tools/lbauto.py). */
#include "lobby_a.h"
extern char CNFile[];
extern char Friend_data[];

#if !defined(G_PART)
void cpn_PutCNData(void) {
    memcpy(&CNFile, Friend_data, 2400);
}
#endif

#if !defined(G_PART) || G_PART == 1
void cnWrap_BgmStop(void) {
    str_stop(0);
}
#endif
