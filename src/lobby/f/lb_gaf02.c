/* lb_gaf02 - near-match fixes 0x005D8680-0x005D875C: Local_main. Whole file in lb_af.c. */
#include "g_alias.h"
#include "lobby_f.h"
extern PLW player_work[];
extern u8 *pNet;
extern u8 D_32D471[];
extern s32 pl_area_top;
extern u8 lit_277_00665D90[];
extern u8 D_3C6FC8[];
extern u8 my_user_id[8];
extern u8 lit_278_00665D98[];
extern u8 put_back[];
extern void (*vs_square_func_213[])();
extern void (*vs_square_func_227[])();
extern u8 network_work[];
extern u8 my_user_handle[0x10];
void Lb_move_common();
void DemoCameraRequest();
int DemoCameraCheck();
void adx_se_set();
void fade_set();
int Fade_busy_ck();
void Disp_NowLoading();
void str_stop_all();
void Lb_reset();
void Lb_stage_load();
void Lb_npc_set();
void lobby_bgm_set();
void lobby_bgm_set2();
void Event_flag_set();
int Event_flag_ck();
void font_set_stack_no();
void lbc_text_lobby_trans();
void Pit_reset();
void Lbc_set_prim();
int Online_ck();
void all_reset();
void Set_userdata();
void Lb_set_mini_data();
void Lb_make_quest_tbl_local();
void Q_camera_init();
void fade_reset();
void InitRenderState();
void Lb_set_player();
void Lb_load_player_all();
void Lb_send_statusReq();
void Lbc_connect();
void load_pit();
void load_eft();
void load_shadow();
void Quest_init();
void Pit_init();
void eft01_set();
void flSndPortStop();
void FlushCache();
void load_bin_req();
int load_busy_ck();
void flSndPackLoadBG();
int flSndPackLoadStatus();
void str_fadeout();
void McOperationSet();
void str_pause();
int McCardOperation();
void str_volume();
void str_fadein();
void str_fadein_vol();
void Lb_send_commer();
void Lbc_SendMiniData();
int Lbc_getDate();
void Lb_check_newCommer();

#ifdef MHG_PS2
#define LBC_SET_PRIM0() Lbc_set_prim(0, 0, 0, 0)   /* G: a fourth argument */
#else
#define LBC_SET_PRIM0() Lbc_set_prim(0, 0, 0)
#endif
int Local_main(void) {
    pNet[PNET_G(0xC)] = 0;
    pNet[GV(0x11, 0x31)] = 0;
    font_set_stack_no(0);
    vs_square_func_213[lb_sys.x03]();
    if (pNet[GV(0x11, 0x31)] == 0) {
#ifdef MHG_PS2
        lbc_text_lobby_trans();
#else
        lbc_text_lobby_trans(network_work);
#endif
    }
    if (lb_sys.x68 == GV(32, 34)) {
        *(s8 *)GV(0x3F35CC, 0x35ED5F) = 0;
        Pit_reset();
        LBC_SET_PRIM0();
        return 1;
    }
    if (lb_sys.x68 == GV(37, 39)) {
        Pit_reset();
        LBC_SET_PRIM0();
        return -1;
    }
    return 0;
}
