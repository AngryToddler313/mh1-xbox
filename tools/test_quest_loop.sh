#!/bin/sh
# Scripted check of the first player loop on the PC build, from power-on:
# NEW GAME (name TEST) -> house -> Village Elder (gift, quest 131 "raw meat")
# -> gate -> camp -> area 1: two Aptonoth killed and carved -> delivered at
# the camp box -> reward screen -> money -> village -> house bed save; then a
# second run: CONTINUE, which must report money 1550 and the kept pouch.
# Test aids used (they move / help the hunter, see docs/pc.md): RT_LB_WARP,
# RT_PL_WARP, RT_PL_WARP_EM, RT_PL_TARGET, RT_DMG_MUL, RT_PL_ITEMS_MIN. Saves go to
# build/show/loop/card (deleted first). Headless, ~20 s on x86.
cd "$(dirname "$0")/.."
BIN=${BIN:-build/pc/mhview}   # RUN=wine BIN=build/win/mhview.exe: the Windows build under Wine (docs/pc.md)
export RT_NOMOVIE=1   # the opening movie would only lengthen the scripted boot (test_movie.sh covers it)
OUT=build/show/loop; mkdir -p $OUT
export MH1_SAVE_DIR="$PWD/$OUT/card"; rm -rf "$MH1_SAVE_DIR"
D=tools/pc_scripts
EV="2860:square*2"
i=0; while [ $i -lt 30 ]; do EV="$EV;$((2930 + 27 * i)):circle*2"; i=$((i + 1)); done
EV="$EV;3770:square*2"
S="$(python3 tools/mk_input.py $D/newgame.txt "$EV" 3787),$(cat $D/quest131_hunt.txt)"
S=$(echo "$S" > $OUT/chain.txt; CUT=8100 python3 tools/mk_input.py $OUT/chain.txt "8129:square*2;8229:$(cat $D/bed_save.txt)" 9500)
RT_NAME=TEST RT_QUEST_TRACE=1 \
RT_LB_WARP="1300,2290,1000,4000;1360,10901,12409,38AB;2210,10650,15225;2301,11225,14400,0;2391,2259,745,4001" \
RT_PL_TARGET="0:0,1250:1" RT_PL_WARP="10,12250,10000;2400,6750,11900;2500,10350,10500" \
RT_PL_WARP_EM=90-520,600,1270-1700,1780 RT_DMG_MUL=40 RT_PL_ITEMS_MIN=2450:18:2 \
    $RUN $BIN disc/mh1 --boot --input "$S" --shot $OUT/end1.png --time 316 2> $OUT/run1.log >/dev/null
grep -E "money \+(1500|32) |rt_village: enter|item 18: " $OUT/run1.log
# the carves: each of the two Aptonoth is carved once, raw meat (18) or a small wyvern bone (227) by a random roll; the
# delivery needs 2 raw meat, so RT_PL_ITEMS_MIN tops the pouch up to 2 before the camp box when the rolls gave fewer.
# What is checked here is that both carves landed in the pouch, whatever they gave.
C=$(sed -n '/item 18: /q; s/^rt_flow: tick [0-9]* pouch://p' $OUT/run1.log | tail -1 | tr ' ' '\n' | awk -F: '$1 == 18 || $1 == 227 {n += $2} END {print n + 0}')
if [ "$C" -lt 2 ]; then echo "loop FAILED: $C of 2 carves in the pouch (see $OUT)"; exit 1; fi
RT_QUEST_TRACE=1 $RUN $BIN disc/mh1 --boot --input "$(cat $D/continue.txt)" --shot $OUT/end2.png --time 60 \
    2> $OUT/run2.log >/dev/null
if grep -q "money 1550" $OUT/run2.log; then echo "loop OK: CONTINUE has 1550z"; else echo "loop FAILED (see $OUT)"; exit 1; fi
