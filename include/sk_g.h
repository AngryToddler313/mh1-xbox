#ifndef SK_G_H
#define SK_G_H
/* G's soft keyboard state (lpSKey) is 12 bytes bigger than MH1's: the 0x44 string moved +8, everything from 0x140 on +12
 * (offsets read off tools/check.py diffs of hk_*, cmd_delete, kbd_free_set, sk_conv_init). SKO(mh1_offset) = G offset. */
#ifdef VERSION_MHG
#define SKO(o) ((o) < 0x44 ? (o) : (o) < 0x140 ? (o) + 8 : (o) + 12)
#else
#define SKO(o) (o)
#endif
#endif
