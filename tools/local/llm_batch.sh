#!/bin/bash
# llm_batch.sh MODEL COUNT [LIST...]: run llm_match.py (3 rounds, no --write) on the first COUNT near-match
# entries (docs/handoff/near_*.txt: `rank diff file func module addr`) whose file exists and which still differ.
cd /root/mh1-xbox
model=$1; count=$2; shift 2
lists=${*:-docs/handoff/near_A.txt docs/handoff/near_C.txt docs/handoff/near_B.txt}
n=0
for l in $lists; do
  while read -r rank diff file func mod addr; do
    [ -f "$file" ] || continue
    [ "$n" -ge "$count" ] && break 2
    out=$(timeout 1200 python3 tools/local/llm_match.py "$file" "$func" --model "$model" --rounds 3 2>&1 < /dev/null | tail -1)
    case "$out" in ALREADY*|function*|baseline*) continue;; esac
    n=$((n+1)); echo "$n $out"
  done < "$l"
done
