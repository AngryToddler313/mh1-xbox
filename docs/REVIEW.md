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

### Browser allocator: unbuilt near-match copy lb_ao.c removed (12 Oct 2026, agent B)
- **Dropped:** src/lobby/f/lb_ao.c (agent F's whole-file near-match of the browser cell allocator, not built).
- **Now:** all six functions are byte-matched in src/lobby/f/lb_ao01.c (BsMemRealloc added, 0x5E8330-0x5E867C).
- **Why:** obsolete copy. **To revert:** the commit "lobby: BsMemRealloc matched; lb_ao.c removed".

### Browser URL helpers: one TU lb_url.c (12 Oct 2026, agent B)
- **Dropped:** the runs src/lobby/f/lb_{m12,z135,z19,ap01,z106,z20,gdr12,gap01,z21,gap02,ap02}.c and agent F's
  whole-file near-match src/lobby/f/lb_ap.c (not built anywhere).
- **Now:** src/lobby/f/lb_url.c, 0x5E8890-0x5E9960, all 25 functions byte-matched (BsUrl*, bs_url_*, the
  Shift-JIS/EUC kanji conversion). The draft-style run C (F(s8, ...) offsets, int pointers) was rewritten as plain C;
  it still matches. Linked on the PC too (tools/pc_lobby_matched.txt): KanjiEuc2SjisEx and bs_url_cmp_list were
  stand-ins there.
- **Why:** the static helpers must share the TU with their callers (MWCC keeps caller values in a3/t0 across calls
  to a static whose register use it knows); one TU replaces 12 files.
- **To revert:** the commit "lobby: browser URL helpers matched as one TU (lb_url.c)".

### Lobby client: 29 b/ run files merged into lb_cli.c (12 Oct 2026, agent B)
- **Dropped:** src/lobby/b/lb_{by95,by34,bz127,bz160,bz49,bz161,bz50,by39,by01,bz51,by03,cb05,by02,cb04,rm01,by04,bz52,
  bz53,bz54,cb02,cb01,bz55,bz56,bz57,bz58,by05,bz59,cb03,by06}.c (one or a few matched functions each, 0x5C0680-0x5C1B5C:
  mail / net comment requests and the server notice callbacks) and their entries in the ONLINE=1 list of
  tools/build_pc.sh.
- **Now:** the same C (unchanged apart from per-run type names, `BrPersonalData_c24`, and aliases for objects a run
  declared with its own type: config/lobby_aliases.txt) is the tail of src/lobby/f/lb_cli.c, which now covers the
  original compilation unit 0x5B7020-0x5C1B5C. CallBack_Event_MatchEntryUser / CallBack_Event_RoomLeaver (not matched)
  are original-bytes stubs there; the PC keeps their C in src/lobby/b/nm/.
- **Why:** one original source file = one TU (BRIEF); fewer files. Bytes and the PC behaviour are unchanged.
- **To revert:** the commit "lobby: lb_cli.c covers its whole compilation unit".

### Online HTML text: PC near-match copies replaced by matched C (12 Oct 2026, agent B)
- **Dropped:** src/lobby/b/nm/html_text.c (Split_TagCode, Analysis_StringData, Display_StringData),
  src/lobby/b/nm/Analysis_TagCode.c, src/lobby/b/nm/check_halfcode.c and src/lobby/b/nm/nwDispStr_Html.c, which the
  ONLINE=1 PC build linked (written from the asm on 11 Oct, never compared with check.py).
- **Now:** the six functions are byte-matched C at the end of the lobby-client TU src/lobby/f/lb_cli.c (now
  0x5B7020-0x5C0678); check_halfcode is still the original bytes on the PS2 (2 lines off as C) and its C for the
  PC sits in src/lobby/f/lb_online_nm.c with the other lb_cli holdouts.
- **Why:** the PC runs the game's own code for server messages / error dialogs instead of a hand-written copy.
- **To revert:** the commit "lobby: HTML text renderer matched in lb_cli.c".

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
### Mode menu / extras menu near-match copy replaced by omake.c (12 Oct 2026, agent C)
- **Dropped:** src/main/omake/omake_nm.c (the whole-file near-match copy the
  PC linked for the title mode menu and the extras menu) and the four
  matched run files omakeb.c, omakec.c, omaked.c, omakee.c.
- **Now:** src/main/omake/omake.c, the whole original file (0x23A0F0-0x23BE10)
  as one translation unit with its statics. 17 of 20 functions match;
  mode_sel, disp_mode_menu and disp_omake_menu link the original bytes on
  the PS2 build and use their near-match C on PC/Xbox. key_rept_du,
  Sel_back_disp and Sel_menu_disp are newly matched. The PC BOOT list in
  tools/build_pc.sh and tools/pc_patch.py's Tsk_Exit(tsk) fix now name
  omake.c; omake_nm left WEAK_B.
- **Why:** owner's rule: matched game C over near-match copies. The C is the
  same code as omake_nm with source-level rewrites that keep the meaning
  (Sel_back_disp builds the colour from one byte, Put_2TF gets only the
  sprite; key_rept_du uses ++cnt; disp_omake_menu computes x as int).
- **Checked:** tools/build_pc.sh, test_menu, test_name_entry,
  test_quest_loop, test_progression, test_movie.
- **To revert:** the agent-C commit "omake: whole-file omake.c TU".

### Host pad driver and system_w_set replaced by the matched game C (12 Oct 2026, agent C)
- **Dropped:** src/pc/rt/rt_pad.c's own copy of ioRead_sub (read_port0 built
  Psw from the host pad: bit table, stick angle/power, trg/rel, key repeat)
  and src/pc/rt/rt_boot.c's copy of system_w_set.
- **Now:** src/main/sys/f_ioread.c (the whole original f_ioread file, all 13
  functions matching) is in the PC/Xbox build for ioRead, ioRead2 (with the
  static ioRead_sub) and system_w_set (PICK_X in tools/build_pc.sh). The
  host only plays the fl pad library: it fills flpad_adr[0] (buttons, stick
  direction bits beyond half way as before, stick angle in degrees and power)
  and calls the game's ioRead once per tick; ports 1-3 read as unplugged.
  The pad structures are in include/ioread.h.
- **Why:** owner's rule: matched game C over host imitations. Behaviour is
  the same by construction (same bit table, same angle formula, same repeat);
  ports 1-3 now get the game's own "unplugged" update every tick instead of
  staying untouched zeros.
- **Checked:** test_quest_loop, test_progression, test_menu, test_name_entry,
  test_pick, test_urgent, test_frog, test_activities (37) pass; Xbox build
  links (0 failed). Not run in xemu.
- **Also dropped:** the runs sys/io01-io04, ior01 and the near-match copies
  ioread_nm.c / mmv_nm.c (replaced by f_ioread.c).
- **To revert:** the agent-C commit "sys: f_ioread.c whole-file TU; PC uses
  the game's ioRead/system_w_set".

### Model work file: 13 runs and 6 near-match copies replaced by one translation unit (12 Oct 2026, agent C)
- **Dropped:** the runs model/gm01-gm05, mkm01, hp01-hp04, light03 (get_mdlw_ptr),
  crmdl03 (Pl_model_id_set) and clay/get_clay_ptr, and the near-match working
  files mkm_nm.c, getm_nm.c, getsh_nm.c, heap_nm.c, armor_nm.c; the copies of
  pl_create_model / weapon_create_model in crmdl_nm.c and of get_mdlw_ptr in
  light_nm.c. None of them was in the PC build.
- **Now:** src/main/model/f_get.c is the whole original file 0x121B70-0x1246D4
  in address order with its statics. Newly matching: Sethierarchy, mkModel,
  mkModel3, mkModel4, model_work_set2, release_model, model_work_free. Seven
  functions stay as original bytes (config/c_rawfuncs.txt) with their
  near-match C beside them: get_start_material/hierarchy/clay/mdlw,
  pl_create_model, weapon_create_model, armor_create_model.
- **Also:** getm_nm.c had model_work_set2's parameters in the wrong order
  (skin and the mkModel flags swapped); the matched version is
  (idx, mat, tex, texlist, flags, skin, type).
- **Why:** one TU per original file is what makes MWCC allocate registers
  like the original; old runs and copies would only drift.
- **To revert:** the agent-C commit "model: f_get.c whole-file TU".

### Xbox Back + Start: the normal shutdown is skipped (12 Oct 2026, commit 583d10b5)
- **Dropped:** Back + Start set `running = 0` and let main end normally (SDL audio close,
  gfx shutdown, then the atexit log screen).
- **Now:** it calls xbox_log_quit(): the log screen, then straight to the dashboard.
- **Why:** the normal end hung on hardware while the audio thread was starved. With the GPU
  waits yielding (same commit) it would likely work again, but nothing in that shutdown
  matters when the console reboots, and the short path can't hang.

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

### nv2a: GPU memory freed at once, with a wait for the GPU (12 Oct 2026, agent E)
- **Dropped:** gfx_release_texture / gfx_release_clay waited for the GPU to go
  idle (`wait_idle`) and gave textures, palettes and vertex buffers back to
  the kernel immediately.
- **Now:** they go on a list and are freed 3 drawn frames later
  (`gpu_free_later`, `gpu_free_pending` in gfx_begin_frame); an allocation
  that fails frees the whole list first and tries again.
- **Why:** NV2A caches texels and the palette by address and has no known
  method to drop that cache; freed at once, the next screen's texture often
  landed on the address the last one had, in the same tick. Suspected cause of
  the owner's "screens flash as they fade in" on hardware (not visible in
  xemu, which re-reads memory). Also spares a GPU wait per release.
- **To watch:** peak memory at a stage change (old and new textures overlap
  until the next frames; the retry covers an allocation failure). Revert the
  commit that adds `gpu_free_later` to go back.
