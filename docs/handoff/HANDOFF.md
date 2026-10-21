# MH G matching decompile: handoff to local PC

State as of 2026-10-21 (link fix). This was written by the cloud thread "MH G decompilation".

## Where things are
- **Code:** github.com/AngryToddler313/mh1-xbox, branch `claude/mhg-decomp-tjvfyr`, HEAD `385b007e`. Every verified match is pushed, and nothing unpushed is left in the cloud.
- **Progress:** **28.112%** (1101516 of 3918368 bytes), link-verified: a clean `VERSION=mhg tools/rebuild.sh` prints all 7 modules OK. The starting point was 20.56% upstream.
- **Correction:** the earlier 34.01% was not link-verified. `tools/build.py` links with `build/mhg/<module>.ld`, which only `tools/rebuild.sh` (setup_split + splat) regenerates, and the cloud loop never re-ran it. Registrations made after that point were byte-checked per function by check.py but never linked. 300 of them did not link byte-identical and were dropped; they are listed with reasons in `dropped_registrations.txt`. Most can be re-added after fixing them: give their .rodata a slot, add the missing PROVIDE, or fix the wrong data address.
- **Commit `40868405 LOCAL: ...`** patches tools/mhg_map.py and tools/mhg_pcmeta.py so the G tools run without a JP MH1 disc. It is for the fork only: drop it from any upstream PR. It is fork-only, and so is this docs/handoff/ folder.
- **Game files:** JP MH G v1.01 (SLPM_658.69 and AFS_DATA.AFS) go in `disc/mhg/`. Never commit them.

## Toolchain (what the cloud used)
- **PS2 binutils 2.47** from ftp.gnu.org: `./configure --target=mips64r5900el-ps2-elf`, then build and link the result as `tools/binutils`. On Windows, use WSL2 or MSYS2. The repo's own README covers the native-Windows route.
- **Compiler:** https://github.com/decompme/compilers/releases/download/compilers/mwcps2-3.0b52-030722.tar.gz, extracted to `tools/compilers/mwcps2-3.0b52-030722/`. It is run through **wibo** on Linux (https://github.com/decompals/wibo/releases/latest/download/wibo-x86_64), or natively on Windows.
- **Python:** a venv with splat64[mips] and the repo's requirements.
- **Build:** always verify with `VERSION=mhg tools/rebuild.sh`. It regenerates the yaml files and linker scripts from c_files.txt, re-splits, and builds. Plain `tools/build.py` links a stale linker script after c_files.txt changes, so a passing result from it does not mean the new entries were linked. All seven modules must print OK: main, sub_main, select, game, yn, lobby, nethttp. A full build took more than 30 minutes on 4 cloud cores.

## Tooling quirks
- **mwcc hangs** forever after a missing include. tools/build.py now has a 180 s timeout per step (in the fork). Also kill any mwccps2 process running longer than 3 minutes; `watchdog2.sh` does this.
- **Missing raw functions:** about 23 MH1 raw function names have no G twin. `rebuild.sh` leaves their .inc files missing, and the build then hangs. Create dummy files `build/mhg/raw/NAME.inc` containing `.word 0`. These are local only and not committed.
- **Merging config lists:** `config/mhg/c_files.txt`, `c_rawfuncs.txt` and `g_names.txt` merge best with `merge=union` in `.git/info/attributes`.
- **Registering functions:** mhg_addfn.py only registers a function when its `#if !defined(G_PART)` guard sits directly above the definition, or above the `#ifdef` block that holds all its variants.
- **mhg_parts error:** mhg_parts reports "guards break the plain mhg compile" when a prototype line carries a trailing comment and the function's name. Move the comment to its own line.
- **Missing prototypes:** an act_set that calls the u16 sub-setters fails the own-part check unless the sub-setters have prototypes at the top of the file.
- **Static asm helpers:** making them non-static changes the callers' register scheduling. Don't.
- **Never read or use 2Tie/mh1j.**

## Near-match recipes
- **GV():** `GV(mh1, g)` from `include/g_alias.h`, which must be the first include.
- **G struct layouts:** put them under `#if defined(VERSION_MHG) && defined(__MWERKS__)`.
- **G-only code:** put it under `MHG_PS2`.
- **STG_POS_N:** from `include/version.h`.
- **New G addresses:** add PROVIDEs to `config/mhg/g_names.txt`.

## Diagnosing a failed rebuild
- `linkerr.sh MODULE` prints the full ld errors (build.py cuts them at 4000 characters).
- **A module larger than expected** means a C object has .rodata with no slot. That rodata lands after all data (`<module>_RODATA_START` in the map), so every address after it moves.
- `shiftfind.py MODULE` finds the points where address-named symbols (D_/jtbl_/func_) start sitting at the wrong address.
- `diffmap.py MODULE` groups differing bytes by c_files entry.

## Helper scripts in this folder
These run in the cloud container. Paths point to /root/mh1-xbox; set the `ROOT` environment variable to change that.
- `reg.py FILE...` registers whatever now matches in each file, runs `tools/rebuild.sh`, and keeps the registration only if all 7 modules are OK. It does not commit.
- `dn.sh FILE FUNC [N]` shows the differing lines between G and our build for one function.
- `merge.sh` merges the worktree branches g-work-A, B and C, verifies all 7 OK, and pushes.
- `watchdog2.sh` kills hung compiles, prints a heartbeat every 5 minutes, and runs merge.sh every 30 minutes.
- `analyze.py` and `survey.sh` produced the near-match lists.

## Work queue
These lists are sorted closest first. Each line is `rank diffcount file func module addr`. Many entries at the top are already done or skipped.
- `near_A.txt`: game module.
- `near_B.txt`: lobby.
- `near_C.txt`: main, sub_main and the rest. The helper had reached about line 176.
- The original carry-over queue is `mhg_carryover_queue.csv` (in this folder).

**Known blocked or skipped:**
- **Register allocation only:** item_kosuu_sel_chk, lb_armor_listItem, SetHelpLineMsg, em_dur_init, sub_main_func_004B7810, select_yes_no, flfntSetPalette, hk_key_henkan.
- **G code differs, or the G map name is shifted** (these need G-only versions): set12_e, eft04_move, em02_aib em_atk04, em21_r10 em_move04/05/06, em17_r02 em_act_search2, Lb_back_money, Lb_get_quest_str/2 (G rebuilt the quest table), CallBack_Result_Plaza_PlazaExit, cnLBS_Read_RoomRuleAllocation, __cnet_SendSet_TelephoneNumber, Set_ErrorDialog.
- **Delay slots:** G leaves branch delay slots unfilled in shell09_m.
- **Trailing nops:** npc_effect_move, main, usbkbdm_init, Quest_clear_bit_ck.
- **Whole files:** fladdm.c (G dropped functions) and f_get.c (SET_CLR macro functions).
- **Remaining size:** about 393 KB of raw "edited" functions, about 591 KB of functions that differ by more than 10 instructions, plus G-only code.
