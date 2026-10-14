/* G-only (SLPM_658.69 0x0010DD20): may this hunter use the recipe e? Recipes without flag bit 0 are open to
 * everyone; the others need the mixing skill (id 0xA8) or the mixing manual (item 6). e == 0 asks about no recipe
 * in particular and takes the second test. */
#include "types.h"
#include "itemprep.h"
#include "pl.h"
#include "plf.h"

s16 Pl_item_num_ck(PLW *, int);

int func_0010DD20(PLW *pl, IPREP *e) {
    if (e != 0 && (e->flags & 1) == 0) {
        return 1;
    }
    if (Pl_Skill_ck(pl, 0xA8) == 1) {
        return 1;
    }
    return Pl_item_num_ck(pl, 6) != 0;
}
