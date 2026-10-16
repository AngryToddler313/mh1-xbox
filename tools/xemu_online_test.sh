#!/bin/sh
# MH1 online on the Xbox build, in xemu (hidden): the online build (tools/build_xbox.py --online --gfx nv2a) logs in to
# our own server (tools/server/mh1_server.py, --open on 127.0.0.1 only: never any other server) through xemu's NAT
# network (the Xbox reaches the laptop's 127.0.0.1 as 10.0.2.2) and goes to the town; then a PC client
# (build/pc/mhview_online, hidden) logs in to the same lobby and must see the Xbox's hunter, and the server must see both.
# The server address comes the way the owner sets it on a real Xbox: an online_server line in settings.ini in the game
# folder (E:\Games\MH1\settings.ini, put on xemu's disk image with tools/xbox_fatx.py).
# xemu uses its own copy of the owner's Xbox files (XBOX_FILES, default the main checkout's xbox_files): a fresh blank
# disk image in build/xemu_online/xbox_files, so the shared image is never written. Screenshots and logs in
# build/show/xemu_online (never committed). About 5 minutes (the Xbox is in the town about 60 s after xemu starts; the shots at 90-120 s show both hunters).
#   build first:  ONLINE=1 tools/build_pc.sh && . ~/xboxdev/env.sh && python3 tools/build_xbox.py --online --gfx nv2a
cd "$(dirname "$0")/.."
SRC=${XBOX_FILES:-xbox_files}
OUT=build/show/xemu_online; rm -rf $OUT; mkdir -p $OUT
XF=build/xemu_online/xbox_files
SP=""; PP=""; XP=""
# run_xemu.sh stops xemu and its Xvfb when it is killed (tools/emu_lib.sh); every way out stops it, the PC client and
# the server
. tools/emu_lib.sh
cleanup() { emu_stop $PP $XP $SP; SP=""; PP=""; XP=""; }
emu_traps cleanup
fail() { echo "xemu online test FAILED: $1 (see $OUT)"; exit 1; }
[ -f build/xbox/online/nv2a/default.xbe ] || fail "no build/xbox/online/nv2a/default.xbe (see the header)"
[ -x build/pc/mhview_online ] || fail "no build/pc/mhview_online (ONLINE=1 tools/build_pc.sh)"
for f in mcpx.bin bios.bin; do [ -f "$SRC/$f" ] || fail "no $SRC/$f (the owner's Xbox files; XBOX_FILES=DIR)"; done
xemu_running && fail "another xemu is running (xemu runs alone)"
rm -rf build/xemu_online; mkdir -p $XF
cp "$SRC/mcpx.bin" "$SRC/bios.bin" $XF/
[ -f "$SRC/eeprom.bin" ] && cp "$SRC/eeprom.bin" $XF/
truncate -s 8G $XF/hdd.img
python3 tools/xbox_fatx.py format $XF/hdd.img >/dev/null || fail "could not format the disk image"

python3 tools/server/mh1_server.py serve --lobby-port 0 --open -v --relay-ports 10431-10439 > $OUT/server.log 2>&1 &
SP=$!
i=0; PORT=""
while [ $i -lt 50 ] && [ -z "$PORT" ]; do
    PORT=$(sed -n 's/^mh1-server lobby on 127.0.0.1:\([0-9]*\).*/\1/p' $OUT/server.log)
    [ -n "$PORT" ] || { sleep 0.2; i=$((i + 1)); }
done
[ -n "$PORT" ] || fail "the server did not start"
printf 'online_server = 10.0.2.2:%s\r\nonline_login = 00000001\r\nonline_password = LOCALTEST0000001\r\n' $PORT > $OUT/settings.ini
python3 tools/xbox_fatx.py put $XF/hdd.img $OUT/settings.ini Games/MH1/settings.ini >/dev/null || fail "could not put settings.ini"
# the Xbox: straight into the online town (RT_ARGS is the command line the Xbox does not have, xbox_env.c), the PC
# test's screens (tools/test_online_town.sh): ID select (new ID), the handle, confirm, the plaza's first lobby
cat > $OUT/env.txt <<'EOF'
RT_ARGS=--quest 10 --play --online
RT_NAME=XBOX
RT_VILLAGE_START=1
RT_VILLAGE_SKIP_INTRO=1
RT_ONLINE_TRACE=1
EOF
echo "idle*200,circle*2,idle*100,circle*2,idle*100,circle*2,idle*150,circle*2,idle*20000" > $OUT/input.txt
XBOX_FILES=$XF tools/run_xemu.sh --online --gfx nv2a --data disc/mh1 --env $OUT/env.txt --input $OUT/input.txt \
    --shot $OUT/xbox.png --after "${AFTER:-90 120 150 240}" > $OUT/xemu_run.log 2>&1 &
XP=$!
# the PC client once the Xbox has entered a lobby (its log over the serial port: "quest download", the lobby entry)
i=0
while [ $i -lt 500 ] && ! grep -aq "online: quest download" $XF/xemu_debug.txt 2>/dev/null; do sleep 2; i=$((i + 2)); done
grep -aq "online: quest download" $XF/xemu_debug.txt && echo "the Xbox entered a lobby after about $i s"
env MH1_SAVE_DIR="$PWD/$OUT/card_PC" RT_NAME=PC RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_NET_PORT=$PORT \
    sh -c "ulimit -v 1500000; exec timeout 200 build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute \
    --input 'idle*200,circle*2,idle*100,circle*2,idle*100,circle*2,idle*150,circle*2,idle*1500' \
    --shot $OUT/pc.png --time 60 --size 640x480" > $OUT/pc.log 2>&1 &
PP=$!
wait $PP; PP=""
wait $XP; XP=""
cp $XF/xemu_debug.txt $OUT/xbox_serial.txt
python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/mh1log.txt $OUT/xbox_stderr.txt >/dev/null 2>&1
python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/settings.ini $OUT/settings_after.ini >/dev/null 2>&1
kill $SP 2>/dev/null; SP=""
ls $OUT/*.png
grep -aq "net: Xbox address" $OUT/xbox_serial.txt $OUT/xbox_stderr.txt || fail "the Xbox network did not come up"
grep -aq "tcp open .* 10.0.2.2:$PORT" $OUT/xbox_serial.txt $OUT/xbox_stderr.txt || fail "the Xbox did not connect to the server from settings.ini"
grep -aq "town step 4" $OUT/xbox_serial.txt $OUT/xbox_stderr.txt || fail "the Xbox did not reach the town"
grep -aq "town step 4" $OUT/pc.log || fail "the PC client did not reach the town"
grep -aq 'slot [0-9] "XBOX"' $OUT/pc.log || fail "the PC client does not see the Xbox's hunter"
echo "xemu online test OK: the Xbox logged in from settings.ini, reached the town, the PC client sees it ($OUT)"
