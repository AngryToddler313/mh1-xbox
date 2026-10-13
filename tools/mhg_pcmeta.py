#!/usr/bin/env python3
"""mhg_pcmeta.py - what the PC port needs to import PS2 Monster Hunter G's data the way it
imports MH1's (docs/mhg.md section 4, docs/pc.md "Port runtime").

MH1's SLPM_654.95 keeps its symbol table and link relocations; the PC runtime
(src/pc/rt/rt_mem.c, rt_data.c) uses them to name pointer targets and to turn every
pointer word in Capcom's data into a host pointer. G's SLPM_658.69 is stripped, so this
tool makes both from what we know:

  pointer words  config/mhg/pc_ptrs.txt (committed: addresses only, no game bytes):
                 (1) MH1's R_MIPS_32 words carried over through data objects that have
                     the same name in G (config/mhg/symbols, tools/mhg_map.py), where
                     G's word there is 0 or an address that resolves;
                 (2) a scan of G's data for words that hold an address that starts a
                     known G symbol (function or data) or points into a sized data
                     object, in a module the word's module can see.
                 `--eval` runs (2) on MH1 against its real relocations (precision/recall).
  symbols        build/mhg/pc/symbols/*.txt: G's symbol files in the MH1 format with
                 data sizes (MH1's size of the same name, cut at the next G symbol),
                 sub_main folded into main (resident). tools/build_pc.sh (VERSION=mhg)
                 points its config/symbols at this directory.

Usage: python3 tools/mhg_pcmeta.py [--eval]
"""
import bisect
import collections
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mhg_map as M  # noqa: E402

P = M.P
LOOSE = os.environ.get("LOOSE") == "1"
PAT = re.compile(r"^(\S+) = 0x([0-9A-Fa-f]+);(?: // (.*))?$")
G_MODS = ["main", "sub_main", "select", "game", "yn", "lobby", "nethttp"]
# which modules a pointer in module X may point into (overlays share a vram)
SEES = {"main": ("main", "sub_main", "game"), "sub_main": ("main", "sub_main", "game"),
        "game": ("main", "sub_main", "game"), "select": ("main", "sub_main", "select"),
        "yn": ("main", "sub_main", "yn"), "lobby": ("main", "sub_main", "lobby", "nethttp"),
        "nethttp": ("main", "sub_main", "lobby", "nethttp")}


def read_syms(path):
    out = []
    for l in open(path):
        m = PAT.match(l.strip())
        if not m:
            continue
        attrs = m.group(3) or ""
        sz = re.search(r"size:0x([0-9A-Fa-f]+)", attrs)
        out.append((int(m.group(2), 16), int(sz.group(1), 16) if sz else 0, m.group(1), "type:func" in attrs))
    out.sort()
    return out


class SymIndex:
    """sorted symbols of one module: start set, and 'inside a sized data object'"""

    def __init__(self, syms, mod_=None, dlo=0):
        self.mod_, self.dlo = mod_, dlo
        self.syms = syms
        self.starts = set(a for a, _s, _n, _f in syms)
        self.addrs = [a for a, _s, _n, _f in syms]

    def str_start(self, v):
        """v starts a string: a NUL before it, then two printable / Shift-JIS bytes"""
        d, o = self.mod_.data, v - self.mod_.vram
        if self.mod_ is None or o < 1 or o + 2 > len(d) or d[o - 1] != 0:
            return False
        return all(0x20 <= c < 0x7F or 0x81 <= c <= 0xFC for c in d[o:o + 2])

    def resolves(self, v, init_hi):
        """v starts a symbol, or points into a sized data object below init_hi (not .bss)"""
        if v in self.starts:
            return True
        if v >= init_hi:
            return False
        if LOOSE and self.dlo <= v and (v & 3 == 0 or self.mod_.data[v - self.mod_.vram - 1] == 0):
            return True
        if self.dlo <= v and self.str_start(v) and self.mod_.data[v - self.mod_.vram + 2] != 0:
            return True         # an unnamed string (3+ characters)
        if v & 3 and not self.str_start(v):
            return False
        k = bisect.bisect_right(self.addrs, v) - 1
        while k >= 0 and self.addrs[k] > v - 0x10000:
            a, sz, _n, isf = self.syms[k]
            if not isf and sz and a <= v < a + sz:
                return True
            k -= 1
        return False


def data_ranges(mods, which):
    """{module: (lo, hi)} of initialised data (where pointer words can be)"""
    out = {}
    for m in which:
        x = mods[m]
        if m == "main":
            out[m] = (x.text_hi, x.vram + len(x.data))
        else:
            out[m] = (x.text_hi, x.vram + len(x.data))
    return out


def mod_range(mods, m):
    x = mods[m]
    return x.vram, x.end


def asciiish(x, a, lo, hi):
    """the word is inside a string: four printable bytes, or the end of a string whose
    previous word is four printable bytes"""
    def full(w):
        return all(0x20 <= c < 0x7F for c in w.to_bytes(4, "little"))
    w = x.word(a)
    if full(w):
        return True
    if a - 4 >= lo and full(x.word(a - 4)):
        b = w.to_bytes(4, "little")
        k = b.index(0) if 0 in b else 4
        return k > 0 and all(0x20 <= c < 0x7F for c in b[:k])
    return False


def scan(mods, idx, dr, sees):
    """words in data that hold an address resolving to a known symbol of a visible module"""
    found = collections.defaultdict(set)
    for m, (lo, hi) in dr.items():
        x = mods[m]
        if m == "main":
            # main.bin of MH1 starts at 0x100000; G's main too
            pass
        for a in range((lo + 3) & ~3, hi - 3, 4):
            v = x.word(a)
            if v < 0x100000 or v >= 0x01000000:
                continue
            if asciiish(x, a, lo, hi):
                continue
            for t in sees[m]:
                if t not in mods:
                    continue
                r0, r1 = mod_range(mods, t)
                if r0 <= v < r1 and idx[t].resolves(v, mods[t].vram + len(mods[t].data)):
                    found[m].add(a)
                    break
    return found


def eval_mh1():
    m1 = M.load_mh1()
    syms, rel = M.read_mh1_elf()
    _names, lists = M.read_split_names()
    idx = {m: SymIndex(lists[m], m1[m], 0x2E5EA0 if m == "main" else m1[m].text_hi) for m in lists}
    sees = {"main": ("main", "game"), "game": ("main", "game"), "select": ("main", "select"),
            "yn": ("main", "yn"), "lobby": ("main", "lobby")}
    dr = data_ranges(m1, list(lists))
    dr["main"] = (0x2E5EA0, m1["main"].vram + len(m1["main"].data))
    found = scan(m1, idx, dr, sees)
    for m in lists:
        lo, hi = dr[m]
        real = set(a for a, (t, _i) in rel[m].items() if t == M.R32 and lo <= a < hi)
        f = found[m]
        tp = len(f & real)
        print("%-7s real %6d found %6d  true %6d  precision %.4f  recall %.4f" % (
            m, len(real), len(f), tp, tp / max(len(f), 1), tp / max(len(real), 1)))
        fp = sorted(f - real)[:8]
        fn = sorted(real - f)[:8]
        print("   false+ %s" % " ".join("%X=%X" % (a, m1[m].word(a)) for a in fp))
        print("   missed %s" % " ".join("%X=%X" % (a, m1[m].word(a)) for a in fn))


def main():
    if "--eval" in sys.argv:
        eval_mh1()
        return
    G = M.load_g()
    gsyms = {m: read_syms(P("config/mhg/symbols/%s.txt" % m)) for m in G_MODS}
    _names, lists = M.read_split_names()
    m1size = {}
    for m, lst in lists.items():
        for a, sz, n, isf in lst:
            if sz:
                m1size.setdefault(n, sz)
    # sizes: MH1's size of the same name, cut at the next G symbol of the module
    sized = {}
    for m in G_MODS:
        lst = gsyms[m]
        out = []
        for k, (a, sz, n, isf) in enumerate(lst):
            nxt = lst[k + 1][0] if k + 1 < len(lst) else G[m].end
            if not sz:
                sz = m1size.get(n, 0)
                if sz and a + sz > nxt:
                    sz = nxt - a
            out.append((a, sz, n, isf))
        sized[m] = out
    idx = {m: SymIndex(sized[m], G[m], 0x253D20 if m == "main" else G[m].text_hi) for m in G_MODS}
    dr = data_ranges(G, G_MODS)
    dr["main"] = (0x253D20, G["main"].vram + len(G["main"].data))
    found = scan(G, idx, dr, SEES)
    # carried over from MH1's relocations through same-name data objects
    syms, rel = M.read_mh1_elf()
    gpos = {}
    for m in G_MODS:
        for a, sz, n, isf in sized[m]:
            if not isf:
                gpos[n] = (m, a, sz)
    carried = collections.defaultdict(set)
    fvotes_src = []
    covered = collections.defaultdict(list)      # G objects whose pointer words come from MH1
    n_obj = n_bad = 0
    for mm, lst in lists.items():
        r32 = sorted(a for a, (t, _i) in rel[mm].items() if t == M.R32)
        for a, sz, n, isf in lst:
            if isf or not sz or n not in gpos:
                continue
            gm, ga, gsz = gpos[n]
            lo, hi = dr[gm]
            if gsz != sz or ga & 3 != a & 3 or not (lo <= ga and ga + gsz <= hi):
                continue
            k = bisect.bisect_left(r32, a)
            pos, ok = [], True
            while k < len(r32) and r32[k] < a + sz:
                off = r32[k] - a
                k += 1
                v = G[gm].word(ga + off)
                if v and not any(t in G and mod_range(G, t)[0] <= v < mod_range(G, t)[1] for t in SEES[gm]):
                    ok = False      # G's layout differs here: leave the object to the scan
                    break
                pos.append(ga + off)
                fvotes_src.append((mm, r32[k - 1], gm, ga + off))
            if not ok:
                n_bad += 1
                continue
            n_obj += 1
            carried[gm].update(pos)
            covered[gm].append((ga, ga + gsz))
    print("data objects with MH1's pointer layout: %d (%d left to the scan: a word is no address)" % (n_obj, n_bad))
    for m in G_MODS:
        cv = sorted(covered[m])
        starts = [c[0] for c in cv]
        keep = set()
        for a in found[m]:
            k = bisect.bisect_right(starts, a) - 1
            if k >= 0 and cv[k][0] <= a < cv[k][1]:
                continue
            keep.add(a)
        found[m] = keep
    tot = collections.Counter()
    with open(P("config/mhg/pc_ptrs.txt"), "w") as fh:
        fh.write("# Generated by tools/mhg_pcmeta.py: PS2 Monster Hunter G data words that hold a pointer\n"
                 "# (G's ELF has no relocations). Addresses only. One module per section.\n")
        for m in G_MODS:
            s = found[m] | carried[m]
            tot["scan"] += len(found[m])
            tot["carried"] += len(carried[m])
            tot["both"] += len(found[m] & carried[m])
            fh.write("[%s] %d\n" % (m, len(s)))
            for a in sorted(s):
                fh.write("%X\n" % a)
    print("pointer words: scan %d, carried from MH1 %d (both %d), total %d" % (
        tot["scan"], tot["carried"], tot["both"], tot["scan"] + tot["carried"] - tot["both"]))
    # unnamed G functions (G-only by tools/mhg_map.py, e.g. one-instruction dummies it could not
    # pair) that a carried pointer word names: the MH1 function at the same table position
    m1 = M.load_mh1()
    m1func = {}
    for mm, lst in lists.items():
        for a, sz, n, isf in lst:
            if isf:
                m1func.setdefault(a, n)
    gidx = {}
    for m in G_MODS:
        for k, (a, sz, n, isf) in enumerate(sized[m]):
            if isf:
                gidx[a] = (m, k)
    used = set(n for m in G_MODS for _a, _s, n, _f in sized[m])
    votes = collections.defaultdict(collections.Counter)
    for mm, a1, gm, g in fvotes_src:
        v1, v2 = m1[mm].word(a1), G[gm].word(g)
        n1 = m1func.get(v1)
        if n1 and v2 in gidx:
            m, k = gidx[v2]
            if re.match(r"^(\w+_)?func_[0-9A-F]{8}$", sized[m][k][2]):
                votes[v2][n1] += 1
    import csv
    weak = set(r["name"] for r in csv.DictReader(open(P("docs/survey/mhg_map.csv")))
               if r["class"] in ("edited", "rewritten") and r["name"])
    where = {}
    for m in G_MODS:
        for k, (a, sz, n, isf) in enumerate(sized[m]):
            where[n] = (m, k)
    nren = 0
    for v2, c in votes.items():
        (n1, cnt), *rest = c.most_common()
        if rest:
            continue
        if n1 in used:
            # a pointer table outvotes an edited pairing by similarity (pl01_effect_move:
            # mhg_map paired it with 0x4A9150 at 0.56, pl01_adr_tbl points at 0x4E2EB0)
            if n1 not in weak or n1 not in where:
                continue
            om, ok_ = where[n1]
            oa, osz, _on, oisf = sized[om][ok_]
            sized[om][ok_] = (oa, osz, ("func_%08X" if om == "main" else om + "_func_%08X") % oa, oisf)
        m, k = gidx[v2]
        a, sz, _n, isf = sized[m][k]
        sized[m][k] = (a, sz, n1, isf)
        used.add(n1)
        nren += 1
    print("G functions named from pointer tables: %d" % nren)
    # symbol view for the PC build
    os.makedirs(P("build/mhg/pc/symbols"), exist_ok=True)
    view = {"main": sized["main"] + sized["sub_main"], "game": sized["game"], "lobby": sized["lobby"] + sized["nethttp"],
            "select": sized["select"], "yn": sized["yn"]}
    for m, lst in view.items():
        with open(P("build/mhg/pc/symbols/%s.txt" % m), "w") as fh:
            fh.write("// Generated by tools/mhg_pcmeta.py from config/mhg/symbols (sizes from MH1).\n")
            for a, sz, n, isf in sorted(lst):
                attrs = (["type:func"] if isf else []) + (["size:0x%X" % sz] if sz else [])
                fh.write("%s = 0x%08X;%s\n" % (n, a, (" // " + " ".join(attrs)) if attrs else ""))
    print("wrote config/mhg/pc_ptrs.txt, build/mhg/pc/symbols/*.txt")


if __name__ == "__main__":
    main()
