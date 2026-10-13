# MH1 International (US, SLUS-208.96): survey, and its official English script

Agent A, 13 Oct 2026. The owner supplied the disc on 13 Oct: disc/mh1_us/ (ISO and files). Purpose of this file
(owner's decision of 13 Oct, docs/DECISIONS.md): the US script is the official English that G's English will be based
on, so this records where the US text lives, how it is encoded, how much there is, and how US strings can be tied to
the matching JP (and later G) strings. Nothing here quotes game text beyond single weapon-class words; the disc's
text never goes into git (same rule as tools/text_dump.py: player's disc -> player's disk).

Marks: **[verified]** measured today with the commands named; **[inferred]** reading of the measurements, not tested.

Survey files (names, sizes and counts only): docs/survey/mh1_us.txt (elf_survey), mh1_us_overlays.txt
(tools/mwo_unpack.py), mh1_us_AFS_DATA/AFS00/AFS01.list.txt (tools/afs_extract.py --list), carry_us.txt
(tools/carry_match.py us). Overlays were unpacked to build/us_ovl/ (git-ignored) with
`tools/mwo_unpack.py disc/mh1_us/AFS_DATA.AFS -o build/us_ovl`.

## 1. The disc next to the Japanese one [verified]

| | Japanese SLPM_654.95 (VER 1.02) | US SLUS_208.96 (VER 1.00) |
|---|---|---|
| Symbols | full .symtab | stripped (no .symtab, no debug info) |
| Compiler | MW MIPS C 2.4.1.01 | the same |
| main segment (code + data) | 2.66 MB | 2.72 MB |
| Overlays (text size) | game 1.08 MB, lobby 0.90, select, yn; stored unpacked; load 0x533980 | game 1.19 MB, lobby 0.59, **sub_main 0.22 (new, like MH G)**, select, yn; all packed (MWo3 LZSS); load 0x5BE900, sub_main 0x552E00 |
| AFS_DATA | 2318 entries | 2353; 2156 names shared, **2067 of those byte-identical** |
| Quest files | m001-m177 (.mib), 153 | u001-u193 (.mib), 159 |
| Network | DNAS overlays, KDDI MMBB portal | DNAS too (IOP/MODULES/DNAS280.IMG, changed dnas_net/ins overlays) plus NTGUIDVD/ (the network adapter start-up files); a different online service (docs/RESEARCH.md: the community has not worked out the US servers) |
| Other files | - | DUMMY.DAT (10 MB padding), 0FLIST.DIR |

Shared data files are the same format (byte-identical in 2067 cases), so the port's loaders read them unchanged.
The 89 shared files that changed: motion tables of eight monsters (`em01/06/08/11/17/20/21/22_tbl.bin`), weapon
models (`lw*`, `lg*`), `loading.apx`, `demo_tex.bin`, `ef_01_*`, the web-UI PNGs. New: `sub_main.bin`,
`softkey1_e.apx` (English soft keyboard), `ichiba.apx`, `st016cmd.bin`, 16 more quest numbers, weapon sounds
`snd_weapon124-134` (AFS01), five demo sounds (AFS00). Gone: `kari_amh.bin`, `back2.apx`, one HTML page.

**What the US adds that the JP lacks: the Dual Swords weapon class** [verified: the class name appears 6 times in
the US main executable, never in the JP one; inferred: the new weapon sounds 124-134 and the changed `lw*`/`lg*`
weapon models belong to it]. The JP port knows six weapon kinds (src/pc/rt/rt_player.c refuses kind > 5). The item
name table grew from 327 to 336 entries (section 3), and there are 16 new quest numbers (u178-u193; 159 quest files against 153).

## 2. Where the English lives, encoding, amount [verified unless marked]

Same places as the Japanese text (docs/english.md 1.1), now in English:

| place | English-looking strings (rough count) | Shift-JIS runs left |
|---|---|---|
| main executable | ~2400 (81 KB) | 54 |
| lobby.bin (village, guild, online UI) | ~2600 (224 KB) | 88 |
| game.bin | ~130 (22 KB) | 3 |
| sub_main.bin | ~200 (14 KB) | 108 |
| yn.bin (memory card, yes/no) | ~200 (7 KB) | 1 |
| quest files u001-u193 | title / goal / failure / details / clear line per quest, 159 files | none: 142 of 143 checked quests have pure-ASCII text |

Counting rule: NUL-terminated printable runs of 12+ bytes with at least two lower-case words. It misses short labels
(item and monster names are often shorter), so the real string count is higher; compare docs/english.md's JP counts
(~2000 + 700 strings in lobby.bin alone).

- **Encoding: plain ASCII**, NUL terminated. The game's `~Cnn` colour codes stay (516 of them). A few strings carry
  single high bytes (0x90, 0xA0, 0x80 ...), probably button-icon or control codes [inferred, not decoded].
- **Font**: AFS_DATA entry 1745 (JP 1746; the index moved by one), same size and layout as the JP font (780800 bytes
  unpacked = 7808 glyphs of 20 x 20 at 2 bits). 7802 of 7808 glyphs differ in bytes but carry about the same amount of
  ink per JIS row, and the JP font's 302 empty slots are filled in the US one (6 empty left) [verified]. So it is a
  redrawn font with extra glyphs, probably including the Latin set the English uses [inferred]. Whether the US code
  draws ASCII proportionally (a width table) or at the JP half-width advance was not checked; the US boxes were laid
  out for whatever it does, which matters for our layout (docs/english.md 4).
- `sfont.apx` (debug 8 x 8 font) and `icon.bin` are byte-identical to the JP ones.

## 3. Tying US strings to JP (and G) strings

The text layer (src/pc/rt/rt_text.c, docs/english.md 5) keys each string by its JP location. Four ways to map US
text to those keys, from easiest:

1. **Quest files by number** [verified]. Unpacked JP m-files and US u-files have the same header layout (title, goal,
   failure, details pointers at +0x60..+0x6C; text at the end). 120 of the 153 JP quests have a US file with the same
   number whose data before the text is at least 95% byte-identical (the differences are the text pointers and one
   small block, rewards or similar). Two were renumbered (JP 146 -> US 163, JP 161 -> US 151). 31 JP quests (most of
   the village quests 131-177, and 101-107) differ more under the same number (rebalanced or reordered; a field
   compare of target monster, stage and goal is the next step). US 178-193 are new.
2. **Pointer tables by index, found through matched code** [verified for two tables]. A JP function that loads a
   table's address (lui/addiu) is found in the US code by its masked instruction pattern (tools/carry_match.py's
   method); the same instruction pair in the US copy gives the US table address. Measured: `item_str` (JP main
   0x33AB40, 327 entries) -> US 0x58D890 in sub_main's data, 336 entries, 335 of them ASCII; `em_name` (35 entries)
   -> US 0x5940F0, 35 entries (not ASCII strings: the table holds something else in both builds). `Skill_name`,
   `info_str0` and `option_menu_msg` were not reached this way (no uniquely found referencing function); a data-side
   search or a second reference fixes that. Whether the 9 extra items are appended at the end (so indices 0-326 line
   up) is not checked [inferred: likely, check a few names by hand].
3. **Literals in code by function** [method, not done]. 7250 JP functions sit at exactly one place in the US code
   (carry_us.txt); a literal's JP address comes from a lui/addiu pair in one of them, and the same pair in the US copy
   points at the US string. This covers the HUD/banner/result literals of docs/english.md 1.1 wherever the function
   carried over. Where the code changed (section 4: the village UI), map by order inside the function or by hand.
4. **The rest by hand**: text that only exists in one build (US-only Dual Swords text; JP-only online/portal text).

For G: G is the main release (13 Oct). Its English can start from these US strings for everything G shares with MH1
(items, monsters, skills, menus, the 120+ shared quests), mapped JP-MH1 -> G by the same code-pattern method, plus the
community G patch for G-only text if its author agrees (docs/english.md 2). Nothing of that is measured here (the G
survey belongs to another agent now).

## 4. How much of our matched C fits the US build [verified]

tools/carry_match.py us (same instruction shape, addresses and 16-bit immediates masked):

| MH1 module | matched functions | found in US | share of matched bytes |
|---|---|---|---|
| main | 2907 | 1925 | 62.8% |
| game | 2600 | 2170 | 80.8% |
| lobby | 1612 | 652 | 33.2% |
| select / yn | 42 / 94 | 15 / 56 | 25% / 56% |
| total | 7255 | 4818 | **66.4%** |

- Unmatched JP code is found even more often (79.5% of its bytes): the US build is mostly the JP code relinked.
- What changed is the village/menu/plaza UI (lobby matched code only 33%: lb_basic_master, Lb_guild, Lb_shop_move,
  DispButtonHelp, the quest-board pages, item box...): the English layout work, plus the US online service.
- 261 main functions moved to sub_main.bin and 179 to game.bin (the same split G has).

## 5. What a US port would need [inferred, nothing started]

1. A JP -> US name map: functions from carry_match (7250 unique), data through code references (section 3.2): tables
   in src/pc/rt/tables.txt, ALIASES in tools/build_pc.sh, and ~320 six-digit hex addresses in src/pc.
2. Install: src/pc/install.c accepts only the JP file sizes/CRCs; AFS entry indices differ (+1 at the font, more
   later), so indices the host hard-codes need a per-version table. Saves: the US folder name is not BISLPM-65495MH
   (not checked).
3. Either (a) keep our JP code and take the English text, font and quest text from the player's US disc at install
   time (the cheapest English; JP boxes may be too small where the US re-laid the UI), or (b) a real US build:
   second splat config, our C where the shape matches, the rest matched again; only (b) gives the Dual Swords and the
   US-only quests.
4. Online: the US client's server protocol is unknown; mh1-server speaks the JP one.
