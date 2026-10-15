/* lb_by44 - agent B promoted near-match 0x00538320-0x005383CC: Lb_put_shopYesNo (first drafted by tools/lbauto.py). */
#include "lobby_a.h"
#include "lbsys_g.h"
extern char lit_869_00654E00[];
extern char lit_870_00654E10[];
extern char lit_871_00654E28[];
extern char lit_872_00654E40[];

#ifdef VERSION_MHG
/* G (0x549FB0): the yes/no line is a message table (main 0x2EC5D8 = yes_or_no, one string per answer state, with ~C colour codes)
 * indexed by the shop's answer byte (MH1's lbShop+0x78), printed by G's func_0012BDD0 (= font_print_sp on the PC). The first
 * port read it through D_2EC5D8, which the PC takes as an MH1 address: the line printed nothing (15 Oct 2026) */
extern char *yes_or_no[];
extern void font_print_sp(char *s, ...);
void Lb_put_shopYesNo(void) {
    flfntSetSize(0x12, 0x12);
    flfntLocate(0x1B0, 0x194);
    font_print_sp(yes_or_no[F(s8, &lbShop, LBSHOP_OFS(0x78)) & 1]);
}
#else
void Lb_put_shopYesNo(void) {
    if (F(s8, &lbShop, LBSHOP_OFS(0x78)) == 0) {
        flfntSetSize(0x12, 0x12);
        flfntLocate(0x1B0, 0x194);
        font_set_palette(2);
        font_print(&lit_869_00654E00);
        font_set_palette(0);
        font_print(&lit_870_00654E10);
        return;
    }
    flfntSetSize(0x12, 0x12);
    flfntLocate(0x1B0, 0x194);
    font_set_palette(0);
    font_print(&lit_871_00654E28);
    font_set_palette(2);
    font_print(&lit_872_00654E40);
}
#endif
