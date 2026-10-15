/* menu.h - the settings menu (menu.c): F10 on the PC, Back + L3 on a controller (PC and Xbox). */
#ifndef PC_MENU_H
#define PC_MENU_H
#include "pad/pad.h"
#define MENU_COMBO (PAD_SELECT | PAD_L3)
int  menu_event(const void *sdl_event);  /* PC: an SDL_Event; 1 = consumed (keys and mouse while open, F10). Xbox: always 0 */
void menu_poll_pad(void);                /* the controller once per drawn frame: the open combo, and the navigation while open */
void menu_pad(const pad_state *p);       /* the navigation from a given pad state (the --input script while open) */
void menu_tick_pad(pad_state *ps);       /* each game tick's pad: the open combo is taken out (also from a script) */
int  menu_hold_ticks(int ticks);         /* 1 while the menu is open: no game ticks (also the RT_MENU_AT test trigger) */
int  menu_open(void);
int  menu_busy(void);                    /* a scripted run (RT_MENU_KEYS) has keys left: a --shot waits for them */
void menu_draw(void);                    /* the overlay (when open) and the title-screen hint; after the frame, before it is presented */
void menu_set_shot(int is_shot);         /* --shot runs do not write the settings file */
#endif
