# Release readiness: Monster Hunter G (PS2) on PC and the original Xbox

Audit by agent A, 15 Oct 2026. Monster Hunter G is the project's main release (owner, 13 Oct 2026); this is the
G counterpart of docs/RELEASE.md (MH1) and uses the same layout. Four player-visible gaps found on the way were fixed
in the same change (section 4); everything else is listed for its lane.

How things are marked:
- **[verified]**: seen today in a hidden run, a coverage run or the code (says which).
- **[docs]**: taken from docs/mhg.md, xbox.md, text_coverage.md, REVIEW.md, COORDINATOR.md or agent notes, not re-run.
- **[inferred]**: my reading of the code or docs; not tested.

Owner lanes: **quest** (agent A: quests, monsters, items, hunt), **main** (agent C: menus, title, pause menu, player),
**village** (agent D: lobby overlay, shops, elder, NPCs), **text** (agent E: English), **Xbox** (agent B/F: nxdk build,
nv2a, loader, hardware), **platform** (src/pc host code, packaging), **owner** (a decision or a hardware check).

## 1. What "release-ready G" means

| # | Criterion | How to check | Today |
|---|---|---|---|
| G1 | Boots from the player's own G disc, nothing Capcom shipped | PC: an installer for the G ISO; Xbox: a player path to get the 1.1 GB of disc files onto E:\Games\MHG | Xbox: `./mh1 xbox DISC` makes dist/xbox/MHG; the loader's HTTP data install exists (tested in xemu; the owner's first try did not start the loader) [docs]. PC: `./mh1 linux|windows` **refuse a G disc** ("Xbox only for now", mh1:63) [verified]; the G PC build opens no window unless MH1_SHOW=1 (agents' rule), tools/play_g.sh is the owner's launcher [docs] |
| G2 | Title -> NEW GAME -> character -> save -> village -> CONTINUE | hidden runs (section 3) | title, NEW GAME, creation, card save, village intro, Kokoto village: OK. CONTINUE crashed in the game menu whenever a save was on the card: **fixed today** (section 4). The 5th row crash is fixed (GB2, 15 Oct, agent C: it was NETWORK OPTIONS, not an EXTRA menu). OPTIONS opens and leaves through the card save [verified 15 Oct] |
| G3 | Every offline quest playable from the elder to the reward | tools/test_all_quests_g.sh (98 quests + carts), the elder's lists, a human play-through | 98/98 + carts pass with test aids [verified today]. The elder's quest lists were empty ("1 / 0"): **fixed today**. No human has finished G on the port |
| G4 | The village works: shops, forge, box, elder, counter | tools/test_gvillage.py (16 cases) | 16/16 OK [verified today]. Equipment detail / skill pages not ported (REVIEW "G village round 2") [docs] |
| G5 | Saves reliable | card save, CONTINUE, bed save | new-game save + CONTINUE OK today [verified]; G save is 78032 bytes, own Xbox title id [docs]. No G save test script (MH1's test_save.sh is MH1 only) [verified] |
| G6 | No stand-in or MH1-only logic on a path a player reaches | coverage (section 3), F8 reports | 43 distinct stand-ins reached (most harmless, section 3); 945 entered functions run MH1 logic where G's code differs; 37 zero-filled G tables still read |
| G7 | English | official US text where it exists, own text for the rest | G 47% official English (docs/text_coverage.md): weapons (G table) 0%, armour (G table) 9%, items (G table) 2%, NPC lines (G table) 5%, quests 44% [docs]. Source for G-only text is an owner decision (Monster Hunter Freedom idea) |
| G8 | Frame rate on the real Xbox | game log, owner's eye | not measured on hardware. xemu (software GL on the laptop, not a measure of the Xbox): G quest 10 8.1 fps, G costs the same as MH1 in the same scene [docs] |
| G9 | Memory on the Xbox | "alive" lines in the log | xemu: village 15.5-16.5 MB free, quest 10 20.8 MB, first village load low point 10.6 MB (MH1 village 16.5 MB) [docs] |
| G10 | Online (G's plaza / gathering hall) | ONLINE=1 build, own server | **nothing exists for G** [verified: no VERSION_MHG in src/pc/net or rt_online.c; build_pc_mhg.sh never passes ONLINE]. G's online code is lobby (128 KB edited plaza code) + nethttp.bin (0.4% matched) [docs]. Owner decision pending (own server only vs PS2 servers) |
| G11 | Controls / settings | PC F10, Xbox Back+L3 menu | shared with MH1 (Xbox settings menu, 16:9) [docs]; G's own OPTIONS screen not checked |
| G12 | Install / run steps a player can follow | a player page per platform | Xbox: docs/xbox.md owner steps; no player page for G; PC: none (G1) |

## 2. Gap list, by player impact

### Blockers

| # | What the player sees | Where | Status | Lane | Basis |
|---|---|---|---|---|---|
| GB1 | Most G-only text is Japanese: weapon and armour names, G items, the G NPC lines, half the quests | rt_text, tools/text_map.py; docs/text_coverage.md | 47% official English; the rest needs a source (owner: Monster Hunter Freedom for G content, own translation in its style) | text + owner | [docs] |
| GB2 | **FIXED 15 Oct (agent C).** The GAME MENU rows are NEW GAME / CONTINUE / GALLERY / OPTIONS / NETWORK OPTIONS: G has no MH1-style omake menu. Its Omake_task (main 0x1180F0) is NETWORK OPTIONS and the real "omake" is the GALLERY (sub_main 0x4A9960); MH1's omake menu ran for both on the PC (zero tables) and crashed on the 5th row (strlen of omake_str) | src/main/omake/omake_g.c (new; MH1's omake menu is fenced out of omake.c for G); build/C/gb2/ has the pictures | done: NETWORK OPTIONS (rows 0/1 change the two network bytes, EXIT saves; rows 2/3 need the PS2 network stack and answer with the "can't" sound); GALLERY (4 pages, page-turn wipe, cell wobble, NEW markers, unlock flags, movies on the port's player, the credits scroll, the save prompt on leaving). **Not ported:** the 15 event scenes (G 0x4A91D0 loads a stage and a monster into the game's demo camera: they answer like a closed entry), the movie clips behind the credits (G's cue list at 0x50B678) | main | [verified: hidden runs, pictures] |
| GB3 | G is not on the PC for players: `./mh1 linux|windows` refuse a G disc; the G PC build is hidden unless MH1_SHOW=1 | mh1:63, viewer.c G hidden rule | **done 15 Oct (agent D)**: `./mh1 linux|windows|xbox` build G from the G disc into dist/<platform>/MHG, play scripts set MH1_SHOW=1; docs/PLAYING.md has a G section | platform | [verified] |
| GB4 | Nobody has played G through on the port; every quest "passes" with test aids (warps, x40 damage, god mode, RT_PL_SLAY) | tools/test_all_quests.py G mode | process gap; owner F8 play-testing | owner + all | [verified: aids in the script] |
| GB5 | On the Xbox, G has not been played on hardware (installed through the loader, pad fixed; hands-on test pending). The loader's data install did not start on the owner's first try | docs/xbox.md "Owner's first try", "G on the owner's Xbox: no pad" | waiting for the owner | Xbox + owner | [docs] |
| GB6 | 945 functions the G PC enters in ordinary play are MH1 C where G's code differs (628 edited, 19 rewritten, 71 missing in G, 227 same shape with other constants); each may behave like MH1 instead of G. Many are layout-only edits the PC already absorbs (it keeps MH1's EMW / lb layouts), so the real number is lower | docs/survey/mhg_cov_g.txt (G-CHANGED list); biggest groups: game/em 102, game/set 37, main/menu 34, game/shell 33, main/ud 28, main/pl 25, lobby/b 39, plus 359 in generated copies | needs triage per lane (sort by `sim`, read the low ones against G's asm with tools/mhg_fdiff.py) | all lanes | [verified: coverage today]; the effect of each [inferred] |
| GB7 | No online play in G (plaza, gathering hall, co-op) | section 1 G10 | not started; blocker only if online is in the G release | platform + server + owner | [verified] |

### Noticeable

| # | What the player sees | Where | Status | Lane | Basis |
|---|---|---|---|---|---|
| GN1 (FIXED today) | CONTINUE (any save on the card) crashed as soon as the game menu came up | omake.c G disp_mode_menu read D_528538 / D_528550 / D_002EC668 as zero arrays (strlen(NULL)) | fixed: config/mhg/pc_names.txt + tools/gen_rt_auto.py | main | [verified: crash before, village after] |
| GN2 (FIXED today) | The elder's quest list showed "1 / 0" and stale rows: G's list builder never ran | lb_village_nm.c vs_square_init_init (G) had a stand-in where G calls Lb_make_quest_tbl_local; G's version exists in lb_guild_g.c | fixed: the call is back; the elder shows "1 / 5" with G's quests | village | [verified: shots before/after] |
| GN3 (FIXED today) | Quest reward screen: the selected item's name and count were blank; "time left" format empty | f_rewardd.c reward_itembox / f_rewardc.c disp_reward: lit_1001 / lit_1002 / lit_843 were zero-filled (G reads them through pointer cells 0x5262BC..) | fixed: G addresses in pc_names.txt (new `+` form: a second name for one G string) | quest | [verified: zero list, code; picture of the empty line] |
| GN4 (FIXED today) | Cephalos (em12) revival on desert stages 49/51/53 used zero positions (would revive at the map origin) | em12 em_die_rev (identical in G) reads st49/51/53_pos_tbl; zero-filled on the G PC | fixed: G addresses in config/mhg/pc_data.txt | quest | [verified: reached in coverage; G floats read]; the revival itself not seen [inferred] |
| GN5 (FIXED today) | Page counters ("1/2") in pause-menu lists drawn at the last text position (MH1 too, but only G reaches it) | chat_nm.c / f_chat.c Put_PageArrow call flfntLocate_i, a no-op stand-in | fixed: alias flfntLocate_i=flfntLocate (tools/build_pc.sh), both games | main | [verified: stand-in reached in G] |
| GN6 | Diablos/Monoblos (em14) and Gravios/Basarios (em17) choose their moves with MH1's code: G rewrote em14_to_normal / em17_to_normal / act_dist_select (G's to_normal is a jump to a 25-instruction act_dist_select); MH1's paths read em14_act_add / em17_act_add, zero on the G PC (would crash in em_act_search if a quest set mode 1/2) | src/game/em/em14_*, em17_nm.c; mhg_fdiff | open | quest | [verified: reached + zero tables]; behaviour difference [inferred] |
| GN7 | Equipment detail windows (shop, box): attack 0, mis-sized sharpness bar, no skill pages; 17 strings of equip_exp_core and the element names (sword_zokusei) are zero | main equip_exp_core (G 4.7 KB), lit_4156..4173, lit_4221/4222, lv123str | open | village / main | [verified: reached, zero tables]; REVIEW "G village round 2" |
| GN8 | Stage set pieces of set19 on stages 67 and 73/75 placed from zero tables (objects at the origin or missing); G edited set19_i / set19_trans | src/game/set/set19*.c, set19_s67_pos_tbl, set19_s73_s75_pos_tbl | open | stage (B/D) | [verified: reached, zero]; what it looks like [inferred] |
| GN9 | Elder's talk: "契約金として z が必要" — the fee number is missing from the sentence | G talk text formatting (village) | open | village / text | [verified: shot build/D/g/t_qf200.png] |
| GN10 | Name entry: G's soft keyboard draws its key grid only partly (G field offsets done, not byte-exact); kana -> kanji (apiask_28_OpenDic) is a stand-in | sk_nm.c, rt_gen.c | partly | main | [docs], stand-in [verified] |
| GN11 | Movie / gallery unlocks: MH1's movie_add_ck runs (missing in G) with a zero movie_add_tbl: nothing is unlocked; the "movie added" screen text is zero (lit_676) | f_reward.c, f_reward3.c add_disp | open (G's gallery is rewritten: GALLERY = sub_main 0x4A9960) | main | [verified: reached, zero] |
| GN12 | Pause menu: the chat-config page (pf_chat_cnfg) and the data page frame list (pfl_menu_data) are zero; quest_condition_print's lit_2331 zero | menu_disp_nm.c | open | main | [verified: reached, zero] |
| GN13 | Quest-score title on the result screens shows nothing after "クエストスコア :" | f_reward*.c result screen | open, cause not looked at | quest | [verified: shot] |
| GN14 | Xbox: G frame rate and memory on hardware unknown (xemu only); rt_meta.c costs 1.5 MB of .rdata on G | docs/xbox.md "Monster Hunter G on the Xbox" | owner test | Xbox + owner | [docs] |
| GN15 | Sound: G's em04 sound script runs MH1's (stand-in sub_main_func_004ECD00); G's Eft13 dust effect left out | rt_quest.c | open | quest | [verified: stand-in reached] |
| GN16 | No G save test (round trip, corrupted card, two slots) like MH1's test_save.sh | tools/ | open | platform | [verified] |

### Minor

| # | What the player sees | Where | Status | Lane | Basis |
|---|---|---|---|---|---|
| GM1 | Kind 40 (unused monster) not ported; no quest on the disc uses it | game 0x647060.. | deliberate | quest | [docs] |
| GM2 | em07_init's G calls, point_cam_sub case 25, G result_prog's other changes, poison knife ailment value | agent-A.md round 7 "Open" | open | quest | [docs] |
| GM3 | Stage_Hate_Add (missing in G) runs with a zero stage_hate_add_tbl: adds nothing, as G (which removed it) | em_core_nm | harmless [inferred] | quest | [verified: reached] |
| GM4 | Chat log name (lit_3181) and two NPC message strings (lit_3440/3441) zero | chat, rt_real_Disp_NPC_message | open | main / village | [verified: reached] |
| GM5 | Sprite sort tables (sort_no, spr_sort_flag, spr_sort_num) zero: draw order of some 2D sprites | trans_sprite_sort | open, maybe host-owned | platform | [verified: reached] |
| GM6 | Event 0 / story talk choices of agent D (Event_flag_set(0) guess, story mode numbering) | REVIEW "G village round 2" | owner review | owner | [docs] |
| GM7 | The 'checking memory card' screen after START stays until a button is pressed in a scripted run (may be faithful) | title -> game menu | unverified vs PS2 | main | [verified: shots], faithfulness [unverified] |

## 3. Coverage run (15 Oct 2026)

Build: `COV=1 tools/build_pc_mhg.sh` (G PC with -finstrument-functions). Runs, all hidden, with
`RT_COV=file RT_STANDIN_FILE=file` (script kept in build/, not in git):
title -> NEW GAME -> character creation -> card save -> village intro -> Kokoto (newgame.txt + walk);
CONTINUE from that card -> house -> bed script; the game menu's 5th row (EXTRA: crashes, GB2);
tools/test_gvillage.py (16 cases: weapon/armour/item shops buy/sell/quantity, forge weapon/armour/upgrade,
box store/take/equip/tidy/sell, the elder's first talk), the elder's quest list with story flags 0,1,4
(accept -> "not enough money", faithful for a 0z hunter); quests 1 7 9 10 11 15 23 25 32 41 10033 10038 10041
10042 10048 through `G=1 tools/test_all_quests.py` (clear + time-out each; G-only kinds 37/44/49 among them);
tools/test_revival_g.sh. All passed except the EXTRA row.

`python3 tools/cov_report_g.py cov.txt` (new: maps entered functions to docs/survey/mhg_map.csv and checks whether
the PC's body is a G version, by a G marker in the body or the G branch of an `#if` on VERSION_MHG):
4635 functions entered: 1976 identical in G, 240 shape, 821 edited, 32 rewritten, 78 missing in G, 1488 PC-only
(host and G-only code). 945 of the shape/edited/rewritten/missing ones have no G version on the PC (GB6); full list
in docs/survey/mhg_cov_g.txt, with the zero-filled G tables an entered function reads.

Stand-ins reached (43 distinct):
- Matter to a player: `apiask_28_OpenDic` (kanji conversion), `sub_main_func_004ECD00` (G em04 sound script runs
  MH1's), `G McCardOperation op 0xB` (no MH1-hunter carry-over: NEW GAME always makes a new hunter),
  `G boot: set-up quest 10` (power-on demo quest not loaded), `func_001F19C0` (a G option byte, menu),
  `select_func_00542B50` (G select overlay, game menu).
- Fixed today: `flfntLocate_i`, the `G village: Lb_make_quest_tbl_local` stand-in.
- Harmless on PC/Xbox (host owns screen, loading, models, draw order), as in MH1's audit: flAdjustScreen, flCalcTrans(SI),
  FlushCache, flCompact, view_reset, View_init, init_view_work, load_busy_ck, load_bin_req, load_eft, load_shadow,
  flSnd* (5), set_viewproj, stage_free, ot_init, round_init, snd_joint_load_init, em_effect_pull, model_work_init,
  ADXM_Lock/Unlock, flReleaseMotionSetHandle, clr_used_heap, all_model_free, armor_model_free, edit_create_model,
  flmatTranspose33 (VU0; the host poses monsters), init_set_work / init_shell_work / init_eft_work (deliberate, MH1 M11).
- Online-side, harmless offline: lb_member_inCheck / outCheck / changeCheck.

Zero-filled G tables (build/mhg/pctree/build/pc/mhg_tables_missing.txt): 77 before today, 71 after; 37 are read by
entered functions (list in docs/survey/mhg_cov_g.txt; GN6-GN8, GN11, GN12, GM3-GM5).

## 4. Fixed in this change (15 Oct 2026)

- CONTINUE crash (GN1): config/mhg/pc_names.txt gives D_528538 / D_528550 / D_002EC668 their G addresses, and
  tools/gen_rt_auto.py no longer aliases a D_ name that G's symbol view places into the MH1 object at the same hex
  (D_002EC668 had become weapon_model_data+0xB8). Side effect, also a fix: D_004FEA50 (the 50 quests of the G story
  gate sub_main 0x4C9410, read by lb_guild_g.c) was aliased into MH1's page_tab+0x1140 and is now G's table.
- The elder's quest lists (GN2): src/lobby/f/lb_village_nm.c calls G's Lb_make_quest_tbl_local at village entry.
- Reward screen item line and time format (GN3): pc_names.txt, with a new `+` form in tools/mhg_pcmeta.py
  (add a name beside an existing one at the same G address; G merged MH1's lit_1002 and lit_4893).
- Cephalos revival points (GN4): config/mhg/pc_data.txt.
- flfntLocate_i alias (GN5): tools/build_pc.sh ALIASES (MH1 and G).
Checks (15 Oct, after the fixes, plain builds): clean `tools/rebuild.sh` 5 OK then `VERSION=mhg tools/rebuild.sh` 7 OK;
MH1 PC build, tools/test_quest_loop.sh OK, tools/test_all_quests.sh 38/38; G PC build, tools/test_all_quests_g.sh 98/98 + carts,
tools/test_dbgmenu.sh OK (MH1 + G), tools/test_gvillage.py 16/16; G nv2a Xbox build OK. Notes: docs/agents/agent-A.md "G round 8"
(a COV=1 build leaves instrumented objects that the next plain build links: delete them first).

## 5. Suggested order [inferred]

1. GB2 EXTRA menu crash (main), GN6 em14/em17 G AI (quest), GN7 equipment windows (village), GN9 elder fee text.
2. GB6 triage: per lane, the low-`sim` entries of docs/survey/mhg_cov_g.txt that a player reaches; start with
   game/em, game/shell, main/pl, main/ud (hunt and save data).
3. GB4/GB5: the owner plays G on PC and Xbox (F8 reports / plain chat), one star level at a time.
4. GB3: `./mh1 linux|windows` for a G disc and a player page; GB1 English source decision; GB7 online decision.

## 6. GB6 triage, main lane (agent C, 15 Oct 2026)

Method: docs/survey/mhg_cov_g.txt, the 243 entries under src/main (the lobby, em*, quest and village lanes left out), each aligned against its G
twin with the register/opcode view of tools/mhg_fdiff.py (immediates and offsets masked, so struct-layout-only differences count as 0): 54 are pure
layout (nothing to port), 65 differ by less than 15 instructions, 105 by more, 19 have no G twin (missing: G removed or rewrote them). Ported or fixed this round (all [verified] in hidden runs unless said):
- **Quest clear bits** (Quest_clear_bit_set / _ck, sub_main 0x4C74D0 / 0x4C7550): G numbers the 256 bits by position in a list of quest numbers
  (sub_main 0x4FECA0, 245 of them); MH1's quest-number-as-bit-number wrote past the bits for every G quest above 255 (10041 = bit 313): memory
  next to User_data was overwritten after each won G quest. Now G's mapping (ud_nm.c, quest_bit_tbl in pc_data.txt); tools/test_all_quests_g.sh 10 10033 10041 pass.
- **Omake flags** (Omake_flag_set / _ck): the G PC build links ud_nm.c (f_ud.c is in tools/pc_g_exclude.txt) whose copy had MH1's s16 at option_w+0xFCC; G's is a
  u32 at +0x2C4C. option_w itself was 0x1200 bytes on the G PC (G's runs to 0x2E80: the flags and the network options wrote past it).
- **Disp_button** (G 0x12DBA0): G's icon sheet, kinds 20/21 (decide / cancel) are cells 0/1, a colour parameter. The G screens drew white squares
  for the O / X prompts (mode menu, gallery, card dialogs); every G caller but sub_main 0x4BF800 (not ported, G-only HUD) passes -1 through G 0x12DD90.
- **Memory card messages**: McardMsgTbl is 63 entries in G (the symbol view had 51): the OPTIONS / GALLERY save crashed at entry 51.
- G's GAME MENU title strips 7 (NETWORK OPTIONS) and 8, PIT_TEX[4] (gallery sheet), putspr_g.c (func_0012A8B0 / 0012A800 were no-op stand-ins on the PC).
- rt_movie.c reads G's own movie table (main 0x2A0DE0, 12 movies; MH1's sfd_tbl has 9).

Still MH1 logic where G differs, that a player can notice (nothing here is a crash; sizes are G's):
| What | Where (MH1 name, G twin) | What the player sees [basis] |
|---|---|---|
| Pause menu status page | PlayerStatusWindow (main 0x222E50, 1.3 KB) + func_00223DD0 / 00223AD0 / 00223610 (skill pages) + the G-only page switch | page counter shows "1/-359208930" (G passes the page count, MH1 does not), money line blank (format read through a G pointer cell), the three skill pages (2-4) missing [verified: shot, gvil_ev run] |
| Pause menu help line | row ステータス shows the friend-list help | the help list has the online entry; offline the rows after it are one off [verified: shot] |
| Equipment detail windows | equip_exp_core 4.7 KB, EquipmentCompareWindowA, sword_zokusei, slash_level_bar (6 sharpness levels), Put_receive_mark | attack 0, element names missing, sharpness bar wrong (= GN7) |
| Standard camera | cam_sub_std (main 0x1D9D60): G adds the stage camera table stage_camera_data_ex (func_001DACA0) and a second pad input (pad bits 0xC00) | camera feel in quests; k_HitEmCamera, QuestClearCameraRequest, NPCZoomInCamera* also edited [asm diff only, not seen] |
| Stage drawing | trans_stage (15 KB, +450 instructions in G), stage_set_set, stage_spr_disp | possibly missing stage effects; the stages look right in the village and quests [asm diff only] |
| Wall collision | GetWallHitBit2 / GetWallHitBitEm / GetWallHitBitPl / HitWallPlayer (f_sphr.c) | how the hunter / monsters slide along walls [asm diff only] |
| Player timers | timer_calc_sub_pl (+68 instructions), pl_move_sub (a new field at +0x940, Pl_act_set instead of Pl_act_set2), Pl_item_stack, St_pick_ck2, body_hit_sub_new | status-effect timers, item stacking rules [asm diff only] |
| Card save screens | disp_savesel / disp_savesel_moji (G draws more of the save slot) | the slot frame shows only its title [verified: shot] |
| Fade | Fade_busy_ck / fade_set / Fade_task (a second state byte) | none seen |
| HUD bits | disp_timer, disp_slash_level (G's levels), trans_box, player_info_sub, disp_item_list_present, disp_whole_map | small differences [asm diff only] |
| Hunter rank | Get_hunter_rank / Get_hunter_status early-out when the rank byte is 0 | none for a normal save |
| effects | eft13* / eft20* / eft06_m / eft02_t (edited 0.65-0.97; they are the effect lane of em*) | monster effects |
| Gallery scenes, credits movies | see GB2 | 15 gallery entries inactive |
The pure-layout and < 15-instruction rest (disp_*, set01_*, Info_*, Get_equip_* ...) are listed by running the method above; they only read other
struct offsets, which the G build already has (UDW / PLW / game_w G layouts).
