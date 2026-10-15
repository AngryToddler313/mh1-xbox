#!/bin/bash
# The coordinator's check before a push: byte-matching rebuild (all five modules OK),
# PC build, every PC test, both Xbox builds. Prints one line per step; logs in build/check/.
cd "$(dirname "$0")/.." || exit 1
mkdir -p build/check
fail=0
# memory caps per step (docs/agents/BRIEF.md): 1.5 GB for PC builds/tests, 3 GB for the PS2 rebuild (wibo),
# none for test_save (AddressSanitizer). Don't run this script itself under a cap: a cap can't be raised again.
step() {  # step NAME CMD...
    local n=$1; shift
    local cap=1500000
    case $n in test_save) cap=unlimited ;; test_ps2_patch) cap=3000000 ;; esac   # ASan; wibo (PS2 compiler)
    if (ulimit -v $cap; "$@") > "build/check/$n.log" 2>&1; then echo "ok   $n"; else echo "FAIL $n (build/check/$n.log)"; fail=1; fi
}
(ulimit -v 3000000; tools/rebuild.sh) > build/check/rebuild.log 2>&1
n=$(grep -cE '^[a-z]+ +OK ' build/check/rebuild.log)
if [ "$n" = 5 ]; then echo "ok   rebuild (5 OK)"; else echo "FAIL rebuild ($n OK, build/check/rebuild.log)"; fail=1; fi
step build_pc tools/build_pc.sh
step build_pc_online env ONLINE=1 tools/build_pc.sh
step build_pc_mhg tools/build_pc_mhg.sh   # tests that also run G use this binary
for t in tools/test_*.sh; do step "$(basename "$t" .sh)" "$t"; done
step build_pc_plain tools/build_pc.sh   # the ONLINE=1 build rewrote objs.txt; the Xbox build reads the plain one
step xbox_null bash -c ". ~/xboxdev/env.sh && python3 tools/build_xbox.py"
step xbox_nv2a bash -c ". ~/xboxdev/env.sh && python3 tools/build_xbox.py --gfx nv2a"
exit $fail
