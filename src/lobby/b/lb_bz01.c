/* lb_bz01 - lobby UI/client 0x005AF260-0x005AF2B8: Lb_put_itemRare (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_a.h"
extern char lit_333_0065E1E0[];

void Lb_put_itemRare(int arg0, int arg1, int arg2) {
    flfntLocate();
    flfntSetSize(0x12, 0x12);
    font_set_palette(Equip_moji_color_rare(arg2 & 0xFF));
#ifdef MHG_PS2
    font_print(&lit_333_0065E1E0, *(char **)0x6364FC, ((s8)arg2) + 1);   /* G: the format takes a string first */
#else
    font_print(&lit_333_0065E1E0, ((s8)arg2) + 1);
#endif
}
