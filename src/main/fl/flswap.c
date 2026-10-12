/* flPS2SwapDBuff (SLPM_654.95 0x0018CF00-0x0018D5EC, the fl system file with flPS2InitRenderBuff): sends the double
 * buffer packet of frame n (flPs2Db, 0xA0 bytes each; in field mode its XYOFFSET is moved half a line on odd
 * fields), waits for the DMA and sets the display registers (SMODE2, PMODE, DISPFB1/2, DISPLAY1/2) for the video
 * mode in flPs2State+4 (0, 1 interlaced, 2). */
#include "types.h"

typedef unsigned long u64;

extern u8 flPs2State[];
#define PS2S(T, o) (*(T *)(flPs2State + (o)))
extern u8 flPs2Db[];
extern int flWidth;

void FlushCache(int);
void iFlushCache(int);
void sceDmaSend(int, void *);
int sceDmaSync(int, int, int);

#define GSREG(a) (*(volatile u64 *)(a))
#define GS_PMODE GSREG(0x12000000)
#define GS_SMODE2 GSREG(0x12000020)
#define GS_DISPFB1 GSREG(0x12000070)
#define GS_DISPLAY1 GSREG(0x12000080)
#define GS_DISPFB2 GSREG(0x12000090)
#define GS_DISPLAY2 GSREG(0x120000A0)
#define GS_EXTWRITE GSREG(0x120000D0)

#define SET_XYOFFSET(x, y) ((u64)(x) | ((u64)(y) << 32))
#define SET_DISPFB(fbp, fbw, psm, dbx, dby) \
    ((u64)(fbp) | ((long)(fbw) << 9) | ((u64)(psm) << 15) | ((u64)(dbx) << 32) | ((u64)(dby) << 43))
#define SET_DISPLAY(dx, dy, magh, magv, dw, dh) \
    ((long)(dx) | ((u64)(dy) << 12) | ((u64)(magh) << 23) | ((u64)(magv) << 27) | ((u64)(dw) << 32) | ((u64)(dh) << 44))

void flPS2SwapDBuff(int n, int intr) {
    u8 *db;
    int dx;
    int dy;

    db = flPs2Db + n * 0xA0;
    if (PS2S(int, 4) == 1) {
        if (PS2S(int, 0x4C) == 1) {
            *(u64 *)(db + 0x60) = SET_XYOFFSET(PS2S(int, 0x3A4) << 4, PS2S(int, 0x3A8) << 4);
            *(u64 *)(db + 0x70) = SET_XYOFFSET(PS2S(int, 0x3A4) << 4, PS2S(int, 0x3A8) << 4);
        } else {
            *(u64 *)(db + 0x60) = SET_XYOFFSET(PS2S(int, 0x3A4) << 4, (PS2S(int, 0x3A8) << 4) + 8);
            *(u64 *)(db + 0x70) = SET_XYOFFSET(PS2S(int, 0x3A4) << 4, (PS2S(int, 0x3A8) << 4) + 8);
        }
    }
    if (intr == 0) {
        iFlushCache(0);     /* from the vsync interrupt */
    } else {
        FlushCache(0);
    }
    sceDmaSend(PS2S(int, 0x3CC), db);
    while (sceDmaSync(PS2S(int, 0x3CC), 1, 0) != 0) {
    }
    dx = PS2S(int, 0x3B4) + PS2S(u32, 0x3BC) * PS2S(u32, 0x14);
    dy = PS2S(int, 0x3B8) + PS2S(int, 0x3C0);
    switch (PS2S(int, 4)) {
    case 0:
        GS_SMODE2 = 3;
        GS_PMODE = 0x66;
        if (n == 0) {
            GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x28) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
        } else {
            GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x2C) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
        }
        GS_DISPLAY2 = SET_DISPLAY(dx, dy, PS2S(u32, 0x14), 0, 0x9FF, PS2S(int, 0x10) - 1);
        break;
    case 1:
        if (PS2S(int, 8) == 0) {
            GS_SMODE2 = 1;
            GS_PMODE = 0x7F27;
            if (n == 0) {
                GS_DISPFB1 = SET_DISPFB(PS2S(u32, 0x28) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
                GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x28) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
            } else {
                GS_DISPFB1 = SET_DISPFB(PS2S(u32, 0x2C) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
                GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x2C) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
            }
            GS_DISPLAY1 = SET_DISPLAY(dx, dy + 1, PS2S(u32, 0x14), 0, 0x9FF, PS2S(int, 0x10) - 1);
            GS_DISPLAY2 = SET_DISPLAY(dx, dy, PS2S(u32, 0x14), 0, 0x9FF, PS2S(int, 0x10) - 1);
        } else {
            GS_SMODE2 = 1;
            GS_PMODE = 0x66;
            if (n == 0) {
                GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x28) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
            } else {
                GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x2C) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
            }
            GS_DISPLAY2 = SET_DISPLAY(dx, dy, PS2S(u32, 0x14), 0, 0x9FF, PS2S(int, 0x10) - 1);
        }
        break;
    case 2:
        GS_SMODE2 = 0;
        GS_PMODE = 0x66;
        if (n == 0) {
            GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x28) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
        } else {
            GS_DISPFB2 = SET_DISPFB(PS2S(u32, 0x2C) >> 5, flWidth / 64, PS2S(u32, 0x1C), 0, 0);
        }
        GS_DISPLAY2 = SET_DISPLAY(dx, dy, PS2S(u32, 0x14), 0, 0x4FF, PS2S(int, 0x10) - 1);
        break;
    }
    GS_EXTWRITE = 0;
}
