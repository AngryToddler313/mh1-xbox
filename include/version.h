#ifndef VERSION_H
#define VERSION_H
/* Per-version constants: MH1 JP (default) or PS2 Monster Hunter G (-DVERSION_MHG).
 * Only values that change C code live here; docs/DECISIONS.md "How G lives in the repo". */
#ifdef VERSION_MHG
#define ITEM_ROW 20 /* sizeof one Item_data row: G added a member */
#define EMW_STRIDE 0xA00
#define IPREP_LIST 0xC80 /* known-recipe bit list: offset in User_data */
#define IPREP_NUM 108 /* recipe pairs in Item_preparation_tbl2 */ /* sizeof EMW (monster work): G is 8 + 8 bytes smaller */
#else
#define ITEM_ROW 16
#define EMW_STRIDE 0xA10
#define IPREP_LIST 0x3D8
#define IPREP_NUM 66
#endif
#endif
