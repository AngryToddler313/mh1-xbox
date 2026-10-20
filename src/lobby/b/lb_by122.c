/* lb_by122 - agent B 0x005C3AE0-0x005C3C08: set_se_type (NPC sound-effect type per NPC kind). */
#include "g_alias.h"
#include "lbnpc_proto.h"
void set_se_type(EMW *em) {
    u8 *ex = em->ex;

    switch (*((u8 *)em + GV(0x452, 0x478))) {
    case 8:
    case 9:
    case 0xB:
    case 0xD:
        ex[GV(0x2C, 0x38)] = 0x14;
        break;
    case 0x1A:
        ex[GV(0x2C, 0x38)] = 0x16;
        break;
    case 0x1B:
    case 0x1C:
        ex[GV(0x2C, 0x38)] = 0x18;
        break;
    case 0x37:
        ex[GV(0x2C, 0x38)] = 0x1A;
        break;
    case 0x12:
        ex[GV(0x2C, 0x38)] = 0x1C;
        break;
    case 0x13:
    case 0x1D:
        ex[GV(0x2C, 0x38)] = 0x1E;
        break;
    case 0x38:
        ex[GV(0x2C, 0x38)] = 0x20;
        break;
    case 0x39:
        ex[GV(0x2C, 0x38)] = 0x22;
        break;
    default:
        ex[GV(0x2C, 0x38)] = 0x14;
        break;
    }
}
