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
- **cockpit2.apx** (24620 bytes; in MH1 JP and G AFS_DATA, several copies): the owner flags it
  as unique. [owner; what makes it unique: to fill in after a look]

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
