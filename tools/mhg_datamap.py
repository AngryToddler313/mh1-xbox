#!/usr/bin/env python3
"""mhg_datamap.py - find the PS2 Monster Hunter G address of MH1 data objects that
tools/mhg_map.py left unnamed (docs/mhg.md section 5, "Data names").

mhg_map.py names G data from relocated operands of identical/shape twins, unique byte
content and exact layout between mapped neighbours. What it leaves (tables only edited code
reads, message tables, .bss work areas, small constants) the PC build turned into function
stand-ins or zero-filled tables. This tool collects evidence for every unmapped MH1 data
object, scores it and records the method:

  ref      the same instruction in a G twin function (any class: identical/shape by
           position, edited/rewritten through an opcode alignment) that references the
           object in MH1: lui/lo (the G lui found by register scan back from the lo) or gp.
  ptr-in   an MH1 pointer word (R_MIPS_32) that points at the object, inside a data object
           G already has: the G word at the same offset (container layout checked).
  ptr-out  the object's own pointer words point at objects/functions G already has: the G
           words that hold those G addresses at the same relative offsets.
  content  the object's bytes (pointer words masked) occur in G's data (unique, or the
           one inside the window the mapped neighbours give).
  layout   between two mapped neighbours with the same MH1 and G distance (exact), or
           next to one neighbour with the bytes there agreeing (near).

Several rounds: what one round places becomes an anchor for the next. Each method is also
run on the names G already has, as a check of its precision (printed).

Output (names and addresses only, no game bytes):
  config/mhg/data_map.txt   `module name 0xADDR conf methods` (conf high / medium / low)
  stdout                    counts, precision per method, the PC's names left unmapped

tools/mhg_pcmeta.py adds the high/medium entries to the G symbol view of the PC build
(config/mhg/pc_names.txt, the hand list, still wins).

Usage: python3 tools/mhg_datamap.py [-v NAME]
"""
import bisect
import collections
import csv
import difflib
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mhg_map as M  # noqa: E402

P = M.P
G_MODS = ["main", "sub_main", "select", "game", "yn", "lobby", "nethttp"]
# G modules an MH1 module's data can have gone to / a G module's code can see
DATA_TO = {"main": ("main", "sub_main", "game"), "game": ("game", "main", "sub_main"),
           "lobby": ("lobby", "nethttp"), "select": ("select",), "yn": ("yn",)}
SEES = {"main": ("main", "sub_main"), "sub_main": ("main", "sub_main"),
        "game": ("game", "main", "sub_main"), "select": ("select", "main", "sub_main"),
        "yn": ("yn", "main", "sub_main"), "lobby": ("lobby", "nethttp", "main", "sub_main"),
        "nethttp": ("nethttp", "lobby", "main", "sub_main")}
OVL_BASE = 0x533980
PAT = re.compile(r"^(\S+) = 0x([0-9A-Fa-f]+);(?: // (.*))?$")
LOOP = {0x09, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x31, 0x39,
        0x37, 0x3F, 0x1E, 0x1F, 0x1A, 0x1B, 0x2C, 0x2D}
W = {"ref-twin": 3.0, "ref-edit": 1.0, "ptr-in": 2.0, "ptr-out": 1.0, "content": 3.0,
     "content-win": 2.0, "layout": 2.5, "layout-near": 1.0}


def sx(v):
    return ((v & 0xFFFF) ^ 0x8000) - 0x8000


def read_g_syms():
    out = {}
    for m in G_MODS:
        lst = []
        for l in open(P("config/mhg/symbols/%s.txt" % m)):
            r = PAT.match(l.strip())
            if r:
                lst.append((int(r.group(2), 16), r.group(1), "type:func" in (r.group(3) or "")))
        out[m] = lst
    return out


def g_mod_at(G, a, prefer):
    for m in prefer:
        if m in G and G[m].vram <= a < G[m].end:
            return m
    return None


def lui_back(words, j, reg, lo=0, maxback=96):
    """the upper half a `lui reg` before index j sets (None if reg is written otherwise first)"""
    k = j - 1
    while k >= lo and j - k <= maxback:
        w = words[k]
        op = w >> 26
        if op == 0xF and ((w >> 16) & 31) == reg:
            return (w & 0xFFFF) << 16
        # another write to reg (common ALU / load forms) ends the search
        if op == 0 and ((w >> 11) & 31) == reg and (w & 0x3F) not in (8, 0x18, 0x19, 0x1A, 0x1B) and w:
            return None
        if op in (8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0x18, 0x19, 0x20, 0x21, 0x23, 0x24, 0x25, 0x27, 0x37,
                  0x1A, 0x1B, 0x1E) and ((w >> 16) & 31) == reg:
            return None
        k -= 1
    return None


class Mapper:
    def __init__(self):
        self.M1, self.G = M.load_mh1(), M.load_g()
        self.syms, self.rel = M.read_mh1_elf()
        _n, self.lists = M.read_split_names()
        # MH1 data objects: name -> (mod, addr, size)
        self.obj = {}
        self.objlist = {}
        for mm, lst in self.lists.items():
            mlst = []
            for k, (a, sz, n, isf) in enumerate(lst):
                if isf or a < 0x100000:
                    continue
                if mm != "main" and a < OVL_BASE:
                    continue
                if not sz:
                    nxt = next((b for b, *_r in lst[k + 1:] if b > a), a + 4)
                    sz = min(nxt - a, 0x1000)
                if n in self.obj:
                    continue
                self.obj[n] = (mm, a, sz)
                mlst.append((a, sz, n))
            mlst.sort()
            self.objlist[mm] = mlst
        self.objstarts = {mm: [x[0] for x in l] for mm, l in self.objlist.items()}
        # MH1 functions by address (for pointer words that point at code)
        self.m1func = {}
        for mm, lst in self.lists.items():
            for a, sz, n, isf in lst:
                if isf:
                    self.m1func.setdefault((mm if a >= OVL_BASE else "main", a), n)
        # G names
        gs = read_g_syms()
        self.gfunc = {}                        # name -> (gm, ga)
        self.known = {}                        # data name -> (gm, ga)   (mhg_map's)
        self.gtaken = {}                       # (gm, ga) -> name
        for m, lst in gs.items():
            for a, n, isf in lst:
                if isf:
                    self.gfunc.setdefault(n, (m, a))
                elif n in self.obj:
                    self.known.setdefault(n, (m, a))
                self.gtaken.setdefault((m, a), n)
        self.base_known = dict(self.known)
        self.placed = {}                       # name -> (gm, ga, conf, methods)
        self.pairs = []
        self.sim = {}
        for r in csv.DictReader(open(P("docs/survey/mhg_map.csv"), encoding="utf-8")):
            if r["g_addr"] and r["mh1_addr"] and r["class"] in ("identical", "shape", "edited", "rewritten"):
                self.pairs.append((r["mh1_module"], int(r["mh1_addr"], 16), int(r["size"]), r["class"],
                                   r["g_module"], int(r["g_addr"], 16), int(r["g_size"] or 0)))
                self.sim[(r["mh1_module"], int(r["mh1_addr"], 16))] = float(r["sim"] or 0.5)
        r32 = collections.defaultdict(list)
        for mm, d in self.rel.items():
            for a, (t, _i) in d.items():
                if t == M.R32:
                    r32[mm].append(a)
        self.r32 = {mm: sorted(v) for mm, v in r32.items()}
        self.r32set = {mm: set(v) for mm, v in r32.items()}

    # ------------------------------------------------------------ helpers
    def m1_obj_at(self, mods, a):
        """(name, offset) of the MH1 data object holding address a"""
        for mm in mods:
            if mm not in self.objlist:
                continue
            if mm != "main" and a < OVL_BASE:
                continue
            if mm == "main" and a >= OVL_BASE:
                continue
            st = self.objstarts[mm]
            k = bisect.bisect_right(st, a) - 1
            while k >= 0 and st[k] > a - 0x40000:
                oa, sz, n = self.objlist[mm][k]
                if oa <= a < oa + max(sz, 1):
                    return n, a - oa
                if oa < a:
                    break
                k -= 1
        return None, 0

    def g_of_name(self, n):
        if n in self.known:
            return self.known[n]
        if n in self.gfunc:
            return self.gfunc[n]
        return None

    def g_data(self, gm):
        x = self.G[gm]
        lo = 0x253D20 if gm == "main" else x.text_hi
        return lo, x.vram + len(x.data)

    def g_word(self, gm, a):
        x = self.G[gm]
        if a & 3 or not (x.vram <= a < x.vram + len(x.data) - 3):
            return None
        return x.word(a)

    # ------------------------------------------------------------ methods
    def m_refs(self):
        """votes[name][(gm, ga)] from MH1 code references, through the twin's instruction"""
        votes = collections.defaultdict(collections.Counter)
        M1, G = self.M1, self.G
        for mm, ma, msz, cls, gm, ga, gsz in self.pairs:
            if msz < 8 or not gsz:
                continue
            rl = self.rel[mm]
            a1 = M1[mm].words(ma, msz // 4)
            a2 = G[gm].words(ga, gsz // 4)
            twin = cls in ("identical", "shape") and len(a2) >= len(a1)
            if not twin:
                continue
            jmap = {i: i for i in range(len(a1))}
            w = W["ref-twin"]
            for i in range(len(a1)):
                r = rl.get(ma + 4 * i)
                if not r or r[0] not in (M.LO16, M.GPREL) or i not in jmap:
                    continue
                j = jmap[i]
                x1, x2 = a1[i], a2[j]
                if (x1 >> 26) != (x2 >> 26):
                    continue
                rs1, rs2 = (x1 >> 21) & 31, (x2 >> 21) & 31
                # either side may use gp or lui/lo (G moved small objects out of .sdata)
                if rs1 == 28:
                    tg1 = M.MH1_GP + sx(x1)
                else:
                    h1 = lui_back(a1, i, rs1)
                    if h1 is None:
                        continue
                    tg1 = (h1 + sx(x1)) & 0xFFFFFFFF
                if rs2 == 28:
                    tg2 = M.G_GP + sx(x2)
                else:
                    h2 = lui_back(a2, j, rs2)
                    if h2 is None:
                        continue
                    tg2 = (h2 + sx(x2)) & 0xFFFFFFFF
                n, off = self.m1_obj_at((mm, "main") if tg1 >= OVL_BASE else ("main",), tg1)
                if not n:
                    continue
                gt = (tg2 - off) & 0xFFFFFFFF
                g = g_mod_at(G, gt, SEES[gm])
                if not g or (gt & 3) != (self.obj[n][1] & 3):
                    continue
                # offset 0 references weigh most (G may have moved fields inside the object)
                votes[n][(g, gt)] += w * (1.0 if off == 0 else 0.5)
        return votes

    def code_refs(self, words, gp, rel=None, base=0):
        """[(index, target, opcode)] of the lo/gp data references in a function"""
        out = []
        for i, w in enumerate(words):
            op = w >> 26
            if op not in LOOP:
                continue
            if rel is not None:
                r = rel.get(base + 4 * i)
                if not r or r[0] not in (M.LO16, M.GPREL):
                    continue
            rs = (w >> 21) & 31
            if rs == 28:
                t = gp + sx(w)
            else:
                if rs == 0:
                    continue
                h = lui_back(words, i, rs)
                if h is None:
                    continue
                t = (h + sx(w)) & 0xFFFFFFFF
            out.append((i, t, op))
        return out

    def g_known_index(self):
        """sorted G (address, end) of every placed data object, per G module"""
        idx = collections.defaultdict(list)
        for n, (gm, ga) in self.known.items():
            idx[gm].append((ga, ga + max(self.obj[n][2], 1), n))
        for gm in idx:
            idx[gm].sort()
        self.gk = idx
        self.gk_starts = {gm: [x[0] for x in l] for gm, l in idx.items()}

    def g_known_at(self, gm, a):
        st = self.gk_starts.get(gm)
        if not st:
            return None
        k = bisect.bisect_right(st, a) - 1
        if k >= 0 and self.gk[gm][k][0] <= a < self.gk[gm][k][1]:
            return self.gk[gm][k][2]
        return None

    def m_refseq(self):
        """edited/rewritten pairs: align the two functions' sequences of data references
        (references to names G already has must land on the same G object; the others pair
        up in order between them) and vote for the unplaced names"""
        votes = collections.defaultdict(collections.Counter)
        self.g_known_index()
        for mm, ma, msz, cls, gm, ga, gsz in self.pairs:
            if cls in ("identical", "shape") or msz < 8 or not gsz:
                continue
            sim = self.sim.get((mm, ma), 0.5)
            r1 = self.code_refs(self.M1[mm].words(ma, msz // 4), M.MH1_GP, self.rel[mm], ma)
            r2 = self.code_refs(self.G[gm].words(ga, gsz // 4), M.G_GP)
            if not r1 or not r2 or len(r1) * len(r2) > 400000:
                continue
            e1 = []
            for i, t, op in r1:
                n, off = self.m1_obj_at((mm, "main") if t >= OVL_BASE else ("main",), t)
                exp = None
                if n and n in self.known:
                    exp = self.known[n][1] + off
                e1.append((n, off, exp, op))
            e2 = []
            for j, t, op in r2:
                g = g_mod_at(self.G, t, SEES[gm])
                e2.append((t, op, self.g_known_at(g, t) if g else None, g))
            n1, n2 = len(e1), len(e2)
            NEG = -1e9
            # DP (Needleman-Wunsch, gaps free)
            S = [[0.0] * (n2 + 1) for _ in range(n1 + 1)]
            B = [[0] * (n2 + 1) for _ in range(n1 + 1)]
            for x in range(1, n1 + 1):
                n, off, exp, op1 = e1[x - 1]
                rowp, row, brow = S[x - 1], S[x], B[x]
                for y in range(1, n2 + 1):
                    t, op2, kn, g = e2[y - 1]
                    if not n or g is None:
                        sc = NEG
                    elif exp is not None:
                        sc = 4.0 if t == exp else NEG
                    elif kn is not None:
                        sc = NEG
                    else:
                        sc = 1.0 + (0.5 if op1 == op2 else 0) + (0.5 if (t & 3) == (self.obj[n][1] + off) & 3 else -2)
                    best, b = rowp[y - 1] + sc, 0
                    if rowp[y] > best:
                        best, b = rowp[y], 1
                    if row[y - 1] > best:
                        best, b = row[y - 1], 2
                    row[y], brow[y] = best, b
            x, y = n1, n2
            pairs = []
            while x > 0 and y > 0:
                b = B[x][y]
                if b == 0:
                    pairs.append((x - 1, y - 1))
                    x, y = x - 1, y - 1
                elif b == 1:
                    x -= 1
                else:
                    y -= 1
            pairs.reverse()
            anch = [k for k, (x_, y_) in enumerate(pairs) if e1[x_][2] is not None]
            for k, (x_, y_) in enumerate(pairs):
                n, off, exp, _op = e1[x_]
                if exp is not None:
                    continue
                t, _op2, _kn, g = e2[y_]
                gt = (t - off) & 0xFFFFFFFF
                if (gt & 3) != (self.obj[n][1] & 3):
                    continue
                lo = any(a_ < k for a_ in anch)
                hi = any(a_ > k for a_ in anch)
                w = W["ref-edit"] * (2 * sim) * (1.0 if (lo and hi) else 0.6 if (lo or hi) else 0.35)
                votes[n][(g, gt)] += w * (1.0 if off == 0 else 0.5)
        return votes

    def container_ok(self, n, gm, ga):
        """MH1's pointer words of object n are addresses (or 0) at the same offsets in G"""
        mm, a, sz = self.obj[n]
        lst = self.r32.get(mm, [])
        k = bisect.bisect_left(lst, a)
        tot = good = 0
        while k < len(lst) and lst[k] < a + sz:
            v = self.g_word(gm, ga + lst[k] - a)
            tot += 1
            if v is not None and (v == 0 or g_mod_at(self.G, v, SEES[gm])):
                good += 1
            k += 1
        return tot, good

    def m_ptr_in(self):
        votes = collections.defaultdict(collections.Counter)
        for mm, lst in self.r32.items():
            for p in lst:
                cn, poff = self.m1_obj_at((mm,), p)
                if not cn or cn not in self.known:
                    continue
                t = self.M1[mm].word(p)
                n, off = self.m1_obj_at((mm, "main") if t >= OVL_BASE else ("main",), t)
                if not n:
                    continue
                gm, gc = self.known[cn]
                csz = self.obj[cn][2]
                tot, good = self.container_ok(cn, gm, gc)
                if tot == 0 or good < tot * 0.9:
                    continue
                v = self.g_word(gm, gc + poff)
                if not v:
                    continue
                gt = v - off
                g = g_mod_at(self.G, gt, SEES[gm])
                if not g:
                    continue
                votes[n][(g, gt)] += W["ptr-in"] * (1.0 if csz <= 0x400 else 0.7)
        return votes

    def build_gword_index(self):
        idx = collections.defaultdict(list)
        for gm in G_MODS:
            lo, hi = self.g_data(gm)
            x = self.G[gm]
            for a in range((lo + 3) & ~3, hi - 3, 4):
                v = x.word(a)
                if 0x100000 <= v < 0x1000000:
                    idx[v].append((gm, a))
        self.gwidx = idx

    def m_ptr_out(self, names):
        votes = collections.defaultdict(collections.Counter)
        for n in names:
            mm, a, sz = self.obj[n]
            lst = self.r32.get(mm, [])
            k = bisect.bisect_left(lst, a)
            exp = []
            while k < len(lst) and lst[k] < a + sz:
                p = lst[k]
                k += 1
                t = self.M1[mm].word(p)
                tm = (mm, "main") if t >= OVL_BASE else ("main",)
                g = None
                fn = None
                for x in tm:
                    fn = self.m1func.get((x, t))
                    if fn:
                        break
                if fn and fn in self.gfunc:
                    g = self.gfunc[fn][1]
                else:
                    tn, off = self.m1_obj_at(tm, t)
                    if tn and tn != n:
                        r = self.g_of_name(tn)
                        if r:
                            g = r[1] + off
                if g:
                    exp.append((p - a, g))
            if not exp:
                continue
            # rarest expected word first
            exp.sort(key=lambda e: len(self.gwidx.get(e[1], ())))
            o0, v0 = exp[0]
            hits = self.gwidx.get(v0, ())
            if len(hits) > 200:
                continue
            for gm, ga0 in hits:
                if gm not in DATA_TO[mm]:
                    continue
                base = ga0 - o0
                cnt = sum(1 for o, v in exp if self.g_word(gm, base + o) == v)
                need = 2 if len(exp) >= 2 else 1
                if cnt >= need and cnt >= 0.6 * len(exp):
                    votes[n][(gm, base)] += W["ptr-out"] * min(cnt, 4)
        return votes

    def masked(self, n):
        mm, a, sz = self.obj[n]
        x = self.M1[mm]
        if a + sz > x.vram + len(x.data):
            return None, None
        b = x.data[a - x.vram:a - x.vram + sz]
        msk = [False] * sz
        rs = self.r32set.get(mm, set())
        for k in range(a & ~3, a + sz, 4):
            if k in rs:
                for j in range(max(k, a), min(k + 4, a + sz)):
                    msk[j - a] = True
        return b, msk

    def similarity(self, n, gm, ga):
        """fraction of MH1's unmasked bytes equal at ga in G (None: no initialised bytes)"""
        b, msk = self.masked(n)
        if b is None:
            return None
        x = self.G[gm]
        o = ga - x.vram
        if o < 0 or o + len(b) > len(x.data):
            return None
        g = x.data[o:o + len(b)]
        cmp = [i for i in range(len(b)) if not msk[i]]
        if not cmp or not any(b[i] for i in cmp):
            return None
        return sum(1 for i in cmp if b[i] == g[i]) / len(cmp)

    def neighbours(self, n):
        """nearest MH1 objects before/after n that G has (same MH1 module): (m1a, gm, ga)"""
        mm, a, sz = self.obj[n]
        lst = self.objlist[mm]
        k = bisect.bisect_left(self.objstarts[mm], a)
        before = after = None
        i = k - 1
        while i >= 0 and k - i < 400:
            on = lst[i][2]
            if on != n and on in self.known:
                before = (lst[i][0],) + self.known[on]
                break
            i -= 1
        i = k + 1
        while i < len(lst) and i - k < 400:
            on = lst[i][2]
            if on != n and on in self.known:
                after = (lst[i][0],) + self.known[on]
                break
            i += 1
        return before, after

    def m_content_layout(self, names):
        lay = collections.defaultdict(collections.Counter)
        votes = collections.defaultdict(collections.Counter)
        for n in names:
            mm, a, sz = self.obj[n]
            before, after = self.neighbours(n)
            # layout
            if before and after and before[1] == after[1] and after[0] - before[0] == after[2] - before[2] \
                    and after[0] - before[0] <= 0x40000:
                lay[n][(before[1], before[2] + a - before[0])] += W["layout"]
            else:
                for nb in (before, after):
                    if not nb or abs(a - nb[0]) > 0x2000:
                        continue
                    ga = nb[2] + a - nb[0]
                    s = self.similarity(n, nb[1], ga)
                    if s is not None and s >= 0.75 and sz >= 8:
                        lay[n][(nb[1], ga)] += W["layout-near"] * (1 + s)
            # content
            b, msk = self.masked(n)
            if b is None:
                continue
            k0 = b.find(0)
            isstr = 2 <= k0 and not any(msk) and not any(b[k0:]) and \
                all(0x20 <= c < 0x7F or 0x81 <= c <= 0xFC or c in (9, 10) for c in b[:k0])
            if isstr:
                best = (0, k0 + 1)
            else:
                if sz < 8:
                    continue
                best, i = (0, 0), 0
                while i < sz:
                    if msk[i]:
                        i += 1
                        continue
                    j = i
                    while j < sz and not msk[j]:
                        j += 1
                    if j - i > best[1] - best[0] and len(set(b[i:j])) > 1:
                        best = (i, j)
                    i = j
                if best[1] - best[0] < 8:
                    continue
            run = bytes(b[best[0]:best[1]])
            cmpn = best[1] if isstr else sz
            hits = []
            for gm in DATA_TO[mm]:
                x = self.G[gm]
                glo, ghi = self.g_data(gm)
                start = glo - x.vram
                while len(hits) < 200:
                    k = x.data.find(run, start, ghi - x.vram)
                    if k < 0:
                        break
                    start = k + 1
                    ga = x.vram + k - best[0]
                    if ga & 3 != a & 3:
                        continue
                    if isstr and x.data[k - 1] != 0:
                        continue
                    seg = x.data[ga - x.vram:ga - x.vram + cmpn]
                    if len(seg) == cmpn and all(msk[t] or seg[t] == b[t] for t in range(cmpn)):
                        hits.append((gm, ga))
            if len(hits) == 1 and not (isstr and k0 < 6):
                votes[n][hits[0]] += W["content"]
            elif 1 <= len(hits) < 200:
                # the hit inside the window the mapped neighbours give
                if before and after and before[1] == after[1]:
                    win = [h for h in hits if h[0] == before[1] and before[2] < h[1] < after[2]]
                else:
                    win = []
                    for nb in (before, after):
                        if nb:
                            pr = nb[2] + a - nb[0]
                            win += [h for h in hits if h[0] == nb[1] and abs(h[1] - pr) < 0x800]
                    win = sorted(set(win))
                if len(win) == 1:
                    votes[n][win[0]] += W["content-win"]
        return lay, votes

    # ------------------------------------------------------------ rounds
    def run(self, verbose=None):
        print("MH1 data objects: %d; with a G name from tools/mhg_map.py: %d" % (len(self.obj), len(self.known)))
        self.build_gword_index()
        ref_votes = self.m_refs()
        ptrin_static = None
        method_check = collections.defaultdict(lambda: [0, 0])
        for rnd in range(4):
            todo = [n for n in self.obj if n not in self.known]
            allv = collections.defaultdict(lambda: collections.defaultdict(collections.Counter))
            lay, con = self.m_content_layout(list(self.obj) if rnd == 0 else todo)
            for meth, v in (("ref", ref_votes), ("refseq", self.m_refseq()), ("ptr-in", self.m_ptr_in()),
                            ("ptr-out", self.m_ptr_out(list(self.obj) if rnd == 0 else todo)),
                            ("layout", lay), ("content", con)):
                for n, c in v.items():
                    for k, w in c.items():
                        allv[n][k][meth] += w
            if rnd == 0:
                # precision of each method on the names mhg_map already placed
                for n, (gm, ga) in self.base_known.items():
                    if n not in allv:
                        continue
                    per = collections.defaultdict(collections.Counter)
                    for k, ms in allv[n].items():
                        for meth, w in ms.items():
                            per[meth][k] += w
                    for meth, c in per.items():
                        top = c.most_common(1)[0][0]
                        method_check[meth][0] += 1
                        method_check[meth][1] += top == (gm, ga)
                print("method check on mhg_map's names (top candidate == mhg_map's address):")
                for meth, (t, ok) in sorted(method_check.items()):
                    print("  %-8s %5d names, agree %5d (%.1f%%)" % (meth, t, ok, 100 * ok / max(t, 1)))
                self.check = dict(method_check)
            new = []
            for n in todo:
                if n not in allv:
                    continue
                cands = []
                for k, ms in allv[n].items():
                    cands.append((sum(ms.values()), k, ms))
                cands.sort(key=lambda c: -c[0])
                # the bytes at the candidate: same as MH1's (pointer words masked) -> bonus;
                # quite different -> G changed the table (the MH1 C would misread it)
                sims = {}
                for c in cands[:4]:
                    sm_ = self.similarity(n, c[1][0], c[1][1])
                    sims[c[1]] = sm_
                    if sm_ is not None and "content" not in c[2]:
                        c[2]["bytes"] = 1.5 * sm_ if sm_ >= 0.6 else (-1.0 if sm_ < 0.25 else 0)
                cands = [(sum(c[2].values()), c[1], c[2]) for c in cands]
                cands.sort(key=lambda c: -c[0])
                sc, k, ms = cands[0]
                simv = sims.get(k)
                if simv is None and k not in sims:
                    simv = self.similarity(n, k[0], k[1])
                b_, _m = self.masked(n)
                if simv is None and b_ is not None and any(b_):
                    x_ = self.G[k[0]]
                    if not (x_.vram <= k[1] < x_.vram + len(x_.data)):
                        simv = 0.0        # MH1 has initialised bytes, G's address is .bss
                        ms["bytes"] = -1.0
                second = cands[1][0] if len(cands) > 1 else 0
                if verbose == n:
                    for c in cands[:6]:
                        print("  %s %s 0x%X %.1f %s" % (n, c[1][0], c[1][1], c[0], dict(c[2])))
                if self.gtaken.get(k) not in (None, n) and not re.match(r"^(\w+_)?func_[0-9A-F]{8}$|^D_", self.gtaken.get(k)):
                    continue
                # confidence
                nmeth = len([m for m, w in ms.items() if w > 0])
                strong = ms.get("ref", 0) >= 3 or ms.get("refseq", 0) >= 2 or ms.get("ptr-out", 0) >= 2 or ms.get("ptr-in", 0) >= 2 \
                    or ms.get("layout", 0) >= 2.5 or ms.get("content", 0) >= 3
                if second * 2 > sc or (simv is not None and simv < 0.25 and "content" not in ms):
                    conf = "low"
                elif nmeth >= 2 and sc >= 3:
                    conf = "high"
                elif strong:
                    conf = "high" if sc >= 4 else "medium"
                elif sc >= 1.5 or (sc >= 1.0 and len(cands) == 1 and ms.get("refseq", 0) >= 1.0):
                    conf = "medium"
                else:
                    conf = "low"
                ms["sim"] = simv
                new.append((sc, n, k, conf, ms))
            # one name per G address: the best score wins
            new.sort(key=lambda x: -x[0])
            used = set(v[:2] for v in self.placed.values()) | set(self.known.values())
            nadd = 0
            for sc, n, k, conf, ms in new:
                if k in used:
                    continue
                used.add(k)
                if conf == "low":
                    self.placed.setdefault(n, (k[0], k[1], conf, ms, sc))
                    continue
                self.placed[n] = (k[0], k[1], conf, ms, sc)
                self.known[n] = k
                nadd += 1
            print("round %d: %d placed (high/medium)" % (rnd + 1, nadd))
            if not nadd:
                break

    def write(self):
        rows = []
        for n, (gm, ga, conf, ms, sc) in self.placed.items():
            if n in self.known and self.known[n] != (gm, ga):
                continue
            meth = "+".join("%s:%.1f" % (m, w) for m, w in sorted(ms.items()) if w and m != "sim")
            if ms.get("sim") is not None:
                meth += " bytes=%.2f" % ms["sim"]
            rows.append((gm, ga, n, conf, meth))
        rows.sort()
        with open(P("config/mhg/data_map.txt"), "w") as fh:
            fh.write("# Generated by tools/mhg_datamap.py: G addresses of MH1 data names tools/mhg_map.py left\n"
                     "# unnamed. module  name  G address  confidence  methods (score per method:\n"
                     "# ref = same instruction in the G twin, ptr-in / ptr-out = pointer words, layout, content).\n")
            for gm, ga, n, conf, meth in rows:
                fh.write("%-8s %-40s 0x%08X %-6s %s\n" % (gm, n, ga, conf, meth))
        c = collections.Counter(r[3] for r in rows)
        print("wrote config/mhg/data_map.txt: %d names (high %d, medium %d, low %d)" % (
            len(rows), c["high"], c["medium"], c["low"]))


def pc_needed():
    """the MH1 data names the G PC build lacks (from the last G build's lists, when present)"""
    out = set()
    d = P("build/mhg/pctree/build/pc")
    for f in ("mhg_tables_missing.txt", "mhg_auto_missing.txt"):
        p = os.path.join(d, f)
        if os.path.exists(p):
            out |= set(l.strip() for l in open(p) if l.strip())
    # rt_gen.c's weak function stand-ins that are data objects in MH1
    p = os.path.join(d, "rt_gen.c")
    if os.path.exists(p):
        objs = set(r["name"] for r in csv.DictReader(open(P("docs/survey/mh1_symbols.csv"), encoding="utf-8"))
                   if r["type"] == "OBJECT")
        out |= set(n for n in re.findall(r"__attribute__\(\(weak\)\) int (\w+)\(\)", open(p).read()) if n in objs)
    return out


def evaluate():
    """hold out every 8th name mhg_map placed, map it like an unplaced one, compare"""
    mp = Mapper()
    hold = set(sorted(mp.known)[::8])
    for n in hold:
        del mp.known[n]
    mp.base_known = {n: v for n, v in mp.base_known.items() if n not in hold}
    truth = {n: v for n, v in read_truth().items() if n in hold}
    mp.run()
    by = collections.defaultdict(lambda: [0, 0])
    for n in hold:
        r = mp.placed.get(n)
        if not r:
            by["none"][0] += 1
            continue
        top = max(((m, w) for m, w in r[3].items() if m not in ("sim", "bytes")), key=lambda x: x[1])[0]
        for k in (r[2], r[2] + "/" + top):
            by[k][0] += 1
            by[k][1] += (r[0], r[1]) == truth[n]
    print("held out %d names:" % len(hold))
    for k in sorted(by):
        t, ok = by[k]
        print("  %-22s %5d  correct %5d (%.1f%%)" % (k, t, ok, 100 * ok / max(t, 1)))


def read_truth():
    out = {}
    for m, lst in read_g_syms().items():
        for a, n, isf in lst:
            if not isf:
                out.setdefault(n, (m, a))
    return out


def main():
    if "--eval" in sys.argv:
        evaluate()
        return
    verbose = sys.argv[sys.argv.index("-v") + 1] if "-v" in sys.argv else None
    mp = Mapper()
    mp.run(verbose)
    mp.write()
    need = pc_needed()
    if need:
        hand = set()
        if os.path.exists(P("config/mhg/pc_names.txt")):
            for l in open(P("config/mhg/pc_names.txt")):
                p = l.split("#", 1)[0].split()
                if len(p) == 3:
                    hand.add(p[1])
                    r = mp.placed.get(p[1])
                    print("hand %-22s 0x%s  tool: %s" % (p[1], p[2][2:].upper(),
                          "0x%08X %s" % (r[1], r[2]) if r else "-"))
        got = [n for n in need if n in mp.placed and mp.placed[n][2] != "low"]
        left = sorted(n for n in need if n not in got and n not in hand)
        print("PC names (last G build's stand-ins + zero tables): %d, placed %d, left %d:" % (
            len(need), len(got), len(left)))
        print("  " + " ".join(left))
        if "--pc" in sys.argv:
            for n in left:
                r = mp.placed.get(n)
                o = mp.obj.get(n)
                print("  %-28s %-6s %s %s" % (n, o[0] if o else "?", "0x%X" % o[1] if o else "",
                      "low %s 0x%X %s" % (r[0], r[1], dict(r[3])) if r else "-"))


if __name__ == "__main__":
    main()
