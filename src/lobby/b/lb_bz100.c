/* lb_bz100 - lobby UI/client 0x005B3600-0x005B363C: lm_mail_box_trans (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#ifdef MHG_PS2
void lobby_func_00574FC0();
#endif
#include "lobby_a.h"
void plaza_mailBoxTrans();

void lm_mail_box_trans(void) {
    GV(plaza_mailBoxTrans, lobby_func_00574FC0)(0xD8, 0x38, 1);
    DispSceneSubTitle();
    DispButtonHelp(pNet);
    Put_receive_mark(1);
}
