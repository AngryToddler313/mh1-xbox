#!/bin/bash
# pipeline_par.sh [WORKERS] [PERM_SECONDS]: run WORKERS pipeline.sh workers on disjoint shards of the queue, splitting
# the CPU between their permuters (24 threads here, 2 kept free for Windows, Ollama and the compiler checks).
cd /root/mh1-xbox
n=${1:-4}; secs=${2:-300}
pj=$(( ($(nproc) - 2) / n )); [ $pj -lt 1 ] && pj=1
echo "$n workers, $pj permuter threads each, permuter budget ${secs}s per function"
for i in $(seq 0 $((n-1))); do SHARD=$i/$n PJ=$pj tools/local/pipeline.sh "$secs" & done
wait
echo "all workers finished"
