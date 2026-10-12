/* crmdl05 - monster and NPC model creation (SLPM_654.95 0x00124820-0x00124C48): em_create_model, npc_create_model.
   Working copy of the whole range in crmdl_nm.c. */
#include "types.h"
#include "mdlw.h"

typedef struct PLM {            /* PLW fields touched here (player_work[], 0xA00 each) */
    u8 _pad00[0xC];
    u16 id;                     /* 0x00C */
    u8 _pad0E[0x11 - 0xE];
    u8 sex;                     /* 0x011 0 male, 1 female */
    u8 _pad12[0x110 - 0x12];
    void *part[32];             /* 0x110 matrix block pointers (pairs: block, block + 0xB0) */
    u8 _pad190[0x4E4 - 0x190];
    s16 x4E4;                   /* 0x4E4 */
    u8 _pad4E6[0x508 - 0x4E6];
    s16 mdl_n;                  /* 0x508 slots used by the body model */
    s16 mdl_no;                 /* 0x50A first slot */
    s32 mdl_ptr;                /* 0x50C */
    u8 _pad510[4];
    s32 wpn_ptr[4];             /* 0x514 */
    s16 wpn_n[4];               /* 0x524 */
    s16 wpn_no[4];              /* 0x52C */
    s32 part_ptr[6];            /* 0x534 armor part model pointers */
    s16 part_n[6];              /* 0x54C slots used by the armor parts */
    s16 part_no[6];             /* 0x558 first slot */
    u8 _pad564[0xA00 - 0x564];
} PLM;

extern PLM player_work[];
extern u8 parts_work[];
extern s32 WEAPON_TEX[];
extern s32 pl_area_top;
extern u8 mdlw_heap[0x80];
extern s32 set_top;
extern s32 set_mdlw;
extern s32 stage_model;
extern s32 set_model_data[];
extern s32 SET_TEX[];
extern s32 EDIT_TEX[];
extern s32 edit_mdlw[2];
extern s32 edit_top[2];


void load_weapon_model(int);
void load_set_model(int);
void load_edit_model(int);
int get_start_mdlw(int);
MDLW *get_mdlw_ptr(int);
void set_used_mdlw(int, int);
void clr_used_mdlw(int, int);
MDLW *model_work_set(int, int, int, int, int, int);
void model_work_free(int);

typedef struct STW {
    u8 _pad00[0x34];
    s16 n;                      /* 0x34 number of stage model slots */
    s16 _pad36;
    s16 first;                  /* 0x38 first slot */
} STW;

extern STW stage_work;

typedef struct GWM {            /* game_w: monster model bookkeeping, 4 slots */
    u8 _pad00[0x28];
    u8 em_kind[4];              /* 0x28 monster kind per slot, 0 = none */
    u8 _pad2C[0x88 - 0x2C];
    s32 mdl_ptr[4];             /* 0x88 */
    s16 mdl_n[4];               /* 0x98 */
    s16 mdl_no[4];              /* 0xA0 */
    s32 sub_ptr[4];             /* 0xA8 */
    s16 sub_n[4];               /* 0xB8 -1 = none */
    s16 sub_no[4];              /* 0xC0 */
} GWM;

extern GWM game_w;
extern s32 ENEMY_TEX[];
extern s32 ENEMY_SUB_TEX[];
extern s32 NPC_TEX[];
extern s32 em_sub_model_data[];

void em_motion_free(int);
void em_motion_load(int, int);
void load_enemy_model(int);
void load_enemy_sub_model();
void load_npc_model(int);
MDLW *em_model_work_set(int, int, int, int, int, int);
MDLW *pl_model_work_set();
extern s32 pl_shader_tbl0[];

void em_create_model(int slot) {
    GWM *g = &game_w;
    int area;
    int h;

    if (game_w.em_kind[slot] == 0) {
        g->mdl_no[slot] = 0;
        g->mdl_n[slot] = 0;
        g->mdl_ptr[slot] = 0;
        return;
    }
    load_enemy_model(g->em_kind[slot]);
    area = pl_area_top;
    h = get_start_mdlw(1);
    if (h >= 0) {
        g->mdl_no[slot] = h;
        g->mdl_n[slot] = 1;
        g->mdl_ptr[slot] = (s32)get_mdlw_ptr(h);
        set_used_mdlw(h, 1);
        em_model_work_set((s16)h, area, (s16)(slot * 20 + 0x9A), ENEMY_TEX[g->em_kind[slot]], 0x900, slot & 0xFF);
        em_motion_load(slot, g->em_kind[slot]);
        if (em_sub_model_data[g->em_kind[slot]] != -1) {
            load_enemy_sub_model(g->em_kind[slot]);
            area = pl_area_top;
            h = get_start_mdlw(1);
            if (h < 0) {
                g->sub_n[slot] = -1;
                return;
            }
            g->sub_no[slot] = h;
            g->sub_n[slot] = 1;
            g->sub_ptr[slot] = (s32)get_mdlw_ptr(h);
            set_used_mdlw(h, 1);
            model_work_set((s16)h, area, (s16)(slot * 20 + 0xA6), ENEMY_SUB_TEX[g->em_kind[slot]], 0x900, 0);
        } else {
            g->sub_n[slot] = -1;
        }
    }
}

void npc_create_model(int slot) {
    GWM *g = &game_w;
    int area;
    int h;

    load_npc_model(slot);
    area = pl_area_top;
    h = get_start_mdlw(1);
    if (h >= 0) {
        g->mdl_no[slot] = h;
        g->mdl_n[slot] = 1;
        g->mdl_ptr[slot] = (s32)get_mdlw_ptr(h);
        set_used_mdlw(h, 1);
        switch (slot) {
        case 0:
            pl_model_work_set((s16)h, area, (s16)(slot * 20 + 0x9A), NPC_TEX[slot], pl_shader_tbl0[0], 0x100, 0);
            break;
        case 1:
            em_model_work_set((s16)h, area, (s16)(slot * 20 + 0x9A), NPC_TEX[slot], 0x900, slot & 0xFF);
            em_motion_load(slot, 0xA);
            break;
        case 2:
            em_model_work_set((s16)h, area, (s16)(slot * 20 + 0x9A), NPC_TEX[slot], 0x900, slot & 0xFF);
            em_motion_load(slot, 9);
            break;
        case 3:
            em_model_work_set((s16)h, area, (s16)(slot * 20 + 0x9A), NPC_TEX[slot], 0x900, slot & 0xFF);
            em_motion_load(slot, 0x20);
            break;
        }
    }
}
