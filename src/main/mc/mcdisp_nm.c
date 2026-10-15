/* Memory card save slot selection screen, SLPM_654.95 main 0x280EF0-0x2814E0.
 * Three slot frames (DispFrameMessageA) at y = 0x58, 0xB0, 0x108 with the slot caption;
 * option_w+0x10 + 0x480*i is the per-slot "used" flag byte. Guesses: field names. */
#include "types.h"

typedef struct SAVEFR {
    s16 x;          /* 0 */
    s16 y;          /* 2 */
    u8 w;           /* 4 */
    u8 h;           /* 5 */
    u8 a;           /* 6 */
    u8 b;           /* 7 */
    s16 c;          /* 8 */
    s16 d;          /* 0xA */
    u32 col;        /* 0xC */
} SAVEFR;

extern u8 option_w[];
extern char lit_159_00384EA8[], lit_160_00384EB0[], lit_161_00384EB8[], lit_162_00384ED0[];
void flfntSetSize(int, int);
void font_print_ex();
extern char *sex_char_tbl[];
void Sel_csr_disp();
void DispFrameMessageA(void *, int, int);

#ifdef VERSION_MHG
/* G (main 0x22A710 / 0x22AB10 / 0x22AC70): a slot has 0xE00 bytes in option_w (the save file and G's load buffer keep MH1's 0x480 stride, which
 * `buf` selects on the PS2, not here); each slot shows its number and name, then play time and sex, then the money (with the hunter rank from the 2nd flag bit
 * 0x10). Formats are G's own (config/mhg/pc_data.txt gsave_fmt: NO DATA / slot:name / TIME / MONEY / MONEY HR). */
extern char gsave_fmt[];

static void gsave_text(int i, int y, int col, u8 *src)
{
    u32 t = *(u32 *)(src + 0xC1C);
    u8 rank = src[0xC23];

    if (src[0] == 0) {
        flfntSetSize(0x18, 0x18);
        font_print_ex(0x7A, (s16)(y + 0x12), col, gsave_fmt, i + 1);
        return;
    }
    flfntSetSize(0x18, 0x18);
    font_print_ex(0x3C, y, col, gsave_fmt + 8, i + 1, src + 8);
    flfntSetSize(0x12, 0x12);
    font_print_ex(0x3C, (s16)(y + 0x18), col, gsave_fmt + 0x10, t / 3600, t % 3600 / 60, sex_char_tbl[src[1] + 2]);
    if (*(u16 *)(src + 0x24) & 0x10) {
        font_print_ex(0x3C, (s16)(y + 0x2A), col, gsave_fmt + 0x30, *(u32 *)(src + 0x20), rank);
    } else {
        font_print_ex(0x3C, (s16)(y + 0x2A), col, gsave_fmt + 0x48, *(u32 *)(src + 0x20));
    }
}

/* mode: 0 / 2 as MH1; flag: the screen's step; buf: see below */
void disp_savesel(sel, mode, flag, buf)
u8 *sel;
int mode;
u8 flag;
int buf;
{
    SAVEFR fr;
    s16 i;
    s16 y;
    s16 col;
    u8 *o;

    flfntSetSize(0x14, 0x14);
    fr.x = 0x32;
    fr.y = 0x58;
    o = option_w + 0x10;
    /* buf: G reads the slots from the card image it just loaded (data_load_ptr + 0x50260, 0x480 each) while it checks the card; the PC's card
     * layer keeps its slots in option_w (as for MH1), and data_load_ptr is the general file buffer there, so buf is ignored */
    (void)buf;
    fr.a = 0xC;
    fr.w = 0x14;
    fr.h = 0x14;
    y = 0x5A;
    fr.b = 3;
    fr.col = 0xC0402010;
    fr.c = 5;
    fr.d = mode == 2 ? 2 : 4;
    for (i = 0; i < 3; i++) {
        col = 5;
        if (i == sel[1]) {
            if (mode == 0 || mode == 2 || flag >= 2) {
                Sel_csr_disp(fr.x + 0x73, fr.y - 3, 0x118, 0x46, 0xFF20C0C0);
            }
        } else {
            col = 0;
        }
        DispFrameMessageA(&fr, 0, 0x80);
        gsave_text(i, y, col, o);
        y += 0x58;
        o += 0xE00;
        fr.y += 0x58;
    }
}

/* waku: `cur` (4th argument) is the highlighted slot, a1 / a2 as MH1's mode / flag */
void disp_savesel_waku(sel, mode, flag, cur)
u8 *sel;
int mode;
u8 flag;
s8 cur;
{
    SAVEFR fr;
    s16 i;

    flfntSetSize(0x14, 0x14);
    fr.x = 0x32;
    fr.y = 0x58;
    fr.a = 0xC;
    fr.w = 0x14;
    fr.h = 0x14;
    fr.b = 3;
    fr.col = 0xC0402010;
    fr.c = 5;
    fr.d = mode == 2 ? 2 : 4;
    for (i = 0; i < 3; i++) {
        if (i == cur && (mode == 0 || mode == 2 || flag >= 2)) {
            Sel_csr_disp(fr.x + 0x73, fr.y - 3, 0x118, 0x46, 0xFF20C0C0);
        }
        DispFrameMessageA(&fr, 0, 0x80);
        fr.y += 0x58;
    }
}

/* moji: cur is the highlighted slot */
void disp_savesel_moji(sel, a1, a2, buf, cur)
u8 *sel;
int a1;
int a2;
int buf;
s8 cur;
{
    s16 i;
    s16 y = 0x5A;
    u8 *o = option_w + 0x10;
    (void)buf;

    for (i = 0; i < 3; i++) {
        gsave_text(i, y, i == cur ? 5 : 0, o);
        y += 0x58;
        o += 0xE00;
    }
}
#else
void disp_savesel(sel, mode, flag)
u8 *sel;
int mode;
u8 flag;
{
    SAVEFR fr;
    s16 i;
    s16 y;
    s16 col;
    u8 *o;

    flfntSetSize(0x14, 0x14);
    fr.x = 0x32;
    fr.y = 0x58;
    o = option_w + 0x10;
    fr.a = 0xC;
    fr.w = 0x14;
    fr.h = 0x14;
    y = 0x5A;
    fr.b = 3;
    fr.col = 0xC0402010;
    fr.c = 5;
    fr.d = mode == 2 ? 2 : 4;
    i = 0;
    do {
        col = 5;
        if (i == sel[1]) {
            if (mode == 0 || mode == 2 || flag >= 2) {
                Sel_csr_disp(fr.x + 0x73, fr.y - 3, 0x118, 0x46, 0xFF20C0C0);
            }
        } else {
            col = 0;
        }
        DispFrameMessageA(&fr, 0, 0x80);
        if (*o == 0) {
            flfntSetSize(0x18, 0x18);
            font_print_ex(0x7A, (s16)(y + 0x12), col, lit_159_00384EA8, i + 1);
        } else {
            flfntSetSize(0x18, 0x18);
            font_print_ex(0x3C, y, col, lit_160_00384EB0, i + 1, o + 8);
            flfntSetSize(0x12, 0x12);
            font_print_ex(0x3C, (s16)(y + 0x18), col, lit_161_00384EB8, sex_char_tbl[o[1] + 2]);
            font_print_ex(0x3C, (s16)(y + 0x2A), col, lit_162_00384ED0, *(u32 *)(o + 0x374) / 3600, *(u32 *)(o + 0x374) % 3600 / 60);
        }
        i++;
        y += 0x58;
        o += 0x480;
        fr.y += 0x58;
    } while (i < 3);
}

void disp_savesel_waku(sel, mode, flag)
u8 *sel;
int mode;
u8 flag;
{
    SAVEFR fr;
    s16 i;

    flfntSetSize(0x14, 0x14);
    fr.x = 0x32;
    fr.y = 0x58;
    fr.a = 0xC;
    fr.w = 0x14;
    fr.h = 0x14;
    fr.b = 3;
    fr.col = 0xC0402010;
    fr.c = 5;
    fr.d = mode == 2 ? 2 : 4;
    i = 0;
    do {
        if (i == sel[1] && (mode == 0 || mode == 2 || flag >= 2)) {
            Sel_csr_disp(fr.x + 0x73, fr.y - 3, 0x118, 0x46, 0xFF20C0C0);
        }
        DispFrameMessageA(&fr, 0, 0x80);
        i++;
        fr.y += 0x58;
    } while (i < 3);
}

void disp_savesel_moji(sel)
u8 *sel;
{
    s16 i;
    s16 y;
    s16 col;
    u8 *o;

    i = 0;
    y = 0x5A;
    o = option_w + 0x10;
    do {
        col = i == sel[1] ? 5 : 0;
        if (*o == 0) {
            flfntSetSize(0x18, 0x18);
            font_print_ex(0x7A, (s16)(y + 0x12), col, lit_159_00384EA8, i + 1);
        } else {
            flfntSetSize(0x18, 0x18);
            font_print_ex(0x3C, y, col, lit_160_00384EB0, i + 1, o + 8);
            flfntSetSize(0x12, 0x12);
            font_print_ex(0x3C, (s16)(y + 0x18), col, lit_161_00384EB8, sex_char_tbl[o[1] + 2]);
            font_print_ex(0x3C, (s16)(y + 0x2A), col, lit_162_00384ED0, *(u32 *)(o + 0x374) / 3600, *(u32 *)(o + 0x374) % 3600 / 60);
        }
        o += 0x480;
        i++;
        y += 0x58;
    } while (i < 3);
}
#endif
