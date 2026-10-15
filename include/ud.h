#ifndef UD_H
#define UD_H
/* User data (saved hunter data, global User_data) as used by f_ud (src/main/ud). Offsets from matched code;
 * field names are guesses. Warehouse/equip helpers take a pointer to this struct (not always &User_data). */
#include "types.h"

typedef struct UD_ITEM { u16 id; s16 num; } UD_ITEM;
typedef struct UD_WARE { u8 use; u8 kind; u16 id; u16 opt; } UD_WARE; /* one stored equipment (6 bytes) */

typedef struct UDW {
    u8 _pad00[0x1];
    u8 x01;                  /* 0x001 (selects armor bit 1 or 2: sex, a guess) */
    u8 _pad02[0x1C - 0x2];
    u32 point;               /* 0x01C hunter points */
    s32 gold;                /* 0x020 gold (zeni), clamped to 0..9999998 by Gold_add */
    u16 evflag[0x10];        /* 0x024 event flags */
    UD_WARE ware[64];        /* 0x044 warehouse equipment */
#ifdef VERSION_MHG
    u8 _padG1[0x588];        /* G: 0x588 bytes of new data here; everything after sits later (lbsys_g.h UDO) */
#endif
    UD_ITEM stock[100];      /* 0x1C4 item box */
    u32 qclear[8];           /* 0x354 quest-clear bits */
    u32 x374;                /* 0x374 */
    u8 _pad378[0x3];
    u8 rank;                 /* 0x37B hunter rank */
#ifdef VERSION_MHG
    u8 _padG2[0x320];        /* G: pouch starts at 0xC24 (0x37C + 0x8A8) */
#endif
    UD_ITEM item[20];        /* 0x37C pouch */
    u8 _pad3CC[0x1];
    u8 wkind;                /* 0x3CD equipped weapon kind (7 = bowgun) */
    u16 wid;                 /* 0x3CE equipped weapon id */
    u16 wopt;                /* 0x3D0 equipped weapon option bits */
    u8 armor[5];             /* 0x3D2 leg, head, body, arm, waist */
    u8 _pad3D7[0x456 - 0x3D7];
#ifdef VERSION_MHG
    u16 widx[6];             /* G: u16 each, at 0xCFE */
#else
    u8 widx[6];              /* 0x456 warehouse index of weapon, leg, head, body, arm, waist (0xFF none) */
#endif
    u8 wyv_kill[2];          /* 0x45C wyvern kill counters (online) */
} UDW;
#ifdef VERSION_MHG
#define UD_SLOT 0xE00        /* G: saved hunter data per memory-card slot */
#else
#define UD_SLOT 0x480
#endif
#ifdef VERSION_MHG
typedef u16 UD_IDX;          /* G: warehouse indexes are 16 bits, 0xFFFF = none */
#define UD_NONE 0xFFFF
#else
typedef u8 UD_IDX;
#define UD_NONE 0xFF
#endif
extern UDW User_data[];
#endif
