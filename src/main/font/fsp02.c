/* fsp02 - font print helpers (SLPM_654.95 0x001623F0-0x00162634): font_print_double, font_print_double2, font_print_uf, strlen_sp. Whole file in fsp_nm.c. */
#include "g_alias.h"
#include "types.h"

extern s32 font_reset_flag;
extern char lit_377_0035BB08[];     /* "%s" (print format used by every helper here) */
extern char lit_761_0035BB18[];     /* "~" (escape lead-in searched by strlen_sp) */

void flfntLocate(s16, s16);
void font_set_palette(int);
void flfntPrintf(char *, ...);
char *strstr(const char *, const char *);

/* Prints str twice (a shadow 3/2 pixels down-right in palette pal1, then the text in pal2). */
#if !defined(G_PART) || G_PART == 1
void font_print_double(s16 x, s16 y, int pal1, int pal2, char *str) {
    if (font_reset_flag == 0) {
        flfntLocate(x + 3, y + 2);
        font_set_palette(pal1);
        flfntPrintf(lit_377_0035BB08, str);
        flfntLocate(x, y);
        font_set_palette(pal2);
        flfntPrintf(lit_377_0035BB08, str);
    }
}
#endif

/* Same with a 2/2 shadow offset. */
#if !defined(G_PART) || G_PART == 1
void font_print_double2(s16 x, s16 y, int pal1, int pal2, char *str) {
    if (font_reset_flag == 0) {
        flfntLocate(x + 2, y + 2);
        font_set_palette(pal1);
        flfntPrintf(lit_377_0035BB08, str);
        flfntLocate(x, y);
        font_set_palette(pal2);
        flfntPrintf(lit_377_0035BB08, str);
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void font_print_uf(char *str) {
    if (font_reset_flag == 0) {
        flfntPrintf(lit_377_0035BB08, str);
    }
}
#endif

/* Length of a string in characters, escape codes ("~..") counted by their displayed width. */
#ifdef MHG_PS2
char *func_00164448(char *, char *);   /* G's strstr */
#define SP_STRSTR func_00164448
#else
#define SP_STRSTR strstr
#endif
#if !defined(G_PART) || G_PART == 2
int strlen_sp(char *s) {
#ifdef VERSION_MHG
    int len;
    s16 w;
    s16 f;      /* G: inside a 0x08xx control pair nothing is counted */

    w = 0;
    f = 0;
    len = strlen(s);
    while (*s != 0) {
        s = SP_STRSTR(s, lit_761_0035BB18);
        if (s == 0) {
            return len;
        }
        font_sp_ck((s8 *)s, &w, 0);
        if ((w & 0xFF00) == 0x800) {
            f ^= 1;
            s = (char *)font_sp_ck((s8 *)s, &w, 1);
            len += w;
        } else if (f == 0) {
            s = (char *)font_sp_ck((s8 *)s, &w, 1);
            len += w;
        } else {
            s++;
        }
    }
#else
    int len = strlen(s);
    s16 w = 0;

    while (*s != 0) {
        char *p = strstr(s, lit_761_0035BB18);
        if (p == 0) {
            return len;
        }
        s = (char *)font_sp_ck((s8 *)p, &w, 1);
        len += w;
    }
#endif
    return len;
}
#endif

