/* tarpad layer: PADRead_for_PS2 (SLPM_654.95 0x00195000-0x00195510, file-static in the original): reads one port
 * through PADReadSub and turns the libpad2 state (ps2pad_state, 0x20 bytes per port: +2 buttons active low, +4..+7
 * the two sticks, +8 twelve pressure bytes) into the tarpad record (tarpad_root, 0x34 bytes: +2 device kind,
 * +8 soft button bits, +0xC sixteen pressure bytes, +0x1C / +0x28 the sticks centred on 0), through the per-port
 * button map ps2pad_hard_to_soft (mask, pressure kind). Keeps a copy of the state in ps2pad_backup. */
#include "types.h"

typedef struct TARPAD {
    u8 x0;
    u8 caps;            /* 0x01 bit 0x10/1/2 pressure kinds, 0x20 left stick, 0x40 right stick */
    u16 kind;           /* 0x02 */
    u8 x4[4];
    u32 bits;           /* 0x08 */
    u8 press[0x10];     /* 0x0C */
    s16 lx;             /* 0x1C */
    s16 ly;             /* 0x1E */
    u8 x20[8];
    s16 rx;             /* 0x28 */
    s16 ry;             /* 0x2A */
    u8 x2C[8];
} TARPAD;

typedef struct PS2STATE {
    u8 x0[2];
    u16 buttons;        /* 0x02 */
    u8 rx, ry;          /* 0x04 */
    u8 lx, ly;          /* 0x06 */
    u8 press[12];       /* 0x08 */
    u8 x14[0xC];
} PS2STATE;

typedef struct H2S {
    u32 mask;
    int kind;
} H2S;

typedef struct PS2SLOT {
    u8 open;
    u8 x1[0x17];
} PS2SLOT;

typedef struct PS2CFG {
    s16 thr;
    s16 lx;
    s16 ly;
    s16 x6;
} PS2CFG;

extern PS2SLOT ps2slot[2];
extern TARPAD tarpad_root[2];
extern PS2STATE ps2pad_state[2];
extern PS2STATE ps2pad_backup[2];
extern H2S ps2pad_hard_to_soft[2][16];
extern u32 flpad_io_map[16];
extern u8 ps2pad_an_rm_map[16];
extern PS2CFG ps2pad_config[2];

void PADReadSub(void);

/* near-match (same switch/loops; the original strength-reduces the clear and halving loops into walking pointers and
 * allocates differently: ~260 lines), original bytes linked; the C is the reading copy */
#ifdef __MWERKS__
asm int PADRead_for_PS2(int port)
{
#include "PADRead_for_PS2.inc"
}
#else
int PADRead_for_PS2(int port) {
    u8 an[12];
    int i;
    u32 btn;
    u32 bits;
    s16 *d;
    s16 *e;

    if (ps2slot[port].open == 0) {
        return 0;
    }
    PADReadSub();
    if (tarpad_root[port].kind != 0 && (tarpad_root[port].kind & 3) == 0) {
        tarpad_root[port].kind = 0x8000;
    }
    for (i = 0; i < 0x10; i++) {
        tarpad_root[port].press[i] = 0;
    }
    for (i = 0; i < 12; i++) {
        an[i] = ps2pad_state[port].press[i] / 2;
    }
    ps2pad_state[port].buttons = ~ps2pad_state[port].buttons;
    switch (tarpad_root[port].kind) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 0x10:
    case 0x20:
        btn = ps2pad_state[port].buttons;
        bits = 0;
        for (i = 0; i < 0x10; i++) {
            if (btn & ps2pad_hard_to_soft[port][i].mask) {
                bits |= flpad_io_map[i];
                tarpad_root[port].press[i] = 0x7F;
            }
            switch (ps2pad_hard_to_soft[port][i].kind) {
            case 1:
                if (tarpad_root[port].caps & 0x10) {
                    if (an[ps2pad_an_rm_map[i]] < ps2pad_config[port].thr) {
                        tarpad_root[port].press[i] = 0;
                    } else {
                        tarpad_root[port].press[i] = an[ps2pad_an_rm_map[i]];
                    }
                }
                break;
            case 2:
                if (tarpad_root[port].caps & 1) {
                    if (an[ps2pad_an_rm_map[i]] < ps2pad_config[port].thr) {
                        tarpad_root[port].press[i] = 0;
                    } else {
                        tarpad_root[port].press[i] = an[ps2pad_an_rm_map[i]];
                    }
                }
                break;
            case 3:
                if (tarpad_root[port].caps & 2) {
                    if (an[ps2pad_an_rm_map[i]] < ps2pad_config[port].thr) {
                        tarpad_root[port].press[i] = 0;
                    } else {
                        tarpad_root[port].press[i] = an[ps2pad_an_rm_map[i]];
                    }
                }
                break;
            }
        }
        tarpad_root[port].bits = bits;
        if (tarpad_root[port].caps & 0x20) {
            tarpad_root[port].lx = ps2pad_state[port].lx - 0x80;
            tarpad_root[port].ly = ps2pad_state[port].ly - 0x80;
            if (tarpad_root[port].lx < -0x7F) {
                tarpad_root[port].lx = -0x7F;
            }
            if (tarpad_root[port].ly < -0x7F) {
                tarpad_root[port].ly = -0x7F;
            }
        }
        if (tarpad_root[port].caps & 0x40) {
            tarpad_root[port].rx = ps2pad_state[port].rx - 0x80;
            tarpad_root[port].ry = ps2pad_state[port].ry - 0x80;
            if (tarpad_root[port].rx < -0x7F) {
                tarpad_root[port].rx = -0x7F;
            }
            if (tarpad_root[port].ry < -0x7F) {
                tarpad_root[port].ry = -0x7F;
            }
        }
        d = (s16 *)&ps2pad_state[port];
        e = (s16 *)&ps2pad_backup[port];
        for (i = 8; i > 0; i--) {
            e[0] = d[0];
            e[1] = d[1];
            d += 2;
            e += 2;
        }
        ps2pad_backup[port].buttons = ~ps2pad_backup[port].buttons;
        break;
    default:
        tarpad_root[port].bits = 0;
        tarpad_root[port].lx = 0;
        tarpad_root[port].ly = 0;
        tarpad_root[port].rx = 0;
        tarpad_root[port].ry = 0;
        break;
    }
    return 1;
}
#endif
