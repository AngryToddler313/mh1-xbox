/* lb_n05 - lobby senders 0x005D8280-0x005D82CC: Lb_put_set01. Whole file in lb_n.c. */
#include "lobby_f.h"








#if defined(VERSION_MHG) && defined(__MWERKS__)
int Lb_put_set01(int n) {
    if (lb_sys.x72 == 0) {
        set01_set2(lb_set01_msg[n]);
        lb_sys.x72 = 0x5A;
        return 1;
    }
    return 0;
}
#else
void Lb_put_set01(int n) {
    if (lb_sys.x72 == 0) {
        set01_set2(lb_set01_msg[n]);
        lb_sys.x72 = 0x5A;
    }
}
#endif
