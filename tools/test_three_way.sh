#!/bin/sh
# Three platforms in one online town: the PS2 (the owner's disc in PCSX2, through the memory-card launcher), the Xbox
# build (xemu) and the PC build (mhview_online), all logged in to one of OUR servers (tools/server/mh1_server.py) on
# this machine's LAN address, standing by the fountain of lobby 1's town; each says a line in the chat. One screenshot
# from each, put side by side with labels: build/show/three_way/three_way.png (never committed). docs/network.md 3.9.
#
# usage: tools/test_three_way.sh            (~10 min; PCSX2 and xemu together, nothing else heavy running)
# Needs what tools/test_ps2_online.sh --launcher and tools/xemu_online_test.sh need: PCSX2 + the owner's BIOS, the
# launcher (tools/ps2launch/build.sh), a PS2 card with the NET file (build/ps2online/card_net.ps2, made by one run of
# tools/test_ps2_online.sh), the owner's Xbox files (XBOX_FILES, default xbox_files: mcpx.bin, bios.bin), the PC
# online build (ONLINE=1 tools/build_pc.sh) and the Xbox online build (tools/build_xbox.py --online --gfx nv2a).
# Order: the PS2 goes first, alone (its scripted key presses depend on timing); once it is in the town, xemu starts and
# then the PC client. Both are hidden (Xvfb); xemu gets a fresh scratch disk image, never the shared hdd.img.
cd "$(dirname "$0")/.."
O=build/show/three_way; rm -rf $O; mkdir -p $O
SP=; RP=; XP=; PCP=
# run_xemu.sh and run_pcsx2.sh stop their emulator and Xvfb when they are killed (tools/emu_lib.sh); this stops them,
# the PC client and the server on every way out (failure, Ctrl-C, kill)
. tools/emu_lib.sh
cleanup() {
    emu_stop $PCP $XP $RP $SP
    SP=; RP=; XP=; PCP=
}
emu_traps cleanup
fail() { echo "three-way test FAILED: $1 (see $O)"; cleanup; exit 1; }
SRC=${XBOX_FILES:-xbox_files}
CARDNET=${CARD_NET:-build/ps2online/card_net.ps2}
[ -f build/ps2launch/MH1LAUNCH.ELF ] || fail "no launcher (tools/ps2launch/build.sh)"
[ -f "$CARDNET" ] || fail "no PS2 card with the NET file ($CARDNET: run tools/test_ps2_online.sh once)"
[ -f build/xbox/online/nv2a/default.xbe ] || fail "no Xbox online build (tools/build_xbox.py --online --gfx nv2a)"
[ -x build/pc/mhview_online ] || fail "no build/pc/mhview_online (ONLINE=1 tools/build_pc.sh)"
for f in mcpx.bin bios.bin; do [ -f "$SRC/$f" ] || fail "no $SRC/$f (the owner's Xbox files; XBOX_FILES=DIR)"; done
xemu_running && fail "another xemu is running (xemu runs alone)"
pcsx2_running && fail "another PCSX2 is running"

[ "$(ulimit -v)" = unlimited ] || fail "a memory cap (ulimit -v) is set: PCSX2 and xemu cannot start under it; run uncapped"
IP=$(ip -4 route get 192.168.0.1 2>/dev/null | sed -n 's/.* src \([0-9.]*\).*/\1/p')
[ -n "$IP" ] || IP=$(hostname -I | awk '{print $1}')
case "$IP" in 10.*|192.168.*|172.1[6-9].*|172.2?.*|172.3[01].*) ;; *) fail "no private LAN address ($IP)";; esac

# the server (LAN address: PCSX2's guest cannot reach 127.0.0.1; xemu's NAT reaches it too), three accounts
cat > $O/mh1-server.conf <<CONF
[server]
bind = $IP
lobby-port = 10200
lobby-relay = yes
relay-ports = 10301-10399
db = accounts.sqlite3
verbose = yes
CONF
for a in ps2 xbox pc; do
    python3 tools/server/mh1_server.py account add --db $O/accounts.sqlite3 --note $a > $O/account_$a.txt || fail "account"
done
id_of() { sed -n 's/^login \([0-9]*\) password .*/\1/p' $O/account_$1.txt; }
pw_of() { sed -n 's/^login [0-9]* password \(.*\)/\1/p' $O/account_$1.txt; }
python3 tools/server/mh1_server.py serve --config $O/mh1-server.conf > $O/server.log 2>&1 &
SP=$!
sleep 2; grep -q "lobby on $IP:10200" $O/server.log || fail "server did not start"

# ---- the PS2: launcher + plain disc, the card with the NET file; same screens as tools/test_ps2_online.sh
printf 'server = %s\nlogin = %s\npassword = %s\n' $IP:10200 $(id_of ps2) "$(pw_of ps2)" > build/ps2launch/MH1LAUNCH.CNF
DNUM=${DNUM:-97}
K=build/pcsx2/xkey
grab() { ffmpeg -nostdin -v error -y -f x11grab -draw_mouse 0 -video_size $2 -i :$1 -frames:v 1 "$3" </dev/null; }
shot() { grab $DNUM 640x480 "$O/ps2_$1.png"; }
DNUM=$DNUM tools/run_pcsx2.sh --net --iso "disc/mh1/Monster Hunter (Japan).iso" --elf build/ps2launch/MH1LAUNCH.ELF \
    --card $CARDNET --seconds 1200 --log $O/ps2_emulog.txt > $O/ps2_run.txt 2>&1 &
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
shot 02_net
k c; k c; k c; k c
t=0; for i in 1 2 3; do k c; is_terms && { t=1; k Up c 10; break; }; done
[ $t = 1 ] || fail "PS2: no terms notice"
k Down Down c 35
shot 03_login
grep -q "login $(id_of ps2) from .*: ok" $O/server.log || fail "the PS2 did not log in"
k Right c 10; k c 6; k c 14; k c 6; k c 25
shot 04_town
grep -q "binary type .* in lobby 1" $O/server.log || fail "the PS2 did not reach the town"
echo "PS2 in the town"

# ---- the Xbox: scratch disk image with settings.ini pointing at the LAN server and the xbox account
XF=build/three_way_xbox; rm -rf $XF; mkdir -p $XF
cp "$SRC/mcpx.bin" "$SRC/bios.bin" $XF/
[ -f "$SRC/eeprom.bin" ] && cp "$SRC/eeprom.bin" $XF/
truncate -s 8G $XF/hdd.img
python3 tools/xbox_fatx.py format $XF/hdd.img >/dev/null || fail "could not format the disk image"
printf 'online_server = %s:10200\r\nonline_login = %s\r\nonline_password = %s\r\n' $IP $(id_of xbox) "$(pw_of xbox)" \
    > $O/settings.ini
python3 tools/xbox_fatx.py put $XF/hdd.img $O/settings.ini Games/MH1/settings.ini >/dev/null || fail "could not put settings.ini"
XSAY=${XSAY:-2100}      # the Xbox's chat line at this town tick (30 ticks a second; the PC is in by then)
cat > $O/xbox_env.txt <<EOF
RT_ARGS=--quest 10 --play --online
RT_NAME=XBOX
RT_VILLAGE_START=1
RT_VILLAGE_SKIP_INTRO=1
RT_ONLINE_TRACE=1
RT_NET_SAY=$XSAY:Hello from the Xbox
EOF
# (the town screens as in xemu_online_test.sh; at tick 1300 a small step left, then turn to the camera: see below)
echo "idle*200,circle*2,idle*100,circle*2,idle*100,circle*2,idle*150,circle*2,idle*742,left*14,down*2,idle*40000" \
    > $O/xbox_input.txt
XBOX_FILES=$XF tools/run_xemu.sh --online --gfx nv2a --data disc/mh1 --env $O/xbox_env.txt --input $O/xbox_input.txt \
    --shot $O/xbox_end.png --after 330 > $O/xemu_run.log 2>&1 &
XP=$!
i=0
while [ $i -lt 400 ] && ! grep -aq "online: quest download" $XF/xemu_debug.txt 2>/dev/null; do sleep 2; i=$((i + 2)); done
grep -aq "online: quest download" $XF/xemu_debug.txt || fail "the Xbox did not enter a lobby"
echo "the Xbox entered a lobby after about $i s"
XD=$(ps -o args= --ppid $XP 2>/dev/null | sed -n 's/.*Xvfb :\([0-9]*\).*/\1/p' | head -1)
[ -n "$XD" ] || fail "no xemu display"

# ---- the PC client: own account, named PC, a line in the chat, screenshots every 2 s from 60 s on.
# Where they stand: the camera follows each one's own hunter and the chat window covers the lower left of the screen,
# so the three stand in a short column in front of the fountain: the PC behind (it walks up from its arrival spot),
# the PS2 where it arrived, the Xbox in front, a step to the left. (A warp, RT_LB_WARP, only moves the hunter on its
# own screen: the others are not told; walking is.)
PSAY=${PSAY:-1500}
SHOTS=$(seq -s, 1800 60 7200)
rm -rf $O/card_pc
IN=$(python3 tools/mk_input.py - "200:circle*2;302:circle*2;404:circle*2;556:circle*2;1300:right*25;1330:up*70;1405:down*2" 7300)
date +%s > $O/pc_start.txt
(ulimit -v 1500000; env MH1_SAVE_DIR="$PWD/$O/card_pc" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 \
    RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_HOST=$IP RT_NET_PORT=10200 RT_NET_ID=$(id_of pc) RT_NET_PASS=$(pw_of pc) \
    RT_NAME=PC RT_NET_SAY="$PSAY:Hello from the PC" RT_SHOTS=$SHOTS \
    timeout 330 build/pc/mhview_online disc/mh1 --quest 10 --play --online --mute --input "$IN" --shot $O/pc.png \
    --time 245 --size 640x480 > $O/pc.log 2>&1) &
PCP=$!

# ---- wait for both chat lines at the server, then the PS2 types "いう" on its software keyboard
i=0
while [ $i -lt 200 ]; do
    grep -aq "chat b'Hello from the PC' in lobby 1" $O/server.log && grep -aq "chat b'Hello from the Xbox' in lobby 1" $O/server.log && break
    sleep 2; i=$((i + 2))
done
grep -aq "chat b'Hello from the PC' in lobby 1" $O/server.log || fail "the PC's chat line did not reach the server"
grep -aq "chat b'Hello from the Xbox' in lobby 1" $O/server.log || fail "the Xbox's chat line did not reach the server"
DISPLAY=:$DNUM $K s sleep:2000 Right c Right c sleep:800 Left:100:400 Left:100:400 Left:100:400 Down:100:400 \
    Down:100:400 Down:100:400 Left:100:400 sleep:300
for i in 1 2 3; do
    k c 4; grep -aq "chat b'\\\\x82\\\\xa2\\\\x82\\\\xa4' in lobby 1" $O/server.log && break
done
grep -aq "chat b'\\\\x82\\\\xa2\\\\x82\\\\xa4' in lobby 1" $O/server.log || fail "the PS2's chat line did not reach the server"
sleep 4
# the moment of the picture: PS2 and Xbox now; the PC took its own shots
NOW=$(date +%s)
shot final
grab $XD 1280x960 $O/xbox_final.png
echo "picture at $NOW"
wait $PCP; PCP=
# the PC's chat window shows a line for a few seconds only: its first shot after the PS2's line arrived there
PCT=$(LC_ALL=C sed -n "s/^online: tick \([0-9]*\) chat log: from .*\"$(printf '\202\242\202\244')\"/\1/p" $O/pc.log | head -1)
[ -n "$PCT" ] || fail "the PC did not get the PS2's chat line"
BEST=$(ls $O/pc_*.png 2>/dev/null | sed -n 's/.*pc_\([0-9]*\)\.png/\1/p' | sort -n |
    awk -v t=$((PCT + 30)) 'BEGIN{b=-1} {b=$1; if ($1 >= t) exit} END{print b}')
[ "$BEST" -ge 0 ] 2>/dev/null || fail "no PC screenshot"
cp $O/pc_$BEST.png $O/pc_final.png
cleanup
cp $XF/xemu_debug.txt $O/xbox_serial.txt
python3 tools/xbox_fatx.py get $XF/hdd.img Games/MH1/mh1log.txt $O/xbox_stderr.txt >/dev/null 2>&1

python3 - "$O" <<'PY' || fail "could not make the combined picture"
import sys
from PIL import Image, ImageDraw, ImageFont
o = sys.argv[1]
W, H, BAR = 640, 480, 40
out = Image.new("RGB", (W * 3, H + BAR), (20, 20, 20))
d = ImageDraw.Draw(out)
try:
    font = ImageFont.truetype("DejaVuSans-Bold.ttf", 26)
except OSError:
    font = ImageFont.load_default()
for i, (name, f) in enumerate((("PS2 (PCSX2)", "ps2_final.png"), ("Xbox (xemu)", "xbox_final.png"), ("PC", "pc_final.png"))):
    im = Image.open("%s/%s" % (o, f)).convert("RGB").resize((W, H), Image.LANCZOS)
    out.paste(im, (i * W, BAR))
    tw = d.textlength(name, font=font) if hasattr(d, "textlength") else 100
    d.text((i * W + (W - tw) / 2, 6), name, fill=(255, 220, 90), font=font)
out.save("%s/three_way.png" % o)
PY
grep -a 'online: tick [0-9]* slot [0-9] "' $O/pc.log | grep -a '"XBOX"' | grep -aq ' shown$' || fail "the PC does not show the Xbox's hunter"
grep -a 'online: tick [0-9]* slot [0-9] "' $O/pc.log | grep -av '"XBOX"\|(me)' | grep -aq ' shown$' || fail "the PC does not show the PS2's hunter"
LC_ALL=C grep -aq "chat log: from .*\"$(printf '\202\242\202\244')\"" $O/pc.log || fail "the PC did not get the PS2's chat line"
grep -aq "chat log: from .*\"Hello from the Xbox\"" $O/pc.log || fail "the PC did not get the Xbox's chat line"
echo "three-way test OK: PS2, Xbox and PC in one town, each said a line ($O/three_way.png)"
