#!/bin/sh
# gfn.sh FUNC [module] : G's asm of FUNC without the hex columns (agent D tooling); module defaults to lobby.
# finds `glabel FUNC` or `alabel FUNC` (spimdisasm puts some functions of a big file under alabel), prints to the next label
cd "$(dirname "$0")/.." || exit 1
M=${2:-lobby}
f=$(grep -lE "^ *(glabel|alabel) $1\$" asm/mhg/$M/text/*.s | head -1)
[ -z "$f" ] && { echo "no label $1" >&2; exit 1; }
awk -v fn="$1" '$0 ~ "^ *(glabel|alabel) "fn"$"{p=1; print; next} p && /^ *(glabel|alabel|endlabel|nonmatching)/{exit} p{print}' "$f" | sed 's/^ *\/\* [0-9A-F]* //; s/ \/\* Failed.*//' | cut -c1-90
