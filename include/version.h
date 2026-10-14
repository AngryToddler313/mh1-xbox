#ifndef VERSION_H
#define VERSION_H
/* Per-version constants: MH1 JP (default) or PS2 Monster Hunter G (-DVERSION_MHG).
 * Only values that change C code live here; docs/DECISIONS.md "How G lives in the repo". */
#ifdef VERSION_MHG
#define ITEM_ROW 20 /* sizeof one Item_data row: G added a member */
#else
#define ITEM_ROW 16
#endif
#endif
