/* flmw01 - Sofdec movie-playback glue in the fl layer 0x002163D0-0x00216480: flmwVSyncCallback,
 * flmwFlip (called once per flip while a movie plays; runs the sound/ADX update, then keeps calling
 * mwPlySwitchToIdle until the vsync handler clears the wait flag at flPs2State+0x60). */
#include "g_alias.h"
#ifdef MHG_PS2 /* G calls another function here (tools/check.py VERSION=mhg) */
#define flAdxControll ADXAMP_Create
#endif
#include "types.h"

extern u8 flPs2State[];

void flSndControll(int);
void ADXM_ExecMain();
void mwPlySwitchToIdle();
void flAdxControll(int);

#if !defined(G_PART) || G_PART == 1
void flmwVSyncCallback(void) {
    flAdxControll(0);
}
#endif

#if !defined(G_PART)
void flmwFlip(void) {
    flSndControll(1);
    ADXM_ExecMain();
    if (*(int *)(flPs2State + 0x5C) <= *(int *)(flPs2State + 0x54)) {
        unsigned long t = *(unsigned long *)0x12001000;
        *(int *)(flPs2State + 0x60) = 1;
        *(int *)(flPs2State + 0x4C) = (t >> 13) & 1;
        do {
            mwPlySwitchToIdle();
        } while (*(int *)(flPs2State + 0x60) != 0);
        return;
    }
    *(int *)(flPs2State + 0x60) = 1;
    mwPlySwitchToIdle();
}
#endif
