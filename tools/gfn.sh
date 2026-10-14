#!/bin/sh
# gfn.sh FUNC [module] : print G's asm of FUNC compactly (agent D tooling); module defaults to lobby
cd "$(dirname "$0")/.." || exit 1
M=${2:-lobby}
f=$(grep -ln "glabel $1\$" asm/mhg/$M/text/*.s | head -1)
awk -v fn="$1" '$0 ~ "^glabel "fn"$"{p=1} p{print} /^endlabel/{if(p)exit}' "$f" | sed 's/^ *\/\* [0-9A-F]* //; s/ \/\* Failed.*//' | cut -c1-90
