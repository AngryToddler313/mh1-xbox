#ifndef LBSYS_G_H
#define LBSYS_G_H
/* lb_sys in PS2 Monster Hunter G (VERSION_MHG). G re-laid it out: the fields of MH1's 0x90-byte lb_sys sit at other
 * offsets of a 0x98-byte object at 0x6548F0, plus a few G-only ones. Field names are MH1's. The map was voted from the
 * twin functions with tools/mhg_addrmap.py (`lobby 0x6EAE50 0x6EAEE0`); LBSYS_OFS(mh1_offset) gives the G offset for
 * code that addresses lb_sys by a raw byte offset. In an MH1 build LBSYS_OFS is the identity. */
#include "types.h"

#ifdef VERSION_MHG
#define LBSYS_OFS(o) ((o) < 0x64 ? (o) + 0x16 : (o) == 0x64 ? 0x14 : (o) == 0x66 ? 0x7A : (o) == 0x68 ? 0 : (o) == 0x6C ? 8 : \
    (o) < 0x74 ? (o) - 0x70 + 0x7B : (o) == 0x74 ? 0x10 : (o) == 0x76 ? 0x12 : (o) == 0x78 ? 0x7F : (o) == 0x7C ? 0x80 : \
    (o) == 0x80 ? 0xC : (o) < 0x87 ? (o) : (o) < 0x8D ? (o) + 1 : (o) + 2)

/* pNet (the net window state, network_work) and cw (client work): MH1 offset -> G offset, from tools/mhg_addrmap.py
 * --ptr pNet / --ptr cw (votes) and G's code; unknown offsets stay as they are. */
#define PNET_OFS(o) ((o) < 0x6 ? (o) + 0x20 : (o) <= 0x10 ? (o) + 0x21 : (o) < 0x14 ? (o) + 0x20 : (o) < 0x24 ? (o) - 0x14 : \
    (o) < 0x30 ? (o) - 8 : (o))
#define CW_OFS(o) ((o) < 0x2BFE ? (o) : (o) < 0x2C06 ? (o) - 0x2BFE + 0xB64 : (o) < 0x2C2F ? (o) - 0x2C06 + 0xB6E : \
    (o) < 0x2C3C ? (o) - 0x2C2F + 0xB97 : (o) < 0x2C43 ? (o) - 0x2C3C + 0xBA4 : (o) < 0x2C46 ? (o) - 0x2C43 + 0xBA7 : \
    (o) < 0x2C4C ? (o) - 0x2C46 + 0xBB0 : (o) < 0x2C70 ? (o) - 0x2C4C + 0xBB4 : \
    ((o) >= 0x2F6E && (o) < 0x2F9A) ? (o) - 0x2F6E + 0xED6 : ((o) >= 0x35D0 && (o) < 0x35E0) ? (o) - 0x35D2 + 0xFA5E : (o))

typedef struct LBSYS_G {       /* at 0x6548F0, 0x98 bytes; comments: G offset (MH1 name) */
    s32 x68;                   /* 0x00 talk/event mode, the exit code Local_main tests (0x22 = quest, 0x27 = title) */
    s32 gx04;                  /* 0x04 G only */
    s32 x6C;                   /* 0x08 */
    s32 x80;                   /* 0x0C */
    s16 x74;                   /* 0x10 */
    u16 x76;                   /* 0x12 */
    union { u16 x64; u16 chair_mask; };   /* 0x14 */
    u8 _pad00;                 /* 0x16 */
    s8 x01;                    /* 0x17 */
    s8 x02;                    /* 0x18 */
    s8 x03;                    /* 0x19 mode (index into vs_square_func_213) */
    s8 x04;                    /* 0x1A */
    s8 x05;                    /* 0x1B */
    s8 x06;                    /* 0x1C */
    union { s8 x07; s8 step; };           /* 0x1D */
    s8 x08;                    /* 0x1E */
    u8 _pad09;                 /* 0x1F */
    union { s8 x0A; u8 _s0[0x1E]; };      /* 0x20 hint string 0 (MH1 0x0A) */
    union { s8 x28; u8 _s1[0x5A - 0x1E]; };   /* 0x3E hint string 1 (MH1 0x28); the strings run on to 0x7A */
    u8 x66;                    /* 0x7A chair number (a byte in G's code) */
    u8 x70;                    /* 0x7B */
    s8 x71;                    /* 0x7C */
    s8 x72;                    /* 0x7D */
    union { u8 x73; u8 _pad73; };   /* 0x7E */
    u8 x78;                    /* 0x7F */
    s32 x7C;                   /* 0x80 */
    s8 x84;                    /* 0x84 */
    s8 x85;                    /* 0x85 */
    s8 x86;                    /* 0x86 */
    u8 _pad87;                 /* 0x87 */
    u8 x87;                    /* 0x88 */
    union { u8 x88[8]; struct { u8 _p[6]; u8 x8D; u8 x8E; }; };   /* 0x89 */
    u8 _pad91[0x98 - 0x91];
} LBSYS_G;
#else
#define LBSYS_OFS(o) (o)
#define PNET_OFS(o) (o)
#define CW_OFS(o) (o)
#endif
#endif
