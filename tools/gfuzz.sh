#!/bin/sh
# G village pad fuzz (agent D): tools/gfuzz.sh SEED SECS ; random buttons from tick 1280 (after the first-visit event); reports crashes
cd "$(dirname "$0")/.." || exit 1
S=$(python3 - $1 <<'PY'
import random,sys
random.seed(int(sys.argv[1]))
b=["up","down","left","right","circle","square","cross","triangle","start","l1","r1"]
out=["idle*1280"]
for i in range(150):
    out.append("%s*%d"%(random.choice(b),random.choice([2,10,40,80])))
    out.append("idle*%d"%random.choice([2,10,30]))
print(",".join(out))
PY
)
TAILN=0 tools/gvil.sh f$1 "$S" $2
ls -t build/test_logs | head -1 > /dev/null
N=$(ls -t build/test_logs | head -1)
if grep -q "===== CRASH" build/test_logs/$N; then echo "seed $1 CRASH"; sed -n '/CRASH/,/last/p' build/test_logs/$N | sed -n 2,14p; else echo "seed $1 ok"; fi
