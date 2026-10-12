/* tx10 - flPS2ConvertTextureFromContext (SLPM_654.95 0x0018A4F0-0x0018A914, file-static in the original): copies a texture
 * context (one FLCTX of 0x48 bytes per mip level) into the texture's system memory: 4/8-bit textures as they are (or swizzled
 * to 32 bit through flPS2Conv4_8_32 when x36 is set), 16/24/32-bit ones converted by flPS2ConvertContext into the GS layout
 * (A1B5G5R5, R8G8B8, A8B8G8R8: the destination channel shifts are set up as R..B and then swapped). Field names are guesses. */
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

extern char lit_1221_0035BFB0[];

int flPS2GetSystemMemoryHandle(int size, int a);
u8 *flPS2GetSystemBuffAdrs(int h);
void flMemcpy(u8 *, u8 *, u32);
u8 *mflTemporaryUse(int size);
void flLogOut();
int flPS2Conv4_8_32();
int flPS2ConvertContext(FLCTX *s, FLCTX *d, int dir, int clut);

int flPS2ConvertTextureFromContext(FLCTX *s, TEXH *t, int clut) {
    FLCTX ctx;
    int w;
    int h;
    u8 *buf;
    int size;
    u32 i;

    t->sysmem = flPS2GetSystemMemoryHandle(t->size, 2);
    buf = flPS2GetSystemBuffAdrs(t->sysmem);
    ctx.bpp = s->bpp;
    i = 0;
    ctx.flags = s->flags;
    w = t->w;
    h = t->h;
    if (i < t->mips) {
        do {
            ctx.w = w;
            ctx.h = h;
            ctx.stride = ctx.w * ctx.bpp;
            switch (t->pix) {
            default:
                flLogOut(lit_1221_0035BFB0);
                break;
            case 0x14:
                size = (w * h) >> 1;
                if (t->x36 == 0) {
                    flMemcpy(buf, s->base, t->size);
                } else {
                    ctx.base = mflTemporaryUse(size);
                    flMemcpy(ctx.base, s->base, size);
                    flPS2Conv4_8_32(w, h, ctx.base, buf, 0);
                }
                break;
            case 0x13:
                size = w * h;
                if (t->x36 == 0) {
                    flMemcpy(buf, s->base, t->size);
                } else {
                    ctx.base = mflTemporaryUse(size);
                    flMemcpy(ctx.base, s->base, size);
                    flPS2Conv4_8_32(w, h, ctx.base, buf, 1);
                }
                break;
            case 2:
                ctx.base = buf;
                size = w * h * 2;
                ctx.c[0].bits = 5;
                ctx.c[0].shift = 10;
                ctx.c[0].mask = 0x1F;
                ctx.c[1].bits = 5;
                ctx.c[1].shift = 5;
                ctx.c[1].mask = 0x1F;
                ctx.c[2].bits = 5;
                ctx.c[2].shift = 0;
                ctx.c[2].mask = 0x1F;
                ctx.c[3].bits = 1;
                ctx.c[3].shift = 0xF;
                ctx.c[3].mask = 1;
                ctx.c[0].shift = 0;
                ctx.c[2].shift = 10;
                ctx.c[1].bits = 5;
                ctx.c[1].mask = 0x1F;
                flPS2ConvertContext(s, &ctx, 0, clut);
                break;
            case 1:
                ctx.base = buf;
                size = w * h * 4;
                ctx.c[0].bits = 8;
                ctx.c[0].shift = 0x10;
                ctx.c[0].mask = 0xFF;
                ctx.c[1].bits = 8;
                ctx.c[1].shift = 8;
                ctx.c[1].mask = 0xFF;
                ctx.c[2].bits = 8;
                ctx.c[2].shift = 0;
                ctx.c[2].mask = 0xFF;
                ctx.c[3].bits = 0;
                ctx.c[3].shift = 0;
                ctx.c[3].mask = 0;
                ctx.c[0].shift = 0;
                ctx.c[2].shift = 0x10;
                flPS2ConvertContext(s, &ctx, 0, clut);
                break;
            case 0:
                ctx.base = buf;
                size = w * h * 4;
                ctx.c[0].bits = 8;
                ctx.c[0].shift = 0x10;
                ctx.c[0].mask = 0xFF;
                ctx.c[1].bits = 8;
                ctx.c[1].shift = 8;
                ctx.c[1].mask = 0xFF;
                ctx.c[2].bits = 8;
                ctx.c[2].shift = 0;
                ctx.c[2].mask = 0xFF;
                ctx.c[3].bits = 8;
                ctx.c[3].shift = 0x18;
                ctx.c[3].mask = 0xFF;
                ctx.c[0].shift = 0;
                ctx.c[2].shift = 0x10;
                flPS2ConvertContext(s, &ctx, 0, clut);
                break;
            }
            buf += size;
            w >>= 1;
            h >>= 1;
            i++;
            s++;
        } while (i < t->mips);
    }
    return 1;
}
