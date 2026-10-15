/* font5x7.h - the 5x7 pixel font of the bug reporter and the settings menu (font5x7.c) */
#ifndef PC_FONT5X7_H
#define PC_FONT5X7_H
const unsigned char *font5x7_rows(int c);   /* 7 rows of 5 bits (bit 4 = left) for ASCII 32..126, else a box */
#endif
