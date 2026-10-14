#ifndef ITEMPREP_H
#define ITEMPREP_H
/* Item preparation (mix recipes): one recipe of Item_preparation_tbl_00. G recipes are 8 bytes (MH1 6, padded to 6). */
#include "version.h"
#include "types.h"

/* one recipe: ingredient b (a is the table index), result item, list bit */
typedef struct IPREP {
    s16 a;                  /* 0x00 other ingredient */
    s16 res;                /* 0x02 result item */
#ifdef VERSION_MHG
    s8 x04;                 /* 0x04 rate table index */
    s8 x05;                 /* 0x05 */
    s8 bit;                 /* 0x06 bit of the known-recipe list (G: moved from 0x05) */
    u8 flags;               /* 0x07 bit 0: needs a mixing skill or the manual (func_0010DD20) */
#else
    s8 x04;                 /* 0x04 rate table index */
    s8 bit;                 /* 0x05 bit of the known-recipe list */
#endif
} IPREP;
#endif
