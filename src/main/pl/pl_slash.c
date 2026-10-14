/* Player code (SLPM_654.95 0x001536D0-0x001537D0): Pl_slash_calc - adds dv to the weapon sharpness (work87E,
 * capped at work884) and queues the sharpness-change message when the level from Pl_slash_lv_ck changes. */
#include "pl.h"
#include "game.h"
#include "plf.h"
#ifdef VERSION_MHG
/* G: the three messages are pointer variables (set at run time), not literals */
extern char *slash_msg_down, *slash_msg_up, *slash_msg_max;
#define SLASH_MSG_DOWN slash_msg_down
#define SLASH_MSG_UP slash_msg_up
#define SLASH_MSG_MAX slash_msg_max
#else
#define SLASH_MSG_DOWN lit_1805_0035B1B0
#define SLASH_MSG_UP lit_1806_0035B1D0
#define SLASH_MSG_MAX lit_1807_0035B1F0
#endif
extern char lit_1805_0035B1B0[];
extern char lit_1806_0035B1D0[];
extern char lit_1807_0035B1F0[];

void Pl_slash_calc(PLW *pl, int dv) {
    u8 up = 0;
    u8 lv;

    if (Pl_master_ck(pl) == 0) {
        return;
    }
    if (pl->kind == 1 || pl->kind == 5) {
        return;
    }
#ifdef VERSION_MHG
    /* G: skills 0x3F / 0x40 halve or double sharpness loss */
    /* G near-match: logic believed right, 60/97 insns differ only in saved-register choice (original: up s0, dv s1, pl s2, (s16)dv s3) */
    if ((s16)dv < 0) {
        if (Pl_Skill_ck(pl, 0x3F) == 1) {
            dv = (s16)((s16)dv / 2);
            if (dv == 0) {
                dv = -1;
            }
        } else if (Pl_Skill_ck(pl, 0x40) == 1) {
            dv = (s16)((s16)dv * 2);
        }
    }
#endif
    pl->work87E = pl->work87E + dv;
    if (pl->work87E <= 0) {
        pl->work87E = 0;
    }
    if (pl->work884 < pl->work87E) {
        pl->work87E = pl->work884;
        up = 1;
    }
    lv = Pl_slash_lv_ck(pl);
    if (lv != pl->work887) {
        if (lv < pl->work887) {
            set01_set2(SLASH_MSG_DOWN);
        } else {
            set01_set2(SLASH_MSG_UP);
        }
        pl->work887 = lv;
    }
    if (up) {
        set01_set2(SLASH_MSG_MAX);
    }
}
