#!/bin/sh
# Monster Hunter G: the revival code G added to em01 / em15 / em17 (include/em_revive_g.h: a quest monster with lives
# left lies 1800 ticks, fades 150, comes back 5% bigger at its start point; Quest_enemy_revival_ck / _set). No offline
# quest revives these kinds, so the test aid RT_QEM_LIVES=kind:2 (src/pc/rt/rt_em.c) gives the quest's monster of that
# kind a second life and makes its hunt goal 2; RT_PL_SLAY kills it, RT_PL_GOTO f follows it. Hidden, ~1 min.
#  quest 10    Rathian (1, em01): dies, comes back on stage 40, dies again -> quest clear
#  quest 40    kind 15 (em15):    dies on 18, comes back on stage 19, the hunter follows, dies again
#  quest 23/26 kinds 22 / 17 (em17): come back at G's fixed start points, stage 64 (kinds 0x16 / 0x30) / stage 90
# Logs: build/show/revival/qN.out (RT_EM_TRACE), qN.err
cd "$(dirname "$0")/.."
BIN=${BIN:-build/mhg/pc/mhview_g}
[ -x "$BIN" ] || { echo "no $BIN (tools/build_pc_mhg.sh)"; exit 1; }
OUT=build/show/revival; rm -rf $OUT; mkdir -p $OUT
fail=0
run() {     # quest kind secs
    RT_NOMOVIE=1 RT_QUEST_TRACE=1 RT_EM_TRACE=1 RT_QEM_LIVES=$2:2 RT_QUEST_STAGE=1 RT_PL_GOTO=60,f RT_PL_TARGET=k$2 \
        RT_PL_SLAY=100,$2 RT_PL_GOD=1 timeout 600 $BIN disc/mhg --quest $1 --input "idle*30" --shot $OUT/q$1.png --time $3 \
        > $OUT/q$1.out 2> $OUT/q$1.err
}
# the act trace of the quest's monster (slot 0): "stage act" per tick, runs collapsed
acts() { grep "^em0: stg" $OUT/q$1.out | awk '{print $3, $7}' | uniq; }
check() {   # name quest condition-text
    if [ "$3" = ok ]; then echo "$1 OK"; else echo "$1 FAILED ($2)"; fail=1; fi
}

run 10 1 120
a=$(acts 10)
r=$(echo "$a" | grep -c "^40 5/0/3"); c=$(grep -c "D5 3" $OUT/q10.err)
check "em01 (quest 10, Rathian): died, came back on stage 40 ($r fade), died again, quest clear" 10 \
    "$([ "$r" -ge 1 ] && echo "$a" | grep -q "^40 0/1/0" && [ "$c" -ge 1 ] && echo ok)"

run 40 15 200
a=$(acts 40)
check "em15 (quest 40, kind 15): came back on stage 19 and died there" 40 \
    "$(echo "$a" | grep -q "^19 0/1/0" && echo "$a" | grep -q "^19 5/0/1" && echo ok)"

run 23 22 120
check "em17 (quest 23, kind 22): came back on stage 64" 23 "$(acts 23 | grep -q "^64 0/1/0" && echo ok)"
run 26 17 120
check "em17 (quest 26, kind 17): came back on stage 90" 26 "$(acts 26 | grep -q "^90 0/1/0" && echo ok)"
exit $fail
