#ifndef EM_REVIVE_G_H
#define EM_REVIVE_G_H
/* Monster Hunter G: a large monster of a quest that wants it again comes back after it died (Quest_enemy_revival_ck).
 * G added this to em01, em15, em17, em20 and em27 (act_set and the die functions); written from G's asm for the PC
* (VERSION_MHG, not byte-checked; the including file declares em_act_set). em20 keeps its own copy (em20_ai_nm.c). The pattern, the same in each:
 *  - the carcass lies 1800 ticks (MH1 450);
 *  - in the carve state, while the quest wants a revival, that time counts down; at 0 the carve points go and a
 *    150-tick fade starts (x798 = alpha), then act_set(em, 5, 3, 4);
 *  - act_set(5, 3): Quest_enemy_revival_set, em_status_init and the monster's own start point (per em);
 *  - die act 3 (EMG_DIE03): em_cmd_reset and back to act 0/1, or x04++ when no revival is wanted. */
#include "em.h"

int Quest_enemy_revival_ck(EMW *);
int Quest_enemy_revival_set(EMW *);
void Em_hagi_point_clr(EMW *);
void em_cmd_reset(EMW *);
void em_status_init(EMW *);

#define EMG_CARVE_TIME 1800

/* carve state: count down while a revival is wanted (G sets x9EA = 5 first in die01 when x9ea != 0) */
#define EMG_DIE_REVIVE_WAIT(em, x9ea)                                   \
    do {                                                                \
        if (x9ea) (em)->x9EA = 5;                                       \
        if (Quest_enemy_revival_ck(em) != 0 && --(em)->work08 <= 0) {   \
            (em)->x05 += 1;                                             \
            if (x9ea) {                                                 \
                (em)->x9EA = 0;                                         \
                Em_hagi_point_clr(em);                                  \
                (em)->work08 = 150;                                     \
            } else {                                                    \
                (em)->work08 = 150;                                     \
                Em_hagi_point_clr(em);                                  \
            }                                                           \
        }                                                               \
    } while (0)

/* the fade, then the revival act */
#define EMG_DIE_REVIVE_FADE(em, act_set)                                \
    do {                                                                \
        if ((em)->work08 > 0) {                                         \
            (em)->work08 -= 1;                                          \
        }                                                               \
        if ((em)->work08 <= 0) {                                        \
            if ((em)->x8C3 == 0) {                                      \
                act_set(em, 5, 3, 4);                                   \
            }                                                           \
        } else {                                                        \
            (em)->x798 = (f32)(em)->work08 / 150.0f;                    \
        }                                                               \
    } while (0)

/* die act 3: get up again, or go */
#define EMG_DIE03(em)                                                   \
    do {                                                                \
        if ((em)->x05 == 0) {                                           \
            if (Quest_enemy_revival_ck(em) == 1) {                      \
                em_cmd_reset(em);                                       \
                (em)->x839 = 0;                                         \
                em_act_set(em, 0, 1);                                   \
            } else {                                                    \
                (em)->x04 += 1;                                         \
            }                                                           \
        }                                                               \
    } while (0)

/* act_set(em, 4..6, 3, ..) (G checks only the act number): the revival itself, then the em's start point (init) */
#define EMG_ACT_REVIVE(em, kind, no, init)                              \
    do {                                                                \
        if ((u16)(no) == 3 && Quest_enemy_revival_ck(em) == 1) {        \
            Quest_enemy_revival_set(em);                                \
            em_status_init(em);                                         \
            init(em);                                                   \
        }                                                               \
    } while (0)

/* a start point: angle and position */
#define EMG_PLACE(em, a, x, y, z)                                       \
    do {                                                                \
        (em)->ang[1] = (a);                                             \
        (em)->pos[0] = (x);                                             \
        (em)->pos[1] = (y);                                             \
        (em)->pos[2] = (z);                                             \
    } while (0)

#endif
