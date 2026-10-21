/* emsrch - SLPM_654.95 0x00109E30-0x0010A208 (one original source file, g_em_search_set.s): monster target search
   over the four players (em_search_set), the per-monster "durability" (hit-point) slots (em_dur_init / em_dur_set)
   and joint matrix accessors (get_joint_pos / wmat / mat, plus the *_em duplicates that tail-call them).
   Every function byte-matches except em_dur_set, which is kept as the original bytes (config/c_rawfuncs.txt;
   2 of 22 instructions off as C: the table address lands in v1, the original reuses a0). Its C is in emsrch_nm.c,
   which the PC build links weak. */
#include "types.h"
#include "em.h"
#include "pl.h"

extern PLW player_work[];
extern s16 *em_dur_tbl[];       /* per monster type: 9 hit-point values (s16), then the 0x953 value */

u16 calc_vec_ang(f32, f32, f32, f32);
f32 flAbs(f32);
f32 flSqrt(f32);
void flmatGetTrans(f32 *, u8 *);

/* Picks the nearest player on the monster's stage within dist_xz (0 = any) horizontally, dist_y (0 = any)
   vertically and inside a cone of +-ang around the monster's facing; stores it in x3B0 / x617 / x3AC (distance)
   and horm_ang (angle to it); returns the player number or -1. The angle test needs the temp t (an inline
   helper in the original source, perhaps), the tie test is if / else if with the shared `best = i` after it. */
#if !defined(G_PART) || G_PART == 1
s8 em_search_set(EMW *em, f32 dist_xz, f32 dist_y, u16 ang) {
    int i;
    int best;
    PLW *pl;
    f32 best_d;
    f32 px, py, pz;
    f32 dy, dx, dz, dxz, d;

    i = 0;
    best = -1;
    pl = player_work;
    em->x3B0 = 0;
    best_d = 0.0f;
    em->x617 = -1;
    em->x3AC = 0;
    px = em->pos[0];
    py = em->pos[1];
    pz = em->pos[2];
    do {
        if (em->stg == pl->stg) {
            dy = pl->pos[1] - py;
            if (dist_y == 0.0f || flAbs(dy) <= dist_y) {
                dx = pl->pos[0] - px;
                dz = pl->pos[2] - pz;
                dxz = flSqrt(dx * dx + dz * dz);
                if (dist_xz == 0.0f || dxz <= dist_xz) {
                    int t;
                    d = flSqrt(dxz * dxz + dy * dy);
                    t = ((calc_vec_ang(px, pz, pl->pos[0], pl->pos[2]) & 0xFFFF) - 0x4000) & 0xFFFF;
                    if (((t + ang) - em->ang[1] & 0xFFFF) <= ang * 2) {
                        if (0.0f == best_d) {
                            best_d = d;
                        } else if (d <= best_d) {
                            best_d = d;
                        } else {
                            goto next;
                        }
                        best = i;
                    }
                }
            }
        }
next:
        i++;
        pl++;
    } while (i < 4);
    if (best != -1) {
        em->x3B0 = &player_work[best];
        em->x617 = best;
        *(f32 *)&em->x3AC = best_d;
        em->horm_ang = ((calc_vec_ang(px, pz, em->x3B0->pos[0], em->x3B0->pos[2]) & 0xFFFF) - 0x4000) & 0xFFFF;
    }
    return (u8)best;
}
#endif

#if defined(VERSION_MHG) && !defined(__MWERKS__)
void sub_main_func_004DC5F0(EMW *em, int part, int base);
#endif
#if defined(VERSION_MHG) && defined(__MWERKS__)
/* G sub_main 0x4DC5D0 / 0x4DC5F0 (PC copy in emsrch_nm.c). em_dur_init needs 0x4DC5F0 defined above it as a
 * static: the original keeps em and the loop values in t registers across the call */
#if !defined(G_PART)
void sub_main_func_004DC5D0(EMW *em, int part) {
    em->dur_lv_bits |= (u16)(1 << part);
}
#endif

#if !defined(G_PART)
static void sub_main_func_004DC5F0(EMW *em, int part, s16 base) {
    f32 r;

    if (!(em->dur_lv_bits & (1 << part))) {
        return;
    }
    if (em->lv < 9) {
        return;
    }
    r = (f32)(em->lv - 9) / 3.0f;
    base = base * (0.33f + 0.08f * r);
    ((s16 *)((u8 *)em + 0x308))[part * 4] += base;
}
#endif
#endif
/* Sets up the nine durability slots (EMW+0x304, 8 bytes each: on, no, 0, 0, hp s16, 0) from em_dur_tbl[type]. */
#if !defined(G_PART)
void em_dur_init(EMW *em) {
    int i;
    s16 *t;
    u8 *slot;

    i = 0;
    slot = (u8 *)em;
    t = em_dur_tbl[em->kind];
    do {
        if (*t > 0) {
            slot[0x304] = 1;
            slot[0x305] = i;
            slot[0x306] = 0;
            slot[0x307] = 0;
            *(s16 *)(slot + 0x308) = *t;
#if defined(VERSION_MHG)
            sub_main_func_004DC5F0(em, i, *t);      /* G (sub_main 0x4DC6A0): level bonus */
#endif
            slot[0x30A] = 0;
        } else {
            slot[0x304] = 0;
        }
        i++;
        t++;
        slot += 8;
    } while (i < 9);
    em->x953 = *t;
}
#endif

/* original bytes kept (2 instructions off as C, see emsrch_nm.c): build/raw/em_dur_set.inc, config/c_rawfuncs.txt */
#ifdef __MWERKS__   /* PS2 only; the PC build takes the near-match C (weak) */
#if !defined(G_PART)
asm void em_dur_set(EMW *em, u32 n)
{
#include "em_dur_set.inc"
}
#endif
#endif

#if !defined(G_PART) || G_PART == 2
void get_joint_pos(EMW *em, s16 joint, f32 *out) {
    flmatGetTrans(out, em->mdl->bone + joint * 0x190);
}
#endif

#if !defined(G_PART) || G_PART == 2
void get_joint_pos_em(EMW *em, s16 joint, f32 *out) {
    get_joint_pos(em, joint, out);
}
#endif

#if !defined(G_PART) || G_PART == 2
u8 *get_joint_wmat(EMW *em, s16 joint) {
    return em->mdl->bone + joint * 0x190;
}
#endif

#if !defined(G_PART) || G_PART == 2
u8 *get_joint_wmat_em(EMW *em, s16 joint) {
    return get_joint_wmat(em, joint);
}
#endif

#if !defined(G_PART) || G_PART == 2
u8 *get_joint_mat(EMW *em, s16 joint) {
    return em->mdl->bone + joint * 0x190 + 0x40;
}
#endif

#if !defined(G_PART) || G_PART == 2
u8 *get_joint_mat_em(EMW *em, s16 joint) {
    return get_joint_mat(em, joint);
}
#endif
