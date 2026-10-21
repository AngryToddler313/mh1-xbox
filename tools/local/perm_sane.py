#!/usr/bin/env python3
"""perm_sane.py DIR OUT.c: exit 0 if OUT.c (a decomp-permuter result in permuter dir DIR) still makes every call the
original base.c makes, counted as R_MIPS_26 relocations (jal / j) after compiling both with DIR/compile.sh.
decomp-permuter can reach score 0 by deleting code (dead `if (0)`, goto over a call) when the target is a different,
smaller function (a wrong name in the near-match list); such a result is not a match."""
import os, subprocess, sys, tempfile
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OD = os.path.join(ROOT, "tools/binutils/bin/mips64r5900el-ps2-elf-objdump")
def calls(d, src):
    with tempfile.TemporaryDirectory() as t:
        o = os.path.join(t, "x.o")
        if subprocess.run([os.path.join(d, "compile.sh"), src, "-o", o], capture_output=True).returncode or not os.path.exists(o):
            return None
        r = subprocess.run([OD, "-r", "-j", ".text", o], capture_output=True, text=True).stdout
        return sorted(l.split()[-1] for l in r.splitlines() if "R_MIPS_26" in l)
d, out = sys.argv[1], sys.argv[2]
b, o = calls(d, os.path.join(d, "base.c")), calls(d, out)
sys.exit(0 if b is not None and o is not None and all(o.count(c) >= b.count(c) for c in set(b)) else 1)
