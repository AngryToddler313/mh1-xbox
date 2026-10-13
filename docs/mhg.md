# Monster Hunter G (PS2, SLPM_658.69) in this repo

Owner's decision (13 Oct 2026, docs/DECISIONS.md): PS2 Monster Hunter G is the main release,
built from the same tree as MH1 JP ("One codebase, several games"; layout: "How G lives in the
repo: a VERSION switch"). This file says what carries over, how to build G, and the plan
for getting G running on the PC/Xbox platform layer. Written by agent B, 13 Oct 2026.
Numbers are measured with the tools named; estimates are marked as such.

## 1. What carries over (measured)

Details and method: docs/survey/mhg_vs_mh1.md. Per-function map: docs/survey/mhg_map.csv.

| | share of MH1 code bytes |
|---|---|
| identical in G (only call targets / global addresses differ) | 56.2% |
| same shape, some constants or struct offsets differ | 5.7% |
| edited (a counterpart >= 50% similar) | 32.5% (62% of it >= 90% similar) |
| rewritten (counterpart at a twin's call site, < 50% similar) | 0.7% |
| no counterpart found | 5.0% |

- By module, identical+shape: main 63%, game 60%, lobby 62%, yn 63%, select 32%.
  Some counterpart: 95% overall (game 99%).
- Code the PC port runs as game C (2589 MH1 functions): identical 1638 (0.44 MB), shape 213
  (0.08 MB), edited 652 (0.52 MB: 379 functions >= 90% similar, 219 at 70-90%, 54 below),
  rewritten 26, missing 60 (0.06 MB). Roughly half of the port's game C runs in G as is.
- Matched C (7374 MH1 functions): 4768 are clean identical twins; 858 of 1719 C files are
  made only of clean twins with the same layout in G. 805 of them now build byte-identical
  inside G (section 3).
- G-only code: about 0.36 MB of G's 3.9 MB of text: main 80 KB, sub_main 102 KB (27% of
  it), game 39 KB, lobby 128 KB (the plaza/gathering-hall code: plaza_*, lb_npc_pig_*,
  quest levels), select 10 KB. Most of it is only called from other G-only code, i.e. a few
  new subsystems rather than scattered edits. Not identified yet.
- Moves: G put a resident overlay sub_main.bin (0x4A0C80, right after main's .bss) holding
  577 former main functions (effects eft01/02/06/13/20, set13, stage drawing trans_*, hit
  tests, quest/reward/result, net sync); 218 player functions (pl_mv*, pl_at*, pl_dm*) moved
  from main into game.bin; lobby's OpenSSL/HTTP (1425 functions) became nethttp.bin
  (0xA06200, loaded with lobby). yn.bin now loads at 0xA06200 too.
- Data: every G file loads with our readers (tools/mhg_data_check.py: 0 rejects in 3676
  files). 1828 of 2002 shared AFS_DATA files are byte-identical. Format changes: collision
  lg/lw -> lwgNNN.bin (a link file of wall + ground HITS), quest files m001..m178 ->
  mNNNNN.mib with a 0x48-byte header (was 0x40). G's new monsters load and pose with
  their own motions (em36: tools/clay_dump.py on disc/mhg, rendered check
  build/mhg/show/em36.png, not committed).
- Struct layouts changed in places: e.g. GAME_W's `master` byte is at +0x3A in G (+0xD1 in
  MH1), seen in pl_master_ck. Work areas moved (MH1 0x3Fxxxx -> G 0x36xxxx), so C that
  still uses literal MH1 addresses (`*(u8 *)0x3F34C1`, m2c drafts) does not transfer.

## 2. Building G (Phase 1: done, byte-identical)

    tools/mwo_unpack.py is used in-process; nothing to extract by hand.
    VERSION=mhg tools/rebuild.sh          # split + build all seven modules
    VERSION=mhg python3 tools/build.py    # rebuild after editing C
    VERSION=mhg python3 tools/progress.py # G matched %
    tools/rebuild.sh                      # MH1, unchanged (all five OK)

- Modules: main (SLPM_658.69 0x100000-0x2EF200; text to 0x23A280, VU1 microcode, data from
  0x253D20, .sdata from _gp-0x7FF0 = 0x2EAB80, _gp 0x2F2B70 from .reginfo), sub_main, select,
  game, yn, lobby, nethttp (overlays from AFS_DATA.AFS, MWo3, unpacked in setup_split.py).
  dnas_net/dnas_ins are not split (replaced, as for MH1).
- Names: config/mhg/symbols/*.txt from tools/mhg_map.py (MH1 names at G addresses: 10428
  function twins + edited counterparts, 13.9k data names from twin operands, content matches
  and layout between mapped neighbours). config/mhg/*_aliases.txt PROVIDE every name (so a
  C file's statics still resolve for the asm around it) and the MH1 `D_XXXXXX` names the C
  uses, moved to G addresses.
- The text is cut into files at the G twins of MH1's file starts, so asm/mhg file names
  follow MH1's (f_em, f_eft17, ...).
- First result, 13 Oct 2026: all seven modules OK with no C (asm only).

## 3. Shared matched C in G

tools/mhg_cfiles.py writes config/mhg/c_files.txt: every MH1 C file whose functions are all
clean identical twins laid out the same way in G, with its jump-table/data slots moved
through the symbol at the slot start. config/mhg/c_skip.txt lists files that were tried and
differ from G after linking (tools/mhg_cdiff.py finds them): they read a struct field G moved
(game_w.master), use literal MH1 addresses, or call a function with no G twin (powf).
Those need G copies (src/mhg/...) or `#ifdef VERSION_MHG`.

Result: see the end of this section (filled in after the build).

## 4. Plan: G on the PC with our platform layer (fastest route)

How the MH1 PC port is built (docs/pc.md "Port runtime"): src/pc (gfx, audio, pad, fmt,
movie, net, menu, install: version-independent) + src/pc/rt (runtime glue: boot, flow,
data import from the user's ELF/overlays by address, overlay call routing, native stand-ins
for undecompiled main functions) + about 800 game C files (tools/build_pc.sh lists).

### What in src/pc/rt is game-specific

1. **Addresses and file names.** SLPM_654.95 / AFS sizes in install.c and viewer.c, overlay
   load address 0x533980 (rt_mem.c OVL_GAME_VRAM, rt_data.c LB_VRAM, rt_online.c LBA),
   ptmat_tbl 0x3018F0, rt_overlay.c's `func_6229B0`-style address names. About 280 distinct
   hex constants in src/pc; most are comments, a few dozen are code.
   -> one per-version table (rt_version.c: ELF name, sizes, overlay addresses, sub_main) and
   names instead of addresses (generated from the version's symbol files). *1-2 days.*
2. **Data import** (rt_data.c, tables.txt, gen_rt_tables.py, gen_rt_auto.py, pc_abs.py): by
   name through config/symbols -> config/mhg/symbols. 756 of the 1054 tables already have a
   G name; the rest need more data mapping (mostly tables only edited code touches).
   **G has no relocation sections**, and rt_mem.c turns pointer words into host pointers
   using MH1's R_MIPS_32 list. For G: carry MH1's pointer positions over through the
   content-matched data objects (7914 matched) and mark G-only tables by hand or by a
   "looks like an address in a module" scan; write the list into config/mhg (positions
   only). *3-4 days.*
3. **Overlays.** Natively there are no overlays; the port links game C from all modules into
   one binary and keeps lobby/select as host copies of the overlay image. G adds sub_main
   (always resident: easy, link it like main) and nethttp (replace like DNAS: the PC never
   speaks SSL to Capcom servers). The rt_overlay.c routing table becomes generated per
   version. *1-2 days.*
4. **File formats.** HITS reader: read lwgNNN.bin (entry 0 wall, entry 1 ground) instead of
   lg/lw. Quest .mib: the G header (0x48). Both small. *1 day.*
5. **Text/English.** G's English patch instead of MH1's (docs/english.md; translators'
   terms first). The text renderer and font code are shared. *depends on the patch.*

### Game C for G

- **Identical/shape functions (about half of what the port runs):** the MH1 C is reused.
  Shape twins need the struct/constant difference: write a tool that, from the 726 shape
  twins, lists for each base symbol (game_w, player_work, quest_w, ...) which MH1 offset
  became which G offset, and gives G headers per version (`#ifdef VERSION_MHG` field
  blocks, or G copies of the structs). *Tool 1-2 days, headers 3-5 days.*
- **Edited functions the port runs:** 652 functions, 0.52 MB; 379 are >= 90% similar. Each
  gets a G copy (src/mhg/...) made from the MH1 C plus the asm diff, checked with the same
  matching setup (G matching works like MH1's: same compiler). Agent rate in MH1 for
  "known logic, small change" work was tens of functions a day. *Estimate: 3-5 agent-weeks;
  the >= 90% group first, by what the first quest needs.*
- **Missing/rewritten in the run-set:** 86 functions; decompile from G's asm. *1 agent-week.*
- **G-only systems:** identify first (call graph from known code: who calls the clusters;
  strings; then name). The plaza (lobby) and the sub_main clusters are the big ones; new
  monsters em35-em50 seem to reuse the monster engine with new tables plus edited AI code
  (game.bin only has 39 KB of G-only code). *Identification 2-3 days; porting unknown,
  likely 2-4 agent-weeks for what offline play needs.*
- **Stop-gap for the rest:** like MH1, native stand-ins (rt_*.c) and PS2Recomp were the way
  to run code not decompiled yet; G has no symbols, but our symbol map gives names to 75% of
  its functions, so the same approach works.

### Fastest route (order)

1. Version table + generated names in src/pc/rt; build the PC binary with VERSION=mhg
   linking only identical/shape game C and stand-ins: show G's village stage and a G-only
   monster in the viewer from G's own disc (*about 1 week*).
2. Data import with transferred pointers, lwg collision, quest .mib: a G quest loads (*1
   week*).
3. G struct headers + G copies of the edited functions on the quest path (player, monster
   core, quest flow, HUD): first G hunt playable (*3-5 agent-weeks*, parallel over agents
   by module).
4. G-only systems, then English text, then online (*afterwards*).

With four agents working in parallel, a first playable G hunt on the PC is an estimate of
3-5 weeks; the village/menus of G-only features come after. These are guesses based on
MH1's pace, not measurements.
