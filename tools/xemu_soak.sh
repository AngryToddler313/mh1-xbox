#!/bin/sh
# Memory soak of the Xbox build in xemu, hidden: power-on -> NEW GAME -> LOOPS (default 3)
# village -> quest 131 -> village loops in one run (pad script and test aids from
# tools/mk_soak.py, as tools/test_soak.sh on the PC). The serial log's "stage a -> b, N KB free"
# lines give the free physical memory when stage a was left; the "alive" lines every 5 s too.
# Prints them per loop and fails when a loop does not come back to the village or the free
# memory when the village is left for the quest falls by more than 256 KB from loop 2 to the
# last loop. E: is formatted first (tools/xbox_fatx.py). Out: build/show/xsoak. About
# 4 + 2.5 * LOOPS minutes.
#   usage: tools/xemu_soak.sh [LOOPS] [--gfx null|nv2a]   (default 3, nv2a)
cd "$(dirname "$0")/.."
N=3; GFX=nv2a
while [ $# -gt 0 ]; do
    case "$1" in
    --gfx) GFX=$2; shift 2 ;;
    *) N=$1; shift ;;
    esac
done
OUT=build/show/xsoak; rm -rf build/show/xsoak; mkdir -p $OUT
python3 tools/mk_soak.py $N $OUT 240 || exit 1     # 240: the opening movie is skipped with start
tools/xbox_fatx.py format xbox_files/hdd.img || exit 1
T=$((240 + 150 * N))
tools/run_xemu.sh --gfx $GFX --data disc/mh1 --input $OUT/input.txt --env $OUT/env.txt --shot $OUT/run.png \
    --after "$(seq -s ' ' 60 60 $T) $T" >/dev/null 2>&1
cp xbox_files/xemu_debug.txt $OUT/serial.txt
tools/xbox_fatx.py get xbox_files/hdd.img Games/MH1/mh1log.txt $OUT/stderr.txt
grep -a "stage .* -> .*KB free\|quest .* START\|back in the village" $OUT/serial.txt | sed 's/^ *[0-9.]* //' | tee $OUT/stages.txt
B=$(grep -c "back in the village" $OUT/stages.txt)
[ "$B" -ge "$N" ] || { echo "xemu soak FAILED: $B of $N loops back in the village (see $OUT)"; exit 1; }
grep "stage 87 -> 21" $OUT/stages.txt | sed 's/.*, \([0-9]*\) KB free.*/\1/' | awk -v n=$N '
    { f[NR] = $1 } END { if (NR < n) { print "xemu soak FAILED: " NR " village departures"; exit 1 }
      d = f[NR] - f[2]; printf "free KB leaving the village per loop:"; for (i = 1; i <= NR; i++) printf " %d", f[i]
      print " (loop 2 -> " n ": " (d >= 0 ? "+" : "") d " KB)"
      if (d < -256) { print "xemu soak FAILED: free memory falls per loop"; exit 1 } print "xemu soak OK" }'
