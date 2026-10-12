/* tx03 - texture handles 0x00188B40-0x00189690: flLockTexture, flLockPalette, flPS2LockTexture. Whole file in tex_nm.c. */
#include "types.h"

typedef struct TEXH {
    int x0, x4;
    int size;                   /* 0x08 */
    u8 _pad0C[0x10 - 0xC];
    s16 x10, x12, x14, x16, x18;
    s16 w;                      /* 0x1A */
    s16 h;                      /* 0x1C */
    u8 _pad1E[0x20 - 0x1E];
    int x20;                    /* 0x20 palette format */
    int sysmem;                 /* 0x24 system memory handle of the pixel data */
    s16 x28;                    /* 0x28 */
    s16 x2A;                    /* 0x2A */
    u8 *x2C;                    /* 0x2C the locked (user) copy of the pixels */
    u32 mips;                   /* 0x30 */
    s8 used;                    /* 0x34 */
    s8 resident;                /* 0x35 */
    s8 x36;                     /* 0x36 */
    u8 x37;                     /* 0x37 lock mode */
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
int flPS2LockTexture();
u8 *flPS2GetSystemBuffAdrs(int h);
u8 *mflTemporaryUse(int size);
void flMemcpy(u8 *, u8 *, u32);
void plConvertContext(PLCTX *, PLCTX *);
int flPS2StoreImageB(u8 *p, int size, s16 vram, s16 bw, s16 fmt, long a, long b, s16 w, s16 h);
int flPS2UnlockTexture(TEXH *);
int flPS2DeleteAllVramList(void);
int flPS2ReloadTexture(int, int);















int flLockTexture(int a, u32 h, int b, int c) {
    TEXH *t = &flTexture[h - 1];

    if (h > 0x100) {
        return 0;
    }
    if (t->used != 0) {
        return flPS2LockTexture(a, t, b, c, 0);
    }
    return 0;
}

int flLockPalette(int a, u32 h, int *out, int c) {
    TEXH *t = &flPalette[h - 1];

    if (h > 0x100) {
        return 0;
    }
    if (t->used == 0) {
        return 0;
    }
    if (flPS2LockTexture(a, t, (int)out, c, 1) == 0) {
        return 0;
    }
    if (t->w == 0x10 && t->h == 0x10) {
        out[1] = 0x100;
        out[2] = 1;
    } else {
        out[1] = 0x10;
        out[2] = 1;
    }
    return 1;
}

/* Locks a texture for the CPU: fills the context c with the texture's layout and gives it a copy of the pixels
 * (x2C): mode 0 converts R/B order (from VRAM through flPS2StoreImageB when there is no system copy), mode 1 copies,
 * 2/3 hand out the memory itself. The context gets flag 2 (locked). */
int flPS2LockTexture(int a, TEXH *t, PLCTX *c, int mode, int kind) {
    PLCTX s;
    u8 *dst;
    u8 *src;

    t->x37 = mode;
    c->flags = t->x4;
    c->w = t->w;
    c->h = t->h;
    switch (mode & 3) {
    case 0:
        if (t->sysmem == 0) {
            src = mflTemporaryUse(t->size << 1);
            dst = src + t->size;
            flPS2StoreImageB(src, t->size, t->x14, (s16)(t->w / 64), t->x20, 0, 0, t->w, t->h);
        } else {
            dst = mflTemporaryUse(t->size);
            src = flPS2GetSystemBuffAdrs(t->sysmem);
        }
        t->x2C = dst;
        c->base = dst;
        s.flags = c->flags;
        s.w = c->w;
        s.h = c->h;
        s.base = src;
        switch (t->x20) {
        case 0x14:
            c->bpp = 0;
            c->stride = c->w / 2;
            flMemcpy(dst, src, t->size);
            break;
        case 0x13:
            c->bpp = 1;
            c->stride = c->w * c->bpp;
            flMemcpy(dst, src, t->size);
            break;
        case 2:
            c->bpp = 2;
            c->c[0].bits = 5;
            c->c[0].shift = 0xa;
            c->c[0].mask = 0x1f;
            c->c[1].bits = 5;
            c->c[1].shift = 5;
            c->c[1].mask = 0x1f;
            c->c[2].bits = 5;
            c->c[2].shift = 0;
            c->c[2].mask = 0x1f;
            c->c[3].bits = 1;
            c->c[3].shift = 0xf;
            c->c[3].mask = 1;
            c->stride = c->w * c->bpp;
            s.bpp = 2;
            s.c[0].bits = 5;
            s.c[0].shift = 0xa;
            s.c[0].mask = 0x1f;
            s.c[1].bits = 5;
            s.c[1].shift = 5;
            s.c[1].mask = 0x1f;
            s.c[2].bits = 5;
            s.c[2].shift = 0;
            s.c[2].mask = 0x1f;
            s.c[3].bits = 1;
            s.c[3].shift = 0xf;
            s.c[3].mask = 1;
            s.c[0].shift = 0;
            s.c[2].shift = 10;
            s.c[1].bits = 5;
            s.c[1].mask = 0x1F;
            s.stride = s.w * s.bpp;
            plConvertContext(c, &s);
            break;
        case 1:
            c->bpp = 3;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 0;
            c->c[3].shift = 0;
            c->c[3].mask = 0;
            c->stride = c->w * c->bpp;
            s.bpp = 3;
            s.c[0].bits = 8;
            s.c[0].shift = 0x10;
            s.c[0].mask = 0xff;
            s.c[1].bits = 8;
            s.c[1].shift = 8;
            s.c[1].mask = 0xff;
            s.c[2].bits = 8;
            s.c[2].shift = 0;
            s.c[2].mask = 0xff;
            s.c[3].bits = 0;
            s.c[3].shift = 0;
            s.c[3].mask = 0;
            s.c[0].shift = 0;
            s.c[2].shift = 0x10;
            s.stride = s.w * s.bpp;
            plConvertContext(c, &s);
            break;
        case 0:
            c->bpp = 4;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 8;
            c->c[3].shift = 0x18;
            c->c[3].mask = 0xff;
            c->stride = c->w * c->bpp;
            s.bpp = 4;
            s.c[0].bits = 8;
            s.c[0].shift = 0x10;
            s.c[0].mask = 0xff;
            s.c[1].bits = 8;
            s.c[1].shift = 8;
            s.c[1].mask = 0xff;
            s.c[2].bits = 8;
            s.c[2].shift = 0;
            s.c[2].mask = 0xff;
            s.c[3].bits = 8;
            s.c[3].shift = 0x18;
            s.c[3].mask = 0xff;
            s.c[0].shift = 0;
            s.c[2].shift = 0x10;
            s.stride = s.w * s.bpp;
            plConvertContext(c, &s);
            break;
        }
        break;
    case 1:
        dst = mflTemporaryUse(t->size);
        if (t->sysmem == 0) {
            flPS2StoreImageB(dst, t->size, t->x14, (s16)(t->w / 64), t->x20, 0, 0, t->w, t->h);
        } else {
            flMemcpy(dst, flPS2GetSystemBuffAdrs(t->sysmem), t->size);
        }
        t->x2C = dst;
        c->base = dst;
        switch (t->x20) {
        case 0x14:
            c->bpp = 0;
            c->stride = c->w / 2;
            break;
        case 0x13:
            c->bpp = 1;
            c->stride = c->w * c->bpp;
            break;
        case 2:
            c->bpp = 2;
            c->c[0].bits = 5;
            c->c[0].shift = 0xa;
            c->c[0].mask = 0x1f;
            c->c[1].bits = 5;
            c->c[1].shift = 5;
            c->c[1].mask = 0x1f;
            c->c[2].bits = 5;
            c->c[2].shift = 0;
            c->c[2].mask = 0x1f;
            c->c[3].bits = 1;
            c->c[3].shift = 0xf;
            c->c[3].mask = 1;
            c->c[0].shift = 0;
            c->c[2].shift = 10;
            c->c[1].bits = 5;
            c->c[1].mask = 0x1F;
            c->stride = c->w * c->bpp;
            break;
        case 1:
            c->bpp = 3;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 0;
            c->c[3].shift = 0;
            c->c[3].mask = 0;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        case 0:
            c->bpp = 4;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 8;
            c->c[3].shift = 0x18;
            c->c[3].mask = 0xff;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        }
        break;
    case 2:
        c->base = t->x2C = t->sysmem == 0 ? mflTemporaryUse(t->size) : flPS2GetSystemBuffAdrs(t->sysmem);
        switch (t->x20) {
        case 0x14:
            c->bpp = 0;
            c->stride = c->w / 2;
            break;
        case 0x13:
            c->bpp = 1;
            c->stride = c->w * c->bpp;
            break;
        case 2:
            c->bpp = 2;
            c->c[0].bits = 5;
            c->c[0].shift = 0xa;
            c->c[0].mask = 0x1f;
            c->c[1].bits = 5;
            c->c[1].shift = 5;
            c->c[1].mask = 0x1f;
            c->c[2].bits = 5;
            c->c[2].shift = 0;
            c->c[2].mask = 0x1f;
            c->c[3].bits = 1;
            c->c[3].shift = 0xf;
            c->c[3].mask = 1;
            c->c[0].shift = 0;
            c->c[2].shift = 10;
            c->c[1].bits = 5;
            c->c[1].mask = 0x1F;
            c->stride = c->w * c->bpp;
            break;
        case 1:
            c->bpp = 3;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 0;
            c->c[3].shift = 0;
            c->c[3].mask = 0;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        case 0:
            c->bpp = 4;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 8;
            c->c[3].shift = 0x18;
            c->c[3].mask = 0xff;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        }
        break;
    case 3:
        c->base = t->x2C = t->sysmem == 0 ? mflTemporaryUse(t->size) : flPS2GetSystemBuffAdrs(t->sysmem);
        switch (t->x20) {
        case 0x14:
            c->bpp = 0;
            c->stride = c->w / 2;
            break;
        case 0x13:
            c->bpp = 1;
            c->stride = c->w * c->bpp;
            break;
        case 2:
            c->bpp = 2;
            c->c[0].bits = 5;
            c->c[0].shift = 0xa;
            c->c[0].mask = 0x1f;
            c->c[1].bits = 5;
            c->c[1].shift = 5;
            c->c[1].mask = 0x1f;
            c->c[2].bits = 5;
            c->c[2].shift = 0;
            c->c[2].mask = 0x1f;
            c->c[3].bits = 1;
            c->c[3].shift = 0xf;
            c->c[3].mask = 1;
            c->c[0].shift = 0;
            c->c[2].shift = 10;
            c->c[1].bits = 5;
            c->c[1].mask = 0x1F;
            c->stride = c->w * c->bpp;
            break;
        case 1:
            c->bpp = 3;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 0;
            c->c[3].shift = 0;
            c->c[3].mask = 0;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        case 0:
            c->bpp = 4;
            c->c[0].bits = 8;
            c->c[0].shift = 0x10;
            c->c[0].mask = 0xff;
            c->c[1].bits = 8;
            c->c[1].shift = 8;
            c->c[1].mask = 0xff;
            c->c[2].bits = 8;
            c->c[2].shift = 0;
            c->c[2].mask = 0xff;
            c->c[3].bits = 8;
            c->c[3].shift = 0x18;
            c->c[3].mask = 0xff;
            c->c[0].shift = 0;
            c->c[2].shift = 0x10;
            c->stride = c->w * c->bpp;
            break;
        }
        break;
    }
    c->flags |= 2;
    return 1;
}
