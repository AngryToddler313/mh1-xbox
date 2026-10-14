#!/usr/bin/env python3
"""mhg_cfiles.py - list the matched MH1 C files that should build byte-identical in G.

A file of config/c_files.txt qualifies when every MH1 function in its range has a
clean identical twin in G (docs/survey/mhg_map.csv, column clean: same instructions
except relocated fields, and every call goes to the callee's twin), all twins sit in
one G module with the same spacing as in MH1, and none of its functions is kept as
original bytes (config/c_rawfuncs.txt). Its data slots (MODULE:SECTION lines) are
moved to G through the symbol at the slot's start (config/mhg/symbols, aliases).

Writes config/mhg/c_files.txt (NAME relative to src/, so the MH1 file is shared).
Files listed in config/mhg/c_skip.txt (one src-relative name per line, with the reason)
are left out: they were tried and do not link or match in G.

Usage: python3 tools/mhg_cfiles.py [--only NAME ...]
"""
import collections
import csv
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HAND = "# --- G files (src/mhg copies or #ifdef VERSION_MHG), added by hand or tools/mhg_addc.py: kept when this tool reruns"


def P(*a):
    return os.path.join(ROOT, *a)


def mh1_names():
    out = collections.defaultdict(dict)
    for mod in ("main", "select", "game", "yn", "lobby"):
        for l in open(P("config/symbols/%s.txt" % mod)):
            m = re.match(r"^(\S+) = 0x([0-9A-F]+);", l)
            if m:
                out[mod].setdefault(int(m.group(2), 16), m.group(1))
    return out


def g_addrs():
    out = {}
    for f in os.listdir(P("config/mhg/symbols")):
        for l in open(P("config/mhg/symbols", f)):
            m = re.match(r"^(\S+) = 0x([0-9A-F]+);", l)
            if m:
                out[m.group(1)] = (f[:-4], int(m.group(2), 16))
    for f in os.listdir(P("config/mhg")):
        if f.endswith("_aliases.txt"):
            for l in open(P("config/mhg", f)):
                m = re.match(r"^(\S+) = 0x([0-9A-F]+);", l)
                if m and m.group(1) not in out:
                    out[m.group(1)] = (None, int(m.group(2), 16))
    return out


def main():
    only = set(sys.argv[sys.argv.index("--only") + 1:]) if "--only" in sys.argv else None
    rows = [r for r in csv.DictReader(open(P("docs/survey/mhg_map.csv"))) if r["mh1_module"]]
    by = collections.defaultdict(list)
    for r in rows:
        by[r["mh1_module"]].append((int(r["mh1_addr"], 16), r))
    for v in by.values():
        v.sort(key=lambda t: t[0])
    raw = set()
    for l in open(P("config/c_rawfuncs.txt")):
        f = l.split("#", 1)[0].split()
        if len(f) >= 4:
            raw.add((f[0], int(f[1], 16)))
    skip = set()
    if os.path.exists(P("config/mhg/c_skip.txt")):
        for l in open(P("config/mhg/c_skip.txt")):
            f = l.split("#", 1)[0].split()
            if f:
                skip.add(f[0])
    texts, slots = [], collections.defaultdict(list)
    for l in open(P("config/c_files.txt")):
        f = l.split("#", 1)[0].split()
        if len(f) != 4:
            continue
        if ":" in f[0]:
            mod, sec = f[0].split(":")
            slots[(mod, f[3])].append((sec, int(f[1], 16), int(f[2], 16)))
        else:
            texts.append((f[0], int(f[1], 16), int(f[2], 16), f[3]))
    names, gsym = mh1_names(), g_addrs()
    out, why = [], collections.Counter()
    for mod, lo, hi, name in texts:
        path = "%s/%s" % (mod, name)
        if only is not None and path not in only:
            continue
        if path in skip:
            why["skipped (c_skip.txt)"] += 1
            continue
        fs = [r for a, r in by[mod] if lo <= a < hi]
        if not fs:
            why["no functions"] += 1
            continue
        if any((mod, int(r["mh1_addr"], 16)) in raw for r in fs):
            why["has raw functions"] += 1
            continue
        if not all(r["clean"] == "1" for r in fs):
            why["not all clean twins"] += 1
            continue
        gm = fs[0]["g_module"]
        g0 = int(fs[0]["g_addr"], 16)
        if not all(r["g_module"] == gm and int(r["g_addr"], 16) - g0 == int(r["mh1_addr"], 16) - lo for r in fs):
            why["layout differs"] += 1
            continue
        lines = ["%s 0x%08X 0x%08X %s" % (gm, g0, g0 + hi - lo, path)]
        ok = True
        for sec, slo, shi in slots.get((mod, name), []):
            nm = names[mod].get(slo) or names["main"].get(slo)
            if nm is None or nm not in gsym:
                ok = False
                break
            smod, ga = gsym[nm]
            lines.append("%s:%s 0x%08X 0x%08X %s" % (smod or gm, sec, ga, ga + shi - slo, path))
        if not ok:
            why["data slot not found in G"] += 1
            continue
        why["registered"] += 1
        out += lines
    keep = []
    if os.path.exists(P("config/mhg/c_files.txt")):
        txt = open(P("config/mhg/c_files.txt")).read()
        if HAND in txt:
            keep = [txt[txt.index(HAND):].rstrip("\n")]
    with open(P("config/mhg/c_files.txt"), "w") as fh:
        fh.write("# Generated by tools/mhg_cfiles.py: matched MH1 C files that rebuild PS2 Monster Hunter G\n"
                 "# unchanged. MODULE START END NAME, NAME relative to src/ (shared with the MH1 build).\n")
        fh.write("\n".join(out + keep) + "\n")
    print(dict(why))


if __name__ == "__main__":
    main()
