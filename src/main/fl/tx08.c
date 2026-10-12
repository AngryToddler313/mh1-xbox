/* tx08 - flPS2GetTextureVramBlock (SLPM_654.95 0x00189FF0-0x0018A488): VRAM size of a texture in 256-byte blocks (pages of
 * 64x32 (4 bit), 64x64 (8 bit/CLUT), 128x64 (16 bit), 128x128 (32 bit) pixels, summed over the mip levels; each level is
 * half the size of the one before). */
#include "types.h"

typedef struct TEXH {
    int x0;             /* 0x00 */
    int x4;             /* 0x04 */
    int size;           /* 0x08 total byte size */
    int x0C;            /* 0x0C */
    s16 x10, x12, x14, x16, x18;
    s16 w;              /* 0x1A */
    s16 h;              /* 0x1C */
    u8 _pad1E[2];
    int pix;            /* 0x20 pixel storage format */
    int sysmem;         /* 0x24 system memory handle of the pixel data */
    s16 x28;            /* 0x28 */
    s16 x2A;            /* 0x2A */
    int x2C;            /* 0x2C */
    u32 mips;           /* 0x30 */
    s8 used;            /* 0x34 */
    s8 resident;        /* 0x35 */
    s8 x36;             /* 0x36 */
    s8 x37;             /* 0x37 */
} TEXH;

s16 flPS2GetTextureVramBlock(TEXH *t) {
    int pw;
    int ph;
    s16 w;
    s16 h;
    s16 n;
    u32 i;

    switch (t->pix) {
    case 0:
    case 1:
        pw = 0x40;
        ph = 0x20;
        break;
    case 2:
    case 0xA:
        pw = 0x40;
        ph = pw;
        break;
    case 0x13:
        pw = 0x80;
        ph = 0x40;
        break;
    case 0x14:
        pw = 0x80;
        ph = pw;
        break;
    }
    w = t->w;
    h = t->h;
    n = 0;
    for (i = 0; i < t->mips; i++) {
        n += (s16)(((w + (s16)pw - 1) / (s16)pw) * ((h + (s16)ph - 1) / (s16)ph) << 5);
        w >>= 1;
        h >>= 1;
    }
    return n;
}
