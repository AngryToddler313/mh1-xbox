#ifndef LBDLG_G_H
#define LBDLG_G_H
/* PS2 Monster Hunter G: dialog message numbers. G's table (netDialogMessage, G 0x636F20) has 109 entries of 8 bytes
 * {u16 kind flags, pad, char *msg} where MH1's has 65 pointers; G inserted and reworded messages. The map MH1 id -> G id was
 * made by exact text match, else the most similar text (tools/mhg_dlgmap.py; 14 of 65 are fuzzy: online / server wording). */
static const unsigned char g_dlg_map[65] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 27, 28, 29, 30, 55, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 52, 53, 54, 55, 56, 57, 58, 59, 60, 104, 81, 82, 83, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101 };
#endif
