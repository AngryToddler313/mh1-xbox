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

- **G can import an MH1 hunter**: G's memory card code has 26 extra screens for carrying a
  hunter over from an MH1 save. [checked: matched code, agent C]
- **The US disc carries the US network adapter start-up software** (`NTGUIDVD/`). [checked]
- **The US release adds the Dual Swords weapon class and quests 178-193** over MH1 JP; G has
  Dual Swords too. [checked: docs/mh1_us.md]
- **G renumbered its quest files** (`mNNNNN.mib`, header grew 0x40 -> 0x48) and stores quest text
  inline. [checked]
