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
#   HUNT=kutku: quest 4 (the Kut-Ku, "怪鳥イャンクック襲来！", the fifth quest card) instead of quest 1: the monster's owner
#           passes PC -> PS2 -> PC as each leaves its area, both hit it (the PS2 alone while it owns it, the PC alone
#           while the PS2 owns it), the PC kills it, both carve. The PS2 is steered through PCSX2's PINE socket
#           (tools/pcsx2/ps2_hunt_aid.py: RAM writes as the PC's RT_PL_* test aids, and a trace of the monster as the
#           PS2 has it); tools/hunt_sync.py compares that trace with the PC's. ~30 min. MUL = the PC's damage factor.
#   DROP=ps2 (with HUNT=kutku): PCSX2 is closed at quest tick 2200, while the PS2 owns the monster and the PC is at the
#           camp: the PC's session must drop it, take the monster over when back on its area, kill it, carve and clear.
#   PC2=1 (with HUNT=kutku XBOX=1): four players, a second PC (BOB) joins too; BOB and the Xbox stay at the camp until
#           3400, then follow and fight to the kill. DROP=xbox: xemu is stopped at quest tick 4200 (mid-fight).
#           A watchdog stops this test's emulators (xemu first) when the machine's available memory drops under 800 MB.
#   XBOX=1: three platforms: the online Xbox build in xemu joins as the third player (room for 3; as
#           tools/xemu_mcs_hunt.sh; XBOX_FILES = the owner's Xbox files). ~25 min.
# Needs: PCSX2 + the owner's BIOS (tools/run_pcsx2.sh), the launcher (tools/ps2launch/build.sh), a PS2 card with the
# NET file (build/ps2online/card_net.ps2, from tools/test_ps2_online.sh), build/pc/mhview_online, ANNA's card
# (tools/test_coop.sh saves).
cd "$(dirname "$0")/.."
O=${OUTDIR:-build/show/ps2_hunt}; rm -rf $O; mkdir -p $O     # OUTDIR: keep several runs apart
. tools/emu_lib.sh
RP=; SP=; PCP=; XP=; AP=; KP=; BP=; WP=
cleanup() {
    rm -f $O/ps2_atk
    emu_stop $WP $KP $AP $PCP $BP $XP $RP $SP
    RP=; SP=; PCP=; XP=; AP=; KP=; BP=; WP=
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
for a in ps2 pc ${XBOX:+xbox} ${PC2:+pc2}; do
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
PSLOT=${PSLOT:-28061}
PINE=$PSLOT DNUM=$DNUM tools/run_pcsx2.sh --net --iso "disc/mh1/Monster Hunter (Japan).iso" --elf build/ps2launch/MH1LAUNCH.ELF \
    --card $CARDNET --cheats $O/warp.pnach --seconds 1500 --log $O/ps2_emulog.txt > $O/ps2_run.txt 2>&1 &
RP=$!
if [ "$HUNT" = kutku ]; then    # the PS2's test aid (PINE): idle until the hunt
    echo idle > $O/ps2_ctl
    python3 tools/pcsx2/ps2_hunt_aid.py --slot $PSLOT --ctl $O/ps2_ctl --kind 6 --exits $O/pc.log > $O/ps2_trace.txt 2>&1 &
    AP=$!
fi
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
    XEND=$((XJ + 10000))
    if [ "$HUNT" = kutku ]; then    # attacks and circles from the hunt on (RT_PL_CARVE keeps circle back while it lives)
        REW=""; t=$((XJ + 2003)); while [ $t -le $((XJ + 17000)) ]; do REW="$REW;$t:circle*2;$((t+15)):cam_u*2"; t=$((t + 30)); done
        XEND=$((XJ + 18000))
    fi
    python3 tools/mk_input.py - "200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3;$XJ:square*3$JOIN;$((XJ+900)):square*3;$((XJ+960)):circle*2;$((XJ+1020)):circle*2;$((XJ+1080)):circle*2$REW" $XEND > $O/xbox_input.txt
    printf 'RT_ARGS=--quest 10 --play --online\nRT_NAME=XBOX\nRT_VILLAGE_START=1\nRT_VILLAGE_SKIP_INTRO=1\nRT_ONLINE_TRACE=1\nRT_NET_REGISTERED=1\nRT_NP_TRACE=1\nRT_NP_POS=150\nRT_LB_WARP=60,5545,2750,8000;300,1850,1500,8000;%d,3100,2100,8000\n' $((XJ + 300)) > $O/xbox_env.txt
    [ "$HUNT" = kutku ] && printf 'RT_PL_GOD=1\nRT_PL_TARGET=k6\nRT_PL_GOTO=3400,f\nRT_GOTO_TRACE=1\nRT_PL_CARVE=10\nRT_DMG_MUL=%s\nRT_PL_WARP_EM=3500-12000\nRT_NP_EM=30\n' ${MUL:-25} >> $O/xbox_env.txt
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
DL="1521:dleft*2;1543:dleft*2;"; [ -n "$XBOX" ] && DL="1521:dleft*2;"; [ -n "$PC2" ] && NPL=$((NPL + 1)) && DL="${DL#1521:dleft\*2;}"    # players 4 -> 2 (3, 4)
[ -n "$PC2" ] && [ -z "$XBOX" ] && DL="1521:dleft*2;"
SHEET=${SHEET:-"${DL}1565:ddown*2;1590:ddown*2;1615:ddown*2;1640:circle*2;1700:circle*2;1760:circle*2;1820:circle*2;1880:circle*2"}
START=${START:-$([ -n "$XBOX" ] && echo 8000 || echo 6000)}     # ANNA's match start (square, circles; the PC tests' timing): the PS2 joins and gets ready before
GO=""     # tried three times, 30 s apart: before every member is ready the counter only says so
for t in $START $((START + 900)) $((START + 1800)); do
    GO="$GO;$t:square*3;$((t+60)):circle*2;$((t+120)):circle*2;$((t+180)):circle*2"
done
HUNTIN=""     # in the hunt: circle now and then (the delivery at the box after RT_PL_WARP, then the reward screens)
t=$((START + 3300)); while [ $t -le $((START + 7500)) ]; do HUNTIN="$HUNTIN;$t:circle*2"; t=$((t + 90)); done
CARD=""; END=16500; SECS=540; AIDS="RT_PL_ITEMS=77:15 RT_PL_WARP=${WARP_A:-2400,10350,10640,7000}"
if [ "$HUNT" = kutku ]; then
    CARD=";1262:dleft*2"     # the fifth quest card (dleft on the first)
    # in the hunt: attacks (right stick up) and circle (carve, then the reward screens), cross / ddown + circle ("end
    # receiving"); who hits when is decided by the warps (a hunter away from the monster swings at the air)
    HUNTIN=""; t=$((START + 707)); while [ $t -le $((START + 14000)) ]; do
        HUNTIN="$HUNTIN;$t:circle*2;$((t+15)):cam_u*2;$((t+30)):circle*2;$((t+45)):cam_u*2;$((t+60)):circle*2;$((t+75)):cam_u*2"
        HUNTIN="$HUNTIN;$((t+90)):circle*2;$((t+105)):cam_u*2;$((t+120)):circle*2;$((t+135)):cam_u*2;$((t+150)):circle*2"
        HUNTIN="$HUNTIN;$((t+165)):cam_u*2;$((t+180)):cross*2;$((t+200)):ddown*2;$((t+220)):circle*2;$((t+235)):cam_u*2"
        t=$((t + 250))
    done
    SECS=$((START / 30 + 480)); END=$((SECS * 30))
    # quest ticks: the PC follows the Kut-Ku (60) and fights alone (600-1500), goes back to the camp (1500: the monster
    # passes to the PS2, which fights alone), follows it again (2500) and hits it (2600-) while the PS2 still owns it
    # (and no longer attacks); the PS2 goes to the camp (3300: the monster comes back to the PC), the PC kills it and
    # carves; the PS2 comes back and carves
    AIDS="RT_PL_GOD=1 RT_PL_TARGET=k6 RT_PL_GOTO=60,f RT_GOTO_TRACE=1 RT_PL_CARVE=10 RT_DMG_MUL=${MUL:-25} RT_PL_GOTO2=1500,21;2500,-1"
    AIDS="$AIDS RT_PL_WARP_EM=600-1500,2600-12000 RT_NP_EM=30"
fi
IA=$(python3 tools/mk_input.py - "$LOGIN$TALK$CARD;$SHEET$GO$HUNTIN" $END)
(env MH1_SAVE_DIR="$PWD/$O/card_ANNA" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_QUEST_TRACE=1 \
    RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc) RT_NET_PASS=$(pw_of pc) \
    RT_LB_WARP="60,5545,2750,8000;300,1300,1600,C000;$((START - 1000)),3100,2100,8000" RT_NP_TRACE=1 RT_NP_POS=150 RT_NP_EM=150 $AIDS \
    RT_SHOTS=${SHOTS:-1700,4000,6100,7400,8000,9600,10200,11200,12200} \
    timeout $((SECS + 360)) build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute --input "$IA" \
    --shot $O/pc.png --time $SECS --size 640x480 > $O/pc.log 2>&1) &
PCP=$!
if [ -n "$PC2" ]; then     # ---- a second PC (BOB): joins ANNA's room at the board first (as in test_online_mcs.sh)
    sleep 2
    rm -rf $O/card_BOB; mkdir -p $O/card_BOB; cp -r build/coop/save_BOB/. $O/card_BOB/ 2>/dev/null || fail "no BOB card (tools/test_coop.sh saves BOB)"
    JB=""; t=2000; while [ $t -le 2450 ]; do JB="$JB;$t:circle*2"; t=$((t + 50)); done
    IB=$(python3 tools/mk_input.py - "$LOGIN;1950:square*3$JB;2700:square*3;2760:circle*2;2820:circle*2$HUNTIN" $END)
    AIDS_B="RT_NP_EM=150"
    [ "$HUNT" = kutku ] && AIDS_B="RT_PL_GOD=1 RT_PL_TARGET=k6 RT_PL_GOTO=3400,f RT_GOTO_TRACE=1 RT_PL_CARVE=10 RT_DMG_MUL=${MUL:-25} RT_PL_WARP_EM=3500-12000 RT_NP_EM=30"
    (env MH1_SAVE_DIR="$PWD/$O/card_BOB" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_QUEST_TRACE=1 \
        RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc2) RT_NET_PASS=$(pw_of pc2) \
        RT_LB_WARP="60,5545,2750,8000;300,1850,1500,8000;2100,3100,2100,8000" RT_NP_TRACE=1 RT_NP_POS=150 $AIDS_B \
        RT_SHOTS=${SHOTS:-1700,4000,6100,7400,8000,9600,10200,11200,12200} \
        timeout $((SECS + 360)) build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute --input "$IB" \
        --shot $O/pc2.png --time $((SECS - 2)) --size 640x480 > $O/pc2.log 2>&1) &
    BP=$!
fi
if [ -n "$XBOX" ]; then     # memory watchdog: this test's emulators stop (xemu first) under 800 MB available
    ( while sleep 5; do
        a=$(awk '/MemAvailable/ {print $2}' /proc/meminfo)
        if [ "$a" -lt 800000 ]; then echo "memory watchdog: $a kB available, stopping xemu" >> $O/watchdog.txt; emu_stop $XP; break; fi
      done ) &
    WP=$!
fi
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
if [ "$HUNT" = kutku ]; then    # ---- the Kut-Ku: the PS2's part, timed from the PC's quest start (30 ticks a second)
    i=0
    while [ $i -lt 400 ] && ! grep -aq "village: quest 4 starts" $O/pc.log; do sleep 2; i=$((i + 2)); done
    grep -aq "village: quest 4 starts" $O/pc.log || fail "the PC did not start the Kut-Ku quest (quest 4)"
    T0=$(date +%s)
    atq() { while [ $(( ($(date +%s) - T0) * 30 )) -lt $1 ]; do sleep 1; done; }
    # keys while $O/ps2_atk exists: its words (t = right stick up: attack); once the monster is dead in the PS2's own
    # trace, circle only (with a weapon drawn circle sheathes it first, then carves)
    ( while :; do
        if [ -f $O/ps2_atk ]; then
            hp=$(grep -a 'np-em:' $O/ps2_trace.txt | tail -1 | awk '{print $13}')
            keys=$(cat $O/ps2_atk 2>/dev/null); [ -n "$hp" ] && [ "$hp" -le 0 ] && keys=c
            for key in $keys; do DISPLAY=:$DNUM $K $key:150:150; done
        else sleep 0.5; fi
      done ) &
    KP=$!
    echo follow > $O/ps2_ctl                     # the PS2 follows the monster from the start
    atq 1200; shot kk_1200_pc_fights
    atq 1550; echo fight > $O/ps2_ctl; echo t > $O/ps2_atk
    atq 2000; shot kk_2000_ps2_fights
    atq 2200; shot kk_2200_ps2_fights
    if [ "$DROP" = ps2 ]; then      # the PS2 is closed while it owns the monster (the PC is at the camp)
        emu_stop $KP $AP $RP; KP=; AP=; RP=; rm -f $O/ps2_atk
        echo "PCSX2 closed at quest tick 2200"
        wait $PCP; PCP=
    else
    atq 2400; shot kk_2400_ps2_fights
    atq 2700; echo follow > $O/ps2_ctl; rm -f $O/ps2_atk
    atq 3000; shot kk_3000_pc_hits
    atq 3300; echo "goto 21" > $O/ps2_ctl
    atq 3900; shot kk_3900_ps2_camp
    atq 4200; echo fight > $O/ps2_ctl; echo t > $O/ps2_atk
    for q in 4800 5400 6000 6600 7200 7800 8400 9000 9600 10200 10800 11400; do
        atq $q; shot kk_$q
        if [ $q = 4800 ] && [ "$DROP" = xbox ] && [ -n "$XP" ]; then     # (quest tick ~4800: mid-fight)
            emu_stop $XP; XP=; echo "xemu stopped at quest tick 4800"
        fi
    done
    rm -f $O/ps2_atk
    wait $PCP; PCP=
    shot kk_end
    fi
else
# in the hunt: screenshots; the PS2's hunter walks (left stick) so the PC sees it move
sleep 30; shot hunt_30
DISPLAY=:$DNUM $K i:2500:300 l:1500:300; sleep 5; shot hunt_40
for s in 60 90 120 150 180 210 240; do sleep 25; shot hunt_$s; done
wait $PCP; PCP=
fi
[ -z "$BP" ] || { wait $BP; BP=; }
if [ -n "$XBOX" ]; then
    [ -z "$XP" ] || { wait $XP; XP=; }
    cp $XF/xemu_debug.txt $O/xbox_serial.txt
    python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/mh1log.txt $O/xbox_stderr.txt >/dev/null 2>&1
fi
cleanup
grep -aq "mcs: every player is in" $O/pc.log || fail "the PC and the PS2 did not complete the select (sys 2)"
grep -aq "mcs: start agreed" $O/pc.log || fail "the start was not agreed (net_start_ck)"
PS2SLOT=$(sed -n "s/.*mcs: login $(id_of ps2): player \([0-9]\) .*/\1/p" $O/server.log | head -1)
[ -n "$PS2SLOT" ] || fail "the PS2 has no player number on the game server"
# the PS2's hunter as the PC sees it: at its start spot first, then somewhere else after its walk
n=$(grep -a "np-pos: .* me 0 slot $PS2SLOT stg" $O/pc.log | awk '{print $11, $13}' | sort -u | wc -l)
[ "$n" -ge 2 ] || fail "the PC did not see the PS2's hunter move"
grep -aq "rt_flow: tick [0-9]* mode 2 step 0 D5 3" $O/pc.log || fail "the PC did not see the quest cleared"
grep -aq "mcs: logged in to the game server: $NPL players" $O/pc.log || fail "not $NPL players in the session"
if [ -n "$XBOX" ]; then
    XSLOT=$(sed -n "s/.*mcs: login $(id_of xbox): player \([0-9]\) .*/\1/p" $O/server.log | head -1)
    [ -n "$XSLOT" ] || fail "the Xbox did not log in to the game server"
    grep -aq "mcs: start agreed" $O/xbox_serial.txt $O/xbox_stderr.txt || fail "the Xbox did not agree on the start"
    grep -aq "np-pos: .* me 0 slot $XSLOT stg" $O/pc.log || fail "the PC does not see the Xbox's hunter"
fi
if [ -n "$PC2" ]; then
    grep -aq "mcs: start agreed" $O/pc2.log || fail "BOB did not agree on the start"
    grep -aq "rt_flow: tick [0-9]* mode 2 step 0 D5 3" $O/pc2.log || fail "BOB did not see the quest cleared"
fi
if [ "$HUNT" = kutku ]; then    # the monster as each machine had it: HP from its owner, the owner PC -> PS2 -> PC, the kill, carves
    L="PC=$O/pc.log PS2=$O/ps2_trace.txt"; C=PC; ST=PC; M=0,$PS2SLOT,0; X=""
    [ "$DROP" = ps2 ] || { C="$C,PS2"; ST="$ST,PS2"; }
    # a dropped owner leaves EMW+0x88E as it was when only one player is left (Em_Master_Change: n < 2 just takes the
    # monster over): the PC then runs it under the old owner number
    [ "$DROP" = ps2 ] && { M=0,$PS2SLOT; [ -z "$PC2$XBOX" ] && X="--runs PC"; }
    [ -n "$PC2" ] && L="$L BOB=$O/pc2.log" && C="$C,BOB" && ST="$ST,BOB"
    [ -n "$XBOX" ] && [ -s $O/xbox_stderr.txt ] && L="$L XBOX=$O/xbox_stderr.txt" && [ "$DROP" != xbox ] && ST="$ST,XBOX"
    python3 tools/hunt_sync.py --kind 6 --masters $M --kill --stay $ST $X --carve $C $L > $O/sync.txt
    r=$?; cat $O/sync.txt; [ $r = 0 ] || fail "the machines did not see the same hunt"
    if [ -n "$DROP" ]; then
        DS=$PS2SLOT; [ "$DROP" = xbox ] && DS=$XSLOT
        grep -a "mcs: player $DS dropped out" $O/pc.log | head -1 | grep . || fail "the PC's session never dropped the $DROP"
    fi
    [ -n "$WP" ] && [ -f $O/watchdog.txt ] && cat $O/watchdog.txt
fi
echo "ps2 hunt test: OK (see $O)"
