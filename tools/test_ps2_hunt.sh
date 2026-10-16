#!/bin/sh
# A PS2 and a PC hunt together (docs/network.md 3.10): the owner's unpatched disc in PCSX2 through the memory-card
# launcher (as tools/test_three_way.sh), an ONLINE=1 PC client, mh1-server with --lobby-mcs on this machine's LAN
# address. The PC (ANNA) posts quest 1 for 2 players at the guild hall's counter, the PS2 joins it at the quest board,
# ANNA starts the match; both connect to the server's game server (tools/server/mcs.py) and run the PS2's own session
# (the PC: rt_mcs.c, the game's mcsls / AQ code). Screenshots of both screens in the hunt, ANNA delivers the quest item,
# both see the clear. build/show/ps2_hunt (never committed).
#
# The PS2 cannot be warped by a test aid as the PC can (RT_LB_WARP): a PCSX2 cheat file (pnach, RAM writes every frame)
# holds its hunter on the guild hall's door while it is in the town square (stage 0x4C) and at the quest board while
# it is in the guild hall (0x4D), and marks the hunter as registered with the guild (event flag 4, what the PC tests'
# RT_NET_REGISTERED does). That is the only RAM the test touches; the game code is unchanged.
#
# usage: tools/test_ps2_hunt.sh     (~20 min; uncapped: PCSX2 cannot start under ulimit -v; PCSX2 alone)
#   XBOX=1: three platforms: the online Xbox build in xemu joins as the third player (room for 3; as
#           tools/xemu_mcs_hunt.sh; XBOX_FILES = the owner's Xbox files). ~25 min.
# Needs: PCSX2 + the owner's BIOS (tools/run_pcsx2.sh), the launcher (tools/ps2launch/build.sh), a PS2 card with the
# NET file (build/ps2online/card_net.ps2, from tools/test_ps2_online.sh), build/pc/mhview_online, ANNA's card
# (tools/test_coop.sh saves).
cd "$(dirname "$0")/.."
O=build/show/ps2_hunt; rm -rf $O; mkdir -p $O
. tools/emu_lib.sh
RP=; SP=; PCP=; XP=
cleanup() {
    emu_stop $PCP $XP $RP $SP
    RP=; SP=; PCP=; XP=
}
emu_traps cleanup
fail() { echo "ps2 hunt test FAILED: $1 (see $O)"; cleanup; exit 1; }
CARDNET=${CARD_NET:-build/ps2online/card_net.ps2}
[ -f build/ps2launch/MH1LAUNCH.ELF ] || fail "no launcher (tools/ps2launch/build.sh)"
[ -f "$CARDNET" ] || fail "no PS2 card with the NET file ($CARDNET: run tools/test_ps2_online.sh once)"
[ -x build/pc/mhview_online ] || fail "no build/pc/mhview_online (ONLINE=1 tools/build_pc.sh)"
pcsx2_running && fail "another PCSX2 is running"
[ -z "$XBOX" ] || ! xemu_running || fail "another xemu is running"
[ "$(ulimit -v)" = unlimited ] || fail "a memory cap (ulimit -v) is set: PCSX2 cannot start under it; run uncapped"
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
for a in ps2 pc ${XBOX:+xbox}; do
    python3 tools/server/mh1_server.py account add --db $O/accounts.sqlite3 --note $a > $O/account_$a.txt || fail "account"
done
id_of() { sed -n 's/^login \([0-9]*\) password .*/\1/p' $O/account_$1.txt; }
pw_of() { sed -n 's/^login [0-9]* password \(.*\)/\1/p' $O/account_$1.txt; }
python3 tools/server/mh1_server.py serve --config $O/mh1-server.conf > $O/server.log 2>&1 &
SP=$!
sleep 2; grep -q "lobby on $IP:10200" $O/server.log || fail "server did not start"

# the cheat file: game_w.stage (0x3F3404) 0x4C -> player_work[game_w.master] (0x3E4BF0 + 0xA00 * master; master at
# 0x3F34C1) at the guild hall's door (5545, 2750), 0x4D -> at the quest board (1850, 1500), facing 0x8000 (the PC test
# clients' RT_LB_WARP spots); in the hall only once (a latch byte), see the comments below.
python3 - $O/warp.pnach <<'PY'
import struct, sys
L = ["gametitle=MH1 test aid: hunter warps (tools/test_ps2_hunt.sh)", "", "[MH1TestWarp]", "author=mh1-xbox tests"]
f2i = lambda v: struct.unpack(">I", struct.pack(">f", v))[0]
# the guild master's hunter registration (event flag 4: User_data 0x3C6FC0 + 0x24, bit 4) as done, as the PC tests'
# RT_NET_REGISTERED: without it the quest board's square does nothing (16-bit OR every frame)
L += ["patch=1,EE,703C6FE4,extended,00100010"]
for stage, x, z in ((0x4C, 5545.0, 2750.0), (0x4D, 1850.0, 1500.0)):
    for k in range(4):
        p = 0x3E4BF0 + 0xA00 * k
        hall = stage == 0x4D
        w = ["patch=1,EE,2%07X,extended,%08X" % (p + 0xAC, f2i(x)),
             "patch=1,EE,2%07X,extended,%08X" % (p + 0xB4, f2i(z)),
             "patch=1,EE,1%07X,extended,00008000" % (p + 0xE)]
        if hall:
            # in the hall: once, right where the hunter comes in from the door (600, 2425: x's high half 0x4416), so it
            # can walk off the spot again
            g = ["patch=1,EE,D%07X,extended,%02X004416" % (p + 0xAE, 3)]
        else:
            # in the square: held on the door every frame until the hall loads (held there, the game takes the hunter
            # in by itself; standing on it, it waits for square)
            g = []
        L += ["patch=1,EE,D03F3404,extended,%02X01%04X" % (len(g) + len(w) + 1, stage),
              "patch=1,EE,D03F34C1,extended,%02X01%04X" % (len(g) + len(w), k)] + g + w
# in the hall while L2 alone is held (Psw[0].on == 0x0004, ioRead_sub's bits): at the departure spot (3100, 2100), where
# square is "ready to depart" for a room member (the PC tests' third RT_LB_WARP)
for k in range(4):
    p = 0x3E4BF0 + 0xA00 * k
    L += ["patch=1,EE,D03F3404,extended,0501004D", "patch=1,EE,D03F34C1,extended,0401%04X" % k,
          "patch=1,EE,D03F3710,extended,03000004",
          "patch=1,EE,2%07X,extended,%08X" % (p + 0xAC, f2i(3100.0)),
          "patch=1,EE,2%07X,extended,%08X" % (p + 0xB4, f2i(2100.0)),
          "patch=1,EE,1%07X,extended,00008000" % (p + 0xE)]
open(sys.argv[1], "w").write("\n".join(L) + "\n")
PY

# ---- the PS2: launcher + plain disc, the card with the NET file, the screens of tools/test_three_way.sh to the town
printf 'server = %s\nlogin = %s\npassword = %s\n' $IP:10200 $(id_of ps2) "$(pw_of ps2)" > build/ps2launch/MH1LAUNCH.CNF
DNUM=${DNUM:-97}
K=build/pcsx2/xkey
grab() { ffmpeg -nostdin -v error -y -f x11grab -draw_mouse 0 -video_size $2 -i :$1 -frames:v 1 "$3" </dev/null; }
shot() { grab $DNUM 640x480 "$O/ps2_$1.png"; }
DNUM=$DNUM tools/run_pcsx2.sh --net --iso "disc/mh1/Monster Hunter (Japan).iso" --elf build/ps2launch/MH1LAUNCH.ELF \
    --card $CARDNET --cheats $O/warp.pnach --seconds 1500 --log $O/ps2_emulog.txt > $O/ps2_run.txt 2>&1 &
RP=$!
sleep 50
NK=0
k() {
    ks=""; w=9
    for a in "$@"; do case "$a" in [0-9]*) w=$a ;; *) ks="$ks $a:200:600" ;; esac; done
    DISPLAY=:$DNUM $K $ks; sleep $w; NK=$((NK + 1))
    [ -z "$STEPSHOTS" ] || shot step_$(printf %03d $NK)_$(echo $ks | tr -c 'A-Za-z0-9\n' _ | cut -c1-20)
}
is_terms() {
    ffmpeg -nostdin -v error -f x11grab -video_size 640x480 -i :$DNUM -frames:v 1 -f rawvideo -pix_fmt rgb24 - </dev/null |
        python3 -c 'import sys
b = sys.stdin.buffer.read()
n = sum(1 for y in range(250, 275) for x in range(240, 400)
        if b[(y * 640 + x) * 3] > 170 and b[(y * 640 + x) * 3 + 1] < 120 and b[(y * 640 + x) * 3 + 2] < 120)
sys.exit(0 if n > 40 else 1)'
}
shot 01_boot
k Return 5; k c 5; k c 6; k c 5; k c 8; k c 8; k Up c 8; k c 9
k c; k c; k c; k c
t=0; for i in 1 2 3; do k c; is_terms && { t=1; k Up c 10; break; }; done
[ $t = 1 ] || fail "PS2: no terms notice"
k Down Down c 35
shot 02_login
grep -q "login $(id_of ps2) from .*: ok" $O/server.log || fail "the PS2 did not log in"
k Right c 10; k c 6; k c 14; k c 6; k c 25
shot 03_town
grep -q "binary type .* in lobby 1" $O/server.log || fail "the PS2 did not reach the town"
sleep 15
k z 10      # square on the door: into the guild hall (the cheat puts the hunter at the board)
shot 04_guild_hall
echo "PS2 in the town (guild hall: $O/ps2_04_guild_hall.png)"

XP=; NPL=2
if [ -n "$XBOX" ]; then     # ---- the Xbox (tools/xemu_mcs_hunt.sh's set-up); it joins after the PS2
    NPL=3
    SRC=${XBOX_FILES:-xbox_files}; XF=build/ps2_hunt_xbox/xbox_files
    [ -f build/xbox/online/nv2a/default.xbe ] || fail "no Xbox online build (tools/build_xbox.py --online --gfx nv2a)"
    for f in mcpx.bin bios.bin; do [ -f "$SRC/$f" ] || fail "no $SRC/$f (XBOX_FILES=DIR)"; done
    rm -rf build/ps2_hunt_xbox; mkdir -p $XF
    cp "$SRC/mcpx.bin" "$SRC/bios.bin" $XF/
    [ -f "$SRC/eeprom.bin" ] && cp "$SRC/eeprom.bin" $XF/
    truncate -s 8G $XF/hdd.img
    python3 tools/xbox_fatx.py format $XF/hdd.img >/dev/null || fail "could not format the disk image"
    printf 'online_server = %s:10200\r\nonline_login = %s\r\nonline_password = %s\r\n' $IP $(id_of xbox) "$(pw_of xbox)" \
        > $O/settings.ini
    python3 tools/xbox_fatx.py put $XF/hdd.img $O/settings.ini Games/MH1/settings.ini >/dev/null || fail "settings.ini"
    XJ=${XJ:-6500}
    JOIN=""; t=$((XJ + 50)); while [ $t -le $((XJ + 500)) ]; do JOIN="$JOIN;$t:circle*2"; t=$((t + 50)); done
    REW=""; t=$((XJ + 2600)); while [ $t -le $((XJ + 9000)) ]; do REW="$REW;$t:circle*2"; t=$((t + 120)); done
    python3 tools/mk_input.py - "200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3;$XJ:square*3$JOIN;$((XJ+900)):square*3;$((XJ+960)):circle*2;$((XJ+1020)):circle*2;$((XJ+1080)):circle*2$REW" $((XJ + 10000)) > $O/xbox_input.txt
    printf 'RT_ARGS=--quest 10 --play --online\nRT_NAME=XBOX\nRT_VILLAGE_START=1\nRT_VILLAGE_SKIP_INTRO=1\nRT_ONLINE_TRACE=1\nRT_NET_REGISTERED=1\nRT_NP_TRACE=1\nRT_NP_POS=150\nRT_LB_WARP=60,5545,2750,8000;300,1850,1500,8000;%d,3100,2100,8000\n' $((XJ + 300)) > $O/xbox_env.txt
    XBOX_FILES=$XF tools/run_xemu.sh --online --gfx nv2a --data disc/mh1 --env $O/xbox_env.txt --input $O/xbox_input.txt \
        --shot $O/xbox.png --after "300 340 380 420 460 500 540 580" > $O/xemu_run.log 2>&1 &
    XP=$!
    i=0
    while [ $i -lt 500 ] && ! grep -aq "online: quest download" $XF/xemu_debug.txt 2>/dev/null; do sleep 2; i=$((i + 2)); done
    grep -aq "online: quest download" $XF/xemu_debug.txt || fail "the Xbox did not enter a lobby"
    echo "the Xbox entered a lobby after about $i s"
fi

# ---- the PC (ANNA): into the guild hall, a room for 2 players without a password, quest 1; after the PS2 joined,
# the match. Ticks are the PC's (30 a second from its start).
LOGIN="200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3"
TALK=""; t=1041; while [ $t -le 1441 ]; do TALK="$TALK;$t:circle*2"; t=$((t + 40)); done
DL="1543:dleft*2;"; [ -n "$XBOX" ] && DL=""    # players 4 -> 2 (or 3)
SHEET=${SHEET:-"1521:dleft*2;${DL}1565:ddown*2;1590:ddown*2;1615:ddown*2;1640:circle*2;1700:circle*2;1760:circle*2;1820:circle*2;1880:circle*2"}
START=${START:-$([ -n "$XBOX" ] && echo 8000 || echo 6000)}     # ANNA's match start (square, circles; the PC tests' timing): the PS2 joins and gets ready before
GO=""     # tried three times, 30 s apart: before every member is ready the counter only says so
for t in $START $((START + 900)) $((START + 1800)); do
    GO="$GO;$t:square*3;$((t+60)):circle*2;$((t+120)):circle*2;$((t+180)):circle*2"
done
HUNT=""     # in the hunt: circle now and then (the delivery at the box after RT_PL_WARP, then the reward screens)
t=$((START + 3300)); while [ $t -le $((START + 7500)) ]; do HUNT="$HUNT;$t:circle*2"; t=$((t + 90)); done
IA=$(python3 tools/mk_input.py - "$LOGIN$TALK;$SHEET$GO$HUNT" 16500)
(env MH1_SAVE_DIR="$PWD/$O/card_ANNA" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_QUEST_TRACE=1 \
    RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc) RT_NET_PASS=$(pw_of pc) \
    RT_LB_WARP="60,5545,2750,8000;300,1300,1600,C000;$((START - 1000)),3100,2100,8000" RT_NP_TRACE=1 RT_NP_POS=150 RT_NP_EM=150 RT_PL_ITEMS=77:15 \
    RT_PL_WARP="${WARP_A:-2400,10350,10640,7000}" RT_SHOTS=${SHOTS:-1700,4000,6100,7400,8000,9600,10200,11200,12200} \
    timeout 900 build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute --input "$IA" \
    --shot $O/pc.png --time 540 --size 640x480 > $O/pc.log 2>&1) &
PCP=$!
i=0
while [ $i -lt 200 ] && ! grep -q "creates room" $O/server.log; do sleep 2; i=$((i + 2)); done
grep -q "creates room" $O/server.log || fail "ANNA did not create a room"
grep -q "room 1 has a password" $O/server.log && fail "ANNA's room has a password (SHEET)"
echo "ANNA's room is up"
# the PS2 at the quest board: square (as the PC's BOB), then circle through the board, the room, yes; then ready
k z 4
for i in 1 2 3 4 5 6 7 8 9 10; do k c 2; done
shot 05_board
i=0
while [ $i -lt 30 ] && ! grep -q "joins room 1" $O/server.log; do k c 2; i=$((i + 1)); done
grep -q "joins room 1" $O/server.log || fail "the PS2 did not join ANNA's room"
shot 06_room
# ready: L2 held (the cheat moves the hunter to the departure spot), then square ("出発準備"), circle for yes
DISPLAY=:$DNUM $K 1:1500:600; sleep 3     # (k() would take "1..." for a wait time)
shot 07_departure
k z 4
for i in 1 2 3 4 5 6; do
    [ "$(grep -c 'cmd 6506' $O/server.log)" -ge 1 ] && break
    k c 3
done
shot 07_ready
i=0
while [ $i -lt 300 ] && ! grep -q "mcs: login .*player 1 of match" $O/server.log; do sleep 2; i=$((i + 2)); done
grep -q "mcs: login .*player 1 of match" $O/server.log || fail "the PS2 did not log in to the game server"
echo "the PS2 is on the game server"
# in the hunt: screenshots; the PS2's hunter walks (left stick) so the PC sees it move
sleep 30; shot hunt_30
DISPLAY=:$DNUM $K i:2500:300 l:1500:300; sleep 5; shot hunt_40
for s in 60 90 120 150 180 210 240; do sleep 25; shot hunt_$s; done
wait $PCP; PCP=
if [ -n "$XP" ]; then
    wait $XP; XP=
    cp $XF/xemu_debug.txt $O/xbox_serial.txt
    python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/mh1log.txt $O/xbox_stderr.txt >/dev/null 2>&1
fi
cleanup
grep -aq "mcs: every player is in" $O/pc.log || fail "the PC and the PS2 did not complete the select (sys 2)"
grep -aq "mcs: start agreed" $O/pc.log || fail "the start was not agreed (net_start_ck)"
# the PS2's hunter (slot 1) as the PC sees it: at its start spot first, then somewhere else after its walk
n=$(grep -a "np-pos: .* me 0 slot 1 stg" $O/pc.log | awk '{print $11, $13}' | sort -u | wc -l)
[ "$n" -ge 2 ] || fail "the PC did not see the PS2's hunter move"
grep -aq "rt_flow: tick [0-9]* mode 2 step 0 D5 3" $O/pc.log || fail "the PC did not see the quest cleared"
grep -aq "mcs: logged in to the game server: $NPL players" $O/pc.log || fail "not $NPL players in the session"
if [ -n "$XBOX" ]; then
    grep -q "mcs: login .*player 2 of match" $O/server.log || fail "the Xbox did not log in to the game server"
    grep -aq "mcs: start agreed" $O/xbox_serial.txt $O/xbox_stderr.txt || fail "the Xbox did not agree on the start"
    grep -aq "np-pos: .* me 0 slot 2 stg" $O/pc.log || fail "the PC has no third hunter"
fi
echo "ps2 hunt test: OK (see $O)"
