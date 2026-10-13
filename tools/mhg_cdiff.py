#!/usr/bin/env python3
"""mhg_cdiff.py - which C files of config/mhg/c_files.txt differ from G's bytes?

After `VERSION=mhg tools/build.py` linked a module (MISMATCH), compares build/mhg/MODULE.bin
with build/mhg/split/MODULE.bin per C range and prints the differing files, the first
differing word and the linked symbol nearest below the address it refers to (from the
link map), which is usually the wrongly mapped name. --skip appends them to
config/mhg/c_skip.txt.

Usage: python3 tools/mhg_cdiff.py [MODULE ...] [--skip]
"""
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MODS = ["main", "sub_main", "select", "game", "yn", "lobby", "nethttp"]


def main():
    mods = [a for a in sys.argv[1:] if not a.startswith("--")] or MODS
    bad = []
    for mod in mods:
        bp = os.path.join(ROOT, "build/mhg/%s.bin" % mod)
        tp = os.path.join(ROOT, "build/mhg/split/%s.bin" % mod)
        if not os.path.exists(bp):
            continue
        built, want = open(bp, "rb").read(), open(tp, "rb").read()
        if built == want:
            continue
        vram = 0x100000 if mod == "main" else struct.unpack_from("<I", want, 8)[0]
        for l in open(os.path.join(ROOT, "config/mhg/c_files.txt")):
            f = l.split("#", 1)[0].split()
            if len(f) != 4 or f[0].split(":")[0] != mod:
                continue
            lo, hi = int(f[1], 16) - vram, int(f[2], 16) - vram
            if built[lo:hi] != want[lo:hi]:
                n = sum(1 for i in range(lo, hi, 4) if built[i:i + 4] != want[i:i + 4])
                i = next(i for i in range(lo, hi, 4) if built[i:i + 4] != want[i:i + 4])
                print("%-9s %-40s %3d words differ, first at 0x%08X: built %s want %s" % (
                    f[0], f[3], n, vram + i, built[i:i + 4][::-1].hex(), want[i:i + 4][::-1].hex()))
                bad.append(f[3])
    if "--skip" in sys.argv and bad:
        with open(os.path.join(ROOT, "config/mhg/c_skip.txt"), "a") as fh:
            for b in sorted(set(bad)):
                fh.write("%s  # differs from G after linking (tools/mhg_cdiff.py)\n" % b)


if __name__ == "__main__":
    main()
