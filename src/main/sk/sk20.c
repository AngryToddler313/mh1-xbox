/* sk20 - f_sk 0x0027BF80-0x0027C0AC: Reibun_print (prints a phrase cut to n characters, ending in an SJIS ellipsis when too long),
 * SoftkeyAppInit (empty), SoftkeyLoad, SoftkeyTextureSet, SetBlendingMode. */
#include "types.h"
#include "sk_g.h"
#include "g_alias.h"

void font_print_uf(char *);
void mkTexture(int, int, int);
void reload_tex(int, int);
void SetTextureStage(int);
void SetTrnslMode(int, int);
#ifdef MHG_PS2
#define RP_T int        /* G passes the count through unmasked */
void func_002277B0(char *, int, char *);
#else
#define RP_T u8
#endif

#if !defined(G_PART) || G_PART == 2
void Reibun_print(RP_T n, char *src) {
    char buf[0x30];
#ifdef MHG_PS2
    func_002277B0(buf, n, src);     /* G: the copy with a length (main 0x2277B0) */
    font_print_uf(buf);
#else
    int cnt;
    char *d;
    char *p;

    d = buf;
    cnt = n + 1;
    if (cnt != 0) {
        do {
            d[0] = src[0];
            d[1] = src[1];
            if (src[0] == 0) {
                font_print_uf(buf);
                return;
            }
            cnt--;
            d += 2;
            src += 2;
        } while (cnt != 0);
    }
    p = buf + (n - 1) * 2;
    *p++ = 0x81;
    *p++ = 0x64;
    *p = 0;
    font_print_uf(buf);
#endif
}
#endif

#if !defined(G_PART) || G_PART == 1
void SoftkeyAppInit(void) {
}
#endif

#if !defined(G_PART)
void SoftkeyLoad(void) {
    mkTexture(4, 0x11C, 0);
}
#endif

#if !defined(G_PART) || G_PART == 1
void SoftkeyTextureSet(void) {
    reload_tex(1, GSLOT(0x11C, 0x143));
    SetTextureStage(GSLOT(0x11C, 0x143));
}
#endif

#if !defined(G_PART) || G_PART == 1
void SetBlendingMode(int mode) {
    if (mode != 1) {
        SetTrnslMode(4, 5);
    } else {
        SetTrnslMode(4, 1);
    }
}
#endif
