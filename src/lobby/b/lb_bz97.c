/* lb_bz97 - lobby UI/client 0x005C4D90-0x005C4DCC: server_select, lobby_top (first drafted by tools/lbauto.py). */
#include "lbsys_g.h"
#include "lobby_a.h"
typedef struct { s8 x00; s8 x01; } LBS1;

int server_select() {
    ((s8 *)&lb_sys)[LBSYS_OFS(1)]++;
    return 0;
}

int lobby_top() {
    ((s8 *)&lb_sys)[LBSYS_OFS(1)]++;
    return 0;
}
