#!/bin/sh
# Memory soak on the PC build: LOOPS (default 3) village -> quest 131 -> village loops in one run
# from power-on (tools/mk_soak.py), RT_MEM on: every "stage a -> b" line of the debug log carries
# the tracked heap and the textures as the Xbox keeps them (KB) when the stage a was left. Prints
# those lines per loop and fails when a loop does not come back to the village or the village's
# heap + textures grow by more than 256 KB from loop 2 to the last loop. Headless, ~25 s per loop.
#   usage: tools/test_soak.sh [LOOPS]
cd "$(dirname "$0")/.."
BIN=${BIN:-build/pc/mhview}
N=${1:-3}
OUT=build/show/soak; rm -rf build/show/soak; mkdir -p $OUT
export RT_NOMOVIE=1 MH1_SAVE_DIR="$PWD/$OUT/card" MH1_LOG_DIR="$PWD/$OUT/logs"
python3 tools/mk_soak.py $N $OUT || exit 1
env $(cat $OUT/env.txt) RT_MEM=1 $BIN disc/mh1 --boot --input "$(cat $OUT/input.txt)" --shot $OUT/end.png \
    --time $(((8800 + 5200 * (N - 1)) / 30)) 2> $OUT/run.log >/dev/null
grep -h " I stage .* -> \| I quest .* START" $OUT/logs/*.log | sed 's/^ *[0-9.]* //' | tee $OUT/stages.txt
B=$(grep -c "quest 131 over, back in the village" $OUT/logs/*.log)
[ "$B" -eq "$N" ] || { echo "soak FAILED: $B of $N loops back in the village (see $OUT)"; exit 1; }
# the village (87) left for the house or the gate: heap + textures on each visit after a quest
grep "stage 87 -> 21" $OUT/stages.txt | sed 's/.*heap \([0-9]*\) KB, textures \([0-9]*\) KB/\1 \2/' | awk -v n=$N '
    { t[NR] = $1 + $2 } END { if (NR < n) { print "soak FAILED: " NR " village departures"; exit 1 }
      d = t[NR] - t[2]; print "village heap+textures per loop:", t[1], t[2], t[NR], "(loop 2 -> " n ": " (d >= 0 ? "+" : "") d " KB)"
      if (d > 256) { print "soak FAILED: memory grows per loop"; exit 1 } print "soak OK" }'
