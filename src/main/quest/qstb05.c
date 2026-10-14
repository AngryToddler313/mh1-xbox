/* qstb05 - 0x002268A0-0x00226900: Stage_mv_data_get (the exits list of stage n). With a quest loaded the table
 * holds offsets into mission_area (0 = none), in free hunts (quest_w.no == 0) pointers. `p += n; *p` (pointer
 * advanced in its register) is what gives the original addu. */
#include "quest.h"

#ifdef VERSION_MHG
/* G: with a quest loaded the stage number goes through sub_main_func_004A1970 (game_w.x2E, n) first;
 * 0xFFFF = no such stage here. */
int sub_main_func_004A1970(int, int);
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
