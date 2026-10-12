# Changes for the owner to review

The owner asked (11 Oct 2026) that every change which gives up old code for a
new approach is listed here, even when we go ahead with it, so they can look
at it later. Each entry: what was dropped, what replaced it, why, and the
commit to revert. Newest first. Mark an entry "reviewed" once the owner has
looked at it; do not delete entries.

Agents and the coordinator: add an entry in the same change whenever you
remove or replace a working (or supposedly working) path, not only for
obvious workarounds.

## Open

### Player near-matches replaced by matched C: `case 0: default:` (12 Oct 2026, agent B)
- **Dropped:** src/main/pl/pl_normal_nm.c (not built: near-match normal_char_set / to_normal); the near-match
  copies of Pl_slash_calc, Pl_basic_flagset (now pl_slash.c / pl_flagset.c, PC-linked), to_normal, basic_com_ck, pl_mv021, pl_dm003 and pl_dm008 in src/main/pl/pl_nm.c (the PC linked these
  weak); `#pragma opt_common_subs off` on normal_char_set (a workaround for the same quirk).
- **Now:** all six are byte-matched C: normal_char_set.c holds normal_char_set and to_normal; f_pl.c holds the
  other four in place of their original-bytes stubs (two basic_com_ck jump tables now come from the C, two
  main:rodata lines). The PC runs this C.
- **Why:** the "reloaded constant in the switch default" open problem in docs/STATUS.md was the source writing
  `case 0: default:`; matched C replaces near-match copies. basic_com_ck's near-match was the most different
  (its game-mode 5/6 and 7/8 tests are ifs, the stick switch lists 0..4 in order).
- **To watch:** player idle stance (to_normal), the hunter's pad handling in town and field (basic_com_ck),
  flinch / knock-down states (pl_dm003/008), pl_mv021 (cart / carry), sharpness messages (Pl_slash_calc). enemy_trans
  is matched too (weapon_em.c, PS2 only; its near-match left weapon3_nm.c, which the PC never linked). Revert: the commit "pl: case 0: default:".

### PC runs more game C: monster neck joints, s16 damage (12 Oct 2026, agent B)
- **Dropped:** the no-op stand-in for mlCalcTransEM (build/pc/rt_gen.c) that enemy_mk called on the PC; the
  near-match files emmk_nm.c (all of it matched) and the copies of old_pos_save, init_pl_work, eft13_set_pos and
  several menu draw functions in the *_nm.c files (matched now).
- **Now:** mlCalcTransEM is matched game C (em/emmk01.c) and runs on the PC when game code calls enemy_mk (the tail
  cut, em_tail_off_sub): it turns the neck joints by em->neck[] and poses kind-specific joints. It reads game.bin's
  em_neck_tbl, now named in config/main_aliases.txt (the PC had only an empty D_63FF90 stand-in: the first build
  crashed in test_activities tail_cut, fixed by the name). hit_hit_sub_em (hit_nm.c, still a near-match) keeps the
  damage in an s16 as the original does.
- **Why:** matched C instead of a stand-in; the near-match is closer to the original.
- **To watch:** monster pose right after a tail cut; damage numbers over 32767 now wrap like on the PS2.
  Revert: the commit "emmk01: mlCalcTransEM and old_pos_save".

### f_quest as one file (12 Oct 2026, agent B)
- **Dropped:** the 22 per-run files of the quest file (src/main/quest/f_quest01..04.c, f_questb..r.c, qstb07.c,
  strg01.c; f_quest.c itself is now the whole file) and in src/main/quest/f_quest_nm.c the 70 near-match copies
  of functions that were matched (as runs or now).
- **Now:** src/main/quest/f_quest.c is the whole quest file (0x226C30-0x22C66C) as one TU, byte-identical, 12
  functions kept as original bytes (c_rawfuncs), their near-match C in f_quest_nm.c (PC, weak). The PC compiles
  f_quest.c with the va_list fix and its statics global. quest_em_die passes `(s16)` casts to em_work_serch2 (the
  original's K&R definition gives no prototype, the call still extends the arguments).
- **Why:** one file per original source file (MWCC context); stolen_item_stack, Ext_pick_point_init and
  em_work_serch2 match now. Tests passed: quest_loop, progression, urgent, activities, all_quests, save.
  Revert: the commit "f_quest: the quest file as one TU".

### f_pl as one file; em_dur_set's host copy; rodata slots renamed .data (12 Oct 2026, agent B)
- **Dropped:** the 57 per-run files of the player file f_pl (src/main/pl/pl01..pl48.c, plx03/04/09/12.c,
  pl_wall.c, pldmv.c, plegg.c, plegg2.c, pl_demo.c) and their 57 text / 17 rodata lines in config/c_files.txt;
  in src/main/pl/pl_nm.c the near-match copies of player_init0, pl_work_clr, pl_dm_value_sub, wall_act_ck,
  wall_vec_set, pl_mv060, pl_at009, pl_at012, pl_demo000, egg_com_ck, pl_egg03 (stale: matched C existed) and of
  the nine functions matched now; the host em_dur_set in src/pc/rt/rt_em.c; emsrch01.c / emsrch02.c (now
  em/emsrch.c).
- **Now:** src/main/pl/f_pl.c is the whole file (0x134950-0x14D1C8) as one TU, byte-identical, with six functions
  kept as original bytes (c_rawfuncs). The PC build compiles it with its statics made global (sed in
  tools/build_pc.sh) so pl_nm.c's remaining near-matches call it. The PC runs the game's em_dur_set C (near-match,
  emsrch_nm.c) instead of the host's copy. tools/build.py split_rodata_objects also handles objects whose rodata
  is renamed .data (a rodata slot in a data region) when they have several slots. include/plf.h: blend_calc
  takes s8, blend_set u16 (proven by the callers' code).
- **Why:** functions of one original source file compile differently alone (MWCC uses what it knows about earlier
  functions of the same file); as one file nine more player functions match. Fewer, truer files.
- **Cost / to watch:** the PC's player code now comes from one object; a behaviour change in the player would
  point here first (all PC tests passed: quest_loop, progression, urgent, activities, save, soak, all_quests).
  Revert: the commit "f_pl: the player file as one TU".

### Host caches kept for the whole run replaced by frees at stage loads (12 Oct 2026, agent F)
- **Dropped:** monster models cached per kind (em_kind_model), village NPC
  models, the common motion files (plcom / lbcom / selcom) and the NPC
  motion files (com_motion_load / em_motion_load), the hunter's own plcom
  copy and every 2D texture (flCreateTextureFromApx_mem) stayed loaded until
  the program ended. The player's weapon motions were made once at start-up.
- **Now:** at each stage load (st_model_load, load_stage_models) the host
  frees what nothing refers to any more and loads it again when the game
  asks (as the PS2 does): rt_2d_gc (2D textures no mem_tex slot holds),
  rt_village_files_gc (motion files no live motion set reads),
  host_models_gc (monster kinds with no em_work entry and no live motion set,
  NPC models outside the village). The motion table's part for a model is
  emptied before create_*_motion fills it again; rt_motion_gc no longer
  counts motion players of unused works, mot1 outside a blend, or a bset not
  mixed in. The weapon motions (wNN_tbl) are made again at every quest start.
- **Why:** free memory on the Xbox fell ~1.5 MB a quest loop (xemu: 9.9 MB
  in the village before the first quest, 6.1 MB after it) and the 2D texture
  table would have filled after ~100 loops. Also the village's lbcom motions
  run past the common part of the motion table into the hunter's weapon
  entries (500-521), so later quests played lbcom motions there.
- **Cost / to watch:** monster / NPC / motion files are read again on area
  changes (e.g. the camp's Aptonoth each time the hunter comes back): more
  disc reads on the Xbox. A motion handle kept somewhere the GC does not look
  would now point at a freed set (mset_get returns NULL: no pose).

### Viewer: catching up every lost tick dropped (12 Oct 2026, agent F)
- **Dropped:** in real-time runs the viewer ran every game tick it was behind
  the clock before drawing the next frame.
- **Now:** at most 4 ticks per drawn frame (CATCH_UP in viewer.c); beyond
  that the clock is moved on and the game slows down, with a "slow:" log line.
  Fixed-time runs (--shot, the tests) are unchanged.
- **Why:** with ticks dearer than 33 ms (xemu's opening movie) the old way
  drew one frame a second and never recovered; after a load pause it also ran
  a burst of ticks. The PS2 slows down too.

### Hunter ground height: viewer's min_y_of dropped (12 Oct 2026, commit 22c82ad3)
- **Dropped:** viewer.c min_y_of read the lowest vertex from each part's CPU
  skinning buffer (skinpos), or the unposed vertices when there was none.
- **Now:** fl_model_min_y (fl_model.c) skins the vertices on the CPU for the
  measurement itself, whatever the graphics backend does.
- **Why:** the nv2a backend skins on the GPU and keeps no skinpos, so the
  unposed vertices gave -104.6 instead of 0.3: on the Xbox the hunter floated
  about 1 m above the ground everywhere (plainly visible on the bed in the
  house, xemu 12 Oct). Same result as before on the PC (test_quest_loop OK).

### Memory card create on the Xbox: fopen "w+b" dropped (12 Oct 2026, commit 6a95cc3d)
- **Dropped:** mc_xbox.c sceMcOpen with the create flag opened a new file "w+b".
- **Now:** it makes the empty file with "wb", then opens it "r+b".
- **Why:** nxdk's pdclib maps "w+" to CreateFile(CREATE_ALWAYS |
  TRUNCATE_EXISTING), an invalid disposition, so every new save failed
  ("could not save" in xemu). A fix in xbox_libc.c's fopen wrapper would cover
  every caller; mc_xbox.c is the only "w+" user on the Xbox today.

### xemu screenshots: QMP screendump attempt dropped (11 Oct 2026, commit 44bfcddb)
- **Dropped:** tools/run_xemu.sh first asked xemu for a screenshot over its QMP
  socket, then fell back to grabbing the whole desktop.
- **Now:** xemu runs on its own hidden Xvfb display and the screenshot grabs
  that display. No window on the owner's screen.
- **Why:** xemu 0.8.136 has no screendump command, and grabbing the desktop
  meant xemu's window was on the owner's screen.

### Xbox movies: GPU YUV conversion dropped (11 Oct 2026, commit 7b611983)
- **Dropped:** gfx_nv2a.c kept one YUY2 texture per movie and let the NV2A
  convert YUV to RGB (`gfx_yuv_capable`, `gfx_create_texture_yuv`,
  `gfx_update_texture_yuv` in gfx.h and every backend).
- **Now:** the Xbox uses the same path as the PC: the decoder converts each
  frame to RGBA (sfd_rgba, integer BT.601) and it is uploaded as a normal
  texture.
- **Why:** on the real Xbox the movie colours were wrong (every hue flipped:
  green grass pink). Inverting the chroma bytes got close, but black areas
  came out teal, so the format was not understood. The CPU path is the one
  the PC already shows correctly.
- **Cost / to watch:** CPU time per movie frame (a 733 MHz CPU converting
  and swizzling ~0.3 Mpixel 30 times a second) and a texture allocation per
  frame. If the movie stutters, the GPU path is worth another look with a
  test pattern (revert 7b611983).

### Xbox network loader: polling for new builds dropped (11 Oct 2026, commit dcb0a0a3)
- **Dropped:** after the game exited, the loader stayed on screen and asked
  the laptop every 3 s for a new build, so builds could run with nobody at
  the pad.
- **Now:** the game exits to the dashboard; the next start (pad: A, A) sends
  the log and fetches the newest build. At most 3 network connections per
  start.
- **Why:** the Xbox's network froze after 5 polls (ping dead too), cause
  unknown; it then needed a power cycle each time.
- **To revisit:** likely cause found the same night: no program called
  nxNetShutdown() before XLaunchXBE, so the NIC kept running into the next
  program (the dumper then hung in nxNetInit right after the loader). The
  loader source now shuts the network down first; after the owner reinstalls
  it, polling could come back.
