/* lb_bz136 - lobby UI/client 0x005B5050-0x005B514C: connecting_20 (first drafted by tools/lbauto.py). */
#include "g_alias.h"
#include "lobby_a.h"
extern s16 Vs_Cnt_1;
extern s8 PPP_ErrorStatus;
extern s8 reset_NG_flag;
extern u8 COM_R_No_2;
extern u8 COM_R_No_5;

s32 connecting_20(void) {
    s32 temp_v0;
    s32 var_s0;

    var_s0 = 0;
    reset_NG_flag = 1;
    if (COM_R_No_5 == 1) {
        Vs_Cnt_1 = (s16) (Vs_Cnt_1 + 1);
        if (Vs_Cnt_1 > 0x78) {
            Vs_Cnt_1 = 0;
        }
        if (Vs_Cnt_1 > 0x3c) {
            Ncm_mssage_disp_req(0x79);
        }
    }
    if ((tk_sw_new_ck(0x8000) != 0) && (COM_R_No_5 == 0)) {
        *(s16 *)GV(0x4E4746, 0x3E31B6) = (s16) (*(s16 *)GV(0x4E4746, 0x3E31B6) + 1);
    } else {
        *(s16 *)GV(0x4E4746, 0x3E31B6) = 0;
    }
    temp_v0 = InetConnectAll();
    switch (temp_v0) {                              /* irregular */
    case 2:
        COM_R_No_2 = (u8) (COM_R_No_2 + 1);
        break;
    case 4:
        var_s0 = -1;
        PPP_ErrorStatus = (s8) *(s8 *)GV(0x4E474F, 0x3E31BF);
        break;
    }
    *(s16 *)GV(0x3A6E94, 0x30C0D4) = (s16) *(u8 *)GV(0x4E474D, 0x3E31BD);
    return var_s0;
}
