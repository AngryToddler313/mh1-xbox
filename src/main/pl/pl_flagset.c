/* Player code (SLPM_654.95 0x0014FB40-0x0014FC14): Pl_basic_flagset - stance (pl->st from the low byte of a),
 * flag 8 from bit 15 of a, flag 1 cleared, flag 2 from c. K&R definition: the original extends a and c itself. */
#include "pl.h"
#include "game.h"
#include "plf.h"

void Pl_basic_flagset(pl, a, b, c)
PLW *pl;
u16 a;
s16 b;
s16 c;
{
    switch (a & 0xFF) {
    case 0:
    default:
        pl->st = 0;
        break;
    case 1:
        pl->st = 1;
        break;
    case 2:
        pl->st = 2;
        break;
    }
    if (a & 0x8000) {
        pl_flag_clr(pl, 8);
    } else {
        pl_flag_set(pl, 8);
    }
    pl_flag_clr(pl, 1);
    if (c == 0) {
        pl_flag_clr(pl, 2);
    } else {
        pl_flag_set(pl, 2);
    }
}
