/* tx04 - texture handles 0x00189690-0x00189764: flUnlockTexture, flUnlockPalette. Whole file in tex_nm.c. */
#include "types.h"

typedef struct TEXH {
    int x0, x4;
    int size;                   /* 0x08 */
    u8 _pad0C[0x10 - 0xC];
    s16 x10, x12, x14, x16, x18;
    s16 w;                      /* 0x1A */
    s16 h;                      /* 0x1C */
    u8 _pad1E[0x20 - 0x1E];
    int x20;                    /* 0x20 pixel / palette format */
    int sysmem;                 /* 0x24 system memory handle of the pixel data */
    s16 x28;                    /* 0x28 */
    s16 x2A;                    /* 0x2A */
    u8 *x2C;                    /* 0x2C the locked (user) copy of the pixels */
    u32 mips;                   /* 0x30 */
    s8 used;                    /* 0x34 */
    s8 resident;                /* 0x35 */
    s8 x36;                     /* 0x36 */
    u8 x37;                     /* 0x37 lock mode (low 2 bits) */
} TEXH;

typedef struct FLCH {
    int bits;
    int shift;
    u32 mask;
} FLCH;

typedef struct PLCTX {
    u32 flags;                  /* 0x00 */
    int w;                      /* 0x04 */
    int h;                      /* 0x08 */
    int stride;                 /* 0x0C */
    u8 *base;                   /* 0x10 */
    int bpp;                    /* 0x14 */
    FLCH c[4];                  /* 0x18 R, G, B, A */
} PLCTX;

extern TEXH flTexture[0x100];
extern TEXH flPalette[0x100];
extern int flCTNum;
extern int flPTNum;

void flPS2DmaTerminate(void);
void flPS2DeleteVramList(TEXH *);
void flPS2ReleaseSystemMemory(int);
void flMemset(void *, int, int);
int flPS2LockTexture(int, TEXH *, int, int, int);
int flPS2UnlockTexture(TEXH *);
int flPS2DeleteAllVramList(void);
int flPS2ReloadTexture(int, int);
u8 *flPS2GetSystemBuffAdrs(int h);
u8 *mflTemporaryUse(int size);
void flMemcpy(u8 *, u8 *, u32);
void plConvertContext(PLCTX *, PLCTX *);
int flPS2VIF1CalcEndLoadImageSize(int size);
int flPS2VIF1CalcLoadImageSize(int size);
long flPS2GetSystemTmpBuff(int size, int align);
void flPS2VIF1MakeEndLoadImage(long p, int a);
long flPS2VIF1MakeLoadImage(long p, int a, int adrs, int size, s16 vram, long bw, long fmt, long z, long a0, long w, long h);
int flPS2DmaAddQueue2(int mode, unsigned long tag, long x, void *c);
extern u8 flPs2VIF1Control[];















#if !defined(G_PART) || G_PART == 1
int flUnlockTexture(u32 h) {
    TEXH *t = &flTexture[h - 1];

    if (h > 0x100) {
        return 0;
    }
    if (t->used != 0) {
        return flPS2UnlockTexture(t);
    }
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
int flUnlockPalette(u32 h) {
    TEXH *t = &flPalette[h - 1];

    if (h > 0x100) {
        return 0;
    }
    if (t->used != 0) {
        return flPS2UnlockTexture(t);
    }
    return 0;
}
#endif

/* Unlocks a texture: writes the locked copy (x2C) back to the texture's own memory in the texture's pixel layout
 * (lock mode 0 converts R/B order through plConvertContext, mode 1 copies, 2/3 use the locked copy in place) and,
 * unless the texture is of kind 1 or 4 (then only its VRAM entry is dropped), uploads it to VRAM again. */
/* near-match (4 instructions: the original copies the buffer pointer into its saved register inside both branches
 * of the sysmem test; here the copy sinks below the join), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
#if !defined(G_PART)
asm int flPS2UnlockTexture(TEXH *t)
{
#include "flPS2UnlockTexture.inc"
}
#endif
#else
int flPS2UnlockTexture(TEXH *t) {
    PLCTX s;
    PLCTX d;
    u8 *buf;
    u8 *src;
    long tmp;
    long r;

    buf = 0;
    switch (t->x37 & 3) {
    case 0:
        buf = t->sysmem != 0 ? flPS2GetSystemBuffAdrs(t->sysmem) : mflTemporaryUse(t->size);
        src = t->x2C;
        s.flags = t->x4;
        s.w = t->w;
        s.h = t->h;
        s.base = src;
        d.flags = t->x4;
        d.w = t->w;
        d.h = t->h;
        d.base = buf;
        switch (t->x20) {
        case 0x14:
        case 0x13:
            flMemcpy(buf, src, t->size);
            break;
        case 2:
            s.bpp = 2;
            s.c[0].bits = 5;
            s.c[0].shift = 10;
            s.c[0].mask = 0x1F;
            s.c[1].bits = 5;
            s.c[1].shift = 5;
            s.c[1].mask = 0x1F;
            s.c[2].bits = 5;
            s.c[2].shift = 0;
            s.c[2].mask = 0x1F;
            s.c[3].bits = 1;
            s.c[3].shift = 0xF;
            s.c[3].mask = 1;
            s.stride = s.w * s.bpp;
            d.bpp = 2;
            d.c[0].bits = 5;
            d.c[0].shift = 10;
            d.c[0].mask = 0x1F;
            d.c[1].bits = 5;
            d.c[1].shift = 5;
            d.c[1].mask = 0x1F;
            d.c[2].bits = 5;
            d.c[2].shift = 0;
            d.c[2].mask = 0x1F;
            d.c[3].bits = 1;
            d.c[3].shift = 0xF;
            d.c[3].mask = 1;
            d.c[0].shift = 0;
            d.c[2].shift = 10;
            d.c[1].bits = 5;
            d.c[1].mask = 0x1F;
            d.stride = d.w * d.bpp;
            plConvertContext(&d, &s);
            break;
        case 1:
            s.bpp = 3;
            s.c[0].bits = 8;
            s.c[0].shift = 0x10;
            s.c[0].mask = 0xFF;
            s.c[1].bits = 8;
            s.c[1].shift = 8;
            s.c[1].mask = 0xFF;
            s.c[2].bits = 8;
            s.c[2].shift = 0;
            s.c[2].mask = 0xFF;
            s.c[3].bits = 0;
            s.c[3].shift = 0;
            s.c[3].mask = 0;
            s.stride = s.w * s.bpp;
            d.bpp = 3;
            d.c[0].bits = 8;
            d.c[0].shift = 0x10;
            d.c[0].mask = 0xFF;
            d.c[1].bits = 8;
            d.c[1].shift = 8;
            d.c[1].mask = 0xFF;
            d.c[2].bits = 8;
            d.c[2].shift = 0;
            d.c[2].mask = 0xFF;
            d.c[3].bits = 0;
            d.c[3].shift = 0;
            d.c[3].mask = 0;
            d.c[0].shift = 0;
            d.c[2].shift = 0x10;
            d.stride = d.w * d.bpp;
            plConvertContext(&d, &s);
            break;
        case 0:
            s.bpp = 4;
            s.c[0].bits = 8;
            s.c[0].shift = 0x10;
            s.c[0].mask = 0xFF;
            s.c[1].bits = 8;
            s.c[1].shift = 8;
            s.c[1].mask = 0xFF;
            s.c[2].bits = 8;
            s.c[2].shift = 0;
            s.c[2].mask = 0xFF;
            s.c[3].bits = 8;
            s.c[3].shift = 0x18;
            s.c[3].mask = 0xFF;
            s.stride = s.w * s.bpp;
            d.bpp = 4;
            d.c[0].bits = 8;
            d.c[0].shift = 0x10;
            d.c[0].mask = 0xFF;
            d.c[1].bits = 8;
            d.c[1].shift = 8;
            d.c[1].mask = 0xFF;
            d.c[2].bits = 8;
            d.c[2].shift = 0;
            d.c[2].mask = 0xFF;
            d.c[3].bits = 8;
            d.c[3].shift = 0x18;
            d.c[3].mask = 0xFF;
            d.c[0].shift = 0;
            d.c[2].shift = 0x10;
            d.stride = d.w * d.bpp;
            plConvertContext(&d, &s);
            break;
        }
        break;
    case 1:
        if (t->sysmem != 0) {
            buf = flPS2GetSystemBuffAdrs(t->sysmem);
            flMemcpy(buf, t->x2C, t->size);
        } else {
            buf = t->x2C;
        }
        break;
    case 2:
        buf = t->x2C;
        break;
    case 3:
        buf = t->x2C;
        break;
    }
    t->x4 &= ~2;
    if (buf != 0) {
        if (t->x0 == 1 || t->x0 == 4) {
            flPS2DeleteVramList(t);
            return 1;
        }
        tmp = flPS2GetSystemTmpBuff(flPS2VIF1CalcLoadImageSize(t->size), 0x10);
        if (t->x36 == 0) {
            r = flPS2VIF1MakeLoadImage(tmp, 1, (int)buf, t->size, t->x14, (s16)(t->w / 64), (s16)t->x20, 0, 0, t->w, t->h);
        } else {
            r = flPS2VIF1MakeLoadImage(tmp, 1, (int)buf, t->size, t->x14, (s16)(t->x28 / 64), 0, 0, 0, t->x28, t->x2A);
        }
        flPS2DmaAddQueue2(0, (unsigned long)tmp & 0xFFFFFFFUL | 0x10000000, r, flPs2VIF1Control);
        tmp = flPS2GetSystemTmpBuff(flPS2VIF1CalcEndLoadImageSize(t->size), 0x10);
        flPS2VIF1MakeEndLoadImage(tmp, 1);
        flPS2DmaAddQueue2(0, (unsigned long)tmp & 0xFFFFFFFUL, tmp, flPs2VIF1Control);
    }
    return 1;
}
#endif
