/* SLPM_654.95 0x00226900-0x00226914: Stage_data_get .. Stage_data_get. */
#include "quest.h"

extern u8 *mission_area;
extern s32 *quest_data_tbl[];
extern s32 Item_get_tbl[];
void *memset(void *, int, unsigned int);




#define QOFS(off) ((off) != 0 ? (void *)((off) + (int)mission_area) : 0)










/* 0x226900: stage n's 32-byte entry */
#ifdef VERSION_MHG
/* G: with a quest loaded the stage number is remapped first (see qstb05.c) */
int sub_main_func_004A1970(int, int);
void *Stage_data_get(int n)
{
    u8 *p = (u8 *)quest_w.x80;
    int r;
    if (quest_w.no == 0) {
        p += n * 32;
    } else {
        r = (u16)sub_main_func_004A1970(game_w.x2E, (u8)n);
        if (r != 0xFFFF) {
            p += r * 32;
        }
    }
    return p;
}
#else
void *Stage_data_get(int n)
{
    return (u8 *)quest_w.x80 + n * 32;
}
#endif
