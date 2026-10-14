/* lb_by43 - agent B promoted near-match 0x00537BA0-0x00537C0C: Lb_shop_tag_init (first drafted by tools/lbauto.py). */
#include "lobby_f.h"
#include "lbsys_g.h"

void Lb_shop_tag_init(void) {
    F(s16, &lbShop, LBSHOP_OFS(0)) = 0x1AE;
    F(s16, &lbShop, LBSHOP_OFS(2)) = 0x46;
    F(s16, &lbShop, LBSHOP_OFS(6)) = 0x6A;
    F(s16, &lbShop, LBSHOP_OFS(0xA)) = 0x8E;
    F(s16, &lbShop, LBSHOP_OFS(0xE)) = 0xB2;
    F(s16, &lbShop, LBSHOP_OFS(0x12)) = 0xD6;
    F(s16, &lbShop, LBSHOP_OFS(4)) = 0x1AE;
    F(s16, &lbShop, LBSHOP_OFS(8)) = 0x1AE;
    F(s16, &lbShop, LBSHOP_OFS(0xC)) = 0x1AE;
    F(s16, &lbShop, LBSHOP_OFS(0x10)) = 0x1AE;
}
