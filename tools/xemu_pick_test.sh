#!/bin/bash
# xemu_pick_test.sh: the bug reporter on the Xbox (src/pc/pick.c with the nv2a id pass, docs/xbox.md "Bug reporter on
# the Xbox") in xemu, hidden, on a SCRATCH hard disk (never xbox_files/hdd.img), then the report's way to the laptop.
#
#   tools/xemu_pick_test.sh [--version mh1|mhg]      PASS / FAIL per check (MH1 default; G needs disc/mhg)
#
#   1. The nv2a game build from xemu's DVD with the owner's game files (run_xemu.sh --data), straight into quest 10
#      (RT_ARGS), a pad script: Back + Black (select + r1) freezes the game; the script then moves the pointer with the
#      right stick, picks with A, drags a box with A held, opens the on-screen keyboard with Y, types "hi", closes it
#      with B and saves with Start. The report must be on E:\Games\<MH1|MHG>\reports\report_*\ (fetched with
#      tools/xbox_fatx.py): report.json with the note, both marks and picked objects (the nv2a id pass worked),
#      screenshot.png, annotated.png, idbuffer.png, log_tail.txt, input.txt.
#   2. The network loader of that game from the same disk (test DVD src/pc/xbox/testlaunch starts it) with this script's
#      own tools/xbox_serve.py on PORT (8797; never the owner's 8765 server): it must POST the report (/report?game=..),
#      the server must save it under build/xbox/reports/<time>-<game>/, and the folder on E: must get its sent.txt.
# Screens (the frozen frame with the panel and the keyboard), serial logs and server log: build/xbox/pick_xemu/.
# Needs: the nv2a build (tools/build_xbox.py --gfx nv2a [--version mhg]), disc/mh1 (or disc/mhg), the owner's
# mcpx.bin / bios.bin (XBOX_FILES_SRC, default the main checkout's xbox_files; read only). ~6 minutes.
# Run it alone: xemu is uncapped and big; it waits while another xemu runs. With other agents about, wrap it:
#   flock "/home/james/claude projects/MH XBOX/xemu.lock" tools/xemu_pick_test.sh
set -e
cd "$(dirname "$0")/.."
VER=mh1; [ "$1" = "--version" ] && VER=$2
case $VER in
mh1) FOLDER=MH1; XB=build/xbox/nv2a/default.xbe; DISC=disc/mh1 ;;
mhg) FOLDER=MHG; XB=build/mhg/xbox/nv2a/default.xbe; DISC=disc/mhg ;;
*) echo "--version mh1|mhg"; exit 2 ;;
esac
T=$(realpath -m "${PICK_TEST_DIR:-build/xbox/pick_xemu}")
SRC=${XBOX_FILES_SRC:-$(git rev-parse --path-format=absolute --git-common-dir)/../xbox_files}
PORT=${PORT:-8797}
FATX="python3 tools/xbox_fatx.py"
for f in "$XB" "$SRC/mcpx.bin" "$SRC/bios.bin"; do [ -f "$f" ] || { echo "missing $f"; exit 1; }; done
[ -d "$DISC" ] || { echo "missing $DISC (the owner's game files)"; exit 1; }
export MH1_HIDDEN=1
. tools/emu_lib.sh
SPID=
emu_traps '[ -z "$SPID" ] || kill $SPID 2>/dev/null || :'      # (and run_xemu.sh, which stops xemu and its Xvfb)
while xemu_running; do echo "another xemu is running: waiting"; sleep 30; done

. ~/xboxdev/env.sh
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj src/pc/xbox/xbox_report.obj
make -C src/pc/xbox/loader -s
LOADER=build/xbox/loader/default.xbe
if [ $VER = mhg ]; then
    make -C src/pc/xbox/loader -s OUTPUT_DIR=../../../../build/xbox/loader_g XBE_TITLE='Monster\ Hunter\ G'
    LOADER=build/xbox/loader_g/default.xbe
fi
make -C src/pc/xbox/testlaunch -s
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj src/pc/xbox/xbox_report.obj

rm -rf "$T"; mkdir -p "$T"
ln -s "$SRC/mcpx.bin" "$T/mcpx.bin"; ln -s "$SRC/bios.bin" "$T/bios.bin"
truncate -s 8G "$T/hdd.img"
$FATX format "$T/hdd.img" >/dev/null

# --- run 1: freeze, pick, note, save (the game from the DVD writes to E:\Games\<folder>)
cat > "$T/env.txt" <<EOF
RT_ARGS=--quest 10 --play
RT_QUEST_STAGE=1
RT_PL_GOD=1
EOF
# ticks until the combo; then one step per drawn frame while frozen (pick_pad): click in the middle (320,240), the
# pointer to (104,300), A held to (500,444): a box; Y, "hi" on the keyboard, a pause for the screenshots, B, Start
K="cross,idle,cam_l*18,cam_d*5,idle,cross+cam_r*33,cross+cam_d*12,cross,idle*400,triangle,idle"
K="$K,ddown,idle,ddown,idle,dright,idle,dright,idle,dright,idle,dright,idle,dright,idle,cross,idle"
K="$K,dup,idle,dright,idle,dright,idle,cross,idle*400,circle,idle,start,idle*30"
echo "idle*400,select+r1*2,idle*3,$K,idle*600" > "$T/input.txt"
XBOX_FILES="$T" VERSION=$VER tools/run_xemu.sh --gfx nv2a --data "$DISC" --input "$T/input.txt" --env "$T/env.txt" \
    --shot "$T/run1.png" --after "40 50 60 70 80 90 100 110 120 130 150" >/dev/null 2>&1 &
wait $! || :
cp "$T/xemu_debug.txt" "$T/serial_1.txt"
FAILS=0
check() { if eval "$2"; then echo "PASS $1"; else echo "FAIL $1"; FAILS=$((FAILS + 1)); fi; }
$FATX get "$T/hdd.img" Games/$FOLDER/mh1log.txt "$T/stderr_1.txt" >/dev/null 2>&1 || :
R=$($FATX ls "$T/hdd.img" Games/$FOLDER/reports 2>/dev/null | awk '/report_/ { print $1 }' | head -1)
check "a report folder on E:\\Games\\$FOLDER\\reports ($R)" "[ -n '$R' ]"
mkdir -p "$T/e_report"
for f in screenshot.png annotated.png idbuffer.png report.json log_tail.txt input.txt; do
    $FATX get "$T/hdd.img" "Games/$FOLDER/reports/$R/$f" "$T/e_report/$f" >/dev/null 2>&1 || :
    check "$f on E: (not empty)" "[ -s '$T/e_report/$f' ]"
done
check "report.json: note, click + box marks, objects picked by the nv2a id pass" "python3 - '$T/e_report/report.json' <<'PY'
import json, sys
d = json.load(open(sys.argv[1]))
m = d['marks']
assert d['note'] == 'hi', d['note']
assert d.get('platform') == 'xbox', d.get('platform')
assert len(m) == 2 and m[0]['type'] == 'click' and (m[0]['x'], m[0]['y']) == (320, 240), m
assert m[1]['type'] == 'box' and (m[1]['x0'], m[1]['y0'], m[1]['x1'], m[1]['y1']) == (104, 300, 500, 444), m
assert d['objects'], 'no objects'
kinds = {o['kind'] for o in d['objects']}
assert {'stage', 'hud'} <= kinds, kinds   # the area under the box and the HUD gauge in it
print('   %d objects: %s' % (len(d['objects']), ', '.join(sorted(kinds))))
PY"
check "the log says it was frozen by the combo and saved" "grep -q 'game frozen (Back + Black)' '$T/stderr_1.txt' && grep -q 'bug report saved' '$T/stderr_1.txt'"

# --- run 2: the loader sends it
printf '10.0.2.2:%s\ngame=%s\n' "$PORT" "$VER" > "$T/loader.txt"
$FATX put "$T/hdd.img" "$LOADER" Games/$FOLDER/default.xbe >/dev/null
$FATX put "$T/hdd.img" "$T/loader.txt" Games/$FOLDER/loader.txt >/dev/null
printf 'E:\\Games\\%s\\default.xbe\n' "$FOLDER" > "$T/launch.txt"
$FATX put "$T/hdd.img" "$T/launch.txt" launch.txt >/dev/null
RDIR=build/xbox/reports; before=$(ls $RDIR 2>/dev/null | wc -l)
python3 tools/xbox_serve.py --port "$PORT" --xbe /nonexistent --xbe-g /nonexistent > "$T/serve.log" 2>&1 & SPID=$!
sleep 2
XBOX_FILES="$T" tools/run_xemu.sh --xbe build/xbox/testlaunch/default.xbe --net --shot "$T/run2.png" --after "45" >/dev/null 2>&1 &
wait $! || :
kill $SPID 2>/dev/null || :; wait $SPID 2>/dev/null || :
cp "$T/xemu_debug.txt" "$T/serial_2.txt"
check "the loader POSTed the report (/report?game=$VER)" "grep -q 'POST /report?game=$VER' '$T/serve.log'"
GOT=$(sed -n 's/.*bug report -> \(.*\)  note:.*/\1/p' "$T/serve.log" | head -1)
check "the server saved it ($GOT)" "[ -n '$GOT' ] && [ \$(ls $RDIR | wc -l) -gt $before ]"
for f in screenshot.png annotated.png report.json; do
    check "$f arrived byte-identical" "cmp -s '$T/e_report/$f' '$GOT/$f'"
done
$FATX get "$T/hdd.img" "Games/$FOLDER/reports/$R/sent.txt" "$T/sent.txt" >/dev/null 2>&1 || :
check "the folder on E: is marked sent (sent.txt)" "[ -s '$T/sent.txt' ]"
echo "screens, logs, the report from E: and the server's copy: $T, $GOT"
[ "$FAILS" = 0 ] && echo "all checks passed" || { echo "$FAILS checks failed"; exit 1; }
