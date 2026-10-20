/* SLPM_654.95 0x00163050-0x001631A0: Disp_load_start_sub .. Disp_NowLoading. See disp1_nm.c. */
#include "g_alias.h"
#include "types.h"
#include "game.h"

typedef struct SP2 { s16 a, b; } SP2;
typedef struct PUT_2TF {
    s16 x0, y0, x1, y1;
    s32 col;
    SP2 uv0;
    SP2 uv1;
} PUT_2TF;

extern GAME_W game_w;
extern s16 load_char_tbl[10][2];
extern char quest_title[];

void reload_tex(int, int);
void SetTextureStage(int);
void flps0008(void *);
void Put_2TF(PUT_2TF *);
void flFlip(int);
void font_stack_reset(void);
void font_draw_stack_no(int);
void flfntSetSize(int, int);
void font_print_double(int, int, int, int, char *);
int strlen(const char *);
u16 *Start_item_data_adrs_get(void);
#ifdef MHG_PS2
/* G (main 0x12D600, 0x12D690; the PC's G version is disp1_nm.c): the loading screen is a request (load_req) */
void disp_load_msg(void);
int func_001D5970();
void func_0012D450(int);
extern int D_002EF26C;      /* load_req */
extern char *D_002EF27C;    /* load_req_msg */
#endif







#ifdef MHG_PS2
#if !defined(G_PART) || G_PART == 1
void Disp_load_start_sub(int arg) {
    disp_load_msg();
    if ((u16)arg != 0) {
        flfntSetSize(0x18, 0x18);
        font_print_double((s16)((0x280 - func_001D5970(quest_title) * 12) / 2), 200, 1, 0, quest_title);
    }
    font_draw_stack_no(0);
}
#endif

#if !defined(G_PART) || G_PART == 1
void Disp_load_start(int arg, int mode) {
    if (mode == 1) {
        font_stack_reset();
        Disp_load_start_sub(arg);
        func_0012D450(0);
    } else {
        D_002EF26C = 1;
        D_002EF27C = quest_title;
    }
}
#endif
#else
void Disp_load_start_sub(int arg) {
    disp_load_spr();
    if ((u16)arg != 0) {
        flfntSetSize(0x18, 0x18);
        font_print_double((s16)((0x280u - strlen(quest_title) * 12) >> 1), 200, 1, 0, quest_title);
    }
    font_draw_stack_no(0);
}

void Disp_load_start(int arg) {
    s16 i;

    for (i = 0; i < 4; i++) {
        flFlip(0);
        font_stack_reset();
        Disp_load_start_sub(arg);
        disp_load_msg();
    }
}
#endif

#if !defined(G_PART)
void Disp_NowLoading(void) {
    s16 i;

    for (i = 0; i < 4; i++) {
        flFlip(0);
        disp_load_spr();
        disp_load_msg();
    }
}
#endif
