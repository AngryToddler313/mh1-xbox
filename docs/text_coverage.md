# Text coverage: official English for MH1 JP and Monster Hunter G

Made by `tools/text_map.py --report` from the three discs (disc/mh1, disc/mh1_us, disc/mhg). Counts and ids only; no game text is quoted. Re-run it to refresh the numbers (about 3 minutes, 1 GB of RAM). Ids are `space:0xADDRESS` (PS2 address) and `quest:N:0xFILEOFFSET`, as in docs/english.md.

## How the English is tied to the Japanese (by structure)

1. **Code reference.** Each MH1 JP function is looked up in the other build by its masked instruction n-grams (addresses and 16-bit immediates masked). Where the code is the same, the same `lui/addiu` pair loads the same object, so JP address -> other address (2433 US and 2451 G addresses). Edited functions are aligned on the n-grams that still match.
2. **Table index.** A JP pointer table is a symbol of the JP build. Its place in the other build comes from (1), from the MH1 names that tools/mhg_map.py put at G addresses, from pointers inside tables already found, or from the neighbouring table's shift; an object is accepted only if the word classes (string pointer / zero / pointer / integer) of the whole object agree with the JP layout (95% for layout guesses, 25-80% for code or name hits because the tables were edited). Rows are then paired by position, or, where rows were inserted, by their number fields (US) or by their Japanese text (G); the item-id arrays follow the Item_data record alignment.
3. **Quest number.** MH1 JP quest files are tied to US files by their non-text data (128 of 153 JP quests, 45 with an identical data block; the village quests 131-177 and a few others were rebalanced and are left out). G quests are tied to JP quests by identical title/goal/failure/details fields, else by the non-text data (monster, map and reward sections, rule words; 128 of 245 G quests).
4. **Content anchor (G only).** A JP string at least 12 bytes long whose bytes are identical and unique in JP and in G gives its table a shift vote (3211 such anchors); nothing is paired by content alone.
5. **Same Japanese text (MH1 JP).** A JP string left unpaired (its US table not located, or the US code no longer reads it, e.g. the item list's title) takes the English of identical Japanese text elsewhere when all paired occurrences agree.

A pair is dropped when its printf conversions differ, when the 'English' is still Japanese, or when it equals the JP bytes.
G strings whose Japanese text changed from MH1 get the English of the old text only if the change is small (text similarity >= 0.8, same digits and conversions); otherwise they are listed as *edited in G*.

Objects located in the other build: US 457 of 689 string tables (graph 5009, code 2318, layout 395, adjacent 93); G 557 (graph 4893, name 4329, name+code 1318, code 1009, adjacent 127, layout 104). Not located: US 1348 string cells, G 744.

## MH1 JP strings with official US English

| area | strings | official English | none |
|---|---:|---:|---:|
| village NPC lines and talk | 2172 | 2100 (96%) | 72 |
| armour | 1105 | 1105 (100%) | 0 |
| weapons | 941 | 937 (99%) | 4 |
| online, keyboard, word filter | 765 | 7 (0%) | 758 |
| quests | 612 | 512 (83%) | 100 |
| items (names, help) | 601 | 597 (99%) | 4 |
| code literals | 469 | 46 (9%) | 423 |
| memory card and yes/no dialogs | 367 | 32 (8%) | 335 |
| quest board / quest text tables | 333 | 315 (94%) | 18 |
| village / plaza / guild / shop UI | 236 | 156 (66%) | 80 |
| menus, options, HUD | 145 | 103 (71%) | 42 |
| tutorial pages | 116 | 104 (89%) | 12 |
| monsters | 97 | 60 (61%) | 37 |
| skills | 37 | 37 (100%) | 0 |
| other tables (pit_help_itembox_str) | 27 | 0 (0%) | 27 |
| other tables (map_name) | 6 | 6 (100%) | 0 |
| other tables (game_menu_msg) | 6 | 0 (0%) | 6 |
| other tables (movie_add_str_tbl) | 6 | 6 (100%) | 0 |
| other tables (gh_boardStr) | 6 | 6 (100%) | 0 |
| other tables (pit_help_chat_str) | 5 | 5 (100%) | 0 |
| other tables (reload_level_str) | 5 | 5 (100%) | 0 |
| other tables (itembox_cmd_str) | 5 | 5 (100%) | 0 |
| other tables (menu02_str_tbl) | 4 | 0 (0%) | 4 |
| other tables (num_chars) | 3 | 0 (0%) | 3 |
| other tables (mix_list_str) | 3 | 3 (100%) | 0 |
| other tables (menu00_str_tbl) | 3 | 0 (0%) | 3 |
| other tables (menu08_str_tbl) | 3 | 0 (0%) | 3 |
| other tables (str$) | 3 | 0 (0%) | 3 |
| other tables (equip_exp_str_gun) | 3 | 3 (100%) | 0 |
| other tables (equip_exp_str_armor) | 3 | 3 (100%) | 0 |
| other tables (wearable_tbl) | 3 | 3 (100%) | 0 |
| other tables (pit_help_reward_str) | 3 | 3 (100%) | 0 |
| other tables (yes_or_no_str) | 2 | 2 (100%) | 0 |
| other tables (data_list_str) | 2 | 2 (100%) | 0 |
| other tables (chat_cfg_str) | 2 | 2 (100%) | 0 |
| other tables (retire_yesno_str) | 2 | 0 (0%) | 2 |
| other tables (load_msg) | 2 | 0 (0%) | 2 |
| other tables (connect_msg) | 2 | 0 (0%) | 2 |
| other tables (nn_tbl) | 2 | 0 (0%) | 2 |
| other tables (menu01_str_tbl) | 2 | 1 (50%) | 1 |
| other tables (menu06_str_tbl) | 2 | 0 (0%) | 2 |
| other tables (menu04_str_tbl) | 2 | 1 (50%) | 1 |
| other tables (menu05_str_tbl) | 2 | 0 (0%) | 2 |
| other tables (menu09_str_tbl) | 2 | 1 (50%) | 1 |
| other tables (menu0a_str_tbl) | 2 | 0 (0%) | 2 |
| other tables (pit_help_data_str) | 2 | 2 (100%) | 0 |
| other tables (demo_cam) | 2 | 2 (100%) | 0 |
| other tables (receive_mes_str) | 2 | 0 (0%) | 2 |
| other tables (status_sub_str) | 2 | 0 (0%) | 2 |
| other tables (equip_exp_str_sword) | 2 | 2 (100%) | 0 |
| other tables (equip_exp_str_sw_attr) | 2 | 2 (100%) | 0 |
| other tables (sex_char_tbl) | 2 | 2 (100%) | 0 |
| other tables (reward_menu_str) | 2 | 2 (100%) | 0 |
| other tables (select_id_msg) | 2 | 0 (0%) | 2 |
| other tables (special_tag_dat) | 2 | 0 (0%) | 2 |
| other tables (mix_cmd_str) | 1 | 1 (100%) | 0 |
| other tables (retire_str) | 1 | 0 (0%) | 1 |
| other tables (q_sendpl_list) | 1 | 1 (100%) | 0 |
| other tables (zen_tbl) | 1 | 0 (0%) | 1 |
| other tables (ten_moji) | 1 | 0 (0%) | 1 |
| other tables (maru_moji) | 1 | 0 (0%) | 1 |
| other tables (bou_sei_tbl) | 1 | 1 (100%) | 0 |
| other tables (zen_num) | 1 | 0 (0%) | 1 |
| other tables (sn_objs) | 1 | 0 (0%) | 1 |
| **total** | **8148** | **6183 (75%)** | 1965 |

(5671 of the 7536 JP strings found in the executables and overlays, plus 512 quest fields.)

Dropped pairs: line-split dialog (needs per-message alignment) 258, same as JP 178, still Japanese 46, format mismatch 1.

## Monster Hunter G strings with official US English

| area | strings | official English | twin without US English | edited in G | G only |
|---|---:|---:|---:|---:|---:|
| armour [G table] | 2759 | 0 (0%) | 0 | 0 | 2759 |
| village NPC lines and talk | 1883 | 1703 (90%) | 55 | 125 | 0 |
| quests | 980 | 438 (44%) | 60 | 14 | 468 |
| weapons | 940 | 931 (99%) | 4 | 5 | 0 |
| armour | 816 | 776 (95%) | 0 | 40 | 0 |
| online, keyboard, word filter | 755 | 6 (0%) | 748 | 1 | 0 |
| weapons [G table] | 741 | 0 (0%) | 0 | 0 | 741 |
| items (names, help) | 533 | 517 (96%) | 4 | 12 | 0 |
| village NPC lines and talk [G table] | 514 | 0 (0%) | 0 | 0 | 514 |
| memory card and yes/no dialogs [G table] | 303 | 0 (0%) | 0 | 0 | 303 |
| other tables (map_name) [G table] | 295 | 0 (0%) | 0 | 0 | 295 |
| memory card and yes/no dialogs | 217 | 28 (12%) | 189 | 0 | 0 |
| items (names, help) [G table] | 193 | 0 (0%) | 0 | 0 | 193 |
| village / plaza / guild / shop UI | 181 | 116 (64%) | 55 | 10 | 0 |
| quest board / quest text tables | 158 | 140 (88%) | 11 | 7 | 0 |
| other tables (lit_445) [G table] | 154 | 0 (0%) | 0 | 0 | 154 |
| menus, options, HUD [G table] | 145 | 0 (0%) | 0 | 0 | 145 |
| menus, options, HUD | 134 | 88 (65%) | 41 | 5 | 0 |
| tutorial pages | 117 | 91 (77%) | 9 | 17 | 0 |
| village / plaza / guild / shop UI [G table] | 109 | 0 (0%) | 0 | 0 | 109 |
| other tables (lit_329_00661C) [G table] | 98 | 0 (0%) | 0 | 0 | 98 |
| code literals (main) | 76 | 0 (0%) | 0 | 0 | 76 |
| other tables (movie_add_str_tbl) [G table] | 70 | 0 (0%) | 0 | 0 | 70 |
| quest board / quest text tables [G table] | 68 | 0 (0%) | 0 | 0 | 68 |
| other tables (lit_333_00661DF) [G table] | 65 | 0 (0%) | 0 | 0 | 65 |
| code literals (lobby) | 63 | 0 (0%) | 0 | 0 | 63 |
| other tables (lit_322_006619E) [G table] | 53 | 0 (0%) | 0 | 0 | 53 |
| code literals (game) | 50 | 0 (0%) | 0 | 0 | 50 |
| other tables (lit_326_00661B) [G table] | 41 | 0 (0%) | 0 | 0 | 41 |
| online, keyboard, word filter [G table] | 38 | 0 (0%) | 0 | 0 | 38 |
| monsters | 38 | 30 (78%) | 8 | 0 | 0 |
| other tables (lit_309) [G table] | 36 | 0 (0%) | 0 | 0 | 36 |
| skills | 34 | 29 (85%) | 0 | 5 | 0 |
| other tables (lit_198_0065F) [G table] | 32 | 0 (0%) | 0 | 0 | 32 |
| other tables (lit_175_00359E) [G table] | 30 | 0 (0%) | 0 | 0 | 30 |
| other tables (wearable_tbl) [G table] | 30 | 0 (0%) | 0 | 0 | 30 |
| other tables (pit_help_itembox_str) | 27 | 0 (0%) | 27 | 0 | 0 |
| other tables (lit_262_006606D) [G table] | 24 | 0 (0%) | 0 | 0 | 24 |
| code literals (sub_main) | 23 | 0 (0%) | 0 | 0 | 23 |
| other tables (lit_290_00660FA) [G table] | 21 | 0 (0%) | 0 | 0 | 21 |
| other tables (edit_menu_msg) [G table] | 21 | 0 (0%) | 0 | 0 | 21 |
| other tables (lit_451_006647A) [G table] | 19 | 0 (0%) | 0 | 0 | 19 |
| other tables (lit_243) [G table] | 17 | 0 (0%) | 0 | 0 | 17 |
| other tables (verify_button_00387D) [G table] | 17 | 0 (0%) | 0 | 0 | 17 |
| other tables (lit_420_00663C) [G table] | 14 | 0 (0%) | 0 | 0 | 14 |
| other tables (lit_319) [G table] | 13 | 0 (0%) | 0 | 0 | 13 |
| other tables (lit_324_00661AB) [G table] | 13 | 0 (0%) | 0 | 0 | 13 |
| other tables (lit_327_00661C) [G table] | 13 | 0 (0%) | 0 | 0 | 13 |
| other tables (lit_188_0065EF) [G table] | 12 | 0 (0%) | 0 | 0 | 12 |
| other tables (lit_256) [G table] | 12 | 0 (0%) | 0 | 0 | 12 |
| other tables (lit_236_0065FDE) [G table] | 9 | 0 (0%) | 0 | 0 | 9 |
| other tables (lit_337_00661F) [G table] | 9 | 0 (0%) | 0 | 0 | 9 |
| other tables (equip_exp_str_armor) [G table] | 9 | 0 (0%) | 0 | 0 | 9 |
| other tables (lit_224_0065FA) [G table] | 8 | 0 (0%) | 0 | 0 | 8 |
| other tables (lit_281_00660C) [G table] | 8 | 0 (0%) | 0 | 0 | 8 |
| other tables (lit_285_00660DF) [G table] | 8 | 0 (0%) | 0 | 0 | 8 |
| other tables (lit_205_0068EEE) [G table] | 7 | 0 (0%) | 0 | 0 | 7 |
| other tables (lit_441_006643B) [G table] | 7 | 0 (0%) | 0 | 0 | 7 |
| other tables (lit_450_006646F) [G table] | 7 | 0 (0%) | 0 | 0 | 7 |
| code literals (yn) | 7 | 0 (0%) | 0 | 0 | 7 |
| other tables (lit_222_0068F) [G table] | 6 | 0 (0%) | 0 | 0 | 6 |
| other tables (gh_boardStr) | 6 | 6 (100%) | 0 | 0 | 0 |
| other tables (map_name) | 6 | 6 (100%) | 0 | 0 | 0 |
| other tables (lit_442) [G table] | 5 | 0 (0%) | 0 | 0 | 5 |
| other tables (itembox_cmd_str) | 5 | 5 (100%) | 0 | 0 | 0 |
| other tables (lit_183_0065ED) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (lit_221_0065F) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (lit_218_0065F) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (lit_232_0065FCD) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (lit_279_00660B) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (lit_357) [G table] | 4 | 0 (0%) | 0 | 0 | 4 |
| other tables (menu02_str_tbl) | 4 | 0 (0%) | 4 | 0 | 0 |
| other tables (game_menu_msg) | 4 | 0 (0%) | 4 | 0 | 0 |
| other tables (lit_190_0068ED) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (lit_214_0068EFC) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (lit_231_0065FCB) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (lit_408_006637E) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (lit_384_00662F) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (num_chars) | 3 | 0 (0%) | 3 | 0 | 0 |
| other tables (menu00_str_tbl) | 3 | 0 (0%) | 3 | 0 | 0 |
| other tables (menu08_str_tbl) | 3 | 0 (0%) | 3 | 0 | 0 |
| other tables (demo_cam) | 3 | 2 (66%) | 0 | 1 | 0 |
| other tables (equip_icon_u_tbl) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (movie_add_str_tbl) | 3 | 3 (100%) | 0 | 0 | 0 |
| other tables (pit_help_reward_str) | 3 | 3 (100%) | 0 | 0 | 0 |
| other tables (mix_list_str) | 3 | 3 (100%) | 0 | 0 | 0 |
| other tables (equip_exp_str_armor) | 3 | 3 (100%) | 0 | 0 | 0 |
| other tables (wearable_tbl) | 3 | 3 (100%) | 0 | 0 | 0 |
| other tables (default_reibun) [G table] | 3 | 0 (0%) | 0 | 0 | 3 |
| other tables (pit_help_chat_str) | 3 | 2 (66%) | 0 | 1 | 0 |
| other tables (lit_354) [G table] | 2 | 0 (0%) | 0 | 0 | 2 |
| other tables (lit_356_006625C) [G table] | 2 | 0 (0%) | 0 | 0 | 2 |
| other tables (lit_416_00663A) [G table] | 2 | 0 (0%) | 0 | 0 | 2 |
| other tables (select_id_msg) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (special_tag_dat) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (nn_tbl) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (menu01_str_tbl) | 2 | 1 (50%) | 1 | 0 | 0 |
| other tables (menu06_str_tbl) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (menu04_str_tbl) | 2 | 1 (50%) | 1 | 0 | 0 |
| other tables (menu05_str_tbl) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (menu09_str_tbl) | 2 | 1 (50%) | 1 | 0 | 0 |
| other tables (menu0a_str_tbl) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (demo_cam) [G table] | 2 | 0 (0%) | 0 | 0 | 2 |
| other tables (zen_num) [G table] | 2 | 0 (0%) | 0 | 0 | 2 |
| other tables (reward_menu_str) | 2 | 2 (100%) | 0 | 0 | 0 |
| other tables (yes_or_no_str) | 2 | 2 (100%) | 0 | 0 | 0 |
| other tables (data_list_str) | 2 | 1 (50%) | 0 | 1 | 0 |
| other tables (equip_exp_str_sword) | 2 | 2 (100%) | 0 | 0 | 0 |
| other tables (equip_exp_str_gun) | 2 | 2 (100%) | 0 | 0 | 0 |
| other tables (load_msg) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (connect_msg) | 2 | 0 (0%) | 2 | 0 | 0 |
| other tables (lit_350_006623B) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_352) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_360) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_363) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_366) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_369_00662A) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_372_00662AF) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_375_00662BE) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_386_00662FA) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_388) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_392) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_381_00662DC) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_394) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_397) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_402_006634E) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_378_00662CD) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_403) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_406) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_415_006639D) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_412_006638E) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_423_00663D) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (lit_452) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (zen_tbl) | 1 | 0 (0%) | 1 | 0 | 0 |
| other tables (ten_moji) | 1 | 0 (0%) | 1 | 0 | 0 |
| other tables (maru_moji) | 1 | 0 (0%) | 1 | 0 | 0 |
| other tables (num_tbl) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (gRomName) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (zen_num) | 1 | 0 (0%) | 1 | 0 | 0 |
| other tables (push) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (srcfile) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (mix_cmd_str) | 1 | 1 (100%) | 0 | 0 | 0 |
| monsters [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (receive_mes_str) | 1 | 0 (0%) | 1 | 0 | 0 |
| other tables (equip_exp_str_gun) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (Snd_weapon_tbl) [G table] | 1 | 0 (0%) | 0 | 0 | 1 |
| other tables (pit_help_data_str) | 1 | 1 (100%) | 0 | 0 | 0 |
| **total** | **13263** | **4943 (37%)** | 1252 | 244 | 6824 |

G strings are counted from a scan of main and the overlays (Shift-JIS runs, as tools/text_dump.py; a few hundred false hits are possible) plus the four text fields of each quest file. Status of the pairs between a JP string and its G twin: identical text 3735, no US English for the JP string 1302, near text (edited a little) 1248, text edited in G (left out) 259.

### What is left, and where it can come from

- **G only** (no MH1 twin: G's new quests, items, monsters, the plaza/gathering hall code and tables): no official English exists on the US disc. Candidate source: the MHG 'English Remix' patch (Amaillo, Rev. 15; the author allows reuse). Not downloaded or used; list only.
- **edited in G** and **twin without US English**: the Remix again, or the US text if a person checks it.
- **Code literals** that no table points to (HUD banners, result lines): not paired in this version (the code-reference pairs reach few of them because most literals are reached through pointer cells in data).
- The full list of ids without English, with area and reason: `docs/survey/mhg_text_untranslated.txt`.
