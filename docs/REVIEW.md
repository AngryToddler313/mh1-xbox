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
