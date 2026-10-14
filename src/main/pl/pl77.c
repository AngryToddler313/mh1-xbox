/* Player code (SLPM_654.95 0x00154D90-0x00154E84): Get_string_pow */
#include "pl.h"
#include "game.h"
#include "plf.h"
extern u8 Gun_data[26][0x14];
f32 flSqrt(f32);
extern u8 Gun_data[26][0x14];
s16 Pl_item_num_ck(PLW *, int);
void adx_se_set(PLW *, int);
void init_set_work();
void init_eft_work();
void init_shell_work();
void init_item_work();
void clr_set_work();
void clr_eft_work();
void clr_shell_work();
void clr_item_work();
void clr_used_heap(int, int);

#ifdef VERSION_MHG
/* G: Gun_data rows are 0x18 bytes at 0x517720, the shell table has 2-byte rows at 0x514E60,
 * and the skill list is longer (0x2E, 0x2F, 0x44 subtract; 0x45, 0x46, 0x47 add). */
extern u8 D_00517720[][0x18];
extern u8 D_00514E60[][2];
s32 Get_string_pow(PLW *pl, u8 ammo) {
    int p = (s16)(D_00514E60[ammo][1] - D_00517720[pl->wpn_kind][3]);
    if (Pl_Skill_ck(pl, 0x2E) == 1) {
        p = (s16)(p - 1);
    }
    if (Pl_Skill_ck(pl, 0x2F) == 1) {
        p = (s16)(p - 2);
    }
    if (Pl_Skill_ck(pl, 0x44) == 1) {
        p = (s16)(p - 3);
    }
    if (Pl_Skill_ck(pl, 0x45) == 1) {
        p = (s16)(p + 1);
    }
    if (Pl_Skill_ck(pl, 0x46) == 1) {
        p = (s16)(p + 2);
    }
    if (Pl_Skill_ck(pl, 0x47) == 1) {
        p = (s16)(p + 3);
    }
    if ((s16)p < 5) {
        return 0;
    }
    if ((s16)p < 8) {
        return 1;
    }
    return 2;
}
#else
s32 Get_string_pow(PLW *pl, u8 ammo) {
    int p = (s16)(Shell_data[ammo]._02[0] - Gun_data[pl->wpn_kind][3]);
    if (Pl_Skill_ck(pl, 0x30) == 1) {
        p = (s16)(p - 1);
    } else if (Pl_Skill_ck(pl, 0x31) == 1) {
        p = (s16)(p - 2);
    }
    if ((s16)p < 5) {
        return 0;
    }
    if ((s16)p < 8) {
        return 1;
    }
    return 2;
}
#endif
