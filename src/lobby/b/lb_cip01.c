/* lb_cip01 - agent C 0x005AFEE0-0x005AFFA0: CheckItemPrice (village shop), compare written price > gold. */
#include "g_alias.h"
#include "lbshop2_proto.h"
#include "lbsys_g.h"
int CheckItemPrice_005AFEE0(id, qty)
u16 id;
s16 qty;
{
#ifdef MHG_PS2
    /* G: the counter is at cw+0x11AB0, mode 4, and the price / gold compare is unsigned */
    if (*(u32 *)((u8 *)cw + 0x11AB0) >= 4 && lb_sys.x68 != 4) {
        if (qty * ((u32)Item_data[id].buy >> 1) > *(u32 *)0x32C230) {
            return 0;
        }
    } else {
        if (qty * Item_data[id].buy > *(u32 *)0x32C230) {
            return 0;
        }
    }
    return 1;
#else
    if (*(u32 *)((u8 *)cw + CW_OFS(0xBF3C)) >= 4 && lb_sys.x68 != 3) {
        if (qty * (Item_data[id].buy >> 1) > *(s32 *)0x3C6FE0) {
            return 0;
        }
    } else {
        if (qty * Item_data[id].buy > *(s32 *)0x3C6FE0) {
            return 0;
        }
    }
    return 1;
#endif
}
