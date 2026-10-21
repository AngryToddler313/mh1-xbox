#!/bin/bash
# pipeline.sh [PERM_SECONDS] [LIST...]: tiered matching over the near-match queue (docs/handoff/near_*.txt).
#   1. tools/check.py: skip what already matches
#   2. decomp-permuter (randomized, PERM_SECONDS, default 600) on near misses (<= 12 differing instructions)
#   3. local model (tools/local/llm_match.py, qwen3-coder:30b, 3 rounds)
#   4. still not matching -> build/mhg/llm/snags.txt (for Claude)
# Matches are saved under build/mhg/matches/ (FILE path in the first line), not written into src/: applying them
# and the 7/7 link check is a separate step. Progress lines go to build/mhg/llm/run.txt (shown by watch.ps1).
# Parallel: SHARD=I/N takes every Nth queue entry (I = 0..N-1); PJ = permuter threads per worker (default 16).
# MODE=perm runs only step 2 (any diff count; CPU), MODE=llm only steps 3-4 (GPU), default both.
cd /root/mh1-xbox
secs=${1:-600}; shift
lists=${*:-docs/handoff/near_A.txt docs/handoff/near_C.txt docs/handoff/near_B.txt}
mkdir -p build/mhg/matches build/mhg/llm
shard=${SHARD:-0/1}; si=${shard%/*}; sn=${shard#*/}; pj=${PJ:-16}; k=-1
wid=${WID:-$si}; cur=build/mhg/llm/current_$wid.txt
log() { echo "$(date +%H:%M:%S) [w$wid] $*" | tee -a build/mhg/llm/run.txt; }
for l in $lists; do
  while read -r rank diff file func mod addr; do
    k=$((k+1)); [ $((k % sn)) -eq "$si" ] || continue
    [ -f "$file" ] || continue
    [ -f "build/mhg/matches/$func.c" ] && continue
    grep -q "^$func " build/mhg/llm/snags.txt 2>/dev/null && continue
    line=$(VERSION=mhg timeout 240 python3 tools/check.py "$file" < /dev/null 2>/dev/null | awk -v f="$func" '$2==f')
    case "$line" in OK*) continue;; "") log "SKIP $func ($file: no result from check.py)"; continue;; esac
    nd=$(echo "$line" | sed -n 's/.*(\([0-9]*\)\/\([0-9]*\) instructions differ).*/\1/p'); nd=${nd:-999}
    echo "w$wid permuter $func ($file, $nd differ)" > $cur
    mode=${MODE:-both}
    if [ "$mode" != llm ] && { [ "$mode" = perm ] || [ "$nd" -le 12 ]; } && python3 tools/local/perm_setup.py "$file" "$func" "$mod" > /dev/null 2>&1 < /dev/null; then
      d=build/mhg/perm/$func
      timeout "$secs" .venv/bin/python tools/permuter/permuter.py "$d" -j$pj --stop-on-zero > "$d/permuter.log" 2>&1 < /dev/null
      # a permuter "zero" that drops a call or jumps over code is a different function, not a match (tools/local/perm_sane.py)
      best=$(for o in "$d"/output-0-*; do [ -f "$o/source.c" ] && python3 tools/local/perm_sane.py "$d" "$o/source.c" && echo "$o"; done | head -1)
      if [ -n "$best" ]; then
        { echo "// $file  (decomp-permuter; preprocessed source)"; cat "$best/source.c"; } > "build/mhg/matches/$func.c"
        log "MATCH permuter $func ($file)"; continue
      fi
    fi
    [ "$mode" = perm ] && continue
    echo "w$wid local model $func ($file, $nd differ)" > $cur
    out=$(timeout 1800 python3 tools/local/llm_match.py "$file" "$func" --model qwen3-coder:30b --rounds 3 --save build/mhg/matches < /dev/null 2>&1 | tail -1)
    case "$out" in MATCH*) log "MATCH local-model $func ($file)"; continue;; esac
    echo "$func $file $mod $nd :: $out" >> build/mhg/llm/snags.txt
    log "SNAG $func ($file, $nd differ) -> Claude queue"
  done < "$l"
done
log "queue finished"; echo "w$wid idle (queue finished)" > $cur
