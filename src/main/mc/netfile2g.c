/* netfile2g - SLPM_654.95 0x0028A170-0x0028A3B0 (dialog_limit_disp): counts net_common_w.x06 down once a frame and shows
 * the remaining seconds (x06 / 60, full-width digits from zen_num) centred at y 352 for mode 0, at y 332 for mode 1, or
 * copies the digits into net_common_w.x8D for mode 2. */
#include "g_alias.h"
#include "types.h"
#include "netcw.h"

extern char *zen_num;
void flfntSetSize();
void flfntLocate(s16, s16);
void flfntSetZ(f32);
void flfntPrintf();
void *memset();
char *strcpy();
int strlen();
#ifdef MHG_PS2
int func_001D5970();   /* G: the length call (main 0x1D5970) */
#endif

void dialog_limit_disp(mode)
u8 mode;
{
    char buf[8];
    char *p;
    int sec;
    int tens;
    int ones;
    char *src;

    if (net_common_w.x06 > 0) {
        net_common_w.x06--;
    }
    sec = net_common_w.x06 / 60;
    tens = sec / 10;
    ones = sec % 10;
    p = buf;
    if (tens > 0) {
        src = zen_num + tens * 2;
        *p++ = src[0];
        *p++ = src[1];
    }
    src = zen_num + ones * 2;
    *p++ = src[0];
    *p++ = src[1];
    *p = 0;
    flfntSetSize(22, 22);
    flfntLocate((640 - (s16)GV(strlen, func_001D5970)(buf) * 11) / 2, 352);
    switch (mode) {
    case 0:
        flfntSetZ(GV(650.0f, 550.0f));
        flfntPrintf(buf);
        flfntSetZ(GV(850.0f, 716.6667f));
        return;
    case 1:
        flfntLocate((640 - (s16)GV(strlen, func_001D5970)(buf) * 11) / 2, 332);
        flfntSetZ(GV(650.0f, 883.3334f));
        flfntPrintf(buf);
        flfntSetZ(GV(850.0f, 716.6667f));
        return;
    case 2:
        memset(net_common_w.x8D, 0, 5);
        strcpy(net_common_w.x8D, buf);
        net_common_w.x8C = 1;
        break;
    }
}
