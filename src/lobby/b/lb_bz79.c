/* lb_bz79 - lobby UI/client 0x005B37B0-0x005B37FC: lm_introduction_mv (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_a.h"

#ifdef MHG_PS2
/* G: the key can also clear the halt flag (0x8000), and the keys come back */
int lm_introduction_mv(int key) {
    u16 k;

    switch (plaza_setMyComment()) {
    case 3:
        ClearMessageHaltFlag();
        return 0x40;
    }
    k = key;
    if (k & 0x8000) {
        ClearMessageHaltFlag();
    }
    return key;
}
#else
int lm_introduction_mv() {
    switch (plaza_setMyComment()) {
    case 3:
        ClearMessageHaltFlag();
        return 0x40;
    default:
        return 0;
    }
}
#endif
