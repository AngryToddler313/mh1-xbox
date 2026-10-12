#ifndef IOREAD_H
#define IOREAD_H
/* Pad structures of f_ioread (src/main/sys/f_ioread.c): the fl library's per-pad data that ioRead_sub reads
 * (flpad_adr, filled by flPADGetALL on the PS2, by src/pc/rt/rt_pad.c on the PC/Xbox) and the game's pad
 * words Psw[n] it writes. Offsets are exact (ioRead_sub, 0x11FAC0); names are guesses. Added by agent C. */
#include "types.h"

/* PS2 pad data (flPADGetALL, 0x88 bytes per pad). */
typedef struct FPADI {
    u8 state;                   /* 0x00 1 = connected */
    u8 mode;                    /* 0x01 analog stick bits (0x30 left, 0x50 right) */
    u16 x02;                    /* 0x02 */
    u8 _pad04[4];
    u32 btn;                    /* 0x08 buttons; 0x10000.. stick directions */
    u8 _pad0C[0x30 - 0xC];
    s16 pow0;                   /* 0x30 left stick strength */
    s16 ang0;                   /* 0x32 left stick angle (degrees) */
    u8 _pad34[0x3C - 0x34];
    s16 pow1;                   /* 0x3C right stick strength */
    s16 ang1;                   /* 0x3E right stick angle (degrees) */
    u8 _pad40[0x88 - 0x40];
} FPADI;

/* Psw[n]: the game's pad word block per pad (0x22 bytes). */
typedef struct PSWE {
    u16 on;                     /* 0x00 held */
    u16 old;                    /* 0x02 held last frame */
    u16 trg;                    /* 0x04 pressed */
    u16 rel;                    /* 0x06 released */
    u16 an;                     /* 0x08 stick directions held */
    u16 aold;                   /* 0x0A */
    u16 atrg;                   /* 0x0C */
    u16 arel;                   /* 0x0E */
    s16 ang0;                   /* 0x10 left stick angle, 0x10000 = full turn */
    s16 ang1;                   /* 0x12 right stick angle */
    s16 pow0;                   /* 0x14 left stick strength */
    s16 pow1;                   /* 0x16 right stick strength */
    u16 rep;                    /* 0x18 repeat: held buttons every few frames */
    u16 rcnt;                   /* 0x1A repeat counter */
    s16 x1C;                    /* 0x1C */
    s16 x1E;                    /* 0x1E last x1C */
    s16 x20;                    /* 0x20 */
} PSWE;

extern u16 Psw[];

#endif
