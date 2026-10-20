/* lb_e09 - lobby member status 0x005CB780-0x005CB82C: Lb_PlStatusSet. Whole file in lb_e.c. */
#include "g_alias.h"
#include "lobby_f.h"

void Lb_PlStatusSet(int id)
{
    s16 *new_var;
    s16 *mini;
    u8 *pl;
    int uid;
    s16 a2;
    s16 new_var2;
    s16 a1;
    mini = (s16 *)GetAdrsMiniData();
    uid = id & 0xFF;
    a2 = mini[GV(4, 5)];
    a1 = mini[GV(5, 6)];
    pl = (u8 *)(&player_work[uid]);
    new_var2 = mini[GV(6, 7)];
    *((s16 *)(pl + 0x35E)) = a2;
    *((s16 *)(pl + 0x360)) = a1;
    new_var = (s16 *)(pl + 0x362);
    *new_var = new_var2;
    pl[0x352] = ((u8 *)mini)[GV(0xE, 3)];
    pl[0x353] = ((u8 *)mini)[GV(0xF, 4)];
    pl[0x354] = ((u8 *)mini)[GV(0x10, 5)];
    pl[0x355] = ((u8 *)mini)[GV(0x11, 6)];
    pl[0x356] = ((u8 *)mini)[GV(0x12, 7)];
    pl[0x357] = ((u8 *)mini)[GV(0x13, 8)];
    Skill_set_PL(pl, a1, a2, uid * 0xA00);
    Lb_get_comment((u8 *)&lb_player[uid] + 0x24);
}
