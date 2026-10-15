/* ps2patch.h - the MH1 online patch for a real PS2 / PCSX2 (docs/network.md "PS2 patch", tools/ps2_patch.py).
 * Our own code, not Capcom's: src/ps2patch/online.c. The PC online build (src/pc/rt/rt_online.c) fills the
 * server table with the same function. */
#ifndef PS2PATCH_H
#define PS2PATCH_H

/* what the KDDI portal used to hand over, now chosen by the player when patching (no string literals in the
 * patch: MWCC would put short ones in .sdata, out of the gp range of lobby.bin) */
typedef struct MH1PCONF {
    char id[0x10];          /* 0x00 the lobby server's id (12 characters, ConnectLbsId / BsLbsInfo) */
    char name[0x20];        /* 0x10 its name in the server list (BsLbsInfo + 0xD) */
    char server[0x40];      /* 0x30 "host:port" (tcp_init reads bsCsvWork + 0x22F; a name or a dotted address) */
    char login[0x10];       /* 0x70 the MMBB id = the login key (D_3A3B71, 10 characters; the login sends 8 digits) */
    char password[0x14];    /* 0x80 the login password (bsCsvWork + 0, 16 characters) */
    char users[4];          /* 0x94 users now (bsCsvWork + 0x2522) */
    char users_max[4];      /* 0x98 users at most (bsCsvWork + 0x266C) */
} MH1PCONF;                 /* 0x9C */

void mh1p_fill_server_table(MH1PCONF *c);

#endif
