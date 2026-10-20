/* camq1 - 0x00225F50-0x00225FBC: Fish Wyvern (monster state 0x15) and Legend
 * Sword demo camera requests. */
#include "version.h"
#include "types.h"

void DemoCameraRequest(int, s32);
extern u8 em_work[];

#if !defined(G_PART) || G_PART == 2
void FishWyvernCameraRequest(void) {
    u8 *e = em_work;
    int i = 20;

    do {
#ifdef VERSION_MHG
        if (e[0] != 0 && (e[2] == 0x15 || e[2] == 0x2E)) {   /* G: kind 0x2E is a fish wyvern too */
#else
        if (e[0] != 0 && e[2] == 0x15) {
#endif
            DemoCameraRequest(0x11, (s32)e);
            return;
        }
        i--;
        e += EMW_STRIDE;
    } while (i != 0);
}
#endif

#if !defined(G_PART) || G_PART == 1
void LegendSwordCameraRequest(void) {
    DemoCameraRequest(0x19, 0);
}
#endif
