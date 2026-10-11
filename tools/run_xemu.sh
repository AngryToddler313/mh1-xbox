#!/bin/sh
# Boot the port's Xbox build in xemu (open-source Xbox emulator). Needs the owner's own
# Xbox files, which are NOT in the repo and never downloaded by us:
#   xbox_files/mcpx.bin   MCPX boot ROM dump (512 bytes; 'mcpx_1.0.bin')
#   xbox_files/bios.bin   BIOS / flash ROM dump (256 KB - 1 MB)
#   xbox_files/hdd.img    raw hard disk image (a formatted Xbox disk; see docs/xbox.md). Optional:
#                         without it a blank 8 GB sparse image is made and the BIOS/kernel has to
#                         format it (untested; saves on E: need a formatted disk).
#   xbox_files/eeprom.bin optional (xemu makes one)
# (all in xbox_files/, gitignored; override the dir with XBOX_FILES=...).
#
# usage: tools/run_xemu.sh [--gfx null|nv2a] [--data DISCDIR] [--shot out.png] [--after SECONDS] [--xemu PATH]
#   --gfx     which build to boot: build/xbox/ (null graphics, default) or build/xbox/nv2a/
#   --data    DISCDIR = the owner's game files (AFS_DATA.AFS, AFS00.AFS, AFS01.AFS, SLPM_654.95):
#             builds build/xbox/mh1_data.iso holding the XBE plus them as D:\data (about 925 MB,
#             local only), which the XBE finds at start-up. Without it the ISO holds only the XBE
#             (the game then reports missing files, which already tests the boot).
#   --shot    after --after seconds (default 60) save a screenshot and quit (ffmpeg x11grab of
#             the hidden display; xemu 0.8.136 has no QMP screendump).
#   --after   one time, or a list ("36 40 45 60"): one screenshot at each time since start,
#             named out_36s.png ... when more than one is given; quits after the last.
#   --show    run on the desktop instead of hidden (debugging by hand only)
# The game's log (rt_log lines, notes, a liveness line every 5 s) comes out of the emulated
# serial port into xbox_files/xemu_debug.txt (src/pc/xbox/xbox_log.c).
# xemu always opens a window, so by default it runs on its own hidden X display: Xvfb unpacked
# from the Ubuntu package into ~/xboxdev/xvfb/root (no sudo; `apt-get download xvfb`, `dpkg -x`),
# OpenGL through Mesa llvmpipe (software, slow). Nothing shows on the owner's screen.
# Everything xemu writes (its config, saves of the emulated HDD) lives in xbox_files/xemu_home/.
set -e
cd "$(dirname "$0")/.."
XF=$(realpath -m "${XBOX_FILES:-xbox_files}")
XEMU=${XEMU:-$HOME/xboxdev/xemu/xemu-0.8.136-x86_64.AppImage}
GFX=null; DATA=""; SHOT=""; AFTER=60; SHOW=
while [ $# -gt 0 ]; do
    case "$1" in
    --gfx) GFX=$2; shift 2 ;;
    --data) DATA=$2; shift 2 ;;
    --shot) SHOT=$2; shift 2 ;;
    --after) AFTER=$2; shift 2 ;;
    --xemu) XEMU=$2; shift 2 ;;
    --show) SHOW=1; shift ;;
    *) echo "unknown option $1"; exit 2 ;;
    esac
done
[ -x "$XEMU" ] || { echo "xemu not found at $XEMU (fetch the AppImage from github.com/xemu-project/xemu/releases into ~/xboxdev/xemu)"; exit 1; }
miss=""
for f in mcpx.bin bios.bin; do [ -f "$XF/$f" ] || miss="$miss $XF/$f"; done
if [ -n "$miss" ]; then
    echo "missing the owner's Xbox files:$miss"
    echo "(MCPX ROM and BIOS dumps from the owner's own console; docs/xbox.md, 'Running in xemu')"
    exit 1
fi
case "$GFX" in null) B=build/xbox ;; nv2a) B=build/xbox/nv2a ;; *) echo "--gfx null|nv2a"; exit 2 ;; esac
[ -f $B/default.xbe ] || { echo "no $B/default.xbe: run  . ~/xboxdev/env.sh; python3 tools/build_xbox.py $( [ $GFX = nv2a ] && echo --gfx nv2a )"; exit 1; }

ISO=$B/mh1.iso
if [ -n "$DATA" ]; then
    for f in AFS_DATA.AFS AFS00.AFS AFS01.AFS SLPM_654.95; do [ -f "$DATA/$f" ] || { echo "$DATA/$f missing"; exit 1; }; done
    D=$B/iso_data; rm -rf $D; mkdir -p $D/data
    cp $B/default.xbe $D/default.xbe
    for f in AFS_DATA.AFS AFS00.AFS AFS01.AFS SLPM_654.95; do ln -s "$(cd "$DATA" && pwd)/$f" $D/data/$f 2>/dev/null || cp "$DATA/$f" $D/data/; done
    ISO=$B/mh1_data.iso; rm -f $ISO
    ~/xboxdev/nxdk/tools/extract-xiso/build/extract-xiso -c $D $ISO >/dev/null
fi

HOME_X="$XF/xemu_home"; CONF="$HOME_X/xemu/xemu/xemu.toml"
mkdir -p "$(dirname "$CONF")"
HDD="$XF/hdd.img"
[ -f "$HDD" ] || { truncate -s 8G "$HDD"; echo "made a blank sparse $HDD (unformatted)"; }
EEPROM="$XF/eeprom.bin"; [ -f "$EEPROM" ] || EEPROM="$HOME_X/xemu/xemu/eeprom.bin"
cat > "$CONF" <<TOML
[general]
show_welcome = false
[sys.files]
bootrom_path = '$XF/mcpx.bin'
flashrom_path = '$XF/bios.bin'
eeprom_path = '$EEPROM'
hdd_path = '$HDD'
dvd_path = '$PWD/$ISO'
TOML

export XDG_DATA_HOME="$HOME_X"
# the game's log lines go out of the emulated serial port (COM1, src/pc/xbox/xbox_log.c): xemu's
# blank HDD image has no E: for E:\Games\MH1\mh1log.txt
DBG="$XF/xemu_debug.txt"; : > "$DBG"
set -- -device lpc47m157 -serial "file:$DBG"
[ -n "$SHOW" ] || set -- "$@" -audio none     # hidden runs make no sound
XVFB=${XVFB:-$HOME/xboxdev/xvfb/root/usr/bin/Xvfb}
if [ -z "$SHOW" ]; then
    [ -x "$XVFB" ] || { echo "no Xvfb at $XVFB (see the header; or --show to use the desktop)"; exit 1; }
    DNUM=$(( 70 + $$ % 20 ))
    "$XVFB" :$DNUM -screen 0 1280x960x24 -nolisten tcp >/dev/null 2>&1 &
    XPID=$!
    export DISPLAY=:$DNUM
    sleep 2
fi
if [ -z "$SHOT" ]; then
    [ -n "$SHOW" ] && exec "$XEMU" "$@"
    trap 'kill $XPID 2>/dev/null' EXIT
    "$XEMU" "$@"
    exit
fi
"$XEMU" "$@" >"$XF/xemu.log" 2>&1 &
PID=$!
# xemu sometimes ignores SIGTERM (11 Oct 2026, left running after the script ended): KILL
# what is left after 5 s
stop() {
    P="$(pgrep -P $PID || :) $PID"
    kill $P 2>/dev/null || :
    for i in 1 2 3 4 5; do kill -0 $P 2>/dev/null || break; sleep 1; done
    kill -9 $P 2>/dev/null || :
    kill ${XPID:-} 2>/dev/null || :
}
trap stop EXIT
T=0
for A in $AFTER; do
    sleep $(( A - T )); T=$A
    OUT=$SHOT; [ "$A" = "$AFTER" ] || OUT="${SHOT%.png}_${A}s.png"
    ffmpeg -v error -y -f x11grab -i "${DISPLAY:-:0}" -frames:v 1 "$OUT"
    echo "screenshot: $OUT"
done
echo "game log (serial port): $DBG"
