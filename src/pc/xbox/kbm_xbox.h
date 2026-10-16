/* kbm_xbox.h - a USB keyboard and mouse on the Xbox (kbm_xbox.c): boot-protocol HID devices through nxdk's USB host
 * stack, for the bug reporter's note and pointer (pick.c). docs/xbox.md "Keyboard and mouse on the Xbox". */
#ifndef KBM_XBOX_H
#define KBM_XBOX_H
/* HID usage codes of the keys the reporter reads */
enum { KBM_ENTER = 0x28, KBM_ESC = 0x29, KBM_BACKSPACE = 0x2A, KBM_F8 = 0x41 };
enum { KBM_SHIFT = 0x22 };       /* left or right shift in the modifier byte */
void kbm_init(void);             /* before SDL's joystick start (pad_init): registers the HID driver with nxdk's USB stack */
int  kbm_key(int *usage, int *mods);     /* the next key press (a HID usage and the modifier byte); 0 = none */
char kbm_ascii(int usage, int mods);     /* a US-layout character for a key press, or 0 */
int  kbm_mouse(int *dx, int *dy, int *buttons);  /* motion since the last call and the buttons (1 left, 2 right); 0 = no mouse */
int  kbm_have_keyboard(void);
int  kbm_have_mouse(void);
#endif
