#ifndef VERSION_H
#define VERSION_H
/* Per-version constants: MH1 JP (default) or PS2 Monster Hunter G (-DVERSION_MHG).
 * Only values that change C code live here; docs/DECISIONS.md "How G lives in the repo". */
#ifdef VERSION_MHG
#define ITEM_ROW 20 /* sizeof one Item_data row: G added a member */
#define EMW_STRIDE 0xA00 /* sizeof EMW (monster work): G is 8 + 8 bytes smaller */
#else
#define ITEM_ROW 16
#define EMW_STRIDE 0xA10
#endif
#endif
