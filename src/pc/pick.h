/* pick.h - the in-game bug reporter (pick.c): PC (GL) and Xbox (nv2a) front-ends. */
#ifndef PICK_H
#define PICK_H
#include "pad/pad.h"
void pick_arm(void);                    /* freeze (F8) */
int  pick_busy(void);                   /* a report is being made (a headless run waits for it) */
int  pick_frozen(void);                 /* the frozen frame is shown: the viewer skips the scene */
int  pick_hold_ticks(int ticks);        /* no game ticks while armed / frozen (and the RT_PICK_AT trigger) */
int  pick_frame_hook(void);             /* at the end of a drawn frame; 1 = draw the frame once more (id pass) */
void pick_frozen_frame(void);           /* draw the frozen frame and the reporter's panel */
int  pick_event(const void *sdl_event); /* an SDL_Event; 1 = consumed */
void pick_poll_pad(void);               /* the PC controller combo (Back/View + Start), read from the controller */
void pick_ring_frame(void);             /* the clip buffer: call before presenting a normal frame (PC only) */
/* The controller UI (docs/pc.md "Bug reporter", docs/xbox.md "Bug reporter on the Xbox"). pick_tick_pad sees every
 * game tick's pad: on the Xbox Back + Black (select + R1) freezes the game there (the combo is taken out of the pad
 * state, so a --input script can press it too). While pick_wants_pad(), the viewer hands it the pad (or the script) once
 * per drawn frame: right stick / left stick / d-pad move a pointer, A picks (held A + move = a box), X undoes, Y opens
 * the on-screen keyboard for the note, Start saves, B cancels. */
void pick_tick_pad(pad_state *ps);
int  pick_wants_pad(void);
void pick_pad(const pad_state *p);
#endif
