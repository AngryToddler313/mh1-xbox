/* lb_bz78 - lobby UI/client 0x005B35B0-0x005B35FC: lm_mail_box_mv (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_a.h"
int plaza_mailBox();
#ifdef MHG_PS2
int lobby_func_0056C550();   /* G's mail box (the G map's plaza_mailBox sits elsewhere) */
#endif

int lm_mail_box_mv() {
    switch (GV(plaza_mailBox, lobby_func_0056C550)()) {
    case 3:
#ifdef MHG_PS2
        *(u8 *)0x35EFEB = 1;   /* G only */
#endif
        ClearMessageHaltFlag();
        return 0x40;
    default:
        return 0;
    }
}
