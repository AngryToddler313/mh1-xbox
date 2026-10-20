/* em_cmd_r33 - monster command 0x00561D90-0x00561F74: em_cmd_quest_no_sel (CMD_SEL_FUNC macro of include/em_cmd.h). Whole file in em_cmd_nm.c. */
#include "g_alias.h"
#include "em_cmd.h"

#ifdef MHG_PS2
/* G: quest numbers are u16, two bytes (high first) in the command */
u8 *em_cmd_quest_no_sel(EMW *em, u8 *p) {
    u8 *r;
    u8 n;
    u16 v;
    u16 c;
    s32 i;
    u16 more;

    switch (*p++) {
    case 0:
        n = *p;
        p += 3;
        for (i = 0; i < n; i++) {
            v = (u16)(p[0] << 8) | p[1];
            c = *(u16 *)0x32D018;
            p += 2;
            if (c == v) {
                break;
            }
            if (v < c) {
                r = cmd_end_search(em, p, 0x57, 3);
                p = r;
                if (r[1] == 2 || r[1] == 3) {
                    p = p + 2;
                    break;
                }
                p = p + 2;
            } else {
                more = 1;
                do {
                    p = cmd_end_search(em, p, 0x57, 3);
                    if ((p[0] == 0x57 && p[1] == 2) ||
                        (p[0] == 0x57 && p[1] == 3)) {
                        more = 0;
                    }
                    p = next_cmd_search(em, p);
                } while (more);
                break;
            }
        }
        break;
    case 1:
        p += 2;
    case 2:
        more = 1;
        do {
            p = cmd_end_search(em, p, 0x57, 3);
            if (p[0] == 0x57 && p[1] == 3) {
                more = 0;
            }
            p = next_cmd_search(em, p);
        } while (more);
        break;
    case 3:
        break;
    }
    return p;
}
#else
CMD_SEL_FUNC(em_cmd_quest_no_sel, 0x57, s16, s16, *(s16 *)0x3C7448)
#endif
