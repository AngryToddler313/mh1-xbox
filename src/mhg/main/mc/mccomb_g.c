/* G only (SLPM_658.69 main 0x22B0D0-0x22BB50): card helpers G added around mccomb (src/main/mc/mccomb.c).
 * func_0022B0D0 is MH1's decode_data (the image written by the MH1 save format, MCIMG words);
 * names are the map's, meanings are guesses. */
#include "types.h"

extern u8 *data_load_ptr;
#define MCIMG 0x8A20

#if !defined(G_PART) || G_PART == 1
void func_0022B0D0(out, buf)
u16 *out;
u16 *buf;
{
    int i;
    u16 key;
    u16 stored;
    int sum;

    sum = 0;
    i = 0;
    out[8] = *buf != 0x100;
    buf++;
    key = *buf++;
    stored = *buf++;
    buf++;
    do {
        *buf ^= key;
        sum = (sum + *buf) & 0xFFFF;
        buf++;
        if ((key & 0xFFFF) == 0) {
            key = 1;
        }
        key = ((key & 0xFFFF) * 0xB0) % 65363 & 0xFFFF;
        i++;
    } while (i < MCIMG);
    out[9] = (stored & 0xFFFF) != (sum & 0xFFFF);
}
#endif

void *memset();
void McActInit();
int McActAvailSet();

#if !defined(G_PART) || G_PART == 2
void func_0022B8C0(u8 *w) {
    memset((void *)((u32)data_load_ptr + 0x40000), 0, 0x11450);
    McActInit(1);
    *(int *)(w + 0x48) = McActAvailSet(0);
}
#endif

/* G: mc_mes_disp for a message list where some lines (flag != 0) are printed at x instead of centred */
extern u8 **McardMsgTbl[];
void flfntSetSize();
void flfntLocate(s16, s16);
void font_set_palette();
int strlen_sp();
void func_0012BDD0();

#if !defined(G_PART) || G_PART == 3
s16 func_0022BA20(s16 x, s16 y, int idx, s8 *flag) {
    u8 **p;
    u8 *s;

    flfntSetSize(0x12, 0x12);
    font_set_palette(0);
    p = McardMsgTbl[idx];
    s = *p;
    if (s != 0) {
        do {
            if (*s != 0) {
                if (*flag == 0) {
                    flfntLocate(0x140 - strlen_sp(s) * 0x12 / 4, y);
                } else {
                    flfntLocate(x, y);
                }
                func_0012BDD0(s);
            }
            p++;
            s = *p;
            y += 0x12;
            flag++;
        } while (s != 0);
    }
    return y - 0x12;
}
#endif
