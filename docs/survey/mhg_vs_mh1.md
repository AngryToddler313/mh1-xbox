# PS2 Monster Hunter G vs MH1: per-function map (13 Oct 2026, agent B)

`tools/mhg_map.py` maps every MH1 function to its G counterpart (G is stripped; MH1 has
symbols and link relocations). Output: `docs/survey/mhg_map.csv` (one row per MH1 function
and per G-only function) and `config/mhg/symbols/*.txt` (MH1 names at G addresses).
Classes:

- identical: same instructions; only MH1's relocated fields (call targets, addresses of
  globals, gp offsets) differ. The MH1 C compiles to G's bytes once the names point at G.
- shape: same instruction shape, some non-relocated 16-bit immediates differ (struct
  offsets, constants).
- edited: a counterpart found by similarity (between matched neighbours in address order,
  at the call sites of a twin, or by a global 5-gram search), at least 50% similar
  (difflib ratio over opcode sequences).
- rewritten: the callee at a twin's call site, less than 50% similar.
- missing: no counterpart found.

How: exact-shape hits (unique, then by address order) give anchors; gaps between anchors are
aligned with Needleman-Wunsch on similarity; call sites of twins pair callees; leftovers go
through a 5-gram search (mutual best, >= 0.6). G function boundaries come from jal targets,
`jr ra`/`j` + delay slot + nop padding at 16-byte boundaries, and the twins themselves (a
twin may split or merge a guessed G boundary). Only exact twins may cross the expected
module mapping (main -> main/sub_main/game, lobby -> lobby/nethttp).

| MH1 module | funcs | identical | shape | edited | rewritten | missing | identical+shape (bytes) | any counterpart (bytes) |
|---|---|---|---|---|---|---|---|---|
| main | 6978 | 5374 | 223 | 855 | 25 | 501 | 63.1% | 92.7% |
| select | 44 | 22 | 0 | 18 | 2 | 2 | 31.8% | 94.2% |
| game | 2640 | 1916 | 230 | 445 | 13 | 36 | 60.0% | 99.1% |
| yn | 141 | 91 | 27 | 12 | 0 | 11 | 63.3% | 96.0% |
| lobby | 3486 | 2299 | 246 | 647 | 15 | 279 | 62.3% | 94.4% |
| **all** | 13289 | 9702 | 726 | 1977 | 55 | 829 | **61.8%** | **95.0%** |

By bytes of MH1 code: identical 56.2%, shape 5.7%, edited 32.5% (of that, 62% is >= 90%
similar, 28% 70-90%, 10% 50-70%), rewritten 0.7%, missing 5.0%.

- The old exact-shape count (52%, mhg_match.py) missed lobby's OpenSSL/HTTP code: G moved it
  into the new overlay nethttp.bin (1425 MH1 lobby functions are there). It also missed
  functions whose G boundaries differ by a trailing nop.
- Where MH1 code went: main -> main 5676, -> sub_main 577, -> game 218; lobby -> lobby 1781,
  -> nethttp 1425; game -> game 2603.
- G-only code (no MH1 counterpart): main 80 KB (6% of its text), sub_main 102 KB (27%),
  game 39 KB (3%), lobby 128 KB (19%), select 10 KB (33%), nethttp 3 KB, yn 0.5 KB. About
  0.36 MB of new code in all, against 3.9 MB of G text.
- Code the PC port runs as game C (functions defined in build/pc/*.o of src/game, src/main,
  src/lobby; 2589 MH1 functions): identical+shape 46.9% of bytes, edited 47.1% (player,
  monsters and quest code changed most), missing 5.1%. Counterparts exist for 95%.
- Matched C (config/c_files.txt minus c_rawfuncs, 7374 functions): 4768 are clean identical
  twins (also every call goes to the callee's twin), 0.95 MB of 2.13 MB. 858 of 1719 C files
  consist only of clean twins laid out the same way in G (0.40 MB): those files should build
  byte-identical at G addresses unchanged.
- Guesses: "edited" pairs at 50-70% similarity in the per-monster files (em_*) can be the
  wrong copy among near-identical siblings; counts are right in size, single names less so.
  Data names in config/mhg/symbols come only from relocated operands of identical/shape
  twins (2465 names; 22 addresses had conflicting votes, majority kept).

## G's data through our readers (13 Oct 2026, agent B)

`tools/mhg_data_check.py` builds `tools/fmt_check.c` (the PC port's own readers in
src/pc/fmt: AFS, Meltw, AMO, AHI, APX, AAN/tbl, HITS, SND, ADX) and runs it over every file
of AFS_DATA, AFS00 and AFS01 of both discs, then compares by file name.

- **Every G file the readers know loads**: 0 rejects in 3347 + 124 + 205 files (MH1 has two
  rejects, kari_amh.bin and test00_amh.bin, test leftovers). Models (1377 *_amh.bin incl.
  skeletons), textures (957 bare APX, 462 *_tex.bin), motion tables (38 *_tbl.bin), sound
  banks (205 .snd/.snp, every VAG scans), ADX streams and Sofdec movies.
- Files in both games: AFS_DATA 2002 (1828 byte-identical, 73 different bytes but identical
  reader statistics, 101 different content such as reworked armour, 7 monster motion tables,
  10 stage models); AFS00 108 (106 identical); AFS01 160 (123 identical, 13 same stats).
- New in G: 1345 AFS_DATA files (101 weapons, ~40 per armour slot, 15 monsters, 7 stages,
  245 quest files), 16 streams/movies (MHG_OP.sfd, ROLL*.sfd credits, ...), 45 sound banks.
  All load.
- **Format differences found:**
  1. Collision: MH1's separate `lgNNN.bin` (ground) and `lwNNN.bin` (wall) HITS files are
     one `lwgNNN.bin` in G: a link file of two HITS blobs, wall first then ground. For 39 of
     the 78 MH1 stages they are exactly MH1's two files; the other 39 have edited collision.
     The HITS format itself is unchanged.
  2. Quest files: MH1 `m001..m178.mib` became `mNNNNN.mib` (groups 00xxx, 01xxx, 02xxx,
     03xxx, 10xxx, 20xxx, 60xxx: 245 files). The header grew from 0x40 to 0x48 bytes (one
     more pointer, then extra fields); text still at the end. Not decoded further.
  3. Movies: G's new movies are Sofdec too (pack header checked only).
- The game finds files by AFS index from tables in the executables (stage_hit_data_f etc.),
  so a G build reads those tables from G's ELF/overlays, at G addresses (symbol map above).

# PS2 Monster Hunter G vs MH1: how much carries over (10 Oct 2026)

Measured with tools/mhg_match.py. It counts MH1 functions whose instructions show up
unchanged in G, ignoring addresses, jump targets and 16-bit immediates.

| MH1 module | functions | found in G | share of code bytes |
|---|---|---|---|
| main   | 6978 | 5040 | 65.5% |
| game   | 2640 | 1797 | 55.0% |
| lobby  | 3486 |  915 | 22.5% |
| select |   44 |   11 | 23.4% |
| yn     |  141 |   96 | 57.4% |

- About 52% of all MH1 code, and about 61% outside the lobby, is in G unchanged in shape.
  Most of the rest is probably edited versions of the same functions, not new code; this
  count does not measure that.
- G moved part of main into a new overlay, sub_main.bin. 295 MH1 main functions turned
  up there.
- AFS archives: 2003 of the 2318 MH1 AFS_DATA names are also in G (G has 3347). Every
  AFS00 and AFS01 name from MH1 is in G as well.
- G is stripped, so the names have to come from MH1. Matched functions give those names
  directly.
- G keeps MH1's compiler and overlay layout, so the same matching setup should work.
- The Wii G (PowerPC, a different compiler) cannot reuse the matched C byte for byte. Only
  the C's meaning carries over.

## MH2 (Dos), SLPM_662.80 VER 1.04, English patch v1.03 image

Run: `tools/mhg_match.py mh2`. The overlays are stored uncompressed in DATA.BIN; carve
them at each 'MWo3' magic, 0x40 + text + data bytes long.

- Same compiler ("MW MIPS C Compiler (2.4.1.01)"), same MWo3 overlay system, same
  overlay names, plus new ones: gm_sub, plsel, lbguild, and stubs for test.
  The ELF is stripped.
- The game is much bigger. game.bin's text is 1.9 MB (MH1: 1.08 MB) and sub_main's is
  1.1 MB. The main ELF shrank to 0.8 MB because code moved into sub_main.
- Unchanged MH1 functions, by share of code bytes: main 27%, game 10%, lobby 6%, yn 26%.
  Overall about 17%.
- Movies are CRI Sofdec (MWSFD/PS2EE 3.33). MH1 uses plain MPEG-2.
- Verdict: still the same engine family, and the tools, compiler setup, file formats and
  PC/Xbox platform layer carry over. Most of the game code is new or rewritten, so MH2
  would be close to a new decompilation that has MH1 to lean on.

## Monster Hunter Portable (PSP, ULJM, 2005)

- BOOT.BIN is an unencrypted ELF. Its .comment says "MW MIPS C Compiler (2.4.1.01)" / PSP,
  so it uses the same Metrowerks linker family. It is stripped: the symbol table has a
  single entry.
- The code sits in the 0.7 MB ELF text plus MWo3 overlays in USRDIR/DATA.BIN. That file
  has a table of sector offsets and no names, and holds 5821 files, 105 of them MWo3.
  Overlays: game_task 1.2 MB, game_sub 0.45 MB, lobby_task 0.38 MB (ad-hoc play),
  download_task, plus 92 small stageNN overlays. About 3 MB of code in all, close to G.
- Code reuse: 14 MH1 functions are found unchanged (tools/mhg_match.py mhp), and none
  match even by instruction shape (opcodes only, 64-bit ops mapped to 32-bit). The PSP
  back end schedules and allocates registers differently, so nothing transfers at the
  byte level.
- Data reuse: 9.6% of MH1 game.bin's distinct 32-byte chunks appear verbatim in MHP's
  code and data. That is mostly game tables, which make up about a quarter of game.bin.
  main 3%, lobby 1%. File formats changed for the PSP GPU (.TMH textures, PSMF movies,
  AT3 audio).
- Verdict: matching decompilation would start from zero. MH1's C helps only as a guide to
  what each function does. Pick PS2 G or MH2 first.
