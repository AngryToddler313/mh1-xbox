# Playing Monster Hunter on PC and original Xbox

This project runs the Japanese PlayStation 2 game **Monster Hunter** (2004) on a Linux PC, a Windows PC and the
original Xbox. It is a fan-made port that uses code rebuilt (decompiled) from the game. It contains none of
Capcom's game data, so **you need your own copy of the Japanese PS2 disc** (product code SLPM-65495). Monster
Hunter is (c) CAPCOM. This project is not affiliated with Capcom.

It is a work in progress: the whole single-player game is playable (village, all offline quests, shops, forge,
fishing, carving, the star-level progression), but expect rough edges (see "Known issues").

## What you need

* The disc as an **.iso file** (a dump of your own disc, 2048-byte sectors), or a folder holding the files copied
  off it. Only the Japanese release works: US/EU Monster Hunter, Monster Hunter G and PSP versions are different
  games. The tool checks the files by size and checksum and tells you if something is wrong.
* About 1.5 GB of free disk space (the game files are about 925 MB) plus room for the build.
* To build the game from this repository you need a Linux computer (Ubuntu or Debian is what it is tested on).
  Windows and Xbox versions are built on that Linux machine and copied over.
* Per target:
  * **Linux**: `gcc`, `make`, the SDL2 development files and 32-bit support (the game code is 32-bit):
    `sudo apt install build-essential libsdl2-dev gcc-multilib libc6:i386 libsdl2-2.0-0:i386 libgl1:i386`
    (on Ubuntu/Debian you may first need `sudo dpkg --add-architecture i386 && sudo apt update`).
    Without root, `./mh1 linux DISC --setup` fetches the compiler side for you; the three `:i386` libraries must
    still be installed.
  * **Windows**: nothing on the Windows PC. On the Linux machine, `./mh1 windows DISC --setup` downloads the
    cross-compiler (about 100 MB, into `~/mh1win`). The result runs on Windows 7 or newer (32-bit program).
  * **Xbox**: a modded original Xbox with a dashboard that can start games from the hard disk (for example
    Evolution X; the tested one) and an FTP server, and the Xbox SDK **nxdk** on the Linux machine (it is large and
    needs system packages: see docs/xbox.md, "Toolchain"). The Xbox build also needs the Linux requirements above.

## Make the game: one command

Run it in the repository folder. `DISC` is your .iso (or the folder with the disc's files).

    ./mh1 linux   DISC          # result: dist/linux/     play with  dist/linux/play.sh
    ./mh1 windows DISC          # result: dist/windows/   copy the folder to the PC, run play.bat in it
    ./mh1 xbox    DISC          # result: dist/xbox/MH1/  copy to the Xbox as E:\Games\MH1
    ./mh1 check   DISC          # only check that the disc is the right one

What it does, in order: checks the disc (a wrong region or a damaged dump is reported and nothing is built),
checks that the build tools are there (and says what to install if not), builds the game, copies the needed disc
files (`AFS_DATA.AFS`, `AFS00.AFS`, `AFS01.AFS`, `SLPM_654.95`, about 925 MB) next to it. The first build takes
roughly 10 to 40 minutes; a second run is much faster. Options: `--out FOLDER` (another output folder),
`--online` (also build the co-op / online version), `--setup` (download missing build tools where possible).
The `dist/` folder holds a copy of your disc files: do not upload or share it.

### Linux

Run `dist/linux/play.sh`. It starts at power-on: logos, opening movie, title screen, then **new hunter**
(character creation) or **continue**. Extra options can be added: `./play.sh --fullscreen`, `--size 1280x720`,
`--widescreen`. The first run opens a window; Esc quits.

### Windows

Copy the whole `dist/windows` folder anywhere you can write to, and double-click **play.bat**. `play.bat quest`
jumps into a Rathian hunt, `play.bat village` into the village. The package is a test build and untested on real
Windows so far (see Known issues). Windows may warn about an unsigned program. If the game stops with an error,
run `bug_report.bat`.

You can also build without the data and let the program install it: drag your .iso onto `mhview.exe` once and
it copies the files out of the image itself.

### Xbox

1. **Where the files go.** On the Xbox hard disk, in `E:\Games\MH1`, exactly this:

       E:\Games\MH1\default.xbe
       E:\Games\MH1\data\AFS_DATA.AFS   (116 MB)
       E:\Games\MH1\data\AFS00.AFS      (740 MB)
       E:\Games\MH1\data\AFS01.AFS      (20 MB)
       E:\Games\MH1\data\SLPM_654.95    (5.4 MB)

   That is the content of `dist/xbox/MH1` (the game also finds the data folder next to the XBE, however it was
   started). Your Xbox needs an FTP server running (Evolution X and most softmod dashboards have one: look for
   its address on the dashboard's network or system page, and its login in its settings; you type that login, the
   scripts never know it).
2. **Copy them.** Either use any FTP program (FileZilla etc.: connect, open `E:`, make `Games\MH1`, drop the
   folder's contents in), or run `tools/xbox_install.sh XBOX_IP dist/xbox/MH1` on the Linux machine: it asks for
   the login, copies each file with curl, skips files that are already complete and can simply be run again if it
   is interrupted. Some dashboards' FTP servers need active mode (the script's default; `XBOX_PASSIVE=1` switches).
   The data is 925 MB: a home Wi-Fi link can take most of an hour. Copy `default.xbe` last or first, it does not
   matter.
   Check afterwards that the sizes match the list above. The game itself checks at start-up: if a file is missing
   or has the wrong size (a cut-short copy) it shows which one, says where the files belong and returns to the
   dashboard after ten seconds; fix the copy and start again. (Only the sizes are checked there; the checksum check of
   the disc was done by `./mh1` on the Linux machine.)
3. On the Xbox dashboard open the games list ("Play Games from Hard Drive" or similar) and start **Monster
   Hunter**.
4. Hold **Back + Start** for 2 seconds to leave the game (a log is shown for a few seconds first).

The Xbox build has no settings menu or window options. It has been tested on one console and in the xemu emulator,
not on every model.

## Controls

The game keeps the PS2 controls. Left stick moves (push lightly to walk), the **right stick draws and attacks**
(the direction of the flick picks the attack), cross rolls, R1 guards, circle sheathes the weapon, square uses an
item, the d-pad turns and zooms the camera and L1 resets it behind you. In menus **circle confirms and cross
cancels**, as on the Japanese PS2 (the settings menu can swap this).

**Keyboard** (PC)

| key | PS2 button | | key | PS2 button |
|---|---|---|---|---|
| W A S D | left stick | | K | cross (roll, cancel) |
| arrow keys | right stick (attack) | | L | circle (sheathe, confirm) |
| J | square (use item) | | I | triangle |
| Q | L1 (camera reset) | | E | R1 (guard) |
| Z / C | L2 / R2 | | T F G H | d-pad up / left / down / right |
| Enter | Start (pause menu) | | Backspace | Select |

Esc quits the game. **F10** opens the settings menu, **F9** the debug menu for testers (below), **F11** or **Alt+Enter** toggles fullscreen, **F8** reports a
bug (see below). Name entry: type with the keyboard, Enter finishes.

**Gamepad** (PC; any controller SDL knows: Xbox, PlayStation and most others; plug in before or during the game)

| Xbox-style pad | PS2 button |
|---|---|
| A / B / X / Y | cross / circle / square / triangle |
| LB / RB | L1 / R1 |
| LT / RT | L2 / R2 |
| Back / Start | Select / Start |
| sticks, d-pad | sticks, d-pad |

**Original Xbox controller**

| Xbox | PS2 button |
|---|---|
| A / B / X / Y | cross / circle / square / triangle |
| White / Black | L1 / R1 |
| left / right trigger | L2 / R2 |
| Back / Start | Select / Start |
| stick clicks | L3 / R3 |
| d-pad, sticks | d-pad, sticks |

Guard is R1 = Black, which is a small button; if that feels bad, tell us (it is easy to swap on request).

## Settings (PC)

Press **F10** at any time. The game pauses while the menu is open. Rows: windowed / fullscreen, 4:3 or 16:9
(widescreen shows more to the sides; menus and the HUD keep their 4:3 layout), vsync, frame-rate cap, anti-aliasing
(next start), anisotropic filtering, 2D art filter (smooth / sharp / nearest), text language (Japanese only unless
you supply a translation table, see docs/english.md), button layout (circle or cross confirms), music and effects
volume. The choices are saved in `mh1pc.ini` (Linux `~/.local/share/mh1pc/mh1pc.ini`, Windows
`%APPDATA%\mh1pc\mh1pc.ini`), a plain text file; the same options exist as command-line flags (docs/pc.md,
"Display options"). The game logic always runs at its original 30 steps per second, whatever the frame rate.

## Debug menu for testers

Testers can jump straight to any quest, area or screen without scripts or settings. Press **Back + R3** (click the
right stick while holding Back) on any controller, original Xbox pad included, or **F9** on the PC keyboard. The game
pauses while the menu is open and keeps the picture underneath. Press the same again, **Start**, or circle on the first
page to close it.

Moving around: d-pad or left stick up/down picks a row, left/right changes a value (in long lists it jumps 10 rows,
L1/R1 jump 100), **cross** chooses, **circle** goes back, **square** empties an inventory slot. On the PC the arrow keys,
PgUp/PgDn, Enter, Backspace/Esc, Delete and the mouse (wheel, click, right click = back) work too. The names are shown
as the game writes them (Japanese, or English with a translation table).

| row | what it does |
|---|---|
| Start a quest | every quest on the disc: number, the Elder's star level (MH1; G shows the quest group), title and goal. The quest starts at once on its base camp, as if accepted at the counter |
| Area warp | (in a quest) every area the quest file links, through the game's own area change |
| Go to the village | with or without the first-visit intro |
| Title screen | back to the logos and title (like a reset) |
| Item pouch / Item box | pick an item for each slot by name, left/right sets the count. In a quest the pouch is the one you carry now |
| Money, Equipment | money in steps of 1000 (L1/R1: 100000); weapon (blade or bowgun) and armour by name. New equipment shows from the next area or quest |
| Hunter rank, Mark all offline quests cleared | MH1 only |
| Can't faint, One-hit kills, Damage x1..x50 | for fights |
| Set monster HP, Kill the monsters in this area, Warp to the monster | (in a quest) for the monsters of the area you are in |
| Quest timer | stop the clock (free roaming) |
| Show hit volumes | the monsters' body spheres (red) and attack volumes (yellow) |
| Frame step | the game advances one step per R3 click (or F7 / . on the PC); hold R3 to run |
| Free camera | PC: fly with W A S D, Space / C, Shift faster, look with the mouse; the controller still moves the hunter |

The menu is on in test builds. A release build (made with `MH1_RELEASE=1`) has it off. To turn it on or off by hand:
on the PC put `debug_menu = 1` (or `0`) in `mh1pc.ini`, or start with `--debug-menu` / `--no-debug-menu`; on the Xbox
put `RT_DEBUG_MENU=1` (or `0`) in `data\env.txt`. Changes you make with it are real: a save made afterwards keeps
them. Use a copy of your memory card for testing.

## Saves

The game saves to a virtual PS2 memory card, the same way as the original: in your house's **bed** in the village
and after quests; load with CONTINUE.

* Linux: `~/.local/share/mh1pc/memcard0/` (set `MH1_SAVE_DIR` to change it)
* Windows: `%APPDATA%\mh1pc\memcard0\`
* Xbox: `E:\UDATA\4D480001\` (shows under "Monster Hunter" in the dashboard's save list, without an icon)

Back up that folder if you care about the hunter.

**Bringing a real PS2 save**: drag the file onto the game (PC: onto `mhview` / `mhview.exe` / `play.bat`, or onto the
open window), or run `mhview --import-save FILE` (on Linux `dist/linux/mhview --import-save FILE`). Accepted:
.psu, .max, .cbs, .sps, .xps and whole memory card images (.ps2, .mcd, .mc2, .bin; the Monster Hunter save is picked
out). The save is checked first; a damaged or foreign file is refused, and an existing save is copied to a
`.backups` folder first. Then choose CONTINUE. To take a hunter back to a PS2 or PCSX2: `mhview --export-save
MyHunter.psu` (also .max, .cbs, .sps, .xps, or .ps2 for a new memory card image). Importing on the Xbox is not
supported yet. A real PS2 save has not been tested by us (only generated ones), so keep your original.

## Playing with friends

**Co-op over a direct connection (up to 4 players, PC)**: build with `--online` (this gives `mhview_online`), then
every player runs `mhview_online DATA_FOLDER --coop`. A small window asks the host for a quest and player count;
the others type the host's address. Each player uses his own hunter. On a home network use the host's LAN address and
allow the program in the firewall. Over the internet the host forwards TCP port 10300 and everyone names the other
side's address on purpose: `mhview_online DATA_FOLDER --allow 203.0.113.7 --coop`. Details: docs/pc.md, "How to play
co-op".

**Your own server (lobby, town, rooms, chat)**: run `python3 tools/server/mh1_server.py` on any machine with
Python 3.8 or newer; it is invite-only (you make an account for each friend) and needs no other software.
Step by step: docs/server.md, "Run your own server". Each player puts the server's address and his account in
`mh1pc.ini` (`online_server`, `online_login`, `online_password`) and chooses the network mode on the title screen.
This talks only to your server. The port does **not** connect to public servers of the original game, and must
not be pointed at them without their operators' permission. Online play on the Xbox is not available yet.

## Known issues

* The game is in **Japanese**. An English translation table can be loaded (docs/english.md) but none is included.
* Windows and Linux builds have mostly been tested headless (automatic scripted play and screenshots) plus owner play
  sessions on Linux; the Windows package has only run under Wine, with no real Windows, GPU driver or audio device
  tested. Reports welcome.
* Lighting uses a fixed light set instead of the game's stage lights, and sound reverb is approximate; nothing has
  been compared frame by frame with a real PS2, so some effects look different.
* Xbox: about 30 fps in the village and hunts on the tested console; long pauses when changing areas (no loading
  screen); some effects may differ; no in-game settings; the dashboard shows a generic save icon.
* Some rare monster/stage combinations may still have glitches. Press **F8** when you see one (PC): the game
  freezes, click the broken things, type what is wrong, Enter. A report folder (screenshots, game state, your
  inputs) is saved under the game's data folder (`.../mh1pc/reports`). Every run also writes a debug log
  (`.../mh1pc/logs`); nothing is ever sent anywhere. Attach them to an issue at
  https://github.com/itsbigcforme67/mh1-xbox/issues (Windows: `bug_report.bat` packs them).

## Credits and licences

* Monster Hunter is (c) CAPCOM CO., LTD. The game's data is **not** part of this project; you supply your own disc.
* The port is made by reconstructing the game's code (matching decompilation against the Japanese executable)
  and writing a new platform layer. Source: https://github.com/itsbigcforme67/mh1-xbox
* libmpeg2 (GPL, movie decoding) and SDL2 (zlib licence) are bundled; nxdk (Xbox SDK) builds the Xbox version.
* Community knowledge about the game's formats and network protocol came from public research (docs/RESEARCH.md).
