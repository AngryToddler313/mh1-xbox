/* Player code (SLPM_654.95 0x00154D90-0x00154E84): Get_string_pow */
#include "pl.h"
#include "game.h"
#include "plf.h"
extern u8 Gun_data[][GUN_ROW];
f32 flSqrt(f32);
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
/* G: Gun_data rows are 0x18 bytes, Shell_data has 2-byte rows {recoil, pow} (0x514E60),
 * and the skill list is longer (0x2E, 0x2F, 0x44 subtract; 0x45, 0x46, 0x47 add). */
s32 Get_string_pow(PLW *pl, u8 ammo) {
    int p = (s16)(Shell_data[ammo].pow - Gun_data[pl->wpn_kind][3]);
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

/* G sub_main 0x4CE8E0 (unnamed in G): a shot's recoil class 0-2 (firing motion and sound; MH1 read
 * it from Shell_data): the shot type's recoil less the gun's (+0xC) unless skill 0x48 is on, less 2 / 4
 * for skills 0x51 / 0x52. Not a gun: 2. */
s32 sub_main_func_004CE8E0(PLW *pl, u8 ammo) {
    int p;

    if (pl->work35F != 7) {
        return 2;
    }
    p = (s16)Shell_data[ammo].recoil;
    if (Pl_Skill_ck(pl, 0x48) == 0) {
        p = (s16)(p - Gun_data[pl->wpn_kind][12]);
        if (Pl_Skill_ck(pl, 0x51) == 1) {
            p = (s16)(p - 2);
        } else if (Pl_Skill_ck(pl, 0x52) == 1) {
            p = (s16)(p - 4);
        }
    }
    if ((s16)p <= 0) {
        p = 0;
    }
    if ((s16)p < 9) {
        return 0;
    }
    if ((s16)p < 11) {
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
