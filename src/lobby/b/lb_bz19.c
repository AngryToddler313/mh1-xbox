/* lb_bz19 - lobby UI/client 0x005B4D30-0x005B4D54: Lb_get_player_id (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"

void *Lb_get_player_id(s32 arg0) {
#ifdef MHG_PS2
    return &lbCommer[arg0 & 0xFF];   /* G: the id is the commer record (0x5C bytes) itself */
#else
    return (u8 *)&lb_player + ((arg0 & 0xFF) * 0x38) + 0x24;
#endif
}
