/* NEAR-MATCH (not linked): flPS2ConvertContext, 171 of 222 instructions differ (register allocation and scheduling of the channel loops). */
/* tx09 - flPS2ConvertContext (SLPM_654.95 0x0018A920-0x0018AC88): converts the pixels of one texture context into another
 * (2/3/4 bytes per pixel, any channel layout: each channel is (pixel >> shift) & mask, scaled to 8 bits). clut = 1 with
 * dir 1 / 0 reads / writes the pixels in the GS CLUT order (clut_tbl_1242); for 32-bit sources the alpha is halved
 * (dir 0, 0xFF -> 0x80) or doubled (dir 1, 0x80 -> 0xFF) between PC and GS alpha. Field names are guesses. */
#include "types.h"

typedef struct FLCH {
    int bits;           /* 0x00 */
    int shift;          /* 0x04 */
    u32 mask;           /* 0x08 */
} FLCH;

typedef struct FLCTX {
    u32 flags;          /* 0x00 */
    int w;              /* 0x04 */
    int h;              /* 0x08 */
    int stride;         /* 0x0C */
    u8 *base;           /* 0x10 */
    int bpp;            /* 0x14 bytes per pixel */
    FLCH c[4];          /* 0x18 R, 0x24 G, 0x30 B, 0x3C A */
} FLCTX;

extern u8 clut_tbl_1242[];

int flPS2ConvertContext(FLCTX *s, FLCTX *d, int dir, int clut) {
    int off;
    int y;
    int k;
    int x;
    u8 *sp;
    u8 *dp;
    u32 v;
    u32 r;
    u32 g;
    u32 b;
    u32 a;
    u32 c;
    u8 *sb;
    u8 *db;

    off = 0;
    sb = s->base;
    db = d->base;
    k = 0;
    for (y = 0; y < d->h; y++) {
        for (x = 0; x < d->w; x++) {
            if (clut == 1 && dir == 1) {
                sp = sb + s->bpp * ((k & 0xE0) + clut_tbl_1242[k & 0x1F]);
            } else {
                sp = sb + off;
                off += s->bpp;
            }
            switch (s->bpp) {
            case 2:
                v = *(u16 *)sp;
                break;
            case 3:
                v = sp[0] | ((sp[2] << 16) | (sp[1] << 8));
                break;
            case 4:
                v = *(u32 *)sp;
                break;
            }
            r = (s->c[0].mask & (v >> s->c[0].shift)) << (8 - s->c[0].bits);
            g = (s->c[1].mask & (v >> s->c[1].shift)) << (8 - s->c[1].bits);
            b = (s->c[2].mask & (v >> s->c[2].shift)) << (8 - s->c[2].bits);
            a = (s->c[3].mask & (v >> s->c[3].shift)) << (8 - s->c[3].bits);
            if (clut == 1 && dir == 0) {
                dp = db + d->bpp * ((k & 0xE0) + clut_tbl_1242[k & 0x1F]);
            } else {
                dp = db + d->stride * y + d->bpp * x;
            }
            if (s->bpp == 4) {
                if (dir == 0) {
                    if (a == 0xFF) {
                        a = 0x80;
                    } else if (a != 0) {
                        a >>= 1;
                        if (a == 0) {
                            a = 1;
                        }
                    }
                } else if (a == 0x80) {
                    a = 0xFF;
                } else {
                    a <<= 1;
                }
            }
            c = ((d->c[3].mask & (a >> (8 - d->c[3].bits))) << d->c[3].shift)
              | (((d->c[2].mask & (b >> (8 - d->c[2].bits))) << d->c[2].shift)
              | (((d->c[0].mask & (r >> (8 - d->c[0].bits))) << d->c[0].shift)
              | ((d->c[1].mask & (g >> (8 - d->c[1].bits))) << d->c[1].shift)));
            switch (s->bpp) {
            case 2:
                *(u16 *)dp = c;
                break;
            case 3:
                dp[0] = r;
                dp[1] = g;
                dp[2] = b;
                break;
            case 4:
                *(u32 *)dp = c;
                break;
            }
            k++;
        }
    }
    return 1;
}
