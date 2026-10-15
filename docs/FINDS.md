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
