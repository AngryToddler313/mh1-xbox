/* G only (SLPM_658.69 0x004F0900): hunter kinds that can guard (the same test MH1 spells out in pl_guard_ck). */
#include "pl.h"

int sub_main_func_004F0900(PLW *pl) {
    switch (pl->kind) {
    case 0:
    case 4:
    case 3:
        return 1;
    default:
        return 0;
    }
}
