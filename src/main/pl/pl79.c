/* Player code (SLPM_654.95 0x0014D320-0x0014DC8C): Basic_item_set (item effects) */
#include "pl.h"
#include "game.h"
#include "plf.h"
#include "flow.h"
void Pl_vital_calc_item(PLW *, int);
void Pl_max_vital_calc(PLW *, int);
void func_639DF0(PLW *, int);
void set01_set(int, int, int);

#ifdef VERSION_MHG
/* G's Basic_item_set (sub_main 0x4F27B0) and its helper 0x4F2730, written from G's asm for the PC (not
   byte-checked; this file is not in the G PS2 build). G renumbered the items (names in the comments) and
   changed some effects: demondrug / armorskin raise the boost to at least 3/5 (armorskin and armorskin G both
   to 3) instead of setting it; mega demondrug / armor pills (G) give 25 for 600 ticks; the potion shares 15
   (MH1 30) and the herb 10 with the skills; most items also run food_bonus. work917 = 15 (MH1 1). */
void Pl_poison_add(PLW *, int);

/* G sub_main 0x4F2730: skill 0x5A, one time in three: max stamina +75. G calls ran_suu(1) (the PS2 build,
   byte-checked); the PC keeps its own state instead. */
#if !defined(G_PART) || G_PART == 1
static void food_bonus_004F2730(PLW *pl) {
#ifdef __MWERKS__
    if (Pl_master_ck(pl) == 1 && Pl_Skill_ck(pl, 0x5A) == 1) {
        if ((u16)ran_suu(1) % 3 == 0) {
#else
    static u16 rnd = 1;
    if (Pl_master_ck(pl) == 1 && Pl_Skill_ck(pl, 0x5A) == 1) {
        rnd = (u16)((u32)(rnd ? rnd : 1) * 0xB0 % 0xFF53);
        if (rnd % 3 == 0) {
#endif
            Pl_max_stamina_calc(pl, 0x4B);
        }
    }
}
#endif

#if !defined(G_PART)
static int share_ok(PLW *pl, PLW *t) { return Pl_master_ck(pl) == 0 && pl->stg == t->stg; }

#if !defined(G_PART)
void Basic_item_set(PLW *pl) {
    PLW *t = &player_work[game_w.master];
    pl->work917 = 15;
    switch (pl->work88A) {
    case 0x11:                                  /* mega demondrug pill (G) */
        if (Pl_master_ck(pl) == 1) {
            pl->work6A5 = 25;
            pl->work6A6 = 600;
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        break;
    case 0x14:                                  /* mega armor pill (G) */
        if (Pl_master_ck(pl) == 1) {
            pl->work6A9 = 25;
            pl->work6AA = 600;
            *(u16 *)((u8 *)pl + 0x93E) = 600;
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        break;
    case 0x0F:                                  /* demondrug */
    case 0x10:                                  /* mega demondrug */
        if (Pl_master_ck(pl) == 1) {
            if (pl->work88A == 0x0F) {
                if (pl->work6A4 < 3) pl->work6A4 = 3;
            } else if (pl->work6A4 < 5) {
                pl->work6A4 = 5;
            }
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        goto chr197;
    case 0x12:                                  /* armorskin */
    case 0x13:                                  /* mega armorskin: G gives 3 as well */
        if (Pl_master_ck(pl) == 1) {
            if (pl->work6A8 < 3) pl->work6A8 = 3;
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 3, 0xA);
        goto chr197;
    case 0x93:                                  /* might seed */
        if (Pl_master_ck(pl) == 1) {
            pl->work6A5 = 0xA;
            pl->work6A6 = 0x1518;
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        if (share_ok(pl, t) && Pl_Skill_ck(pl, 0x1E) == 1) {
            t->work6A5 = 0xA;
            t->work6A6 = 0x1518;
            Eft06_set(4.0f, t, 0, 2, 0xA);
        }
        goto chr197;
    case 0x94:                                  /* adamant seed */
        if (Pl_master_ck(pl) == 1) {
            pl->work6A9 = 0x14;
            pl->work6AA = 0x1518;
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 3, 0xA);
        if (share_ok(pl, t) && Pl_Skill_ck(pl, 0x1E) == 1) {
            t->work6A9 = 0x14;
            t->work6AA = 0x1518;
            Eft06_set(4.0f, t, 0, 3, 0xA);
        }
        goto chr197;
    case 0x16:                                  /* hot drink */
        pl->work91A = 18000;
        Eft06_set(4.0f, pl, 0, 6, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x11, 0);
            food_bonus_004F2730(pl);
        }
        goto chr197;
    case 0x15:                                  /* cool drink */
        pl->work918 = 18000;
        Eft06_set(4.0f, pl, 0, 5, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x10, 0);
            food_bonus_004F2730(pl);
        }
        goto chr197;
    case 0x17:                                  /* psychoserum */
        pl->work930 = 300;
        Eft06_set(4.0f, pl, 0, 9, 0xA);
        food_bonus_004F2730(pl);
        goto chr197;
    case 0x0C:                                  /* catalyst (G) */
        pl->work91C = 9000;
        Eft06_set(4.0f, pl, 0, 7, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x17, 0);
            food_bonus_004F2730(pl);
        }
        goto chr197;
    case 0x0D:                                  /* dash juice */
        Pl_max_stamina_calc(pl, 0x4B);
        pl->work8CC = 5400;
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x18, 0);
            food_bonus_004F2730(pl);
        }
        goto eft0;
    case 0x0E:                                  /* mega dash juice */
        Pl_max_stamina_calc(pl, 0x96);
        pl->work8CC = 10800;
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x18, 0);
            food_bonus_004F2730(pl);
        }
        goto eft0;
    case 0x1C:                                  /* ancient potion */
        Pl_max_stamina_calc(pl, 0x1C2);
    case 0x1B:                                  /* max potion */
        Pl_max_vital_calc(pl, 0x96);
        Pl_vital_calc_item(pl, 0x96);
        food_bonus_004F2730(pl);
        goto eft0;
    case 0x81:                                  /* herb */
        Pl_vital_calc_item(pl, 0x14);
        food_bonus_004F2730(pl);
        if (share_ok(pl, t) && Pl_Skill_ck(pl, 0x73) == 1) {
            Pl_vital_calc_item(t, 0xA);
            Eft06_set(4.0f, t, 0, 0, 0xA);
        }
        goto eft0;
    case 0x20:                                  /* lifedust */
        Pl_vital_calc_item(pl, 0x32);
        food_bonus_004F2730(pl);
        if (share_ok(pl, t)) {
            Pl_vital_calc_item(t, 0x32);
            Eft06_set(4.0f, t, 0, 0, 0xA);
        }
        goto eft0;
    case 0x07:                                  /* potion */
        Pl_vital_calc_item(pl, 0x1E);
        food_bonus_004F2730(pl);
        if (share_ok(pl, t) && Pl_Skill_ck(pl, 0x1C) == 1) {
            Pl_vital_calc_item(t, 0xF);
            Eft06_set(4.0f, t, 0, 0, 0xA);
        }
        goto eft0;
    case 0x1FB:                                 /* first-aid med (G) */
        Pl_vital_calc_item(pl, 0x1E);
        food_bonus_004F2730(pl);
        goto eft0;
    case 0xAB:                                  /* sushifish */
        Pl_vital_calc_item(pl, 0x14);
        food_bonus_004F2730(pl);
        goto eft0;
    case 0x8F:                                  /* toadstool */
        food_bonus_004F2730(pl);
        Pl_poison_add(pl, 0xA);
        if (pl->cnt39A & 1) {
            Pl_max_vital_calc(pl, 0xA);
        } else {
            Pl_max_vital_calc(pl, -0xA);
        }
        pl_chr_set2(pl, 0x198, 4, 0);
        break;
    case 0xB9:                                  /* bitterbug */
        Pl_vital_calc(pl, -0xA);
        if (pl->vital <= 0) {
            pl->vital = 1;
        }
        if (pl->cnt39A & 1) {
            pl->x7BA = 0;
            pl_chr_set2(pl, 0x197, 4, 0);
            Eft06_set(4.0f, pl, 0, 1, 0xA);
            food_bonus_004F2730(pl);
        } else {
            pl_chr_set2(pl, 0x198, 4, 0);
        }
        break;
    case 0x82:                                  /* antidote herb */
        if (pl->cnt39A & 1) {
            pl->x7BA = 0;
            pl_chr_set2(pl, 0x197, 4, 0);
            Eft06_set(4.0f, pl, 0, 1, 0xA);
            food_bonus_004F2730(pl);
        } else {
            pl_chr_set2(pl, 0x198, 4, 0);
        }
        break;
    case 0x08:                                  /* mega potion */
        Pl_vital_calc_item(pl, 0x32);
        food_bonus_004F2730(pl);
eft0:
        Eft06_set(4.0f, pl, 0, 0, 0xA);
chr197:
        pl_chr_set2(pl, 0x197, 4, 0);
        break;
    case 0x09:                                  /* nutrients */
        Pl_max_vital_calc(pl, 0xA);
        food_bonus_004F2730(pl);
        goto eft0;
    case 0x0A:                                  /* mega nutrients */
        Pl_max_vital_calc(pl, 0x14);
        food_bonus_004F2730(pl);
        goto eft0;
    case 0x91:                                  /* kimchi */
    case 0x1FC:                                 /* ration (G) */
        Pl_max_stamina_calc(pl, 0x4B);
        food_bonus_004F2730(pl);
        Eft06_set(4.0f, pl, 0, 4, 0xA);
        goto chr197;
    case 0x26:                                  /* rare steak */
        Pl_max_stamina_calc(pl, 0x4B);
        goto steak;
    case 0x28:                                  /* well-done steak G */
        Pl_max_stamina_calc(pl, 0x1C2);
        goto steak;
    case 0x27:                                  /* well-done steak */
        Pl_max_stamina_calc(pl, 0x96);
steak:
        if (Pl_Skill_ck(pl, 0x58) == 1) {
            Pl_max_stamina_calc(pl, 0x4B);
        } else if (Pl_Skill_ck(pl, 0x59) == 1) {
            Pl_max_stamina_calc(pl, 0x96);
        } else {
            food_bonus_004F2730(pl);
        }
        Eft06_set(4.0f, pl, 0, 4, 0xA);
        pl_chr_set2(pl, 0x192, 4, 0);
        break;
    case 0x29:                                  /* burnt meat */
        if (pl->cnt39A & 1) {
            Pl_max_stamina_calc(pl, 0x4B);
            goto steak;
        }
        Pl_max_stamina_calc(pl, -0x4B);
        pl_chr_set2(pl, 0x198, 4, 0);
        break;
    case 0x1A:                                  /* herbal medicine */
        Pl_vital_calc_item(pl, 0xA);
    case 0x0B:                                  /* antidote */
        food_bonus_004F2730(pl);
        pl->x7BA = 0;
        pl_chr_set2(pl, 0x197, 4, 0);
        Eft06_set(4.0f, pl, 0, 1, 0xA);
        if (share_ok(pl, t) && Pl_Skill_ck(pl, 0x1D) == 1 && pl->work88A == 0x0B) {
            t->x7BA = 0;
            Eft06_set(4.0f, t, 0, 1, 0xA);
        }
        break;
    }
    Pl_item_cnt_up(pl);
    if (Game_clear_ck(1) == 0) {
        Pl_item_stack(pl, pl->work88A, -1);
    }
#endif
}
#endif
#else
void Basic_item_set(PLW *pl) {
    PLW *t = &player_work[game_w.master];
    pl->work917 = 1;
    switch (pl->work88A) {
    case 0xD:
    case 0xE:
        if (Pl_master_ck(pl) == 1) {
            if (pl->work88A == 0xD) {
                pl->work6A4 = 3;
            } else {
                pl->work6A4 = 5;
            }
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        goto chr197;
    case 0xF:
    case 0x10:
        if (Pl_master_ck(pl) == 1) {
            if (pl->work88A == 0xF) {
                pl->work6A8 = 3;
            } else {
                pl->work6A8 = 5;
            }
        }
        Eft06_set(4.0f, pl, 0, 3, 0xA);
        goto chr197;
    case 0x53:
        if (Pl_master_ck(pl) == 1) {
            pl->work6A5 = 0xA;
            pl->work6A6 = 0x1518;
        }
        Eft06_set(4.0f, pl, 0, 2, 0xA);
        if (Pl_master_ck(pl) == 0 && pl->stg == t->stg && Pl_Skill_ck(pl, 0x1E) == 1) {
            t->work6A5 = 0xA;
            t->work6A6 = 0x1518;
            Eft06_set(4.0f, t, 0, 2, 0xA);
        }
        goto chr197;
    case 0x54:
        if (Pl_master_ck(pl) == 1) {
            pl->work6A9 = 0x14;
            pl->work6AA = 0x1518;
        }
        Eft06_set(4.0f, pl, 0, 3, 0xA);
        if (Pl_master_ck(pl) == 0 && pl->stg == t->stg && Pl_Skill_ck(pl, 0x1F) == 1) {
            t->work6A9 = 0x14;
            t->work6AA = 0x1518;
            Eft06_set(4.0f, t, 0, 3, 0xA);
        }
        goto chr197;
    case 0xA1:
        pl->work91A = 0x4650;
        Eft06_set(4.0f, pl, 0, 6, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x11, 0);
        }
        goto chr197;
    case 0xA0:
        pl->work918 = 0x4650;
        Eft06_set(4.0f, pl, 0, 5, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x10, 0);
        }
        goto chr197;
    case 0xA4:
        pl->work930 = 0x12C;
        Eft06_set(4.0f, pl, 0, 9, 0xA);
        goto chr197;
    case 0x99:
        pl->work91C = 0x2328;
        Eft06_set(4.0f, pl, 0, 7, 0xA);
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x17, 0);
        }
        goto chr197;
    case 9:
        Pl_max_stamina_calc(pl, 0x4B);
        pl->work8CC = 0x1518;
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x18, 0);
        }
        goto eft0;
    case 0xA:
        Pl_max_stamina_calc(pl, 0x96);
        pl->work8CC = 0x2A30;
        if (Pl_master_ck(pl) == 1) {
            set01_set(0, 0x18, 0);
        }
        goto eft0;
    case 8:
        Pl_max_stamina_calc(pl, 0x1C2);
    case 7:
        Pl_max_vital_calc(pl, 0x96);
        Pl_vital_calc_item(pl, 0x96);
        goto eft0;
    case 0x41:
        Pl_vital_calc_item(pl, 0x14);
        goto eft0;
    case 0x9A:
        Pl_vital_calc_item(pl, 0x32);
        if (Pl_master_ck(pl) == 0 && pl->stg == t->stg) {
            Pl_vital_calc_item(t, 0x32);
            Eft06_set(4.0f, t, 0, 0, 0xA);
        }
        goto eft0;
    case 1:
        Pl_vital_calc_item(pl, 0x1E);
        if (Pl_master_ck(pl) == 0 && pl->stg == t->stg && Pl_Skill_ck(pl, 0x1C) == 1) {
            Pl_vital_calc_item(t, 0x1E);
            Eft06_set(4.0f, t, 0, 0, 0xA);
        }
        goto eft0;
    case 0x9C:
        Pl_vital_calc_item(pl, 0x1E);
        goto eft0;
    case 0x5F:
        Pl_vital_calc_item(pl, 0x14);
        goto eft0;
    case 0x4B:
        func_639DF0(pl, 0xA);
        if (pl->cnt39A & 1) {
            Pl_max_vital_calc(pl, 0xA);
        } else {
            Pl_max_vital_calc(pl, -0xA);
        }
        pl_chr_set2(pl, 0x198, 4, 0);
        break;
    case 0x5B:
        Pl_vital_calc(pl, -0xA);
        if (pl->vital <= 0) {
            pl->vital = 1;
        }
        if (pl->cnt39A & 1) {
            pl->x7BA = 0;
            pl_chr_set2(pl, 0x197, 4, 0);
            Eft06_set(4.0f, pl, 0, 1, 0xA);
        } else {
            pl_chr_set2(pl, 0x198, 4, 0);
        }
        break;
    case 0x42:
        if (pl->cnt39A & 1) {
            pl->x7BA = 0;
            pl_chr_set2(pl, 0x197, 4, 0);
            Eft06_set(4.0f, pl, 0, 1, 0xA);
        } else {
            pl_chr_set2(pl, 0x198, 4, 0);
        }
        break;
    case 2:
        Pl_vital_calc_item(pl, 0x32);
eft0:
        Eft06_set(4.0f, pl, 0, 0, 0xA);
chr197:
        pl_chr_set2(pl, 0x197, 4, 0);
        break;
    case 3:
        Pl_max_vital_calc(pl, 0xA);
        goto eft0b;
    case 4:
        Pl_max_vital_calc(pl, 0x14);
eft0b:
        Eft06_set(4.0f, pl, 0, 0, 0xA);
        pl_chr_set2(pl, 0x197, 4, 0);
        break;
    case 0x9D:
        Pl_max_stamina_calc(pl, 0x4B);
        Eft06_set(4.0f, pl, 0, 4, 0xA);
        goto chr197;
    case 0x13:
        Pl_max_stamina_calc(pl, 0x4B);
        goto eft4;
    case 0x14:
        Pl_max_stamina_calc(pl, 0x96);
eft4:
        Eft06_set(4.0f, pl, 0, 4, 0xA);
        pl_chr_set2(pl, 0x192, 4, 0);
        break;
    case 0x15:
        if (pl->cnt39A & 1) {
            Pl_max_stamina_calc(pl, 0x4B);
            goto eft4;
        }
        Pl_max_stamina_calc(pl, -0x4B);
        pl_chr_set2(pl, 0x198, 4, 0);
        break;
    case 6:
        Pl_vital_calc_item(pl, 0xA);
    case 5:
        pl->x7BA = 0;
        pl_chr_set2(pl, 0x197, 4, 0);
        Eft06_set(4.0f, pl, 0, 1, 0xA);
        if (Pl_master_ck(pl) == 0 && pl->stg == t->stg && Pl_Skill_ck(pl, 0x1D) == 1 && pl->work88A == 5) {
            t->x7BA = 0;
            Eft06_set(4.0f, t, 0, 1, 0xA);
        }
        break;
    }
    Pl_item_cnt_up(pl);
    if (Game_clear_ck(1) == 0) {
        Pl_item_stack(pl, pl->work88A, -1);
    }
}
#endif
