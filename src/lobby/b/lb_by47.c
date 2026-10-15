/* lb_by47 - agent B promoted near-match 0x00538620-0x0053863C: lb_process_init (first drafted by tools/lbauto.py). */
#include "lobby_s.h"
#ifdef VERSION_MHG
extern s32 shop_default_tag_00610A68[];
#define GTAG(x) ((s32 *)(shop_default_tag_00610A68 + 1))
#else
#define GTAG(x) (x)
#endif
extern s32 shop_process00_tag[1];

void lb_process_init(void) {
    lbShop.tag = GTAG(shop_process00_tag);
    lbShop.x17 = 2;
}
