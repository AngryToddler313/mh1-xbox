#!/bin/bash
# xemu_loader_test.sh: the two network loaders (one per game, src/pc/xbox/loader, docs/xbox.md
# "Network loader") in xemu, hidden, on a SCRATCH hard disk (never xbox_files/hdd.img).
#
#   tools/xemu_loader_test.sh            all four runs below, PASS / FAIL per check
#
# The disk gets both loaders as the owner's Xbox has them after tools/xbox_upload.sh --loader:
# E:\Games\MH1\default.xbe ("Monster Hunter", loader.txt with game=mh1) and E:\Games\MHG\default.xbe
# ("Monster Hunter G", loader.txt WITHOUT a game line: the folder name picks G). A test DVD
# (src/pc/xbox/testlaunch) starts the loader named in E:\launch.txt from the hard disk, as the
# dashboard would (it deletes E:\launch.txt, so it starts only once). tools/xbox_serve.py runs on
# PORT (8799) and is stopped at the end (by its PID).
#   1. G loader, laptop up: downloads mhg.xbe, starts it; G (no data on the scratch disk) exits through
#      the game's normal quit path and returns to G's loader, which sends G's log (?game=mhg).
#   2. MH1 loader, laptop up: the same for mh1.xbe.
#   3. G loader, laptop down: starts the mhg.xbe already on the disk (its log lines on the serial port).
#   4. G loader with --data (a small fake G folder): installs it into E:\Games\MHG\data, keeps the loader.
# Each run ends at the kernel's "error 21" screen: the scratch disk has no dashboard for "back to the
# dashboard" (expected; the real Xbox has Evolution X there).
# Builds: the G and MH1 nv2a game builds must exist (XBE1 / XBEG to use others; they must have the
# 16 Oct 2026 xbox_log.c, else the game goes to the dashboard instead of its loader). xemu needs the
# owner's mcpx.bin / bios.bin (XBOX_FILES_SRC, default the main checkout's xbox_files; read only).
# Run it alone (xemu is uncapped and big); it refuses to start while another xemu runs.
set -e
cd "$(dirname "$0")/.."
T=$(realpath -m "${LOADER_TEST_DIR:-build/xbox/loader_xemu}")
SRC=${XBOX_FILES_SRC:-$(git rev-parse --path-format=absolute --git-common-dir)/../xbox_files}
PORT=${PORT:-8799}
XBE1=${XBE1:-build/xbox/nv2a/default.xbe}
XBEG=${XBEG:-build/mhg/xbox/nv2a/default.xbe}
FATX="python3 tools/xbox_fatx.py"
. tools/emu_lib.sh
for f in "$XBE1" "$XBEG" "$SRC/mcpx.bin" "$SRC/bios.bin"; do [ -f "$f" ] || { echo "missing $f"; exit 1; }; done
if xemu_running; then echo "another xemu is running: not starting"; exit 1; fi
export MH1_HIDDEN=1

. ~/xboxdev/env.sh
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj src/pc/xbox/xbox_report.obj
make -C src/pc/xbox/loader -s
make -C src/pc/xbox/loader -s OUTPUT_DIR=../../../../build/xbox/loader_g XBE_TITLE='Monster\ Hunter\ G'
make -C src/pc/xbox/testlaunch -s
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj src/pc/xbox/xbox_report.obj      # nothing test-specific is in them, but keep it clean
L1=build/xbox/loader/default.xbe; LG=build/xbox/loader_g/default.xbe

rm -rf "$T"; mkdir -p "$T"
ln -s "$SRC/mcpx.bin" "$T/mcpx.bin"; ln -s "$SRC/bios.bin" "$T/bios.bin"
truncate -s 8G "$T/hdd.img"
$FATX format "$T/hdd.img" >/dev/null
printf '10.0.2.2:%s\ngame=mh1\n' "$PORT" > "$T/lt1"; printf '10.0.2.2:%s\n' "$PORT" > "$T/ltg"
$FATX put "$T/hdd.img" "$L1" Games/MH1/default.xbe >/dev/null
$FATX put "$T/hdd.img" "$T/lt1" Games/MH1/loader.txt >/dev/null
$FATX put "$T/hdd.img" "$LG" Games/MHG/default.xbe >/dev/null
$FATX put "$T/hdd.img" "$T/ltg" Games/MHG/loader.txt >/dev/null

SPID=
serve() { python3 tools/xbox_serve.py --port "$PORT" "$@" > "$T/serve_$RUN.log" 2>&1 & SPID=$!; sleep 2; }
unserve() { [ -z "$SPID" ] || { kill "$SPID" 2>/dev/null || :; wait "$SPID" 2>/dev/null || :; SPID=; }; }
emu_traps unserve      # (and run_xemu.sh, which stops xemu and its Xvfb: tools/emu_lib.sh)
# boot TARGET SECONDS: the test DVD starts TARGET; screenshots along the way, serial log kept
boot() {
    printf '%s\n' "$1" > "$T/launch.txt"
    $FATX put "$T/hdd.img" "$T/launch.txt" launch.txt >/dev/null
    XBOX_FILES="$T" tools/run_xemu.sh --xbe build/xbox/testlaunch/default.xbe --net --shot "$T/run$RUN.png" \
        --after "$2" >/dev/null 2>&1 &
    wait $! || :
    cp "$T/xemu_debug.txt" "$T/serial_$RUN.txt"
}
FAILS=0
check() { if eval "$2"; then echo "PASS run $RUN: $1"; else echo "FAIL run $RUN: $1"; FAILS=$((FAILS + 1)); fi; }
size_on_disk() { { $FATX ls "$T/hdd.img" "$1" 2>/dev/null || :; } | awk -v n="$2" '$1 == n { print $2 }'; }
has() { grep -q -- "$2" "$T/$1"; }

RUN=1; serve --xbe "$XBE1" --xbe-g "$XBEG"
boot 'E:\Games\MHG\default.xbe' "30 60 100"
unserve
check "G loader asked for G's version" "has serve_1.log 'GET /version?game=mhg'"
check "G loader downloaded mhg.xbe" "has serve_1.log 'GET /mhg.xbe?game=mhg'"
check "G exited back to its loader, which sent G's log" "has serve_1.log 'POST /log?game=mhg'"
check "the log it sent is G's (names G's program file)" "has serve_1.log 'SLPM_658.69'"
check "mhg.xbe on the disk is the served build" "[ \"\$(size_on_disk Games/MHG mhg.xbe)\" = $(stat -c %s "$XBEG") ]"
check "E:\\Games\\MHG\\default.xbe is still the loader" "[ \"\$(size_on_disk Games/MHG default.xbe)\" = $(stat -c %s "$LG") ]"
check "nothing went to E:\\Games\\MH1" "[ -z \"\$(size_on_disk Games/MH1 mh1.xbe)\" ]"

RUN=2; serve --xbe "$XBE1" --xbe-g "$XBEG"
boot 'E:\Games\MH1\default.xbe' "30 60 100"
unserve
check "MH1 loader asked for MH1's version" "has serve_2.log 'GET /version?game=mh1'"
check "MH1 loader downloaded mh1.xbe" "has serve_2.log 'GET /mh1.xbe?game=mh1'"
check "MH1 exited back to its loader, which sent MH1's log" "has serve_2.log 'POST /log?game=mh1'"
check "the log it sent is MH1's (names MH1's program file)" "has serve_2.log 'SLPM_654.95'"
check "mh1.xbe on the disk is the served build" "[ \"\$(size_on_disk Games/MH1 mh1.xbe)\" = $(stat -c %s "$XBE1") ]"
check "E:\\Games\\MH1\\default.xbe is still the loader" "[ \"\$(size_on_disk Games/MH1 default.xbe)\" = $(stat -c %s "$L1") ]"

RUN=3                                   # no server
boot 'E:\Games\MHG\default.xbe' "40 80"
check "no laptop: G started from the disk anyway, once (its loader did not start it again after the exit)" \
    "[ \"\$(grep -c 'no SLPM_658.69 in' '$T/serial_3.txt')\" = 1 ]"

RUN=4
mkdir -p "$T/fakeg/data"
for f in SLPM_658.69 AFS01.AFS AFS_DATA.AFS AFS00.AFS; do head -c $((300000 + RANDOM)) /dev/urandom > "$T/fakeg/data/$f"; done
head -c 4096 /dev/urandom > "$T/fakeg/default.xbe"       # must never reach the Xbox
serve --data "$T/fakeg" --xbe-g "$XBEG"
check "MH1's version has no DATA line (G's install only)" "! curl -s 'http://127.0.0.1:$PORT/version?game=mh1' | grep -q DATA"
check "the old MH1 loader's /version (no ?game=) has no DATA line" "! curl -s 'http://127.0.0.1:$PORT/version' | grep -q DATA"
boot 'E:\Games\MHG\default.xbe' "40 90"
unserve
check "G loader fetched the G data list" "has serve_4.log 'GET /manifest?game=mhg'"
check "the log left from run 3 (no laptop) and the one after the install were both sent" \
    "[ \"\$(grep -c 'POST /log?game=mhg' '$T/serve_4.log')\" = 2 ]"
for f in SLPM_658.69 AFS01.AFS AFS_DATA.AFS AFS00.AFS; do
    $FATX get "$T/hdd.img" "Games/MHG/data/$f" "$T/got_$f" >/dev/null 2>&1 || :
    check "data/$f installed byte-identical" "cmp -s '$T/fakeg/data/$f' '$T/got_$f'"
done
check "E:\\Games\\MHG\\default.xbe is still the loader after the install" "[ \"\$(size_on_disk Games/MHG default.xbe)\" = $(stat -c %s "$LG") ]"
echo "screens, serial logs and server logs: $T"
[ "$FAILS" = 0 ] && echo "all checks passed" || { echo "$FAILS checks failed"; exit 1; }
