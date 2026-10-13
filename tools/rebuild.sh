#!/bin/sh
# Regenerate the split for the given modules (default: all) and build.
# Usage: tools/rebuild.sh [main game ...]
#        VERSION=mhg tools/rebuild.sh [main sub_main ...]   (PS2 Monster Hunter G, docs/mhg.md)
cd "$(dirname "$0")/.." || exit 1
if [ "${VERSION:-mh1}" = mhg ]; then
    mods="${*:-main sub_main select game yn lobby nethttp}"
    VERSION=mhg python3 tools/setup_split.py > /dev/null || exit 1
    mkdir -p build/mhg
    for m in $mods; do
        rm -rf "asm/mhg/$m" "build/mhg/asm/mhg/$m"
        .venv/bin/python -m splat split "config/mhg/$m.yaml" > "build/mhg/split_$m.log" 2>&1 || { echo "splat failed for $m (build/mhg/split_$m.log)"; exit 1; }
    done
    VERSION=mhg python3 tools/build.py $mods 2>&1 | grep -v "Warning\|Assembler messages\|macro instruction"
    exit 0
fi
mods="${*:-main select game yn lobby}"
python3 tools/setup_split.py > /dev/null || exit 1
for m in $mods; do
    rm -rf "asm/$m" "build/asm/$m"
    .venv/bin/python -m splat split "config/$m.yaml" > "build/split_$m.log" 2>&1 || { echo "splat failed for $m (build/split_$m.log)"; exit 1; }
done
python3 tools/build.py 2>&1 | grep -v "Warning\|Assembler messages\|macro instruction"
