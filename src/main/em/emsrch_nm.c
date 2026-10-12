/* emsrch_nm - near-match C of em_dur_set (SLPM_654.95 0x0010A0F0), the one function of emsrch.c
   (0x00109E30-0x0010A208) that is kept as original bytes on the PS2 (config/c_rawfuncs.txt).
   2 of 22 instructions differ: the final table address goes to v1 here, the original reuses a0
   (tried: index/pointer spellings, u32/int n, inline helpers, a 10-minute permuter run).
   The PC build links it (tools/build_pc.sh PICK_X). */
#include "types.h"
#include "em.h"

extern s16 *em_dur_tbl[];       /* per monster type: 9 hit-point values (s16), then the 0x953 value */

/* Refills durability slot n (EMW+0x304 + 8n: on flag; +0x308 hit points) from em_dur_tbl[kind][n],
   if the slot is on. */
void em_dur_set(EMW *em, u32 n) {
    u8 *slot;

    if (n >= 9 || (slot = (u8 *)(n * 8) + (int)em, slot[0x304] == 0)) {
        return;
    }
    *(s16 *)(slot + 0x308) = *(s16 *)((int)em_dur_tbl[em->kind] + n * 2);
}
