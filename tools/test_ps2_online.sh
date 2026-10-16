#!/bin/sh
# The PS2 online patch for real: the owner's disc patched with tools/ps2_patch.py for OUR server (tools/server) on this
# machine, booted in PCSX2 (hidden, DEV9 network adaptor in "Sockets" mode: no root), driven through the game's own
# screens by scripted key presses (build/pcsx2/xkey): title -> CONTINUE -> the hunter -> 街へ行く (network mode) ->
# NET file (made on the card the first time, through the game's network settings = Sony's "Add Setting" guide) ->
# the connection -> the login -> the plaza -> lobby 1 -> the town. Then an ONLINE=1 PC client (headless) joins the same
# town: each sees the other, chat both ways (the PS2 types "いう" on its software keyboard). docs/network.md 3.6.
#
# usage: tools/test_ps2_online.sh [--launcher]      (~5-7 min; PCSX2 alone, nothing else heavy running)
#   --launcher  boot the UNPATCHED disc through the memory-card launcher (build/ps2launch/MH1LAUNCH.ELF,
#               tools/ps2launch/build.sh) with its config, instead of a patched image (docs/network.md 3.8)
#   FRESH=1     make the NET file again (else the card saved after the first network setup is reused)
# Needs: PCSX2 + the owner's BIOS (tools/run_pcsx2.sh), the rebuild (tools/rebuild.sh main lobby OK), the PC online
# build (ONLINE=1 tools/build_pc.sh), a PC save to start from (CARD_FROM, default ~/.local/share/mh1pc/memcard0).
# The server binds this machine's LAN address (PCSX2's guest cannot reach 127.0.0.1); it is invite-only (two fresh
# accounts in build/ps2online). Screenshots, logs, the card: build/ps2online (never committed).
cd "$(dirname "$0")/.."
O=build/ps2online; mkdir -p $O
LAUNCH=; [ "$1" = "--launcher" ] && LAUNCH=1
fail() { echo "ps2 online test FAILED: $1 (see $O)"; cleanup; exit 1; }
cleanup() {
    [ -n "$SLP" ] && kill $SLP 2>/dev/null
    [ -n "$RP" ] && wait $RP 2>/dev/null
    [ -n "$SP" ] && { kill $SP 2>/dev/null; wait $SP 2>/dev/null; }
    RP=; SP=; SLP=
}
# PCSX2 reserves several GB of address space (fastmem): under a `ulimit -v` cap it stops at once ("Failed to allocate
# fastmem area" in emulog.txt) and the screenshots stay black. Run this test uncapped (ps2_patch.py caps itself).
[ "$(ulimit -v)" = unlimited ] || fail "a memory cap (ulimit -v $(ulimit -v)) is set: PCSX2 cannot start under it; run uncapped"
IP=$(ip -4 route get 192.168.0.1 2>/dev/null | sed -n 's/.* src \([0-9.]*\).*/\1/p')
[ -n "$IP" ] || IP=$(hostname -I | awk '{print $1}')
case "$IP" in 10.*|192.168.*|172.1[6-9].*|172.2?.*|172.3[01].*) ;; *) fail "no private LAN address ($IP)";; esac
DNUM=${DNUM:-97}
K=build/pcsx2/xkey
shot() { ffmpeg -nostdin -v error -y -f x11grab -video_size 640x480 -i :$DNUM -frames:v 1 "$O/$1.png" </dev/null; }
key() { DISPLAY=:$DNUM $K "$@"; }

# the server (LAN address, verbose log) and two accounts
cat > $O/mh1-server.conf <<CONF
[server]
bind = $IP
lobby-port = 10200
lobby-relay = yes
relay-ports = 10301-10399
db = accounts.sqlite3
verbose = yes
CONF
rm -f $O/accounts.sqlite3 $O/accounts.sqlite3-wal $O/accounts.sqlite3-shm $O/accounts.sqlite3-journal
for a in ps2 pc; do
    python3 tools/server/mh1_server.py account add --db $O/accounts.sqlite3 --note $a > $O/account_$a.txt || fail "account"
done
id_of() { sed -n 's/^login \([0-9]*\) password .*/\1/p' $O/account_$1.txt; }
pw_of() { sed -n 's/^login [0-9]* password \(.*\)/\1/p' $O/account_$1.txt; }
python3 tools/server/mh1_server.py serve --config $O/mh1-server.conf > $O/server.log 2>&1 &
SP=$!
sleep 2; grep -q "lobby on $IP:10200" $O/server.log || fail "server did not start"

# the disc: patched image, or the launcher with the plain disc
if [ -n "$LAUNCH" ]; then
    [ -f build/ps2launch/MH1LAUNCH.ELF ] || fail "no launcher (tools/ps2launch/build.sh)"
    # the config next to the ELF (PCSX2 starts it as host:MH1LAUNCH.ELF; on a PS2 e.g. mc0:/APPS/)
    printf 'server = %s\nlogin = %s\npassword = %s\n' $IP:10200 $(id_of ps2) "$(pw_of ps2)" \
        > build/ps2launch/MH1LAUNCH.CNF
else
    (ulimit -v 3000000; python3 tools/ps2_patch.py "disc/mh1/Monster Hunter (Japan).iso" --server $IP:10200 \
        --login $(id_of ps2) --password "$(pw_of ps2)" --out $O/mh1_online.iso > $O/patch.txt 2>&1) || fail "ps2_patch"
fi
# the card: the PC save as a fresh card; the NET file is made in the game the first time and the card kept
if [ -f $O/card_net.ps2 ] && [ -z "$FRESH" ]; then CARD="--card $O/card_net.ps2"; NEWNET=; else
    CARD="--card-from ${CARD_FROM:-$HOME/.local/share/mh1pc/memcard0}"; NEWNET=1; fi

if [ -n "$LAUNCH" ]; then
    DNUM=$DNUM tools/run_pcsx2.sh --net --iso "disc/mh1/Monster Hunter (Japan).iso" \
        --elf build/ps2launch/MH1LAUNCH.ELF $CARD --seconds 900 --log $O/emulog.txt > $O/run.txt 2>&1 &
else
    DNUM=$DNUM tools/run_pcsx2.sh --net --iso $O/mh1_online.iso $CARD --seconds 900 --log $O/emulog.txt > $O/run.txt 2>&1 &
fi
RP=$!
sleep 50; SLP=$(pgrep -P $RP sleep | head -1)
shot 01_boot
# k KEYS... [SECONDS]: press the keys (one xkey call, each held 200 ms, 600 ms apart), then wait (default 9 s: the game
# ignores keys during its fades; a key sent in its own call right after another was lost now and then, so pairs like
# "Up c" go in one call). STEPSHOTS=1: a screenshot after each step (build/ps2online/step_*.png).
NK=0
k() {
    ks=""; w=9
    for a in "$@"; do case "$a" in [0-9]*) w=$a ;; *) ks="$ks $a:200:600" ;; esac; done
    DISPLAY=:$DNUM $K $ks; sleep $w; NK=$((NK + 1))
    [ -z "$STEPSHOTS" ] || shot step_$(printf %03d $NK)_$(echo $ks | tr -c 'A-Za-z0-9\n' _ | cut -c1-20)
}
# the "terms" notice (ブロードバンド接続... マナーを守りましょう, red text in the middle) is on screen
is_terms() {
    ffmpeg -nostdin -v error -f x11grab -video_size 640x480 -i :$DNUM -frames:v 1 -f rawvideo -pix_fmt rgb24 - </dev/null |
        python3 -c 'import sys
b = sys.stdin.buffer.read()
n = sum(1 for y in range(250, 275) for x in range(240, 400)
        if b[(y * 640 + x) * 3] > 170 and b[(y * 640 + x) * 3 + 1] < 120 and b[(y * 640 + x) * 3 + 2] < 120)
sys.exit(0 if n > 40 else 1)'
}
# from the autosave notice to the terms notice: circle until it shows (a menu "ネットワーク接続 / 前回と同じ接続 /
# ログインID削除" comes in between on some runs: its first entry), then 進む (up, circle)
to_terms_and_on() {
    for i in 1 2 3; do k c; is_terms && { k Up c 10; return 0; }; done
    fail "no terms notice"
}
# title -> CONTINUE -> slot 1 -> the hunter -> yes -> OK -> 街へ行く -> the autosave notice -> (NET file?)
k Return 5; k c 5; k c 6; k c 5; k c 8; k c 8; k Up c 8; k c 9
shot 02_net
if [ -n "$NEWNET" ]; then
    # no NET file: yes -> slot 1 -> make it (はい) -> saved -> the autosave notice -> (menu) -> terms (進む) ->
    # ネットワーク接続 screen: ネットワーク設定の編集 -> yes
    k c; k c; k Up c; k c
    to_terms_and_on
    k c; k Up c 15
    shot 03_netcnf
    # Sony's network settings: Add Setting, slot 1, guide (right), SCE/Ethernet (circle, right), confirm (OK), no PPPoE,
    # IP auto, DNS auto, the name, save, saved (OK), no connection test, back to the menu (OK), cross, Quit
    k c 5; k c 5; k Right 4; k c Right 4; k c 4; k Down c Right 4; k c Right 4; k c Right 4; k Right 4
    k c 9; k c 5; k c 5; k c 5; k x 5; k Left c 30
    shot 04_restart
    cp ~/ps2dev/pcsx2/data/PCSX2/memcards/Mcd001.ps2 $O/card_net.ps2     # the card with the NET file, before any login
    # the game restarted itself: the auto-load notice, the intro, then as above to the network
    k c 10; k Return 5; k c 5; k c 6; k c 5; k c 8; k c 8; k Up c 8; k c 9
fi
# NET file found: load it (yes) -> slot 1 -> yes -> loaded -> the autosave notice -> (menu) -> terms (進む)
k c; k c; k c; k c
to_terms_and_on
shot 05_connection
k Down Down c 35
shot 06_login
grep -q "login $(id_of ps2) from .*: ok" $O/server.log || fail "the PS2 did not log in"
# user registration: 登録しない; Capcom ID: the first free one, yes -> the plaza
k Right c 10; k c 6; k c 14
shot 07_plaza
# 街に入る -> lobby 1 -> the town
k c 6; k c 25
shot 08_town
grep -q "binary type .* in lobby 1" $O/server.log || fail "the PS2 did not reach the town"

# the PC client joins the town (fresh card, own account), says a line; the PS2 answers on its software keyboard
IN=$(python3 tools/mk_input.py - "200:circle*2;302:circle*2;404:circle*2;556:circle*2" 5000)
rm -rf $O/card_pc
(ulimit -v 1500000; env MH1_SAVE_DIR="$PWD/$O/card_pc" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 \
    RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc) RT_NET_PASS=$(pw_of pc) \
    RT_NAME=PCHUNTER RT_NET_SAY="700:PS2 and PC in one town" RT_SHOTS=800 timeout 200 build/pc/mhview_online disc/mh1 \
    --quest 10 --play --online --mute --input "$IN" --shot $O/pc.png --time 75 --size 640x480 > $O/pc.log 2>&1) &
PCP=$!
sleep 45; shot 09_ps2_sees_pc
# keyboard on (triangle); the cursor starts on あ: right, い, right, う; back to the first column, down to 記, left
# (wraps) to 決定
key s sleep:2000 Right c Right c sleep:800 Left:100:400 Left:100:400 Left:100:400 Down:100:400 Down:100:400 \
    Down:100:400 Left:100:400 sleep:300
shot 10_keyboard
for i in 1 2 3; do      # 決定 (a lone key press is lost now and then: again until the server has the line)
    k c 4; grep -aq "chat b'\\\\x82\\\\xa2\\\\x82\\\\xa4' in lobby 1" $O/server.log && break
done
shot 11_ps2_chat
wait $PCP
cleanup
grep -aq "chat b'PS2 and PC in one town' in lobby 1" $O/server.log || fail "the PC's chat line did not reach the server"
grep -a 'online: tick [0-9]* slot [0-9] "' $O/pc.log | grep -aq ' shown$' || fail "the PC does not show the PS2's hunter"
grep -aq "chat b'\\\\x82\\\\xa2\\\\x82\\\\xa4' in lobby 1" $O/server.log || fail "the PS2's chat line did not reach the server"
LC_ALL=C grep -aq "chat log: from .*\"$(printf '\202\242\202\244')\"" $O/pc.log || fail "the PC did not get the PS2's chat line"
echo "ps2 online test: OK (PS2 in PCSX2 on our server: login, plaza, town; a PC client in the same town; each sees the other; chat both ways)"
