/* stage_set - SLPM_654.95 0x0015BBE0-0x0015C210 (first function of the
 * f_stage range; the rest is in f_stage.c, not linked yet).
 * stage_set_set: spawns each stage's set objects (props and effects) when a
 * stage is entered. The set*_set functions live in the game.bin overlay, so
 * main calls them by address (func_XXXXXX; names from config/symbols/game.txt
 * in the comments). Jump table 0x0035B870-0x0035B9A0. */
#include "set.h"
#include "game.h"
#ifdef VERSION_MHG
#include "flow.h"
#endif

void clr_flash(void);
void Set13_set(int arg);
void func_6213F0(void);                 /* set00_set: light shafts */
void func_61ED10(void);                 /* Set03_set */
void func_61F2C0(void);                 /* Set04_set */
void func_61F7E0(int stage);            /* Set05_set */
void func_6206D0(void);                 /* set07_set */
void func_620B70(void);                 /* set08_set */
void func_618CA0(void);                 /* set09_set */
void func_621C70(void);                 /* Set10_set */
void func_622180(void);                 /* set11_set */
void func_6229B0(void);                 /* set14_set: UV-scrolled water */
void func_624050(void);                 /* set15_set */
void func_625120(void);                 /* set16_set */
void func_625630(void);                 /* set17_set */
void func_625BB0(void);                 /* Set18_set */
void func_6263B0(void);                 /* Set19_set */
void func_626E70(int kind);             /* Set20_set */
void func_627840(void);                 /* Set22_set */
void func_633B50(f32 *pos, int a, int stage, int b);  /* Shell10_set */
void func_54B8C0(f32 *pos, int kind);   /* Eft14_set2 */

#ifndef VERSION_MHG
void stage_set_set(int stage) {
    f32 pos[3];
    s16 i;

    clr_flash();
    switch ((u8)stage) {
    case 0:
    case 0x1A:
        func_625120();
        func_620B70();
        func_6229B0();
        func_624050();
        break;
    case 1:
        func_6229B0();
        func_625630();
        break;
    case 2:
        func_625630();
        break;
    case 3:
        func_6229B0();
        func_625630();
        break;
    case 4:
        func_6213F0();
        Set13_set(0);
        func_6229B0();
        break;
    case 5:
        Set13_set(0);
        func_618CA0();
        break;
    case 6:
    case 7:
        func_620B70();
        func_625120();
        break;
    case 0xA:
        func_621C70();
        break;
    case 0xB:
        Set13_set(3);
        func_6263B0();
        func_61F7E0(game_w.stage);
        break;
    case 0xC:
        func_61F7E0(stage);
        Set13_set(3);
        func_6263B0();
        break;
    case 0xE:
        func_61F2C0();
        Set13_set(3);
        break;
    case 0x10:
        func_618CA0();
        break;
    case 0x11:
        for (i = 0; i < 10; i++) {
            pos[0] = 1400.0f + 300.0f * (f32)i;
            pos[1] = 30.0f;
            pos[2] = 2000.0f + 300.0f * (f32)i;
            func_633B50(pos, 0, game_w.stage, 0);
        }
        break;
    case 0x12:
    case 0x16:
    case 0x17:
        Set13_set(0);
        func_625BB0();
        break;
    case 0x13:
        Set13_set(0);
        break;
    case 0x14:
        func_626E70(0);
        break;
    case 0x15:
        pos[0] = 11250.0f;
        pos[1] = 40.0f;
        pos[2] = 9500.0f;
        func_54B8C0(pos, 2);
        func_54B8C0(pos, 3);
        Set13_set(4);
        func_6263B0();
        break;
    case 0x19:
        func_61F7E0(stage);
        func_626E70(1);
        func_627840();
        break;
    case 0x1C:
        func_61F7E0(stage);
        func_61F2C0();
        Set13_set(3);
        func_6263B0();
        func_622180();
        break;
    case 0x1E:
        func_6263B0();
        Set13_set(3);
        func_61F7E0(game_w.stage);
        break;
    case 0x1F:
        Set13_set(3);
        break;
    case 0x20:
        func_618CA0();
        Set13_set(0);
        break;
    case 0x1B:
    case 0x21:
        func_618CA0();
        Set13_set(0);
        break;
    case 0x18:
    case 0x22:
        Set13_set(1);
        func_6263B0();
        break;
    case 0x23:
        Set13_set(6);
        break;
    case 0x24:
        func_618CA0();
        break;
    case 0x25:
        Set13_set(0);
        break;
    case 0x26:
        Set13_set(0);
        break;
    case 0x1D:
    case 0x27:
        Set13_set(0);
        break;
    case 0x28:
        Set13_set(0);
        func_618CA0();
        break;
    case 0x29:
        func_6213F0();
        func_618CA0();
        break;
    case 0x2A:
        func_6213F0();
        func_6206D0();
        Set13_set(0);
        func_6229B0();
        break;
    case 0x2B:
        func_6213F0();
        break;
    case 0x2D:
        Set13_set(7);
        break;
    case 0x2E:
        func_625630();
        break;
    case 0x2F:
        func_61ED10();
        Set13_set(0);
        break;
    case 0x30:
        func_61ED10();
        Set13_set(0);
        func_6263B0();
        break;
    case 0x31:
        func_61ED10();
        Set13_set(0);
        break;
    case 0x32:
        Set13_set(0);
        break;
    case 0x33:
        Set13_set(0);
        func_618CA0();
        func_6229B0();
        break;
    case 0x34:
        Set13_set(0);
        func_6229B0();
        func_6263B0();
        func_618CA0();
        break;
    case 0x35:
        Set13_set(0);
        func_6229B0();
        func_618CA0();
        break;
    case 0x36:
        func_6263B0();
        break;
    case 0x37:
        Set13_set(0);
        func_618CA0();
        break;
    case 0x38:
        Set13_set(7);
        break;
    case 0x39:
        Set13_set(0);
        func_6263B0();
        break;
    case 0x3D:
        pos[0] = 3600.0f;
        pos[2] = 6600.0f;
        pos[1] = 0.0f;
        func_54B8C0(pos, 2);
        func_54B8C0(pos, 3);
        Set13_set(0);
        func_6263B0();
        break;
    case 0x3E:
        Set13_set(0);
        break;
    case 0x43:
        pos[0] = 10296.0f;
        pos[2] = 9429.0f;
        pos[1] = 0.0f;
        func_54B8C0(pos, 2);
        func_54B8C0(pos, 3);
        func_6263B0();
        break;
    case 0x46:
        Set13_set(6);
        func_625BB0();
        break;
    case 0x49:
    case 0x4B:
        Set13_set(6);
        func_6263B0();
        break;
    }
}
#else
/* G (sub_main 0x4A0E50, matches G): first the stage's sun data for stage_spr_disp into stage_work+0xC8..0xF4 (sun
 * position, the gradient's lines, 8 colour words) unless the stage file's environment block (version 2) gave them;
 * then each stage's set objects. Against MH1: stage 7 gets only the sun (set13 0) instead of set08 + set16, stage 0x18
 * only Set19, 0x1B only set09, 0x1D G's game-side copy of Set13_set (game 0x64EFA0: type 2 with its own dispatcher
 * game 0x64EFF0, random prims and a clay; not ported, the PC skips it) and set13 kind 7; new stages 0xF (set13 kind 8
 * + Set19), 0x4A (Set19), 0x4C / 0x4D (Set06 0-3 / 0-1: the town's and the tavern's candles), 0x59-0x5B.
 * Set13_main is the sub_main Set13_set (0x4E5F00, type 13), the PC's Set13_set. */
#ifdef __MWERKS__
void Set06_set(s16 arg);
#else
void Set06_set(int arg);
#endif
void set00_set(void); void Set03_set(void); void Set04_set(void);
#ifdef __MWERKS__
void Set05_set();
#else
void Set05_set(u8 arg);
#endif
 void set07_set(void);
void set08_set(void); void set09_set(void); void Set10_set(void); void set11_set(void); void set14_set(void);
void set15_set(void); void set16_set(void); void set17_set(void); void Set18_set(void); void Set19_set(void);
void Set20_set(int arg); void Set22_set(void);
void Eft14_set2(f32 *pos, s16 kind);
void Shell10_set(f32 *pos, int a, int stage, void *b);
#ifdef __MWERKS__
void sub_main_func_004E5F00(int arg);
#define Set13_main sub_main_func_004E5F00
#else
#define Set13_main Set13_set
#endif
extern f32 *sun_pos_tbl[];
extern s16 *st_sun_tb_tbl[];
extern u32 *st_sun_rgba_tbl[];

void stage_set_set(int stage) {
    STGW *w = &stage_work;
    f32 pos[3];
    s16 i;
    f32 *sp;
    s16 *tb;
    u32 *c;

    if (stage_work.x4D < 2) {
        sp = sun_pos_tbl[game_w.stage];
        w->sun_pos[0] = *sp++;
        w->sun_pos[1] = *sp++;
        w->sun_pos[2] = *sp++;
        tb = st_sun_tb_tbl[game_w.stage];
        c = st_sun_rgba_tbl[game_w.stage];
        w->sun_tb[0] = tb[0];
        w->sun_tb[1] = tb[1];
        w->sun_col[0] = *c++;
        w->sun_col[1] = *c++;
        w->sun_col[2] = *c++;
        w->sun_col[3] = *c++;
        w->sun_col[4] = *c++;
        w->sun_col[5] = *c++;
        w->sun_col[6] = *c++;
        w->sun_col[7] = *c++;
    }
    clr_flash();
    switch ((u8)stage) {
    case 0:
    case 0x1A:
        set16_set();
        set08_set();
        set14_set();
        set15_set();
        break;
    case 1:
        set14_set();
        set17_set();
        break;
    case 2:
        set17_set();
        break;
    case 3:
        set14_set();
        set17_set();
        break;
    case 4:
        set00_set();
        Set13_main(0);
        set14_set();
        break;
    case 5:
        Set13_main(0);
        set09_set();
        break;
    case 6:
        set08_set();
        set16_set();
        break;
    case 7:
        Set13_main(0);
        break;
    case 0xA:
        Set10_set();
        break;
    case 0xB:
        Set13_main(3);
        Set19_set();
        Set05_set(game_w.stage);
        break;
    case 0xC:
        Set05_set(stage);
        Set13_main(3);
        Set19_set();
        break;
    case 0xE:
        Set04_set();
        Set13_main(3);
        break;
    case 0xF:
        Set13_main(8);
        Set19_set();
        break;
    case 0x10:
        set09_set();
        break;
    case 0x11:
        for (i = 0; i < 10; i++) {
            pos[0] = 1400.0f + 300.0f * (f32)i;
            pos[1] = 30.0f;
            pos[2] = 2000.0f + 300.0f * (f32)i;
            Shell10_set(pos, 0, game_w.stage, 0);
        }
        break;
    case 0x12:
    case 0x16:
    case 0x17:
        Set13_main(0);
        Set18_set();
        break;
    case 0x13:
        Set13_main(0);
        break;
    case 0x14:
        Set20_set(0);
        break;
    case 0x15:
        pos[0] = 11250.0f;
        pos[1] = 40.0f;
        pos[2] = 9500.0f;
        Eft14_set2(pos, 2);
        Eft14_set2(pos, 3);
        Set13_main(4);
        Set19_set();
        break;
    case 0x18:
        Set19_set();
        break;
    case 0x19:
        Set05_set(stage);
        Set20_set(1);
        Set22_set();
        break;
    case 0x1B:
        set09_set();
        break;
    case 0x1C:
        Set05_set(stage);
        Set04_set();
        Set13_main(3);
        Set19_set();
        set11_set();
        break;
    case 0x1D:
#ifdef __MWERKS__
        Set13_set(0);           /* G's game-side copy (game 0x64EFA0) */
#endif
        Set13_main(7);
        break;
    case 0x1E:
        Set19_set();
        Set13_main(3);
        Set05_set(game_w.stage);
        break;
    case 0x1F:
        Set13_main(3);
        break;
    case 0x20:
        set09_set();
        Set13_main(0);
        break;
    case 0x21:
        set09_set();
        Set13_main(0);
        break;
    case 0x22:
        Set13_main(1);
        Set19_set();
        break;
    case 0x23:
        Set13_main(6);
        break;
    case 0x24:
        set09_set();
        break;
    case 0x25:
        Set13_main(0);
        break;
    case 0x26:
        Set13_main(0);
        break;
    case 0x27:
        Set13_main(0);
        break;
    case 0x28:
        Set13_main(0);
        set09_set();
        break;
    case 0x29:
        set00_set();
        set09_set();
        break;
    case 0x2A:
        set00_set();
        set07_set();
        Set13_main(0);
        set14_set();
        break;
    case 0x2B:
        set00_set();
        break;
    case 0x2D:
        Set13_main(7);
        break;
    case 0x2E:
        set17_set();
        break;
    case 0x2F:
        Set03_set();
        Set13_main(0);
        break;
    case 0x30:
        Set03_set();
        Set13_main(0);
        Set19_set();
        break;
    case 0x31:
        Set03_set();
        Set13_main(0);
        break;
    case 0x32:
        Set13_main(0);
        break;
    case 0x33:
        Set13_main(0);
        set09_set();
        set14_set();
        break;
    case 0x34:
        Set13_main(0);
        set14_set();
        Set19_set();
        set09_set();
        break;
    case 0x35:
        Set13_main(0);
        set14_set();
        set09_set();
        break;
    case 0x36:
        Set19_set();
        break;
    case 0x37:
        Set13_main(0);
        set09_set();
        break;
    case 0x38:
        Set13_main(7);
        break;
    case 0x39:
        Set13_main(0);
        Set19_set();
        break;
    case 0x3D:
        pos[0] = 3600.0f;
        pos[2] = 6600.0f;
        pos[1] = 0.0f;
        Eft14_set2(pos, 2);
        Eft14_set2(pos, 3);
        Set13_main(0);
        Set19_set();
        break;
    case 0x3E:
        Set13_main(0);
        break;
    case 0x43:
        pos[0] = 10296.0f;
        pos[2] = 9429.0f;
        pos[1] = 0.0f;
        Eft14_set2(pos, 2);
        Eft14_set2(pos, 3);
        Set19_set();
        break;
    case 0x46:
        Set13_main(6);
        Set18_set();
        break;
    case 0x49:
    case 0x4B:
        Set13_main(6);
        Set19_set();
        break;
    case 0x4A:
        Set19_set();
        break;
    case 0x4C:
        for (i = 0; i < 4; i++)
            Set06_set(i);
        break;
    case 0x4D:
        for (i = 0; i < 2; i++)
            Set06_set(i);
        break;
    case 0x59:
        Set04_set();
        Set13_main(0);
        break;
    case 0x5A:
        Set13_main(0);
        set09_set();
        set14_set();
        Set04_set();
        break;
    case 0x5B:
        Set13_main(0);
        Set04_set();
        Set19_set();
        break;
    }
}
#endif
