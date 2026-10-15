/* src/ps2patch/online.c - the MH1 online patch for a real PS2 / PCSX2 (our own code; docs/network.md "PS2 patch").
 *
 * tools/ps2_patch.py compiles this with MWCC (-DMH1_ONLINE_PATCH, the player's server and login in a generated
 * mh1p_conf.h) and links it in place of the original internet_browser, lobby.bin 0x5B5980-0x5B5C50 (720 bytes;
 * code, then mh1p_conf). The normal PS2 build does not compile this directory (tools/build.py skips src/ps2patch).
 *
 * The original internet_browser (src/lobby/b/nm/internet_browser.c) opened the KDDI Multi-Matching BB portal in the
 * in-game browser (HTTPS, gone since 2007), whose pages left the login password and the lobby server table in
 * bsCsvWork, the MMBB id in D_3A3B71 and the server list in BsLbsInfo / BsLbsCount, then loaded the network models
 * (steps 4-6). The patched one fills the same work from mh1p_conf (as the PC port's rt_online.c does with
 * mh1pc.ini) and keeps steps 0 and 4-6 as they were; after it the game's own server select (create_server_table),
 * tcp_init (name lookup with the console's DNS, then TCP) and the lobby login run unchanged.
 *
 * mh1p_fill_server_table is also linked into the PC online build (rt_online.c), so the PC tests run this code. */
#include "types.h"
#include "ps2patch.h"

extern char bsCsvWork[];            /* the portal's work: +0 password, +0x11 = D_3A3B71, server table from +0x22F */
extern char D_3A3B71[];             /* the MMBB id (login key), 10 characters, space padded */
extern char ConnectLbsId[];         /* the chosen lobby server's id */
extern char BsLbsInfo[];            /* the portal's server list: 0x102 a server, id at +0, name at +0xD */
extern s8 BsLbsCount;               /* servers in that list */

void *memset(void *, int, unsigned int);
char *strncpy(char *, const char *, unsigned int);

#ifdef MH1_ONLINE_PATCH
#include "mh1p_conf.h"      /* MH1P_CONF: written by tools/ps2_patch.py from --server / --login / --password */

extern u8 COM_R_No_2;
extern s8 Net_Err_No;
extern s8 MMBB_LOGIN;
void Net_fade_kill(void);
void cnLbc_LoadNetModel(int);
int cnLbc_LoadModelWait(int);

MH1PCONF mh1p_conf = MH1P_CONF;

/* server_connect step 0 (main, connect_nm.c) calls this every frame: 0 = working, 1 = done, -1 = failed.
 * First in the file: it has to start at 0x5B5980, the address its callers jump to. */
s32 internet_browser(void)
{
    switch (COM_R_No_2) {
    case 0:                             /* as the original's step 0 */
        COM_R_No_2 = 1;
        Net_fade_kill();
        Net_Err_No = 0;
        break;
    case 1:                             /* the portal's place (original steps 1-3: browser start, pages, dispose) */
        mh1p_fill_server_table(&mh1p_conf);
        MMBB_LOGIN = 1;                 /* what the original sets when the portal's login succeeded */
        COM_R_No_2 = 4;
        break;
    case 4:                             /* original steps 4-6 unchanged: the network models */
        cnLbc_LoadNetModel(1);
        COM_R_No_2 = 5;
        break;
    case 5:
        if (cnLbc_LoadModelWait(1) != 0)
            COM_R_No_2 = 6;
        break;
    case 6:
        return 1;
    default:                            /* steps 2, 3 and 0x63 of the original do not happen any more */
        COM_R_No_2 = 1;
        break;
    }
    return 0;
}
#endif

/* the layout the portal left (read by tcp_init, lb_tcp01.c, and create_server_table): one server */
void mh1p_fill_server_table(MH1PCONF *c)
{
    char *w = bsCsvWork;
    int i;

    memset(w, 0, 0x1000);
    strncpy(w, c->password, 0x10);
    strncpy(ConnectLbsId, c->id, 0xC);
    strncpy(w + 0x603, c->id, 0xC);
    strncpy(w + 0x22F, c->server, 0x40);
    strncpy(w + 0x2522, c->users, 4);
    strncpy(w + 0x266C, c->users_max, 4);
    memset(BsLbsInfo, 0, 0xA14);
    strncpy(BsLbsInfo, c->id, 0xC);
    strncpy(BsLbsInfo + 0xD, c->name, 0x20);
    BsLbsCount = 1;     /* one server: the first login goes straight on to the lobby (lbc_login_finish) */
    memset(D_3A3B71, ' ', 0xA);
    for (i = 0; i < 0xA && c->login[i] != 0; i++)
        D_3A3B71[i] = c->login[i];
}
