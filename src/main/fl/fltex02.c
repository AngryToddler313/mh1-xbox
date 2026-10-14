/* fl texture palette conversion: flPS2Conv4_8_32 and its file-static workers (4/8 bit swizzled pages to 32 bit). SLPM_654.95 0x0018AC90-0x0018B3A4.
 * Conv4to32 / Conv8to32 copy a 128x128 (4 bit) or 128x64 (8 bit) page worth of rows into a stack buffer and hand it to PageConv*, which walks the
 * block order table (block_table4/8) and calls BlockConv* for each 16 byte wide block through the idx32_h / idx32_v position tables. */
#include "types.h"

extern int block_table4[32];
extern int block_table8[32];
extern int idx32_h[32];
extern int idx32_v[32];
typedef struct CLMP { int a, b; } CLMP;
extern CLMP *clm_tbl4_ptr[4];
extern CLMP *clm_tbl8_ptr[4];

void *memcpy(void *, const void *, int);

static void Conv4to32();
static void Conv8to32();
static void PageConv4to32(u8 *buf, u8 *dst, int bw);
static void PageConv8to32(u8 *buf, u8 *dst, int bw);
static void BlockConv4to32(u8 *src, u8 *dst, int bw);
static void BlockConv8to32(u8 *src, u8 *dst, int bw);

#if !defined(G_PART) || G_PART == 1
int flPS2Conv4_8_32(a, b, c, d, mode)
int a;
int b;
int c;
int d;
int mode;
{
    switch (mode) {
    case 0:
        Conv4to32();
        return 0;
    case 1:
        Conv8to32();
        return 0;
    default:
        return -1;
    }
}
#endif

/* near-match (register choice: d and the row stride swap, 28 lines), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
#if !defined(G_PART)
static asm void Conv4to32()
{
#include "Conv4to32.inc"
}
#endif
#else
static void Conv4to32(w, h, src, dst)
int w;
int h;
u8 *src;
u8 *dst;
{
    u8 buf[0x2000];
    int bw;
    int bh;
    int i;
    u8 *fp;
    u8 *d;
    int j;
    u8 *s;
    u8 *b;
    int k;

    bw = (w - 1) / 128 + 1;
    bh = (h - 1) / 128 + 1;
    for (i = 0; i < bh; i++) {
        fp = src + bw * (i * 0x2000);
        d = dst + bw * (i * 0x2000);
        for (j = 0; j < bw; j++) {
            s = fp;
            b = buf;
            for (k = 0; k < 0x80; k++) {
                memcpy(b, s, 0x40);
                s += bw << 6;
                b += 0x40;
            }
            PageConv4to32(buf, d, bw);
            fp += 0x40;
            d += 0x100;
        }
    }
}
#endif

#if !defined(G_PART) || G_PART == 2
static void PageConv4to32(u8 *buf, u8 *dst, int bw) {
    
    int by;
    int bx;
    int t;
    int *tp;

    for (tp = block_table4, by = 0; by < 8; by++) {
        for (bx = 0; bx < 4; bx++) {
            t = *tp++;
            BlockConv4to32(buf + ((by * 16) << 6) + bx * 16, dst + bw * (idx32_v[t] << 11) + (idx32_h[t] << 5), bw);
        }
    }
}
#endif

/* near-match (nibble select order and registers), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
#if !defined(G_PART)
static asm void BlockConv4to32(u8 *src, u8 *dst, int bw)
{
#include "BlockConv4to32.inc"
}
#endif
#else
static void BlockConv4to32(u8 *src, u8 *dst, int bw) {
    CLMP **tp = clm_tbl4_ptr;
    CLMP *e;
    int a;
    int b;
    int x;
    int lo;
    int hi;
    int n;

    for (a = 0; a < 4; a++) {
        e = *tp++;
        for (b = 0; b < 2; b++) {
            for (x = 0; x < 0x20; x++) {
                n = e->a;
                if (n & 1) {
                    lo = (src[n / 2] & 0xF0) >> 4;
                } else {
                    lo = src[n / 2] & 0xF;
                }
                n = e->b;
                e++;
                if (n & 1) {
                    hi = src[n / 2] & 0xF0;
                } else {
                    hi = (src[n / 2] & 0xF) * 0x10;
                }
                *dst++ = (lo & 0xFF) | (hi & 0xFF);
            }
            dst += (bw << 8) - 0x20;
        }
        src += 0x100;
    }
}
#endif

/* near-match (register choice, as Conv4to32), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
#if !defined(G_PART)
static asm void Conv8to32()
{
#include "Conv8to32.inc"
}
#endif
#else
static void Conv8to32(w, h, src, dst)
int w;
int h;
u8 *src;
u8 *dst;
{
    u8 buf[0x2000];
    int bw;
    int bh;
    int i;
    int j;
    int k;
    u8 *fp;
    u8 *d;
    u8 *s;
    u8 *b;
    int off;

    bw = (w - 1) / 128 + 1;
    bh = (h - 1) / 64 + 1;
    for (i = 0; i < bh; i++) {
        off = bw * (i * 0x2000);
        fp = src + off;
        d = dst + off;
        for (j = 0; j < bw; j++) {
            s = fp;
            b = buf;
            for (k = 0; k < 0x40; k++) {
                memcpy(b, s, 0x80);
                s += bw << 7;
                b += 0x80;
            }
            PageConv8to32(buf, d, bw);
            fp += 0x80;
            d += 0x100;
        }
    }
}
#endif

#if !defined(G_PART) || G_PART == 3
static void PageConv8to32(u8 *buf, u8 *dst, int bw) {
    int by;
    int bx;
    int t;
    int *tp;

    for (tp = block_table8, by = 0; by < 4; by++) {
        for (bx = 0; bx < 8; bx++) {
            t = *tp++;
            BlockConv8to32(buf + ((by * 16) << 7) + bx * 16, dst + bw * (idx32_v[t] << 11) + (idx32_h[t] << 5), bw);
        }
    }
}
#endif

/* near-match (the original steps the table pointer after every pair; MWCC merges the steps here), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
#if !defined(G_PART)
static asm void BlockConv8to32(u8 *src, u8 *dst, int bw)
{
#include "BlockConv8to32.inc"
}
#endif
#else
static void BlockConv8to32(u8 *src, u8 *dst, int bw) {
    CLMP **tp = clm_tbl8_ptr;
    CLMP *e;
    int a;
    int b;
    int x;

    for (a = 0; a < 4; a++) {
        e = *tp++;
        for (b = 0; b < 2; b++) {
            for (x = 0; x < 0x20; x += 8) {
                *dst++ = src[e->a];
                *dst++ = src[e->b];
                e++;
                *dst++ = src[e->a];
                *dst++ = src[e->b];
                e++;
                *dst++ = src[e->a];
                *dst++ = src[e->b];
                e++;
                *dst++ = src[e->a];
                *dst++ = src[e->b];
                e++;
            }
            dst += (bw << 8) - 0x20;
        }
        src += 0x200;
    }
}
#endif
