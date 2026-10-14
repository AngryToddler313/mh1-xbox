/* dbgmenu.h - the testers' debug menu (dbgmenu.c; docs/pc.md "Debug menu", docs/PLAYING.md). PC and Xbox, MH1 and G:
 * opened with Back + R3 on the controller (F9 on the PC), drawn over the frozen game with the game's own font. */
#ifndef PC_DBGMENU_H
#define PC_DBGMENU_H
#include "pad/pad.h"

typedef struct {
    void (*start_quest)(int no);        /* the viewer's quest start (as from the village counter) */
    void (*village)(int intro);         /* to the village: 1 with the first-visit event, 0 without */
    void (*title)(void);                /* back to the title screen (power-on sequence) */
    void (*shot)(const char *png);      /* write the current frame (scripted test shots, RT_DBG_SHOTS) */
} dbgmenu_actions;

void dbgmenu_init(int enabled, const dbgmenu_actions *a);
int  dbgmenu_enabled(void);
void dbgmenu_tick_pad(pad_state *ps);   /* each game tick's pad: the open combo (Back + R3) is taken out; frame-step count */
int  dbgmenu_hold_ticks(void);          /* 1: no game ticks now (menu open, or frame step waiting for a step) */
int  dbgmenu_wants_pad(void);           /* the viewer reads the pad once per drawn frame for the menu (open / frame step) */
void dbgmenu_pad(const pad_state *p);   /* that pad state */
int  dbgmenu_event(const void *sdl_event);  /* PC: F9, keys, mouse while open; 1 = consumed */
void dbgmenu_draw(void);                /* after the frame (over the HUD), before the shot / present */
int  dbgmenu_busy(void);                /* open in a scripted run: a --shot waits */
int  dbgmenu_freecam(void);             /* PC: the free camera is on (WASD + mouse fly; the keyboard no longer drives the hunter) */
const char *dbgmenu_take_shot(void);    /* a test shot is due for this frame (RT_DBG_SHOTS=prefix), else NULL */
#endif
