/* lb_by54 - agent B promoted near-match 0x00538A00-0x00538A68: lb_process_tag_decide00 (first drafted by tools/lbauto.py). */
#include "lobby_s.h"
#ifdef VERSION_MHG
extern s32 shop_default_tag_00610A68[];
#define GTAG(x) ((s32 *)(shop_default_tag_00610A68 + 1))
#else
#define GTAG(x) (x)
#endif
extern s32 armorIndex;
extern s32 shop_process01_tag[3];

void lb_process_tag_decide00(void) {
    if (lbShop.mode == 0) {
        lbShop.tag = GTAG(shop_process01_tag);
        lbShop.x17 = 2;
    } else {
        lbShop.tag = GTAG(shop_process01_tag + 2);
        lbShop.x17 = 5;
    }
    lbShop.x70 = 0;
    armorIndex = 0;
    lbShop.f40 = 0;
}
