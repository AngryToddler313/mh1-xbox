/* lbem04, run 2: em04_effect_move_0053E790 .. dummy_em_prog_0053E840 (lobby.bin 0x0053E790-0x0053E848): the matching functions of lb_em04_nm.c. */
#include "g_alias.h"
#include "lobby.h"
#include "em.h"

typedef struct { u8 pad[GV(0x2A, 0x36)]; s8 eff; } LBEFW;   /* G: the effect step at +0x36 */

int em_frame_check(EMW *, f32, int);
int em_frame_check3(EMW *, int, f32, f32);
void Npc_se_req();
int Code_Make();
void Eft13_set_em_scl(EMW *, int, f32, int);
void Eft25_set();
void eft01_set();
void sound_call_0053E7E0(EMW *em, int frame, int se);

#if !defined(G_PART) || G_PART == 2
void em04_effect_move_0053E790(EMW *em) {
    LBEFW *w = (LBEFW *)em->ex;

    switch (w->eff) {
    case 0:
        w->eff++;
        break;
    case 1:
        ef_move_sub_0053E360(em, w);
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void sound_call_0053E7E0(EMW *em, int frame, int se) {
    if (em_frame_check(em, (f32)frame, 0) != 0) {
        Npc_se_req(em, se, (u8 *)em + 0xAC, 2);
    }
}
#endif

#if !defined(G_PART)
void dummy_em_prog_0053E840(void) {
}
#endif
