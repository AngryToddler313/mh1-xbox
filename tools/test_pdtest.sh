#!/bin/sh
# The memory-card patch test of docs/network.md 3.8 in PCSX2 (hidden; ~2 min). TEST ONLY.
#   boot 1: build/ps2patch/pdtest/install/SLPM_654.95 - at the title, personalize a plain "M-HUNTER" list on this
#           console without the DNAS server step, decode it again, apply it, save it into the card's patch slot
#   boot 2: build/ps2patch/pdtest/probe/SLPM_654.95 - what the plain game's PatchLoadinDNAS_Main does, with a log line
#           per step (stored length, decode results, "M-HUNTER", the target before/after PatchExecCS)
# The card: a fresh image holding the PC port's save (CARD_FROM, default ~/.local/share/mh1pc/memcard0), so the
# title's auto-load finds a save. Needs tools/ps2_pdtest.py's two executables, PCSX2 and the BIOS (tools/run_pcsx2.sh).
# usage: tools/test_pdtest.sh [DISC.iso]     PAD=0x580 to build with the experiment's zero header (tools/ps2_pdtest.py)
set -e
cd "$(dirname "$0")/.."
ISO=${1:-disc/mh1/Monster Hunter (Japan).iso}
LOGD=build/ps2patch/pdtest/logs; mkdir -p $LOGD
python3 tools/ps2_pdtest.py "$ISO" --pad "${PAD:-0}"
tools/run_pcsx2.sh --iso "$ISO" --elf build/ps2patch/pdtest/install/SLPM_654.95 \
    --card-from "${CARD_FROM:-$HOME/.local/share/mh1pc/memcard0}" --seconds ${SECS:-45} --log $LOGD/boot1.txt
tools/run_pcsx2.sh --iso "$ISO" --elf build/ps2patch/pdtest/probe/SLPM_654.95 --seconds ${SECS:-45} --log $LOGD/boot2.txt
echo "--- boot 1 (install)"; grep PDTEST $LOGD/boot1.txt | sed 's/^\[ *[0-9.]*\] //' | uniq | head -40
echo "--- boot 2 (probe)";   grep PDTEST $LOGD/boot2.txt | sed 's/^\[ *[0-9.]*\] //' | uniq | head -40
if grep -q "PDTEST target .* after PatchExecCS.*PATCHED" $LOGD/boot2.txt; then
    echo "RESULT: the saved, personalized patch was decoded and applied at boot"
elif grep -q "PERSONALIZE OK" $LOGD/boot1.txt; then
    echo "RESULT: personalize worked, but boot 2 did not apply it (see above)"
else
    echo "RESULT: personalize of plain data FAILED (see boot 1)"
fi
