/* src/ps2patch/pdtest.c - TEST ONLY, never for players: does the game's DNAS "personalize" encrypt a plain patch list
 * on this console without the DNAS server step, and does the plain game then apply it at boot? (docs/network.md 3.8)
 *
 * tools/ps2_pdtest.py compiles this with MWCC (-DMH1_PDTEST and -DPDTEST_MODE=1 or 2) and links it at 0x2687E0, the
 * body of ms_net_patch_set (main, 6920 bytes; only reached when an official server sends a patch, which ours never
 * do), and turns the first two instructions of PatchLoadinDNAS_Main (0x28CAE0) into `j pdtest_main; nop`. The
 * title's card auto-load (CardAtld14, after a save was read and copied into patch_buff by mc_copy_patch(0)) calls
 * PatchLoadinDNAS_Main every frame until it returns non-zero, so pdtest_main runs once at the title, at the same
 * moment and with the same overlays loaded as the original. Nothing else changes. The normal PS2 build does not
 * compile this directory (tools/build.py skips src/ps2patch).
 *
 * PDTEST_MODE 1 (install): puts the plain list below into patch_buff (as ms_net_patch_set has it after
 *   InetDNAS2ConnectInstall), loads dnas_ins.bin the way PatchLoadinDNAS_Main does, runs the game's
 *   InetDNAS2PersonalEncode (func_A2FBD0) on it WITHOUT InetDNAS2ConnectInstall / sceDNAS2AuthInstall, logs every
 *   step with the sce result (dnerrno_0038AA50, what func_A2FBC0 returns) and the length (dndst_len), decodes the
 *   result again with InetDNAS2PersonalDecode (func_A2FD30, the boot path) and checks "M-HUNTER", runs
 *   PatchExecCS(0, 0) and checks the target, then saves the card image with the encrypted list in the patch slot
 *   (save+0x208, tail +0x10208) through the game's own save calls (mc_copy_patch(1), encode_data_002814E0,
 *   McActSaveSet; the card task runs McActMain), as the menu saves do.
 * PDTEST_MODE 2 (probe): what the original PatchLoadinDNAS_Main does (patch02.c), step for step, plus a log line per
 *   step: the stored length, each decode step's sce result, "M-HUNTER" or not, the target before and after
 *   PatchExecCS(0, 0). For the second boot; the plain unpatched disc does the same without the log.
 *
 * The test patch: one main entry (key 0) writing the 8 bytes "PDTESTOK" at PDT_TARGET, the last 8 bytes of the dead
 * ms_net_patch_set range (zero in this build; in the original the end of that function, which only an official
 * server's patch download reaches, so never run in the test).
 * Log: printf (the game's libc, shown in PCSX2's IOP/EE console log) with the prefix "PDTEST". */
#include "types.h"

#ifdef MH1_PDTEST
#pragma readonly_strings on

#define PDT_TARGET 0x0026A2E0

extern u8 patch_buff[];
extern u8 *data_load_ptr;
extern u8 card_w[];
extern u8 MediaVersion[];
extern s32 dnerrno_0038AA50;        /* the last sceDNAS2Inst* result the glue stored (func_A2FBC0 returns it) */
extern s32 dndst_len;               /* the glue's output length (personalize / extract) */
extern s8 DP_end;
extern s8 DP_r_no;
extern s8 DP_cnt;
extern s8 DR_NO_0;

int printf(const char *, ...);
void *memcpy(void *, const void *, unsigned int);
void *memset(void *, int, unsigned int);
char *strncpy(char *, const char *, unsigned int);
int strncmp(const char *, const char *, unsigned int);
int Net_put_overlay_data(int);
int NetLoadWait(int);
int PatchExecCS(int, int);
int PatchInitCS(void);
int mc_copy_patch(int);
void encode_data_002814E0(u8 *);
int decode_data_for_net(u8 *);
void McActSaveSet(int, u8 *);
int McActResult(void);
int func_A2FBC0(void);
int func_A2FBD0(s8 *step, s8 *cnt, u8 *buf, int len, int limit);     /* InetDNAS2PersonalEncode */
int func_A2FD30(s8 *step, s8 *cnt, u8 *buf, int len, int limit);     /* InetDNAS2PersonalDecode */

typedef struct PDT {
    s32 st;             /* our step */
    s32 frames;         /* frames in this step */
    s32 len;            /* plain / encrypted length */
    s32 calls;          /* glue calls so far */
    s8 gstep;           /* the glue's step byte */
    s8 gcnt;
    s8 lastg;           /* last logged glue step */
    s8 end;             /* our DP_end */
    s32 lasterr;
} PDT;

PDT pdt = { 0, 0, 0, 0, 0, 0, -1, 0, 0x7FFFFFFF };

static u32 rd32(u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24; }

static void log_glue(const char *what, int r)
{
    /* one line per change of the glue's step or of the stored sce result, and every result != 0 */
    if (pdt.gstep != pdt.lastg || dnerrno_0038AA50 != pdt.lasterr || r != 0) {
        printf("PDTEST %s call %d: ret %d glue step %d sce result %d (0x%08X) len %d\n", what, pdt.calls, r,
               pdt.gstep, dnerrno_0038AA50, dnerrno_0038AA50, dndst_len);
        pdt.lastg = pdt.gstep;
        pdt.lasterr = dnerrno_0038AA50;
    }
}

static void log_target(const char *when)
{
    u8 *t = (u8 *)PDT_TARGET;
    printf("PDTEST target 0x%08X %s: %02X %02X %02X %02X %02X %02X %02X %02X (%s)\n", PDT_TARGET, when, t[0], t[1],
           t[2], t[3], t[4], t[5], t[6], t[7], strncmp((char *)t, "PDTESTOK", 8) == 0 ? "PATCHED" : "original");
}

static void finish(int end)
{
    pdt.end = end;
    DP_end = end;           /* what PatchLoadinDNAS_Main leaves for the card code */
    DR_NO_0 = 0;
    DP_r_no = 0;
    DP_cnt = 0;
    printf("PDTEST done: %d\n", end);
}

#if PDTEST_MODE == 1
/* the plain list: "M-HUNTER", count, then {key, address, length, bytes} (PatchExecCS, docs/network.md 3.8) */
#ifndef PDTEST_PAD
#define PDTEST_PAD 0        /* experiment: this many zero bytes before the list (0x580 = the size of the header the
                               library's parser reads first, see docs/network.md 3.8) */
#endif
static int make_list(u8 *b)
{
    u32 *w;
    memset(b, 0, 0x10000);
    b += PDTEST_PAD;
    w = (u32 *)b;
    memcpy(b, "M-HUNTER", 8);
    w[2] = 1;               /* entries */
    w[3] = 0;               /* key 0: main (PatchExecCS(0, 0) at boot) */
    w[4] = PDT_TARGET;
    w[5] = 8;
    memcpy(b + 0x18, "PDTESTOK", 8);
    return PDTEST_PAD + 0x20;
}
#endif

int pdtest_main(void)
{
    u8 *tail = patch_buff + 0x20000;
    int r;

    if (pdt.end != 0)
        return pdt.end;
    pdt.frames++;
    switch (pdt.st) {
    case 0:
        printf("PDTEST mode %d start: tail version '%.10s' tag '%.4s' length %d, MediaVersion '%.10s', card port %d\n",
               PDTEST_MODE, tail, tail + 0xA, rd32(tail + 0x14), MediaVersion, *(s32 *)(card_w + 0x18));
        log_target("at start");
#if PDTEST_MODE == 1
        pdt.len = make_list(patch_buff);
        strncpy((char *)tail, (char *)MediaVersion, 0xA);
        strncpy((char *)tail + 0xA, "TEST", 4);
        *(s32 *)(tail + 0x14) = 0;
        printf("PDTEST plain list %d bytes: %.8s count %d key %d addr 0x%08X len %d\n", pdt.len, patch_buff,
               rd32(patch_buff + 8), rd32(patch_buff + 0xC), rd32(patch_buff + 0x10), rd32(patch_buff + 0x14));
#else
        pdt.len = *(s32 *)(tail + 0x14);
        if (pdt.len == 0) {
            printf("PDTEST no patch on the card (tail length 0): nothing to do\n");
            finish(1);
            return 1;
        }
        printf("PDTEST card slot: first bytes %02X %02X %02X %02X %02X %02X %02X %02X\n", patch_buff[0],
               patch_buff[1], patch_buff[2], patch_buff[3], patch_buff[4], patch_buff[5], patch_buff[6], patch_buff[7]);
        memcpy(patch_buff + 0x10000, patch_buff, 0x10000);
#endif
        Net_put_overlay_data(2);            /* dnas_ins.bin, as PatchLoadinDNAS_Main */
        pdt.st = 1;
        pdt.frames = 0;
        break;
    case 1:
        if (NetLoadWait(2) == 0) {
            printf("PDTEST dnas_ins loaded after %d frames\n", pdt.frames);
            pdt.gstep = 0;
            pdt.gcnt = 0;
            pdt.st = PDTEST_MODE == 1 ? 2 : 3;
            pdt.frames = 0;
        }
        break;
#if PDTEST_MODE == 1
    case 2:                                 /* personalize (encrypt for this console), no AuthInstall before */
        pdt.calls++;
        r = func_A2FBD0(&pdt.gstep, &pdt.gcnt, patch_buff, pdt.len, 0x10000);
        log_glue("PersonalEncode", r);
        if (r > 0) {
            printf("PDTEST PERSONALIZE OK: %d plain bytes -> %d bytes; first bytes %02X %02X %02X %02X %02X %02X %02X "
                   "%02X\n", pdt.len, r, patch_buff[0], patch_buff[1], patch_buff[2], patch_buff[3], patch_buff[4],
                   patch_buff[5], patch_buff[6], patch_buff[7]);
            *(s32 *)(tail + 0x14) = r;
            pdt.len = r;
            memcpy(patch_buff + 0x10000, patch_buff, 0x10000);
            pdt.gstep = 0;
            pdt.gcnt = 0;
            pdt.lastg = -1;
            pdt.st = 3;
            pdt.frames = 0;
        } else if (r < 0) {
            printf("PDTEST PERSONALIZE FAILED: ret %d, func_A2FBC0 %d (0x%08X), glue step %d\n", r, func_A2FBC0(),
                   func_A2FBC0(), pdt.gstep);
            PatchInitCS();
            finish(-1);
        } else if (pdt.gstep == 0) {
            /* the glue went 100 -> 0 returning its length 0: it treats that as "not done" and would start again */
            printf("PDTEST PERSONALIZE FAILED: the glue finished with length 0 (dndst_len %d, last sce result %d)\n",
                   dndst_len, func_A2FBC0());
            PatchInitCS();
            finish(-1);
        } else if (pdt.frames > 3600) {
            printf("PDTEST PERSONALIZE TIMEOUT (60 s) at glue step %d\n", pdt.gstep);
            PatchInitCS();
            finish(-1);
        }
        break;
#endif
    case 3:                                 /* decode (the boot path), on half 1 */
        pdt.calls++;
        r = func_A2FD30(&pdt.gstep, &pdt.gcnt, patch_buff + 0x10000, pdt.len, 0x10000);
        log_glue("PersonalDecode", r);
        if (r > 0) {
            printf("PDTEST DECODE OK: %d -> %d bytes, magic '%.8s' (%s), count %d\n", pdt.len, r, patch_buff + 0x10000,
                   strncmp((char *)patch_buff + 0x10000, "M-HUNTER", 8) == 0 ? "M-HUNTER OK" : "BAD MAGIC",
                   rd32(patch_buff + 0x10008));
            PatchExecCS(0, 0);
            log_target("after PatchExecCS(0, 0)");
            pdt.st = PDTEST_MODE == 1 ? 4 : 9;
            pdt.frames = 0;
        } else if (r < 0) {
            printf("PDTEST DECODE FAILED: ret %d, func_A2FBC0 %d (0x%08X), glue step %d\n", r, func_A2FBC0(),
                   func_A2FBC0(), pdt.gstep);
            PatchInitCS();
            finish(-1);
        } else if (pdt.gstep == 0) {
            printf("PDTEST DECODE FAILED: the glue finished with length 0 (dndst_len %d, last sce result %d)\n",
                   dndst_len, func_A2FBC0());
            PatchInitCS();
            finish(-1);
        } else if (pdt.frames > 3600) {
            printf("PDTEST DECODE TIMEOUT (60 s) at glue step %d\n", pdt.gstep);
            PatchInitCS();
            finish(-1);
        }
        break;
#if PDTEST_MODE == 1
    case 4:                                 /* save: the encrypted half 0 and the tail into the card image */
        mc_copy_patch(1);
        encode_data_002814E0(data_load_ptr);
        McActSaveSet(*(s32 *)(card_w + 0x18), data_load_ptr);
        printf("PDTEST saving to port %d, slot length %d\n", *(s32 *)(card_w + 0x18), rd32(tail + 0x14));
        pdt.st = 5;
        pdt.frames = 0;
        break;
    case 5:
        r = McActResult();
        if (r != -1) {
            printf("PDTEST SAVE %s: McActResult %d after %d frames\n", r == 0 ? "OK" : "FAILED", r, pdt.frames);
            decode_data_for_net(data_load_ptr);     /* the image back in plain form, as after the auto-load */
            finish(r == 0 ? 2 : -1);
        } else if (pdt.frames > 1800) {
            printf("PDTEST SAVE TIMEOUT (30 s)\n");
            finish(-1);
        }
        break;
#endif
    case 9:
        finish(2);
        break;
    }
    return pdt.end;
}
#endif
