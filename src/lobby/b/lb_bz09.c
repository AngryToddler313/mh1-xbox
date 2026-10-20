/* lb_bz09 - lobby UI/client 0x005B2C20-0x005B2CA4: cnLbc_LoadNetModel, cnLbc_LoadModelWait, cnWrap_IsBBConnect, cnWrap_GetConnectType, cnWrap_IsAccessMemoryCard, cnLbc_SetIspRestTime, lm_place_i (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_f.h"
#ifdef MHG_PS2
int Lbs_InRoomCheck();
#endif

#if !defined(G_PART) || G_PART == 1
void cnLbc_LoadNetModel(void) {

}
#endif

#if !defined(G_PART) || G_PART == 1
s32 cnLbc_LoadModelWait(void) {
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
s32 cnWrap_IsBBConnect(void) {
    return 1;
}
#endif

#if !defined(G_PART)
s32 cnWrap_GetConnectType(void) {
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 3
s32 cnWrap_IsAccessMemoryCard(void) {
    return *(s8 *)GV(0x3F36CC, 0x35F00C) != 0;
}
#endif

#if !defined(G_PART) || G_PART == 2
void cnLbc_SetIspRestTime(s32 *arg0) {
    *arg0 = 0x1A5E0;
}
#endif

#if !defined(G_PART) || G_PART == 4
s32 lm_place_i(void) {
    return GV(Online_ck, Lbs_InRoomCheck)() != 1;   /* G: not while in a room */
}
#endif
