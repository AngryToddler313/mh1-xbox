#!/bin/bash
# Every 5 min: kill hung compiles (>3 min) and report; every 30 min merge + verify + push helper work.
i=0
while true; do
  for p in $(pgrep -f 'mwccps2.exe'); do
    e=$(ps -o etimes= -p $p 2>/dev/null | tr -d ' ')
    [ -n "$e" ] && [ "$e" -gt 180 ] && { kill $p; echo "HUNG compile killed (pid $p, ${e}s)"; }
  done
  c=""; for w in A B C; do c="$c $w:$(git -C /root/wt/$w log --oneline claude/mhg-decomp-tjvfyr..HEAD 2>/dev/null | wc -l)"; done
  echo "heartbeat $(date -u +%H:%MZ) unmerged$c"
  i=$((i+1))
  if [ $((i % 6)) -eq 0 ]; then echo "merge: $(/root/merge.sh 2>&1 | tr '\n' ' ' | cut -c1-200)"; fi
  sleep 300
done
