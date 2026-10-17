#!/bin/sh
# Real Monster Hunter G reference pictures of any stage, next to the PC port's picture from the same camera.
#
# The owner's G disc runs in PCSX2 (hidden, tools/run_pcsx2.sh with PINE=slot: memory read/write through
# tools/pcsx2/pine.py). A save state "in a quest" is made once by playing the real game's menus with scripted keys
# (NEW GAME, a name, the house, the Village Elder's first quest 10001 "raw meat", the village gate) and kept in
# $PCSX2_HOME. Each shot then loads that state and moves the hunter to the stage the way an area exit does
# (stage_mv_ck: PLW+0x73A = stage, +0x73C = position, +0x570 = angle, +0x738 = 1; game2 loads the area), waits for the
# load, grabs the screen and reads the game camera (lpView: eye, target, angle of view). The PC G build draws the same
# stage from that camera (mhview_g --stage N --view eye,target,fov) and both go side by side, PS2 left, PC right.
#
# usage: tools/g_ref_shot.sh [--out DIR] [--no-pc] SHOT...      (~1 min per shot; the first run ~4 min more)
#   SHOT  STAGE[:ANGLE[:X:Z]]   stage 0-87; ANGLE the hunter's facing (0-0xFFFF, the camera ends up behind him;
#         default 0); X Z where he stands (default the stage's start position, stage_start_pos)
#   --out DIR   pictures: DIR/sNN[_a].png (pair), DIR/ps2_sNN[_a].png, DIR/pc_sNN[_a].png (default build/ref)
#   --no-pc     PS2 pictures only
#   POKE="CMDS" after each PS2 picture, run tools/pcsx2/pine.py CMDS and take a second one, OUT/ps2_TAG_poke.png
#   PCQUEST=N   the PC picture from a quest (N = 10001, the state's quest) after the same area change, with the game's
#               camera, set objects and HUD, instead of the --stage viewer (the camera is not forced: compare after the
#               camera has settled behind the hunter on both)
#   --pc-only   the PC pictures again (after a PC change) from the cameras of an earlier run (DIR/views.txt); no PCSX2
#   NEWSTATE=1  make the in-quest save state again (STEPSHOTS=1: a picture after each scripted step, DIR/step_NNN.png)
#   WEAPON=KIND:ID  the hunter takes this weapon into the quest (KIND 6 sword, 7 gun; poked into the equipped item-box
#               entry before the gate, Set_equip_idx copies it at departure). Its own save state: slot 9 - KIND
#               (WEAPON=6:234, the first Dual Swords: slot 3). Made like NEWSTATE the first time.
#   --combo KEYS  no stage shots: load the state, then record the screen (30 fps, DIR/combo.mp4 and every 3rd frame as
#               DIR/combo_NNN.png) while pressing KEYS: a list of KEY[:HOLD_MS] / pause:SECONDS (keys of
#               tools/run_pcsx2.sh: t/f/g/h right stick, e = R1, i/j/k/l left stick ...); peek:OFF prints the
#               hunter's PLW byte at OFF (e.g. peek:0x93D, the Dual Swords' demon mode). Example:
#               WEAPON=6:234 tools/g_ref_shot.sh --out build/A/dual --combo "t pause:0.6 t pause:0.6 t pause:2"
# Needs: PCSX2 + the owner's BIOS (tools/run_pcsx2.sh), disc/mhg/Monster Hunter G (Japan).iso, build/mhg/pc/mhview_g
# (tools/build_pc_mhg.sh), Xvfb. Takes the shared emulator lock itself (one emulator at a time: xemu.lock next to the
# main checkout).
# Nothing here goes to git: the pictures and the state hold Capcom data. G addresses used (G main / sub_main, from
# config/mhg/*_aliases.txt): player_work 0x34D450, select_w 0x35EC30, stage_start_pos 0x520A30, lpView 0x2EF28C.
# The PS2 picture has the HUD; the PC one has no HUD and its default hunter at the stage's start position.
cd "$(dirname "$0")/.."
# the lock file next to the main checkout (worktrees too: git's common dir is the main checkout's .git); EMU_LOCK= overrides
LOCK=${EMU_LOCK:-$(cd "$(git rev-parse --git-common-dir)/../.." && pwd)/xemu.lock}
case " $* " in *" --pc-only "*) G_REF_LOCKED=1 ;; esac      # (no emulator)
if [ -z "$G_REF_LOCKED" ]; then
    G_REF_LOCKED=1 exec flock "$LOCK" "$0" "$@"
fi
OUT=build/ref; PC=1; PCONLY=; COMBO=
while [ $# -gt 0 ]; do
    case "$1" in
    --out) OUT=$2; shift 2 ;;
    --no-pc) PC=; shift ;;
    --pc-only) PCONLY=1; shift ;;
    --combo) COMBO=$2; PC=; shift 2 ;;
    -*) echo "unknown option $1"; exit 2 ;;
    *) break ;;
    esac
done
[ $# -gt 0 ] || [ -n "$COMBO" ] || { sed -n '2,36p' "$0"; exit 2; }
mkdir -p "$OUT"
# the PC picture of one shot: pc_shot TAG STAGE VIEW
pc_shot() {
    if [ -n "$PCQUEST" ]; then  # in a quest: the same area change as on the PS2 (RT_AREA_GO), the game's own camera,
        # set objects, effects, the stage's screen gradient and the HUD; the picture 300 ticks after the change
        AG="100,$2"; [ -z "$X" ] || AG="$AG,$X,$Z,$(printf %x "$ANG")"
        (ulimit -v 1500000; MH1_HIDDEN=1 RT_NOMOVIE=1 RT_AREA_GO="$AG" RT_SHOTS=400 timeout 300 build/mhg/pc/mhview_g \
            disc/mhg --quest "$PCQUEST" --input 'idle*2000' --size 640x480 --shot "$OUT/pcq_$1.png" --time 15 \
            > "$OUT/pc_$1.txt" 2>&1)
        mv -f "$OUT/pcq_$1_400.png" "$OUT/pc_$1.png" 2>/dev/null
    else
    (ulimit -v 1500000; MH1_HIDDEN=1 timeout 120 build/mhg/pc/mhview_g disc/mhg --stage "$2" --view "$3" \
        --size 640x480 --shot "$OUT/pc_$1.png" > "$OUT/pc_$1.txt" 2>&1)
    fi
    ffmpeg -nostdin -v error -y -i "$OUT/ps2_$1.png" -i "$OUT/pc_$1.png" -filter_complex hstack "$OUT/$1.png" \
        && echo "  pair: $OUT/$1.png (PS2 left, PC right)"
}
# SHOT -> STG ANG X Z TAG
parse() {
    IFS=: read STG ANG X Z <<EOF
$1
EOF
    ANG=${ANG:-0}
    TAG=$(printf s%02d "$STG"); [ "$ANG" = 0 ] || TAG=${TAG}_$ANG
}
if [ -n "$PCONLY" ]; then     # the PC side again, from the cameras an earlier run kept in OUT/views.txt
    for S in "$@"; do
        parse "$S"
        V=$(awk -v s="$STG" -v a="$ANG" '$1 == s && $2 == a { v = $6 } END { print v }' "$OUT/views.txt" 2>/dev/null)
        [ -n "$V" ] || { echo "stage $STG: no camera in $OUT/views.txt"; continue; }
        pc_shot "$TAG" "$STG" "$V"
    done
    exit 0
fi
[ "$(ulimit -v)" = unlimited ] || { echo "PCSX2 cannot start under a memory cap (ulimit -v); run uncapped"; exit 1; }
[ -z "$PC" ] || [ -x build/mhg/pc/mhview_g ] || { echo "no build/mhg/pc/mhview_g (tools/build_pc_mhg.sh)"; exit 1; }
export PCSX2_HOME=${PCSX2_HOME:-$PWD/build/ref/pcsx2home}
mkdir -p "$PCSX2_HOME/PCSX2"
[ -e "$PCSX2_HOME/PCSX2/bios" ] || ln -s "$HOME/ps2dev/pcsx2/data/PCSX2/bios" "$PCSX2_HOME/PCSX2/bios"
export PINE=${PINE:-28050} DNUM=${DNUM:-93}
SLOT=9              # the save-state slot that holds "in a quest"
UD=0x32C210         # User_data
if [ -n "$WEAPON" ]; then
    WK=${WEAPON%%:*}; WID=${WEAPON#*:}
    SLOT=$((9 - WK))
fi
ST=$PCSX2_HOME/PCSX2/sstates
PL=0x34D450; SELW=0x35EC30
K=build/pcsx2/xkey
p() { python3 tools/pcsx2/pine.py --slot $PINE "$@"; }
grab() { ffmpeg -nostdin -v error -y -f x11grab -video_size 640x480 -i :$DNUM -frames:v 1 "$1" </dev/null; }
# k WAIT KEY...: keys (KEY or KEY:HOLD_MS), then wait WAIT seconds
k() {
    w=$1; shift; ks=""
    for a in "$@"; do case "$a" in *:*) ks="$ks $a:400" ;; *) ks="$ks $a:150:400" ;; esac; done
    [ -z "$ks" ] || DISPLAY=:$DNUM $K $ks
    sleep "$w"
    NK=$((NK + 1))
    [ -z "$STEPSHOTS" ] || grab "$OUT/step_$(printf %03d $NK).png"
}
NK=0
f2() { p rf "$1" | awk '{print $2}'; }

. tools/emu_lib.sh
RP=
cleanup() { emu_stop $RP; RP=; }
emu_traps cleanup
tools/run_pcsx2.sh --iso "disc/mhg/Monster Hunter G (Japan).iso" --seconds 7200 --log "$OUT/emulog.txt" \
    > "$OUT/pcsx2.txt" 2>&1 &
RP=$!
i=0
until p status >/dev/null 2>&1; do
    i=$((i + 1)); [ $i -lt 60 ] || { echo "PCSX2 did not start (see $OUT/pcsx2.txt)"; exit 1; }
    sleep 1
done
echo "PCSX2 up"

if [ -n "$NEWSTATE" ] || ! ls "$ST"/*.0$SLOT.p2s >/dev/null 2>&1; then
    echo "making the in-quest save state (slot $SLOT)"
    # the opening movie, the title -> GAME MENU -> NEW GAME -> character creation: name "aa", DECIDE, yes, don't save
    k 35; k 4 Return; k 12 c; k 2 c; k 2 Down c c Return; k 3 Up c; k 6 Left c; k 20 Right c
    # the village intro scenes (start skips each) -> the house -> walk left, then down to the door -> square: into the
    # village (square away from the door opens the save dialog instead)
    k 8 Return; k 10 Return; k 10 Return; k 2 j:1500; k 4 k:2500; k 12 z
    X0=$(p rf $((PL + 0xAC)) | awk '{printf "%d", $2}')
    [ "$X0" -gt 9000 ] 2>/dev/null || { grab "$OUT/state_fail.png"; echo "not out of the house (x $X0, $OUT/state_fail.png)"; exit 1; }
    # the Village Elder (warp next to him, face him, talk): circle until quest 10001 is taken (select_w+0xD0), then
    # cross through the rest of his lines (circle again would start a new talk that offers to cancel the quest)
    p wf $((PL + 0xAC)) 11100 ";" wf $((PL + 0xB4)) 12150
    k 2 k:300
    i=0
    until [ "$(p r16 $((SELW + 0xD0)) | awk '{print $2}')" = 0x2711 ]; do
        i=$((i + 1)); [ $i -lt 40 ] || { grab "$OUT/state_fail.png"; echo "no quest from the elder ($OUT/state_fail.png)"; exit 1; }
        k 1.5 c
    done
    k 2 x x x x x x
    if [ -n "$WEAPON" ]; then     # the weapon to take along: the equipped set is an index into the item box (User_data
        # +0xCFE; box entries of 6 bytes at +0x44: +1 kind, +2 id); Set_equip_idx copies it to +0xC75 / +0xC76 at departure
        EI=$(p r16 $((UD + 0xCFE)) | awk '{print $2}')
        EA=$((UD + 0x44 + EI * 6))
        p w8 $((EA + 1)) "$WK" ";" w16 $((EA + 2)) "$WID" ";" w8 $((UD + 0xC75)) "$WK" ";" w16 $((UD + 0xC76)) "$WID"
        echo "equipped weapon kind $WK id $WID (box entry $EI)"
    fi
    # the village gate: square departs -> the quest loads (camp of the Forest and Hills, stage 21)
    p wf $((PL + 0xAC)) 10650 ";" wf $((PL + 0xB4)) 15225
    k 2; k 25 z
    [ "$(p r8 $((PL + 0x736)) | awk '{print $2}')" = 0x15 ] || { grab "$OUT/state_fail.png"; echo "not in the quest ($OUT/state_fail.png)"; exit 1; }
    p save $SLOT
    sleep 5
    echo "saved the in-quest state"
fi

if [ -n "$COMBO" ]; then         # record a move sequence in the quest's camp
    p load $SLOT; sleep 4
    echo "hunter: job $(p r8 $((PL + 2)) | awk '{print $2}') weapon $(p r16 $((PL + 0x360)) | awk '{print $2}')"
    rm -f "$OUT"/combo_*.png
    ffmpeg -nostdin -v error -y -f x11grab -framerate 30 -video_size 640x480 -i :$DNUM -t 30 "$OUT/combo.mp4" </dev/null &
    FP=$!
    sleep 1
    T0=$(date +%s)
    for a in $COMBO; do
        case "$a" in
        pause:*) sleep "${a#pause:}" ;;
        peek:*) echo "  t+$(( $(date +%s) - T0 ))s PLW+${a#peek:} = $(p r8 $((PL + ${a#peek:})) | awk '{print $2}')" ;;
        *:*) DISPLAY=:$DNUM $K "$a:0" ;;
        *) DISPLAY=:$DNUM $K "$a:120:0" ;;
        esac
    done
    sleep 1
    kill -INT $FP 2>/dev/null; wait $FP 2>/dev/null
    ffmpeg -nostdin -v error -y -i "$OUT/combo.mp4" -vf "select=not(mod(n\\,3))" -vsync vfr "$OUT/combo_%03d.png"
    echo "combo: $OUT/combo.mp4, $(ls "$OUT"/combo_*.png | wc -l) frames $OUT/combo_NNN.png"
    cleanup
    exit 0
fi
for S in "$@"; do
    parse "$S"
    p load $SLOT; sleep 4
    if [ -z "$X" ]; then
        X=$(f2 $((0x520A30 + STG * 12))); Y=$(f2 $((0x520A30 + STG * 12 + 4))); Z=$(f2 $((0x520A30 + STG * 12 + 8)))
    else
        Y=0
    fi
    # an area exit to STG (PLW+0x570 is the facing the exit gives, the zone's angle + 0x4000)
    p w16 $((PL + 0x73A)) "$STG" ";" wf $((PL + 0x73C)) "$X" "$Y" "$Z" ";" w16 $((PL + 0x570)) $(( ANG )) ";" \
        w8 $((PL + 0x738)) 1
    i=0
    until [ "$(p r8 $((PL + 0x736)) | awk '{print $2}')" = "$(printf 0x%X "$STG")" ] &&
          [ "$(p r8 $((PL + 0x738)) | awk '{print $2}')" = 0x0 ]; do
        i=$((i + 1)); [ $i -lt 90 ] || { echo "stage $STG: not loaded"; continue 2; }
        sleep 1
    done
    sleep 5         # the fade-in and the camera settling behind the hunter
    grab "$OUT/ps2_$TAG.png"
    V=$(p rf 0x2FF0C0 12 | awk '{print $2","$3","$4","$5","$6","$7","$13}')
    VP=$(p r32 0x2EF28C | awk '{print $2}')
    [ "$VP" = 0x2FF0C0 ] || V=$(p rf "$VP" 12 | awk '{print $2","$3","$4","$5","$6","$7","$13}')
    # the fl render state the stage is drawn with (G main: flFogColor 0x2EF458, flFogEnd / Start 0x2EF45C / 0x2EF460,
    # flPS2FadeColor 0x384530, flPS2Ambient 0x384540): OUT/fl_TAG.txt
    { echo "fogcol $(p r32 0x2EF458 | awk '{print $2}') fog end/start $(p rf 0x2EF45C 2 | awk '{print $2, $3}')"
      echo "fade $(p rf 0x384530 4 | awk '{print $2, $3, $4, $5}') ambient $(p rf 0x384540 4 | awk '{print $2, $3, $4, $5}')"
    } > "$OUT/fl_$TAG.txt"
    echo "stage $STG: PS2 camera $V -> $OUT/ps2_$TAG.png ($(head -1 "$OUT/fl_$TAG.txt"))"
    echo "$STG $ANG $X $Z view $V" >> "$OUT/views.txt"
    if [ -n "$POKE" ]; then      # an experiment: POKE="pine.py commands" (e.g. "wf 0x2EF45C 1e9 1e9": no fog), then
        # a second picture OUT/ps2_TAG_poke.png
        p $POKE > "$OUT/poke_$TAG.txt"
        sleep 1
        grab "$OUT/ps2_${TAG}_poke.png"
        echo "  after POKE: $OUT/ps2_${TAG}_poke.png"
    fi
    if [ -n "$PC" ]; then
        pc_shot "$TAG" "$STG" "$V"
    fi
done
cleanup
