#!/bin/sh
# gfn2.sh FUNC [module] : G asm of FUNC, only the instructions (no hex), labels kept, G addresses of branch targets as .L labels
cd "$(dirname "$0")/.." || exit 1
M=${2:-lobby}
f=$(grep -ln "glabel $1\$" asm/mhg/$M/text/*.s | head -1)
awk -v fn="$1" '$0 ~ "^glabel "fn"$"{p=1} p{print} /^endlabel/{if(p)exit}' "$f" | sed 's/^ *\/\* [0-9A-F]* \([0-9A-F]*\) [0-9A-F]* \*\/ */\1 /; s/ \/\* Failed.*//; s/^\([0-9A-F]\{8\}\) \([a-z]\)/\1 \2/' | cut -c1-80
