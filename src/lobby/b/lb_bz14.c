/* lb_bz14 - lobby UI/client 0x005B3840-0x005B3894: lm_logout_i (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
#include "lbsys_g.h"

s32 lm_logout_i(void) {
    if (F(u8, (u8 *)cw, CW_OFS(0x35D3)) == 1) {
        Lb_put_set01(1);
        return 1;
    }
#ifdef MHG_PS2
    SetDialogData(0x35);   /* G: one argument, and clears 0x2FF2A0 */
    SetDialogYesNo(1);
    *(u8 *)0x2FF2A0 = 0;
#else
    SetDialogData(0x27, 2);
    SetDialogYesNo(1);
#endif
    return 0;
}
