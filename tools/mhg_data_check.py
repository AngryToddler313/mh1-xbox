#!/usr/bin/env python3
"""mhg_data_check.py - do MH1's data readers load Monster Hunter G's files?

Builds tools/fmt_check.c (the PC port's own readers, src/pc/fmt) into build/mhg/,
runs it over AFS_DATA/AFS00/AFS01 of disc/mh1 and disc/mhg, and compares by
file name: byte-identical, same reader statistics, different, only in one game.
Also checks G's combined collision files (lwgNNN.bin) against MH1's lgNNN/lwNNN.

Prints a summary (counts only). Usage: python3 tools/mhg_data_check.py
"""
import collections
import hashlib
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
from clay_dump import melt  # noqa: E402
from afs_extract import read_table  # noqa: E402

OUT = os.path.join(ROOT, "build/mhg")
SRC = ["afs", "melt", "amo", "apx", "ahi", "aan", "hits", "snd"]


def afs(path):
    """[(name, raw bytes)]"""
    f = open(path, "rb")
    entries, names = read_table(f, os.path.getsize(path))
    out = []
    for (o, s), nm in zip(entries, names):
        f.seek(o)
        out.append((nm, f.read(s)))
    return out


def run_check(game, arc):
    exe = os.path.join(OUT, "fmt_check")
    if not os.path.exists(exe):
        os.makedirs(OUT, exist_ok=True)
        subprocess.check_call(["gcc", "-O1", "-Isrc/pc/fmt", "tools/fmt_check.c"] +
                              ["src/pc/fmt/%s.c" % s for s in SRC] + ["-lm", "-o", exe], cwd=ROOT)
    p = subprocess.run([exe, os.path.join(ROOT, "disc", game, arc + ".AFS")], capture_output=True, text=True)
    res = {}
    for l in p.stdout.splitlines():
        f = l.split(" ", 3)
        if len(f) >= 4:
            res[f[1]] = f[3]
    return res


def kind(name):
    return re.sub(r"\d+", "#", name)


def main():
    for arc in ("AFS_DATA", "AFS00", "AFS01"):
        m1 = dict(afs(os.path.join(ROOT, "disc/mh1", arc + ".AFS")))
        g = dict(afs(os.path.join(ROOT, "disc/mhg", arc + ".AFS")))
        r1, rg = run_check("mh1", arc), run_check("mhg", arc)
        print("\n## %s: MH1 %d files, G %d files" % (arc, len(m1), len(g)))
        print("| kind | both | identical bytes | same reader stats | differ | G loads | G only (loads) | MH1 only |")
        print("|---|---|---|---|---|---|---|---|")
        kinds = collections.defaultdict(lambda: collections.Counter())
        for n in set(m1) | set(g):
            k = kinds[kind(n)]
            if n in m1 and n in g:
                k["both"] += 1
                if m1[n] == g[n]:
                    k["same"] += 1
                elif r1.get(n) == rg.get(n):
                    k["stats"] += 1
                else:
                    k["diff"] += 1
                if " ok" in " " + rg.get(n, "FAIL"):
                    k["gok"] += 1
            elif n in g:
                k["gonly"] += 1
                if " ok" in " " + rg.get(n, "FAIL"):
                    k["gonly_ok"] += 1
            else:
                k["m1only"] += 1
        tot = collections.Counter()
        for kk in sorted(kinds, key=lambda x: -sum(kinds[x].values())):
            c = kinds[kk]
            tot.update(c)
            if sum(c.values()) < 3 and arc == "AFS_DATA":
                continue
            print("| %s | %d | %d | %d | %d | %d | %d (%d) | %d |" % (kk, c["both"], c["same"], c["stats"], c["diff"],
                                                                    c["gok"], c["gonly"], c["gonly_ok"], c["m1only"]))
        c = tot
        print("| **all** | %d | %d | %d | %d | %d | %d (%d) | %d |" % (c["both"], c["same"], c["stats"], c["diff"],
                                                                     c["gok"], c["gonly"], c["gonly_ok"], c["m1only"]))
        fails = [n for n, v in rg.items() if "FAIL" in v]
        print("G files a reader rejects: %d %s" % (len(fails), " ".join(sorted(fails)[:20])))
        if arc == "AFS_DATA":
            # lwgNNN.bin = link file {HITS, HITS}: compare with MH1 lgNNN / lwNNN
            same = cnt = 0
            order = collections.Counter()
            for n in sorted(g):
                mm = re.match(r"lwg(\d+)\.bin$", n)
                if not mm:
                    continue
                d = melt(g[n])
                k = struct.unpack_from("<I", d, 0)[0]
                parts = [d[struct.unpack_from("<I", d, 4 + 8 * i)[0]:][:struct.unpack_from("<I", d, 8 + 8 * i)[0]]
                         for i in range(k)]
                lg, lw = "lg%s.bin" % mm.group(1), "lw%s.bin" % mm.group(1)
                if lg in m1 and lw in m1:
                    cnt += 1
                    a, b = melt(m1[lg]), melt(m1[lw])
                    if parts == [a, b]:
                        same += 1
                        order["ground,wall"] += 1
                    elif parts == [b, a]:
                        same += 1
                        order["wall,ground"] += 1
            print("lwgNNN.bin: %d have MH1 lgNNN+lwNNN; %d hold exactly those two files (%s)" % (cnt, same, dict(order)))


if __name__ == "__main__":
    main()
