/* qstb05 - 0x002268A0-0x00226900: Stage_mv_data_get (the exits list of stage n). With a quest loaded the table
 * holds offsets into mission_area (0 = none), in free hunts (quest_w.no == 0) pointers. `p += n; *p` (pointer
 * advanced in its register) is what gives the original addu. */
#include "quest.h"

#ifdef VERSION_MHG
/* G: with a quest loaded the stage number goes through sub_main_func_004A1970 (game_w.x2E, n) first;
 * 0xFFFF = no such stage here. */
extern u16 D_00520930[];        /* stages per map */
extern u16 *D_00520A00[];       /* each map's stage numbers */

/* G (sub_main 0x4A1970, stage code; here for the PC until its file is done): the index of stage
 * st in map's list, 0xFFFF when the map has no such stage. G's mission stage tables are per map. */
int sub_main_func_004A1970(int map, int st)
{
    u16 *l;
    s16 i;

    map = (u16)map;
    if (D_00520930[map] > 0) {
        l = D_00520A00[map];
        for (i = 0; i < D_00520930[map]; i++, l++) {
            if (*l == (u8)st) {
                return (u16)i;
            }
        }
    }
    return 0xFFFF;
}

void *Stage_mv_data_get(int n)
{
    s32 v;
    s32 *p;
    int r;
    if (quest_w.no == 0) {
        p = quest_w.x7C;
        p += n;
        return (void *)*p;
    }
    r = (u16)sub_main_func_004A1970(game_w.x2E, (u8)n);
    if (r != 0xFFFF) {
        p = quest_w.x7C;
        p += r;
        v = *p;
        if (v == 0) {
            return 0;
        }
        return (void *)(v + (int)mission_area);
    }
    return 0;
}
#else
void *Stage_mv_data_get(int n)
{
    s32 v;
    s32 *p;
    if (quest_w.no == 0) {
        p = quest_w.x7C;
        p += n;
        return (void *)*p;
    }
    p = quest_w.x7C;
    p += n;
    v = *p;
    if (v == 0) {
        return 0;
    }
    return (void *)(v + (int)mission_area);
}
#endif
