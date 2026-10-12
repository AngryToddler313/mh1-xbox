#!/bin/sh
# The first player loop (tools/test_quest_loop.sh) on the Xbox build in xemu, hidden:
# power-on -> opening movie (skipped with start) -> NEW GAME (name TEST) -> village ->
# Village Elder, quest 131 -> the hunt -> delivered -> reward -> village -> house bed save;
# then a second boot: CONTINUE must report money 1550. The pad script and test aids are
# the PC test's (tools/pc_scripts/, tools/mk_input.py), with the start press that skips
# the opening movie in front (the PC test runs with RT_NOMOVIE): everything after it is
# 240 ticks later. Test aids reach the Xbox through D:\data\env.txt (xbox_env.c).
# E: of xemu's disk image is formatted first (tools/xbox_fatx.py; the save is deleted).
# Screenshots: build/show/xloop/run1_*.png, run2_*.png; game logs (serial port and the
# game's stderr from E:) next to them. About 10 minutes.
#   usage: tools/xemu_quest_loop.sh [--gfx null|nv2a]   (default nv2a)
#   RECORD=1 also records each run to build/show/xloop/run1.mp4, run2.mp4 (run_xemu.sh --record)
cd "$(dirname "$0")/.."
GFX=nv2a; [ "$1" = "--gfx" ] && GFX=$2
OUT=build/show/xloop; rm -rf $OUT; mkdir -p $OUT
D=tools/pc_scripts; DL=240     # the movie skip: idle*700,start*2,idle*200 instead of idle*600,start*2,idle*60
sed 's/^idle\*600,start\*2,idle\*60,/idle*700,start*2,idle*200,/' $D/newgame.txt > $OUT/newgame.txt
EV="$((2860 + DL)):square*2"
i=0; while [ $i -lt 30 ]; do EV="$EV;$((2930 + DL + 27 * i)):circle*2"; i=$((i + 1)); done
EV="$EV;$((3770 + DL)):square*2"
echo "$(python3 tools/mk_input.py $OUT/newgame.txt "$EV" $((3787 + DL))),$(cat $D/quest131_hunt.txt)" > $OUT/chain.txt
CUT=$((8100 + DL)) python3 tools/mk_input.py $OUT/chain.txt "$((8129 + DL)):square*2;$((8229 + DL)):$(cat $D/bed_save.txt)" $((9500 + DL)) > $OUT/input1.txt
cat > $OUT/env1.txt <<'EOF'
RT_NAME=TEST
RT_QUEST_TRACE=1
RT_LB_WARP=1300,2290,1000,4000;1360,10901,12409,38AB;2210,10650,15225;2301,11225,14400,0;2391,2259,745,4001
RT_PL_TARGET=0:0,1250:1
RT_PL_WARP=10,12250,10000;2400,6750,11900;2500,10350,10500
RT_PL_WARP_EM=90-520,600,1270-1700,1780
RT_DMG_MUL=40
RT_PL_ITEMS_MIN=2450:18:2
EOF
echo RT_QUEST_TRACE=1 > $OUT/env2.txt
tools/xbox_fatx.py format xbox_files/hdd.img || exit 1
run() {     # run N INPUT ENV "times"
    tools/run_xemu.sh --gfx $GFX --data disc/mh1 --input $2 --env $3 --shot $OUT/run$1.png --after "$4" \
        ${RECORD:+--record $OUT/run$1.mp4} >/dev/null 2>&1
    cp xbox_files/xemu_debug.txt $OUT/serial$1.txt
    tools/xbox_fatx.py get xbox_files/hdd.img Games/MH1/mh1log.txt $OUT/stderr$1.txt
}
run 1 $OUT/input1.txt $OUT/env1.txt "60 120 180 240 300 360 420 480"
grep -E "quest 131 (START|CLEAR)|money \+|back in the village" $OUT/serial1.txt $OUT/stderr1.txt | sed 's/^[^:]*://'
run 2 $D/continue.txt $OUT/env2.txt "60 90 120"
if grep -q "money 1550" $OUT/stderr2.txt; then echo "xemu loop OK: CONTINUE has 1550z (see $OUT)"; else echo "xemu loop FAILED (see $OUT)"; exit 1; fi
