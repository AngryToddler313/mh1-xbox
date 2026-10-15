# Interesting finds, tidbits and unused content

A scrapbook of curiosities found while porting MH1 JP, MH1 US and Monster Hunter G: unused
content, leftovers, oddities. Owner's idea (14 Oct 2026). Descriptions only: no game files,
images or text are committed. Each entry says where it is and how sure we are:
**[checked]** seen in the files by us, **[guess]** inferred, **[unverified]** from outside sources
(tcrf.net and forums are leads only, never facts) or memory.

Agents: when you trip over something curious, add a short entry here in the same commit.

## Xbox leftovers

- **Xbox button versions of the HUD ("cockpit") textures.** Next to the HUD textures the game
  uses (`cpit1ps.apx`, ...) the disc has `xb` twins: `cpit1xb.apx` (9620 bytes) and
  `cpit2xb.apx` (23056 bytes), several copies each in AFS_DATA, in both MH1 JP and PS2 G.
  The `ps` / `xb` pairing suggests an Xbox version was planned or prototyped (the owner has
  heard one was). [checked: files and sizes in docs/survey/mh1_AFS_DATA.list.txt;
  the textures' content not looked at yet; the Xbox link is [guess]]
  Fun fact: our Xbox port could actually use them for an authentic Xbox HUD.
- **cockpit2.apx: an early HUD mock-up that is listed but never loaded.** (AFS_DATA entry 10,
  24620 bytes packed; in MH1 JP and in G.) Decoded, it is a 256x256 texture like the real HUD
  atlases, but instead of a sheet of small icons it holds what look like **finished pieces of a
  different, earlier HUD design**: [checked: decoded with tools/clay_dump.py; PNGs in
  build/show/cockpit2/, not committed]
  - a large **sword-shaped sharpness gauge** with a metallic green-to-purple blade and a gold hilt
    (the final game uses small blade icons in cpit1ps instead);
  - an **item name box with the Japanese text baked into the picture** (a meat item and an ammo
    type), where the final game draws item names with the font;
  - a **radial item/ammo wheel**: five round slots around a centre (coloured item icons) with a
    "4 / 12" counter, like loaded / total shots. The shipped MH1 HUD has no wheel; it uses a
    horizontal item bar;
  - a long grey bar with a dark end cap at the bottom (a gauge frame, maybe health or stamina).
  Baked-in text and finished-looking art suggest a **design mock-up or an early in-game HUD**
  from before the final cockpit was built [guess].
  **Functionality:** the game's file table `filedef_sys` (main 0x2EF2D0, a list of 16-bit AFS
  entry numbers) has it in **slot 3**, right between the two real HUD atlases (slot 2 = 8
  cpit1ps, slot 5 = 24 cpit2ps). But no code ever asks for slot 3: every `mkTexture` call in
  MH1 is in matched C and uses slots 0, 2, 4, 5, 6, 7 and 8 only (src/main/menu/pit01.c,
  src/main/font/gfs01.c, src/main/plsel/plsel00.c). So its loader was removed or never written,
  and the table entry is a leftover [checked for the main module's mkTexture callers; a load by
  a computed slot number elsewhere is not ruled out]. The Xbox `xb` files, by contrast, are not
  even in the table.
  **Seen in game (14 Oct):** the PC build has a debug switch `RT_FILEDEF=SLOT:ENTRY`
  (src/pc/rt/rt_2d.c). With `RT_FILEDEF=2:10` cockpit2 replaces the HUD atlas cpit1ps: the HUD
  cuts its sprites out of the wrong picture, so the mock-up's baked-in item name box and its red
  "12" counter appear top left where the clock / compass belongs [checked: hidden quest-10 shot].
  The HUD's sprite rectangles were made for cpit1ps, so this only shows pieces, not the design as
  it was meant to look; the mock-up's own layout code no longer exists [guess].
  Try it: `RT_FILEDEF=2:10 tools/play.sh quest 10` (or 5:10 for the other atlas slot).
  **The pieces, measured (agent F, 14 Oct; texel rectangles x0..x1, y0..y1 inclusive, from the alpha channel and the
  colours of the decoded texture):** [checked]
  - sword: 19..255 x 17..44 (the gold hook runs off the right edge). The blade's slot is a **see-through hole**
    42..186 x 28..32 (5 rows) and the gold hilt's slot another hole 97..179 x 37..38 (2 rows, three rivets across it):
    made for a bar drawn *under* the frame. A tiny eye hole sits at the tip (28..31 x 29..31).
  - top name box 0..95 x 79..106 (black inside 84..100, rounded end on the right, cut flat at x = 0) with a tab above it
    (0..28 x 72..78) reading "..H", probably "..SH" (the first letter is cut by the texture's left edge). Baked text:
    "ーンの肉" (".. meat", the start is cut off; no MH1 item ends like that, the nearest are the 肉 items 18-24).
  - bottom name box 0..~83 x 111..136, joined on the right to the wheel's left slot, with a tab under it (0..45 x
    138..144) reading "..AG" (MAG for magazine is a guess). Baked text, grey-green: "?化骨弾" ("? bone shot"), first
    glyph cut; no MH1 ammo has that name (the shipped ammo are 通常弾, 貫通弾, 散弾, 徹甲榴弾, ...).
  - wheel: five oval slots 47x27, centres about (106,124) left (the one the box joins), (134,97) and (187,97) above,
    (134,150) and (187,150) below, joined by spokes to a hub near (150,124). Their insides are opaque black; four hold
    baked icons (yellow pouch upper left, blue left and lower left, red lower right), the upper right is empty. None of
    the shipped ammo uses them: the game's ammo items all share one bullet icon in different colours (Item_data +5 = 26);
    whether they exist elsewhere in the item atlas was not checked.
  - pod 192..239 x 108..140 on the rod to the right: white "4" over a red "12" (rounds in the gun over rounds left,
    is the natural reading [guess]); the rod runs on to the texture's right edge.
  - bottom: a translucent dark panel 0..~235 x 176..247 (alpha 128) closed by a gold bracket 236..243 with a gold line
    to the right edge at y 212: a long gauge or message window, empty.
  **cockpit.amo / cockpit.ahi / cockpit_tex.bin (AFS_DATA 6, 7, 31) are not related:** decoded, cockpit.amo is a tiny
  model of 16 vertices, an upside-down four-sided pyramid (tip at the origin, square base 20 wide at height 20, UVs
  spanning a half texture), and cockpit_tex.bin one 64x64 texture of plain white. A 3D marker (a pointer over a target
  or player) by the look of it [guess about its use]; nothing of the 2D mock-up. [checked: tools/clay_dump.py melt + tree]
  **Made playable (14 Oct, agent F):** the PC/Xbox port can draw a working HUD from these pieces for bowgun users:
  `hud = prototype gunner` in mh1pc.ini or the HUD row of the F10 settings menu (docs/pc.md, "Prototype gunner HUD").

## Leftover developer features

- **Test-server selection screen** in the lobby overlay (`test_server_sel_disp`, lobby
  0x5C2CF0): a Capcom screen for picking a test server. [checked: function in the symbols;
  never seen on screen] (docs/server.md)
- **USB keyboard driver** (`usbkbdm` in main): the game supports a USB keyboard for chat.
  [checked: decompiled]
- **Debug symbols**: the JP MH1 executable shipped with its full symbol table (function and
  variable names), the reason this project started from MH1 JP. G and the US release are
  stripped. [checked]

## Data oddities

- **The font is listed under the wrong name.** AFS_DATA entry 1746 is named `s20rm04.bin` in the
  name directory but is the font (7808 glyphs, 20x20, 2 bits per pixel); the directory is
  shifted around it. [checked] (docs/english.md 1.1)
- **A kana-to-kanji dictionary on the disc** (`ask_mh.dic`, 621 KB, ACCESS "ASK") for the
  soft keyboard's Japanese input. [checked]
- **G's font is byte-identical to the US MH1 font**, which redrew the JP font with its empty
  slots filled. [checked by agent E]

## Version differences

- **G's mega armorskin works like the plain one**: G's Basic_item_set (sub_main 0x4F27B0) raises the defence boost
  to at least 3 for both armorskin (G item 0x12) and mega armorskin (0x13); MH1 gave 3 and 5. The demondrugs still
  give 3 and 5. [checked: G asm; whether it is meant is a guess]
- **G no longer lets an earth crystal sharpen**: MH1's item_action_set sent the earth crystal to the whetstone act;
  G's does not, but G's whetstone act (game 0x5560A0) still has the earth crystal's +200 case. [checked: G asm]
- **A stray random-number call in G**: G's food-bonus helper (sub_main 0x4F2730, after eating/drinking with skill
  0x5A) calls ran_suu with the player-work pointer left in a0 as the stream number, so it steps a 16-bit word far
  past Rnd_w instead of a random stream. [checked: G asm; what that word is, unverified]
- **Quest 10030's program has a G-only command 0x2F** (sets the section a time-out jumps to); it is the only offline
  quest that uses it. [checked: dry runs of all G offline quests]
- **G can import an MH1 hunter**: G's memory card code has 26 extra screens for carrying a
  hunter over from an MH1 save. [checked: matched code, agent C]
- **The US disc carries the US network adapter start-up software** (`NTGUIDVD/`). [checked]
- **The US release adds the Dual Swords weapon class and quests 178-193** over MH1 JP; G has
  Dual Swords too. [checked: docs/mh1_us.md]
- **G renumbered its quest files** (`mNNNNN.mib`, header grew 0x40 -> 0x48) and stores quest text
  inline. [checked]

## G village (agent D, 14 Oct 2026)
- **G's stored equipment and item box are 100 / 200 / 300 entries** (two expansions in User_data+0xC97 bits 0 and 1); the shops' sell list and the
  box window are 10 x 10 pages, and the box's "tidy" is a merge-and-sort view (stacks merged up to each item's maximum, by id).
  [checked: lobby 0x555850, 0x558230]
- **A table of "ダミー" (dummy) lines in main** (0x2EEF20, 32 entries) stands in for talk lines that were never written; the elder's talk reached it
  until the row choice followed G's story tests. [checked]
- **Offline, G's story level function knows only the tests 0x23..0x3C** (lobby 0x543180); the talk code also asks it for small numbers (0, 3, 13 ...),
  which are the online town's and answer 0 offline. [checked: asm]
- **G's talk rows for shop NPCs mark lines as heard by setting event flags** (flags 32-34, 37-42, 83-88, 94-96): the first visit after each story step
  plays the new line once. [checked: lobby 0x544CD0 ..]
- **G's workshop has 1049 armour recipes and 170 weapon recipes** (MH1: 359 / 234); the upgrade table is by weapon id (381 entries). [checked]

## Monster Hunter G differences found while matching (14 Oct 2026, agent C)

- **G has 10 player slots where MH1 has 8**, tutorial quests numbered 10001+ instead of 0x83.., and only stage 36 keeps a "shagami" area
  table (the ground-material lookup in G ignores the per-stage table pointers of MH1). [checked: G code vs MH1 code, tools/check.py diffs]
- **G's chat log entries are 0x5F bytes** (two 0x20-byte display lines) against MH1's 0x5D, and the soft keyboard state is 12 bytes bigger.
  [checked]
- **G reads most of its on-screen strings through pointer cells** in its data instead of addressing the string directly (a localisation
  hook?); the cells sit in two blocks around 0x526xxx-0x528xxx. [checked, purpose is a guess]

## The US build's offline village text is G's lineage (15 Oct 2026, agent E)
- **The US release's lobby overlay holds the offline village UI that MH1 JP lacks and G has:** the bench / chair action menu (sit, stand, price list,
  banquet camera on/off, the five room grades), the item box commands, the cooking results, the quest result messages. The JP MH1 disc has none of
  these, so the US build came from a later Japanese source than the JP disc we have. [checked: string pools of the three discs]
- **The US quest-result table keeps a few Japanese debug lines** (a failed-mission line, a "test 3" entry). [checked]
- **MH1 US has a dual-sword tutorial page and a bug-collecting / mining / bug-net / pickaxe block:** G's tutorial list has the same pages, MH1 JP does not. [checked]

## Monster Hunter G round 7 (15 Oct 2026, agent A)

- **Start-only areas.** G's jungle stage 74 (a shore with a fishing spot) has exits to 73 and 75 but no area leads into
  it: a G quest starts there by the random start-area roll (3 in 100: game.bin 0x6D5DB0 lists {10: camp 67, 25: 44, 22: 70,
  20: 73, 20: 75, 3: 74}); once left, it cannot be walked back into. [checked: exit lists of all 13 jungle stages and the
  start lists] Two other maps' start lists end the same way with a 3% stage 7 (the map starting at camp 50) and stage 8 (camp
  61). [guess: those two stages' exits not dumped]
- **The Lao-Shan's route.** Unattended in a G fortress quest the Lao-Shan waits on stage 14 for its demo, walks
  14 -> 30 -> 28, rams the gate on 28 until it breaks (a game_w flag; the gate object hides by it, from its comment: not looked at), then 11 -> 12, where
  it can at last die (below 1000 hp it cannot die anywhere else). About 58000 ticks (32 min). [checked on the PC]
- **em17's revival points are fixed**: a returning em17 monster of kind 0x16 / 0x30 always comes back on
  stage 64, kinds 17 / 47 on stage 90, whatever the quest's map. [checked: game 0x606800]

## Prototypes and demo (agent F, 15 Oct 2026)

Three extra MH1 builds from the owner (disc/mh1_proto_0521, disc/mh1_proto_0813, disc/mh1_demo_1227; all stripped).
Tools: `tools/carry_match.py p0521|p0813|d1227` (added today; unpack the overlays first with tools/mwo_unpack.py, see its
docstring), `tools/afs_extract.py --list`, `tools/text_map.py`'s quest reader, `tools/clay_dump.py` for textures. Hidden shots
and decoded pictures in build/proto/shots/ (not committed). Short table in docs/mh1_us.md section 7.

**What they are**
- **May 21, 2004 (SLUS_291.10, "VER 1.01")** is a US build two months after the JP release and four before US retail. It
  already has the US layout (sub_main overlay at 0x548180, English text, Dual Swords name and sounds) but its code is the
  closest of all to JP: 74.5% of all JP function bytes appear with the same instruction shape (US retail 70.7%), 75% of our
  matched C. 172 JP functions exist unchanged only in May and were changed later (monster AI and effects like em01_init,
  em_cmd_ck, em08_main, em16_main, eft13/eft16, the save/net file code NetFileLoad, SaveNetFile, NetAutoLoad; Quest_start,
  Pl_item_stack). Nothing that May lacks came back in Aug/US. It still ships JP leftovers: 14 Japanese quest files (m001..
  m136, text identical to JP) next to their English u-twins, JP's player motion tables (plcom/lbcom byte-identical to JP),
  JP's loading.apx, and kari_amh / test00_amh / back2.apx (gone in US retail). [checked]
- **Aug 13, 2004 (TLES_527.07, "VER 0.10", PAL)** is an EU test build of the US game: its AFS data equals US retail where
  names overlap (AFS00/AFS01 identical, AFS_DATA 949 same, 66 changed), the item names are already the final US ones, the
  GMR magazine text is in. Code: 68% of matched C (as US). EU bits: memory card ids BETLES-52707MH / NET (52707 = the later
  EU product number), SCEE DNAS legal text, a DNAS overlay stamped 2004-07-07 "eu" (US and May: 2003-07-08 "us"). English
  only; no language or 50/60 Hz menu in the strings [checked; whether it really runs at 50 Hz is a guess from SYSTEM.CNF].
- **Dec 27, 2004 EU demo (SLED_530.83)** is a five-language trial of the EU release: 62.9% of matched C (the rest is the
  localisation: font printing, equipment/text getters, soft keyboard, memory card, credits, shops, renderer setup).
  [checked]

**All three are cut-down discs** (AFS_DATA 1036 / 1021 / 808 entries against 2353): 31 stage models (US 78), 15 monster
models (demo 13; US 30), 48 weapon model files (demo 10; US 278), 487 armour parts (demo 252; US 1116). The protos carry
only 14 quests (1-4, 6, 36-38, 131-136); the demo has no quest files at all (below). No Rathian, Fatalis, Lao-Shan, Diablos,
Khezu, Gravios/Basarios, Plesioth or Kirin models; Kut-Ku and (if file number = monster kind) Rathalos and Gypceros are in
[checked: file lists; kind mapping is a guess from docs/pc.md's table]. Dual Swords: name and its weapon sounds 124-134
are on all three discs, its motion table w06_tbl.bin on none. [checked]

**Unused / early content**
- **A quest 108 that retail never had** (Aug proto, u108.mib): "Slay the Rathalos!", a copy of quest 9 (data before the
  text 99.9% identical) under a new number, with an earlier requestor: "The Hostile Lord" instead of retail's "The Kindly
  Lord". Neither JP nor US has a 108. [checked]
- **Early English names (May proto)**: the item table has the same 336 slots as retail, 17 names differ: Plesioth parts
  were "Sea Wyvern" Fang / Liver / Jaw (retail "Piscine"), Sonic Bomb was "Firecracker", Fire Herb "Powdergrass",
  Scatterfish "Sprayfish", "Rathian Spine" (Spike), "Rubbery Skin" (Hide), "Citation" (Commendation); Felynes are
  "Felyx" (the demo's English file still says Felyx twice). Monster notes describe Mosswine as "wild pigs" (also in the
  demo). [checked: pointer tables compared by index]
- **The magazine tie-in quest, mid-localisation**: JP's reward items for the Dengeki magazine event survive in May as
  "Dengeki Ticket" and "GMR Ticket" (US retail: "Dummy item" and "Chrome Voucher"), and the event's thank-you line still
  thanks Dengeki Magazine; by August the US magazine GMR has its own thank-you line. The EU demo's text files keep the
  Dengeki line, translated into all five languages. [checked]
- **Online web-page pictures** (Aug and demo; JP/US have other ones): an MH logo banner, "Yian Kut-Ku" and "Rathalos" quarry
  cards, a "QUARRY" header, monster banners for the online news pages (the demo adds 14: R_KHEZU, a black "unknown"
  silhouette, an event cartoon of hunters roasting meat, ...). [checked: decoded PNGs]
- Two demo-only stage models, st27 and st29 (no textures of their own; their lw/lg companions are byte copies of stage 33's
  and stage 45's, the desert), and textureless copies of st28/30/31: probably demo versions of the maps with exits closed
  [guess].

**EU demo specifics**
- **Language select + 50/60 Hz menu at boot**: "Select your language." in five languages, then Start Game / Change video
  mode / Alter TV display position, a 60 Hz test screen ("If you can see this message...") whose background,
  videotest.apx, is a 512x512 Rathalos artwork, and a screen-offset adjuster (X/Y). The text lives in five data-only
  overlays str_e/f/g/i/s.bin (ids 6-10, loaded at 0x888000, 0x4C-0x55 KB each), plus per-language HUD atlases
  (cpit1ps_e..s: the German one says "MUNITION WECHS." where English says "CHANGE BULLETS"), loading screens, three
  controller help pages per language (con_E1..3) and a small 7.9 KB Latin font (pal_s20rm04.bin) in place of the 632 KB
  JIS font. [checked]
- **Trial screen**: "TRIAL VER." on the title and a "trial demo of a game still in development... discs will not be
  exchanged" notice, in five languages. [checked]
- **How the demo locks content**: by data. There are no mNNN/uNNN quest files; instead two quest files per language
  (x036.mbt "Jungle Menace", Kut-Ku; x136.mbt "Your First Monster Hunt!", Velociprey) and a quest-text index (xqidx.mbt) that
  still describes about 99 quests. Monsters, stages and gear outside those quests are simply not on the disc. The functions
  that check key quests and the event market (lb_key_quest_ck, lb_guild_check_keyQuest, Lb_event_market, Lb_event_GH) differ
  from retail [checked]; whether they are the lock or only the localisation is a guess.
- **Bonus movie "E3 2003 Demo Movie"** (BONUS3.sfd, 2:35, 320x448 MPEG-2 + ADX; the gallery's other movies are cut): the
  2003 E3 trailer, including a Japanese-HUD gunner fight in the desert. [checked: frames at build/proto/shots/bonus3_*.png]
- **A bigger name/chat word filter**: JP, US and the protos keep a list of about 670 banned words in select.bin (the
  same list in US and May); the demo moves it into the main executable and grows it to about 885, adding about 240
  French, German, Italian and Spanish swear words (22 English ones dropped). The JP KinshiYogo_chk ("forbidden words
  check") changed shape in the demo. [checked: counts of the uppercase word runs]
- Credits add the Capcom Europe ("CEE Staff") localisation team. [checked]

**Network**
- All three keep the online mode and DNAS overlays. May and Aug point at `*.reo.capcom.sf.yav4.com` hosts (regweb, snap01,
  app01: "reo" test servers [guess]); US retail keeps app01.reo but adds an `mh-beta` bootstrap and an `mh.capcom...` registration page; the demo at an EU beta bootstrap
  (`bootstrap01.mheu-beta...`) and an EU registration page. The lobby web browser identifies as "Y/K/Browser 1.50 for
  MHUSA" in May, 1.60 in Aug/US, "1.60 for MHEU" in the demo. May ships NTGUI2/ (an older network-adapter setup) instead of
  NTGUIDVD/. [checked; nothing was connected to]

**Developer leftovers**
- Aug proto, yn.bin: a debug banner "SNAP THREAD MOVING" between two rows of exclamation marks, and a network-setting
  dump (Hardware / IP Address / Netmask / Primary DNS ... with printf formats). [checked: strings; never seen on screen]
- No debug menu text was found in any of the three. JP's DebugProgram, debug_model_load and the test-server screen
  (test_server_sel_disp) have no same-shape copy in any western build (removed or changed; carry_match cannot tell which).
  [checked]
- JP's omake (bonus gallery) player omake_play is unchanged in all three but changed in US retail. [checked]

**cockpit2.apx and the Xbox HUD textures**: cpit1xb, cpit2xb and cockpit2.apx are byte-identical in all five builds (JP, US,
May, Aug, demo), and cockpit2 stays in filedef_sys **slot 3** everywhere, even in the demo, whose table grew from 9 to 19
slots (slots 9-13 the per-language loading screens, 14-18 the per-language HUD atlases; map slots moved 16 -> 24). The xb
files are in no build's table. US/May/Aug use softkey1_e in slot 4, the demo softkey1_pal. [checked: tables read at US
0x217974, May 0x214344, Aug 0x216B74, demo 0x1DD194 (file offsets); no loader looked at]

**G's lobby client protocol** (cnet, lobby.bin): the command numbers of the lobby server requests are renumbered against MH1 (about +30 for the user
data group, +41 for mail / admin, unrelated for the room group), the notice event ids shifted by up to 6, member lists hold 10 members (MH1 8) and the
lobby / plaza tables have 26 / 35 entries instead of 14 / 10 (room table 8). The room count became a byte. [checked: G asm of the cnet request functions]
