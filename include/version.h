#ifndef VERSION_H
#define VERSION_H
/* Per-version constants: MH1 JP (default) or PS2 Monster Hunter G (-DVERSION_MHG).
 * Only values that change C code live here; docs/DECISIONS.md "How G lives in the repo". */
#ifdef VERSION_MHG
#define ITEM_ROW 20 /* sizeof one Item_data row: G added a member */
#else
#define ITEM_ROW 16
#endif
/* G: its Em_Sleep_End / Em_Sleep2_End symbols (map names) sit at swapped call targets in the sleep/paralysis code */
#ifdef VERSION_MHG
#define EM_SLEEP_END Em_Sleep2_End
#define EM_SLEEP2_END Em_Sleep_End
#else
#define EM_SLEEP_END Em_Sleep_End
#define EM_SLEEP2_END Em_Sleep2_End
#endif
#ifdef VERSION_MHG
#define STG_POS_N 92 /* stages in the per-stage position tables (MH1 88) */
#define MAHI_MAX 4
#else
#define STG_POS_N 88
#define MAHI_MAX 3
#endif
#endif
