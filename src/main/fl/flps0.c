/* fl PS2 2D primitives (f_flps0, SLPM_654.95 main 0x175120-0x177540): flPS2psAddQueue sends a packet built on the
 * scratchpad, flPS2CheckGSClip tests a screen point against the viewport, and the flps* handlers each build one GS
 * primitive (line, rectangle, gradient, sprite, triangle, ...) from a template packet on the scratchpad. */
#include "g_alias.h"
#ifdef MHG_PS2 /* G calls another function here (tools/check.py VERSION=mhg) */
#define flPS2_Mem_move16_16A func_00136088
#endif
#include "types.h"

typedef unsigned long u64;

extern u8 flPs2State[];
extern f32 flViewportCX, flViewportCY, flViewportLX, flViewportLY;
extern u8 flPs2VIF1Control[];
#define PS2S(T, o) (*(T *)(flPs2State + (o)))

extern u8 DrawLineF_data_279[];
extern u8 DrawSprF_data_335[];
extern u8 DrawSprG_data_354[];
extern u8 DrawSpr2TF_data_425[];
extern u8 DrawTriF_data_442[];
extern u8 DrawTriTF_data_514[];
extern u8 DrawSpr3DF_data_866[];
extern u8 DrawSpr3DTF_data_929[];
extern u8 DrawSprF_data_1023[];
extern u8 DrawSpr3DTF_data_1057[];
extern u8 DrawTri3DTG_data_1137[];
extern u8 flMATRIX[];
extern u8 flACRVIEWPROJ[];
typedef struct FMTX { f32 m[4][4]; } FMTX;
void flmatMul(void *, void *, void *);
void flvecApplyMatTrans(f32 *, f32 *, FMTX *);
typedef struct TEXH { s16 x[0x1C]; } TEXH;
extern TEXH flTexture[0x100];
extern u16 flCTH;
void flPS2CheckUV(TEXH *, s16 *, s16 *);

u64 flPS2GetSystemTmpBuff(int, int);
void flPS2_Mem_move16_16A();
void flPS2DmaAddQueue2(int, u64, u64, void *);
u32 flPS2ConvColor(u32, int);
int flPS2ConvScreenX(int);
int flPS2ConvScreenY(int);
f32 flPS2ConvScreenFX(f32);
f32 flPS2ConvScreenFY(f32);
f32 flPS2ConvScreenFZ(f32);

#define SET_RGBAQ(r, g, b, a, q) ((u64)(r) | ((u64)(g) << 8) | ((u64)(b) << 16) | ((u64)(a) << 24) | ((u64)(q) << 32))
#define SET_XYZ(x, y, z) ((u64)(x) | ((u64)(y) << 16) | ((u64)(z) << 32))

#if !defined(G_PART) || G_PART == 1
int flPS2psAddQueue(u32 *p) {
    int n;
    u64 buf;
    u64 t;

    n = (p[0] & 0x7FFF) + 1;
    buf = flPS2GetSystemTmpBuff(n * 16, 16);
    p[0] |= 0x80000000;
    p[2] = 0x13000000;
    p[3] = (n - 1) | 0x51000000;
    flPS2_Mem_move16_16A(p, buf, n);
    t = (buf & 0xFFFFFFFULL) | 0x40000000;
    flPS2DmaAddQueue2(0, t, buf, flPs2VIF1Control);
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
static int flPS2CheckGSClip(f32 x, f32 y, f32 z) {
    if (flViewportCX + flViewportLX < x) {
        return 0;
    }
    if (!(flViewportCX - flViewportLX <= x)) {
        return 0;
    }
    if (flViewportCY + flViewportLY < y) {
        return 0;
    }
    if (!(flViewportCY - flViewportLY <= y)) {
        return 0;
    }
    if (z < 0.0f) {
        return 0;
    }
    if (*(f32 *)(flPs2State + 0x44) < z) {
        return 0;
    }
    return 1;
}
#endif

typedef struct LINEF {
    s16 x0, y0, x1, y1;
    u32 col;
} LINEF;

#if !defined(G_PART) || G_PART == 1
int flps0002(LINEF *q) {
    u32 c;
    s16 x;
    s16 y;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawLineF_data_279, (void *)0x70000000, 3);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 1);
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, 1);
    x = flPS2ConvScreenX(q->x0);
    y = flPS2ConvScreenY(q->y0);
    if (!flPS2CheckGSClip((int)x, y, 1.0f)) {
        return 0;
    }
    *p++ = SET_XYZ((PS2S(int, 0x3A4) + x) << 4, (PS2S(int, 0x3A8) + y) << 4, (u32)PS2S(f32, 0x44));
    x = flPS2ConvScreenX(q->x1);
    y = flPS2ConvScreenY(q->y1);
    if (!flPS2CheckGSClip((int)x, y, 1.0f)) {
        return 0;
    }
    *p = SET_XYZ((PS2S(int, 0x3A4) + x) << 4, (PS2S(int, 0x3A8) + y) << 4, (u32)PS2S(f32, 0x44));
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

#if !defined(G_PART) || G_PART == 1
int flps0004(LINEF *q) {
    u32 c;
    s16 x;
    s16 y;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSprF_data_335, (void *)0x70000000, 3);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 1);
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, 1);
    x = flPS2ConvScreenX(q->x0);
    y = flPS2ConvScreenY(q->y0);
    *p++ = SET_XYZ((PS2S(int, 0x3A4) + x) << 4, (PS2S(int, 0x3A8) + y) << 4, (u32)PS2S(f32, 0x44));
    x = flPS2ConvScreenX(q->x1);
    y = flPS2ConvScreenY(q->y1);
    *p = SET_XYZ((PS2S(int, 0x3A4) + x) << 4, (PS2S(int, 0x3A8) + y) << 4, (u32)PS2S(f32, 0x44));
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct SPRG {
    s16 x0, y0, x1, y1;
    u32 col[4];
} SPRG;

#define RGBAQ_OF(c) SET_RGBAQ(((c) >> 16) & 0xFF, ((c) >> 8) & 0xFF, (c) & 0xFF, ((c) >> 24) & 0xFF, 1)
#define XYZ_OF(x, y) SET_XYZ((PS2S(int, 0x3A4) + (x)) << 4, (PS2S(int, 0x3A8) + (y)) << 4, (u32)PS2S(f32, 0x44))

#if !defined(G_PART) || G_PART == 1
int flps0005(SPRG *q) {
    u32 c;
    s16 x0;
    s16 y0;
    u64 *p;
    s16 y1;
    s16 x1;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSprG_data_354, (void *)0x70000000, 3);
    x0 = flPS2ConvScreenX(q->x0);
    x1 = flPS2ConvScreenX(q->x1);
    y0 = flPS2ConvScreenY(q->y0);
    y1 = flPS2ConvScreenY(q->y1);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col[0], 1);
    *p++ = RGBAQ_OF(c);
    *p++ = XYZ_OF(x0, y0);
    c = flPS2ConvColor(q->col[1], 1);
    *p++ = RGBAQ_OF(c);
    *p++ = XYZ_OF(x1, y0);
    c = flPS2ConvColor(q->col[2], 1);
    *p++ = RGBAQ_OF(c);
    *p++ = XYZ_OF(x0, y1);
    c = flPS2ConvColor(q->col[3], 1);
    *p++ = RGBAQ_OF(c);
    *p = XYZ_OF(x1, y1);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

#define SET_UV(u, v) ((u64)(u) | ((u64)(v) << 16))

typedef struct SPR2TF {
    s16 x, y, w, h;
    u32 col;
    s16 u0, v0, u1, v1;
} SPR2TF;

#if !defined(G_PART)
int flps0008(SPR2TF *q) {
    u32 c;
    s16 x;
    s16 y;
    TEXH *t;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSpr2TF_data_425, (void *)0x70000000, 3);
    t = &flTexture[flCTH - 1];
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 0);
    *p++ = RGBAQ_OF(c);
    flPS2CheckUV(t, &q->u0, &q->v0);
    *p++ = SET_UV(q->u0 << 4, q->v0 << 4);
    x = flPS2ConvScreenX(q->x);
    y = flPS2ConvScreenY(q->y);
    *p++ = XYZ_OF(x, y);
    flPS2CheckUV(t, &q->u1, &q->v1);
    *p++ = SET_UV(q->u1 << 4, q->v1 << 4);
    x = flPS2ConvScreenX((s16)(q->x + q->w));
    y = flPS2ConvScreenY((s16)(q->y + q->h));
    *p = XYZ_OF(x, y);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct TRIF {
    s16 x0, y0, x1, y1, x2, y2;
    u32 col;
} TRIF;

#if !defined(G_PART) || G_PART == 2
int flps0009(TRIF *q) {
    u32 c;
    s16 x;
    s16 y;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawTriF_data_442, (void *)0x70000000, 3);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 1);
    *p++ = RGBAQ_OF(c);
    x = flPS2ConvScreenX(q->x0);
    y = flPS2ConvScreenY(q->y0);
    *p++ = XYZ_OF(x, y);
    x = flPS2ConvScreenX(q->x1);
    y = flPS2ConvScreenY(q->y1);
    *p++ = XYZ_OF(x, y);
    x = flPS2ConvScreenX(q->x2);
    y = flPS2ConvScreenY(q->y2);
    *p = XYZ_OF(x, y);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct TRITF {
    s16 x0, y0, x1, y1, x2, y2;
    u32 col;
    s16 u0, v0, u1, v1, u2, v2;
} TRITF;

#if !defined(G_PART) || G_PART == 2
int flps000C(TRITF *q) {
    u32 c;
    s16 x;
    s16 y;
    TEXH *t;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawTriTF_data_514, (void *)0x70000000, 3);
    t = &flTexture[flCTH - 1];
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 0);
    *p++ = RGBAQ_OF(c);
    flPS2CheckUV(t, &q->u0, &q->v0);
    *p++ = SET_UV(q->u0 << 4, q->v0 << 4);
    x = flPS2ConvScreenX(q->x0);
    y = flPS2ConvScreenY(q->y0);
    *p++ = XYZ_OF(x, y);
    flPS2CheckUV(t, &q->u1, &q->v1);
    *p++ = SET_UV(q->u1 << 4, q->v1 << 4);
    x = flPS2ConvScreenX(q->x1);
    y = flPS2ConvScreenY(q->y1);
    *p++ = XYZ_OF(x, y);
    flPS2CheckUV(t, &q->u2, &q->v2);
    *p++ = SET_UV(q->u2 << 4, q->v2 << 4);
    x = flPS2ConvScreenX(q->x2);
    y = flPS2ConvScreenY(q->y2);
    *p = XYZ_OF(x, y);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

#define XYZF_OF(x, y, z) SET_XYZ((int)((f32)PS2S(int, 0x3A4) + (x)) << 4, (int)((f32)PS2S(int, 0x3A8) + (y)) << 4, (u32)(z))

typedef struct SPR3DF {
    f32 x0, y0, z0;
    u32 q0;
    f32 x1, y1, z1;
    u32 q1;
    u32 col;
} SPR3DF;

#if !defined(G_PART) || G_PART == 2
int flps0D00(SPR3DF *q) {
    u32 c;
    f32 x;
    f32 y;
    f32 z;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSpr3DF_data_866, (void *)0x70000000, 3);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 1);
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, q->q0);
    x = flPS2ConvScreenFX(q->x0);
    y = flPS2ConvScreenFY(q->y0);
    z = flPS2ConvScreenFZ(q->z0);
    *p++ = XYZF_OF(x, y, z);
    x = flPS2ConvScreenFX(q->x1);
    y = flPS2ConvScreenFY(q->y1);
    z = flPS2ConvScreenFZ(q->z1);
    *p = XYZF_OF(x, y, z);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

#define SET_ST(s, t) ((u64)(s) | ((u64)(t) << 32))

typedef struct SPR3DTF {
    f32 x0, y0, z0, q0;
    f32 x1, y1, z1, q1;
    u32 col;
    f32 s0, t0, s1, t1;
} SPR3DTF;

#if !defined(G_PART) || G_PART == 2
int flps0F00(SPR3DTF *q) {
    u32 c;
    f32 x;
    f32 y;
    f32 z;
    f32 st;
    f32 tt;
    u64 *p;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSpr3DTF_data_929, (void *)0x70000000, 3);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 0);
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&q->q0);
    st = q->s0 * q->q0;
    tt = q->t0 * q->q0;
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = q->x0 - (f32)PS2S(int, 0x3AC);
    if (PS2S(int, 4) == 0) {
        y = 0.5f * (q->y0 - (f32)PS2S(int, 0x3B0));
    } else {
        y = q->y0 - (f32)PS2S(int, 0x3B0);
    }
    z = -0.5f * (q->z0 - 1.0f) * PS2S(f32, 0x44);
    *p++ = XYZF_OF(x, y, z);
    st = q->s1 * q->q1;
    tt = q->t1 * q->q1;
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = q->x1 - (f32)PS2S(int, 0x3AC);
    if (PS2S(int, 4) == 0) {
        y = 0.5f * (q->y1 - (f32)PS2S(int, 0x3B0));
    } else {
        y = q->y1 - (f32)PS2S(int, 0x3B0);
    }
    z = -0.5f * (q->z1 - 1.0f) * PS2S(f32, 0x44);
    *p = XYZF_OF(x, y, z);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct SPR3DM {
    f32 v0[3];
    f32 v1[3];
    u32 col;
} SPR3DM;

#if !defined(G_PART) || G_PART == 2
int flps1300(SPR3DM *q) {
    u32 c;
    f32 x;
    f32 y;
    f32 z;
    u64 *p;
    f32 v[4];
    FMTX m;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSprF_data_1023, (void *)0x70000000, 3);
    flmatMul(&m, flMATRIX, flMATRIX + 0x840);
    flmatMul(&m, &m, flACRVIEWPROJ);
    p = (u64 *)0x70000028;
    flvecApplyMatTrans(v, q->v0, &m);
    v[3] = 1.0f / v[3];
    c = flPS2ConvColor(q->col, 1);
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&v[3]);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p++ = XYZF_OF(x, y, z);
    flvecApplyMatTrans(v, q->v1, &m);
    v[3] = 1.0f / v[3];
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p = XYZF_OF(x, y, z);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct SPR3DMT {
    f32 v0[3];
    f32 v1[3];
    u32 col;
    f32 s0, t0, s1, t1;
} SPR3DMT;

#if !defined(G_PART) || G_PART == 2
int flps1400(SPR3DMT *q) {
    u32 c;
    f32 x;
    f32 y;
    f32 z;
    f32 st;
    f32 tt;
    u64 *p;
    f32 v[4];
    FMTX m;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawSpr3DTF_data_1057, (void *)0x70000000, 3);
    flmatMul(&m, flMATRIX, flMATRIX + 0x840);
    flmatMul(&m, &m, flACRVIEWPROJ);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col, 0);
    flvecApplyMatTrans(v, q->v0, &m);
    v[3] = 1.0f / v[3];
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&v[3]);
    st = q->s0 * v[3];
    tt = q->t0 * v[3];
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p++ = XYZF_OF(x, y, z);
    flvecApplyMatTrans(v, q->v1, &m);
    v[3] = 1.0f / v[3];
    st = q->s1 * v[3];
    tt = q->t1 * v[3];
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p = XYZF_OF(x, y, z);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif

typedef struct TRI3DTG {
    f32 v0[3];
    f32 v1[3];
    f32 v2[3];
    u32 col[3];
    f32 st[3][2];
} TRI3DTG;

#if !defined(G_PART) || G_PART == 2
int flps1600(TRI3DTG *q) {
    u32 c;
    f32 x;
    f32 y;
    f32 z;
    f32 st;
    f32 tt;
    u64 *p;
    f32 v[4];
    FMTX m;

    if (PS2S(int, 0x3F4) != 0) {
        return 1;
    }
    flPS2_Mem_move16_16A(DrawTri3DTG_data_1137, (void *)0x70000000, 3);
    flmatMul(&m, flMATRIX, flMATRIX + 0x840);
    flmatMul(&m, &m, flACRVIEWPROJ);
    p = (u64 *)0x70000028;
    c = flPS2ConvColor(q->col[0], 0);
    flvecApplyMatTrans(v, q->v0, &m);
    v[3] = 1.0f / v[3];
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&v[3]);
    st = q->st[0][0] * v[3];
    tt = q->st[0][1] * v[3];
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p++ = XYZF_OF(x, y, z);
    c = flPS2ConvColor(q->col[1], 0);
    flvecApplyMatTrans(v, q->v1, &m);
    v[3] = 1.0f / v[3];
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&v[3]);
    st = q->st[1][0] * v[3];
    tt = q->st[1][1] * v[3];
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p++ = XYZF_OF(x, y, z);
    c = flPS2ConvColor(q->col[2], 0);
    flvecApplyMatTrans(v, q->v2, &m);
    v[3] = 1.0f / v[3];
    *p++ = SET_RGBAQ((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF, *(u32 *)&v[3]);
    st = q->st[2][0] * v[3];
    tt = q->st[2][1] * v[3];
    *p++ = SET_ST(*(u32 *)&st, *(u32 *)&tt);
    x = flPS2ConvScreenFX(v[0]);
    y = flPS2ConvScreenFY(v[1]);
    z = flPS2ConvScreenFZ(v[2]);
    *p = XYZF_OF(x, y, z);
    flPS2psAddQueue((u32 *)0x70000000);
    return 1;
}
#endif
