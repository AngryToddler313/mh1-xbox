/* lb_bz99 - lobby UI/client 0x005B3520-0x005B355C: lm_friend_list_trans (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_a.h"
void plaza_checkFriendTrans();
void plaza_mailBoxTrans();

void lm_friend_list_trans(void) {
    GV(plaza_checkFriendTrans, plaza_mailBoxTrans)(0xD8, 0x38, 1);
    DispSceneSubTitle();
    DispButtonHelp(pNet);
    Put_receive_mark(1);
}
