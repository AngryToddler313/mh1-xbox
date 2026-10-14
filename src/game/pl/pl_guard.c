/* pl_guard_ck - game.bin 0x0063A010-0x0063A0E4. Can the player guard right
 * now: enough stamina (>= 0x4B), a guard-capable weapon kind, the guard
 * action flag set, and the hit coming from the front (Guard_dir_ck). */
#include "pl.h"

int pl_flag_ck(PLW *, int);
int Guard_dir_ck(u16 ang, u16 dm_ang);

/* G: original bytes (build/mhg/raw/pl_guard_ck.inc, config/mhg/c_rawfuncs.txt); the C below is the MH1 version */
#if defined(VERSION_MHG) && defined(__MWERKS__)
asm u8 pl_guard_ck(PLW *pl)
{
#include "pl_guard_ck.inc"
}
#else
u8 pl_guard_ck(PLW *pl) {
    u8 ret = 0;

    if (pl->stamina < 0x4B) {
        return 0;
    }
    switch (pl->kind) {
    case 0:
    case 4:
    case 3:
        if (pl->flag12 != 0) {
            switch (pl->st) {
            case 0:
            case 1:
                if (pl_flag_ck(pl, 0x8000) != 0 && Guard_dir_ck(pl->ang[1], pl->dm_ang) == 1) {
                    ret = 1;
                }
                break;
            }
        }
        break;
    }
    return ret;
}
#endif
