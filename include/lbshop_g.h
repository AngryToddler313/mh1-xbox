#ifndef LBSHOP_G_H
#define LBSHOP_G_H
/* lbShop in PS2 Monster Hunter G (VERSION_MHG): the shop / forge / armour-shop state, re-laid out. The field names are
 * MH1's (include/lobby.h); the G offsets come from tools/mhg_addrmap.py votes over the twin functions
 * (`lobby 0x6EABF0 0x6EAC90`; MH1 0x6EABF0 = G 0x654840) and the per-function diffs (tools/mhg_fdiff.py). G's symbol lbShop
 * is 0x654834, 12 bytes before the first field. Included inside the struct-definition of lobby.h / lobby_s.h. */
typedef struct LB_SHOP {
    u8 _padG[0xC];      /* G 0x654834 (the symbol); the fields start at G 0x654840 */
    s16 pos[5][2];                     /* G +0x00 (from 0x654840) */
    void (*f20)();                     /* G +0x14 (from 0x654840) */
    void (*f24)();                     /* G +0x18 (from 0x654840) */
    void (*f28)();                     /* G +0x1C (from 0x654840) */
    int (*f2C)();                      /* G +0x20 (from 0x654840) */
    int (*f30)();                      /* G +0x24 (from 0x654840) */
    void (*f34)();                     /* G +0x28 (from 0x654840) */
    int (*f38)();                      /* G +0x2C (from 0x654840) */
    void (*f3C)();                     /* G +0x30 (from 0x654840) */
    void (*f40)();                     /* G +0x34 (from 0x654840) */
    void (*f44)(int, int, int, s16);   /* G +0x38 (from 0x654840) */
    s32 *tag;                          /* G +0x3C (from 0x654840) */
    s32 help;                          /* G +0x40 (from 0x654840) */
    void *list;                        /* G +0x44 (from 0x654840) */
    u8 x54[6];                         /* G +0x48 (from 0x654840) */
    u8 x5A[6];                         /* G +0x4E (from 0x654840) */
    s32 wait;                          /* G +0x54 (from 0x654840) */
    s32 *tbl;                          /* G +0x58 (from 0x654840) */
    u8 _pad60[0x4];
    s32 x70;                           /* G +0x60 (from 0x654840) */
    s32 cur;                           /* G +0x64 (from 0x654840) */
    s32 qty;                           /* G +0x68 (from 0x654840) */
    s32 count;                         /* G +0x6C (from 0x654840) */
    s32 x84;                           /* G +0x70 (from 0x654840) */
    u8 _pad78[0x4];
    s16 x6C;                           /* G +0x78 (from 0x654840) */
    s16 x6D;                           /* G +0x7A (from 0x654840) */
    s16 xG7C;                          /* G +0x7C (from 0x654840): cleared with the page by the sell list (lb_armor_tag_decide00); use unknown */
    s16 xG7E;                          /* G +0x7E (from 0x654840): number of detail pages of the list's item (3 or 4) */
    u16 key;                           /* G +0x80 (from 0x654840) */
    s8 step;                           /* G +0x82 (from 0x654840) */
    s8 x15;                            /* G +0x83 (from 0x654840) */
    s8 x16;                            /* G +0x84 (from 0x654840) */
    s8 x17;                            /* G +0x85 (from 0x654840) */
    s8 x18;                            /* G +0x86 (from 0x654840) */
    s8 mode;                           /* G +0x87 (from 0x654840) */
    s8 x1A;                            /* G +0x88 (from 0x654840) */
    s8 x6E;                            /* G +0x89 (from 0x654840) */
    s8 x78;                            /* G +0x8A (from 0x654840) */
    s8 x1B;                            /* G +0x8B (from 0x654840) */
    s8 x1C;                            /* G +0x8C (from 0x654840) */
    s8 x8E;                            /* G +0x8D (from 0x654840) */
    s8 x8F;                            /* G +0x8E (from 0x654840) */
    u8 _padEnd[0x1];
} LB_SHOP;
#endif
