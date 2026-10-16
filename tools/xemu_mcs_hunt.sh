#!/bin/sh
# The Xbox and a PC hunt together on the game's own in-hunt protocol (docs/network.md 3.10): the online Xbox build in
# xemu (hidden), an ONLINE=1 PC client, mh1-server --lobby-mcs on this machine's LAN address (xemu's NAT reaches it, as
# in tools/test_three_way.sh). The PC (ANNA) posts quest 1 for 2 players, the Xbox joins at the quest board and gets
# ready at the departure spot (RT_LB_WARP, as the PC tests), ANNA starts; both run the PS2's session code (rt_mcs.c)
# through the server's mcs relay. Screenshots of both screens in the hunt, ANNA delivers, both see the clear.
# build/show/xemu_mcs (never committed). About 10 minutes; uncapped (xemu); xemu alone.
#   build first: ONLINE=1 tools/build_pc.sh && . ~/xboxdev/env.sh && python3 tools/build_xbox.py --online --gfx nv2a
cd "$(dirname "$0")/.."
SRC=${XBOX_FILES:-xbox_files}
O=build/show/xemu_mcs; rm -rf $O; mkdir -p $O
XF=build/xemu_mcs/xbox_files
. tools/emu_lib.sh
SP=; PP=; XP=
cleanup() { emu_stop $PP $XP $SP; SP=; PP=; XP=; }
emu_traps cleanup
fail() { echo "xemu mcs hunt FAILED: $1 (see $O)"; cleanup; exit 1; }
[ -f build/xbox/online/nv2a/default.xbe ] || fail "no build/xbox/online/nv2a/default.xbe (see the header)"
[ -x build/pc/mhview_online ] || fail "no build/pc/mhview_online (ONLINE=1 tools/build_pc.sh)"
for f in mcpx.bin bios.bin; do [ -f "$SRC/$f" ] || fail "no $SRC/$f (the owner's Xbox files; XBOX_FILES=DIR)"; done
xemu_running && fail "another xemu is running (xemu runs alone)"
[ "$(ulimit -v)" = unlimited ] || fail "a memory cap (ulimit -v) is set: xemu cannot start under it; run uncapped"
[ -f build/coop/save_ANNA/BISLPM-65495MH/BISLPM-65495MH ] || tools/test_coop.sh saves ANNA >/dev/null || fail "no ANNA card"
rm -rf $O/card_ANNA; mkdir -p $O/card_ANNA; cp -r build/coop/save_ANNA/. $O/card_ANNA/
IP=$(ip -4 route get 192.168.0.1 2>/dev/null | sed -n 's/.* src \([0-9.]*\).*/\1/p')
[ -n "$IP" ] || IP=$(hostname -I | awk '{print $1}')
case "$IP" in 10.*|192.168.*|172.1[6-9].*|172.2?.*|172.3[01].*) ;; *) fail "no private LAN address ($IP)";; esac

cat > $O/mh1-server.conf <<CONF
[server]
bind = $IP
lobby-port = 10200
lobby-mcs = yes
mcs-port = 10298
db = accounts.sqlite3
verbose = yes
CONF
for a in xbox pc; do
    python3 tools/server/mh1_server.py account add --db $O/accounts.sqlite3 --note $a > $O/account_$a.txt || fail "account"
done
id_of() { sed -n 's/^login \([0-9]*\) password .*/\1/p' $O/account_$1.txt; }
pw_of() { sed -n 's/^login [0-9]* password \(.*\)/\1/p' $O/account_$1.txt; }
python3 tools/server/mh1_server.py serve --config $O/mh1-server.conf > $O/server.log 2>&1 &
SP=$!
sleep 2; grep -q "lobby on $IP:10200" $O/server.log || fail "server did not start"

# ---- the Xbox: settings.ini with the LAN server and the xbox account; the PC tests' town screens (no card: a new
# hunter, XBOX), onto the hall's door (RT_LB_WARP) and in (square), to the quest board, the join late (ANNA's room must be up), then the
# departure spot and ready
rm -rf build/xemu_mcs; mkdir -p $XF
cp "$SRC/mcpx.bin" "$SRC/bios.bin" $XF/
[ -f "$SRC/eeprom.bin" ] && cp "$SRC/eeprom.bin" $XF/
truncate -s 8G $XF/hdd.img
python3 tools/xbox_fatx.py format $XF/hdd.img >/dev/null || fail "could not format the disk image"
printf 'online_server = %s:10200\r\nonline_login = %s\r\nonline_password = %s\r\n' $IP $(id_of xbox) "$(pw_of xbox)" \
    > $O/settings.ini
python3 tools/xbox_fatx.py put $XF/hdd.img $O/settings.ini Games/MH1/settings.ini >/dev/null || fail "could not put settings.ini"
XJ=${XJ:-3600}      # the Xbox's join at the board (its ticks)
JOIN=""; t=$((XJ + 50)); while [ $t -le $((XJ + 500)) ]; do JOIN="$JOIN;$t:circle*2"; t=$((t + 50)); done
REW=""; t=$((XJ + 2600)); while [ $t -le $((XJ + 8000)) ]; do REW="$REW;$t:circle*2"; t=$((t + 120)); done
XIN=$(python3 tools/mk_input.py - "200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3;$XJ:square*3$JOIN;$((XJ+900)):square*3;$((XJ+960)):circle*2;$((XJ+1020)):circle*2;$((XJ+1080)):circle*2$REW" $((XJ + 9000)))
echo "$XIN" > $O/xbox_input.txt
cat > $O/xbox_env.txt <<EOF
RT_ARGS=--quest 10 --play --online
RT_NAME=XBOX
RT_VILLAGE_START=1
RT_VILLAGE_SKIP_INTRO=1
RT_ONLINE_TRACE=1
RT_NET_REGISTERED=1
RT_NP_TRACE=1
RT_NP_POS=150
RT_LB_WARP=60,5545,2750,8000;300,1850,1500,8000;$((XJ + 300)),3100,2100,8000
EOF
XBOX_FILES=$XF tools/run_xemu.sh --online --gfx nv2a --data disc/mh1 --env $O/xbox_env.txt --input $O/xbox_input.txt \
    --shot $O/xbox.png --after "${AFTER:-200 260 320 380 440 500 560}" > $O/xemu_run.log 2>&1 &
XP=$!
i=0
while [ $i -lt 500 ] && ! grep -aq "online: quest download" $XF/xemu_debug.txt 2>/dev/null; do sleep 2; i=$((i + 2)); done
grep -aq "online: quest download" $XF/xemu_debug.txt || fail "the Xbox did not enter a lobby"
echo "the Xbox entered a lobby after about $i s"

# ---- the PC (ANNA): as tools/test_ps2_hunt.sh
LOGIN="200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3"
TALK=""; t=1041; while [ $t -le 1441 ]; do TALK="$TALK;$t:circle*2"; t=$((t + 40)); done
SHEET="1521:dleft*2;1543:dleft*2;1565:ddown*2;1590:ddown*2;1615:ddown*2;1640:circle*2;1700:circle*2;1760:circle*2;1820:circle*2;1880:circle*2"
START=${START:-6000}
GO=""; for t in $START $((START + 900)) $((START + 1800)); do
    GO="$GO;$t:square*3;$((t+60)):circle*2;$((t+120)):circle*2;$((t+180)):circle*2"
done
HUNT=""; t=$((START + 3300)); while [ $t -le $((START + 7500)) ]; do HUNT="$HUNT;$t:circle*2"; t=$((t + 90)); done
IA=$(python3 tools/mk_input.py - "$LOGIN$TALK;$SHEET$GO$HUNT" 14000)
(env MH1_SAVE_DIR="$PWD/$O/card_ANNA" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_QUEST_TRACE=1 \
    RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc) RT_NET_PASS=$(pw_of pc) \
    RT_LB_WARP="60,5545,2750,8000;300,1300,1600,C000;$((START - 1000)),3100,2100,8000" RT_NP_TRACE=1 RT_NP_POS=150 \
    RT_NP_EM=150 RT_PL_ITEMS=77:15 RT_PL_WARP="2400,10350,10640,7000" RT_SHOTS=${SHOTS:-1700,6100,7400,8000,9600,10200} \
    sh -c "ulimit -v 1500000; exec timeout 900 build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute \
    --input '$IA' --shot $O/pc.png --time 460 --size 640x480" > $O/pc.log 2>&1) &
PP=$!
wait $PP; PP=
wait $XP; XP=
cp $XF/xemu_debug.txt $O/xbox_serial.txt
python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/mh1log.txt $O/xbox_stderr.txt >/dev/null 2>&1
cleanup
grep -q "mcs: login .*player 0 of match" $O/server.log || fail "ANNA did not log in to the game server"
grep -q "mcs: login .*player 1 of match" $O/server.log || fail "the Xbox did not log in to the game server"
grep -aq "mcs: every player is in" $O/pc.log || fail "the select handshake did not complete"
grep -aq "mcs: start agreed" $O/pc.log || fail "the start was not agreed"
grep -aq "mcs: start agreed" $O/xbox_serial.txt $O/xbox_stderr.txt || fail "the Xbox did not agree on the start"
grep -aq "rt_flow: tick [0-9]* mode 2 step 0 D5 3" $O/pc.log || fail "the PC did not see the quest cleared"
echo "xemu mcs hunt OK: the Xbox and a PC in one hunt on the game's own protocol ($O)"
