/* fl pad layer part 1 (SLPM_654.95 0x0018E430-0x0018E624): flpad_ram_clear, flPADInitialize, flPADDestroy, flPADWorkClear (flPADConfigSet follows in flpad_nm.c, 10 of 32 off).
 * flpad_root / flpad_conf are two 0x110 byte work blocks (one per port set), flpad_config holds 0x2C bytes of
 * (s16, s16) button-map pairs per port. */
#include "g_alias.h"
#include "types.h"

typedef struct PADPAIR { s16 a, b; } PADPAIR;
extern u8 flpad_root[];
extern u8 flpad_conf[];
extern u8 *flpad_adr[2];
typedef struct PADCFG { PADPAIR k[11]; } PADCFG;
extern PADCFG flpad_config[2];
extern u8 fltpad_config_basic[];

int tarPADInit(void);
void tarPADDestroy(void);
void flPADConfigSetACRtoXX(int, s16, s16, s16);

#if !defined(G_PART) || G_PART == 1
void flpad_ram_clear(s32 *p, int n) {
    int i;
    int r;
    int w;
    u8 *q;

    r = n & 3;
    i = 0;
    w = n / 4;
    for (; i < w; i++) {
        *p++ = 0;
    }
    q = (u8 *)p;
    if (r != 0) {
        for (i = 0; i < r; i++) {
            *q++ = 0;
        }
    }
}
#endif

void flPADWorkClear(void);
void flPADConfigSet(PADPAIR *, int);

#if !defined(G_PART) || G_PART == 3
int flPADInitialize(void) {
    int i;
    int r;

    r = tarPADInit();
    flPADWorkClear();
    flpad_adr[0] = flpad_root;
    flpad_adr[1] = flpad_conf;
#ifdef MHG_PS2
    flPADConfigSet((PADPAIR *)fltpad_config_basic, 0);     /* G: port 0 only */
#else
    for (i = 0; i < 2; i++) {
        flPADConfigSet((PADPAIR *)fltpad_config_basic, i);
    }
#endif
    return r;
}
#endif

#if !defined(G_PART) || G_PART == 2
void flPADDestroy(void) {
    tarPADDestroy();
}
#endif

#if !defined(G_PART) || G_PART == 2
void flPADWorkClear(void) {
    flpad_ram_clear((s32 *)flpad_root, GV(0x110, 0x88));
    flpad_ram_clear((s32 *)flpad_conf, GV(0x110, 0x88));
}
#endif
