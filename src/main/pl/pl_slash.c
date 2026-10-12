/* Player code (SLPM_654.95 0x001536D0-0x001537D0): Pl_slash_calc - adds dv to the weapon sharpness (work87E,
 * capped at work884) and queues the sharpness-change message when the level from Pl_slash_lv_ck changes. */
#include "pl.h"
#include "game.h"
#include "plf.h"
extern char lit_1805_0035B1B0[];
extern char lit_1806_0035B1D0[];
extern char lit_1807_0035B1F0[];

void Pl_slash_calc(PLW *pl, int dv) {
    u8 up = 0;
    u8 lv;

    if (Pl_master_ck(pl) == 0) {
        return;
    }
    if (pl->kind == 1 || pl->kind == 5) {
        return;
    }
    pl->work87E = pl->work87E + dv;
    if (pl->work87E <= 0) {
        pl->work87E = 0;
    }
    if (pl->work884 < pl->work87E) {
        pl->work87E = pl->work884;
        up = 1;
    }
    lv = Pl_slash_lv_ck(pl);
    if (lv != pl->work887) {
        if (lv < pl->work887) {
            set01_set2(lit_1805_0035B1B0);
        } else {
            set01_set2(lit_1806_0035B1D0);
        }
        pl->work887 = lv;
    }
    if (up) {
        set01_set2(lit_1807_0035B1F0);
    }
}
