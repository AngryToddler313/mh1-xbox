#!/usr/bin/env python3
"""mhg_map.py - map every MH1 function to its counterpart in PS2 Monster Hunter G.

MH1 (SLPM_654.95) has symbols and link relocations; G (SLPM_658.69) is stripped.
Both were built with the same Metrowerks compiler, so most of G is MH1's code,
unchanged or edited. This tool finds, per MH1 function, its G twin:

  identical  same instructions; only relocated fields (call targets, addresses of
             globals, gp offsets) differ. The MH1 C should compile to G's bytes.
  shape      same instruction shape; some non-relocated 16-bit immediates differ
             (struct offsets, constants). The C needs small edits.
  edited     a counterpart found by similarity (address order between anchors,
             call sites, or global n-gram search); code changed.
  rewritten  a counterpart identified by a call site in an identical/shape twin,
             but less than 50% similar.
  missing    no counterpart found (deleted, or changed beyond recognition).

G functions with no MH1 counterpart are G-only.

Outputs (names and addresses only, no game bytes):
  docs/survey/mhg_map.csv            one row per MH1 function + one per G-only one
  config/mhg/symbols/<module>.txt    G symbol files: MH1 names at G addresses
                                     (functions, and data from relocated operands
                                     of identical/shape twins)
  stdout                             the summary tables

Needs disc/mh1 (split by tools/setup_split.py), disc/mhg/SLPM_658.69 and
disc/mhg/overlays (tools/mwo_unpack.py -o).

Usage: python3 tools/mhg_map.py [--no-write]
"""
import collections
import csv
import difflib
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def P(*a):
    return os.path.join(ROOT, *a)


# ---------------------------------------------------------------- images
class Mod:
    def __init__(self, name, vram, data, text_lo, text_hi, end):
        self.name, self.vram, self.data = name, vram, data
        self.text_lo, self.text_hi, self.end = text_lo, text_hi, end
        n = len(data) // 4
        self.w = struct.unpack("<%dI" % n, data[:n * 4])

    def word(self, a):
        return self.w[(a - self.vram) >> 2]

    def words(self, a, n):
        i = (a - self.vram) >> 2
        return self.w[i:i + n]

    def has(self, a):
        return self.vram <= a < self.vram + len(self.data)


def overlay(name, blob):
    load, text, data, bss = struct.unpack_from("<4I", blob, 8)
    return Mod(name, load, blob, load + 0x40, load + 0x40 + text, load + 0x40 + text + data + bss)


MH1_OVL = ["select", "game", "yn", "lobby"]
G_OVL = ["sub_main", "select", "game", "yn", "lobby", "nethttp"]
MH1_GP, G_GP = 0x38EB70, 0x2F2B70
G_MAIN = dict(off=0x200, size=0x1EF200, text_hi=0x23A280, mem_end=0x4A0C80)


def load_mh1():
    mods = {"main": Mod("main", 0x100000, open(P("disc/mh1/split/main.bin"), "rb").read(),
                        0x100000, 0x293B80, 0x533980)}
    for o in MH1_OVL:
        mods[o] = overlay(o, open(P("disc/mh1/split/%s.bin" % o), "rb").read())
    return mods


def load_g():
    e = open(P("disc/mhg/SLPM_658.69"), "rb").read()
    m = G_MAIN
    mods = {"main": Mod("main", 0x100000, e[m["off"]:m["off"] + m["size"]], 0x100000,
                        m["text_hi"], m["mem_end"])}
    for o in G_OVL:
        mods[o] = overlay(o, open(P("disc/mhg/overlays/%s.bin" % o), "rb").read())
    return mods


# ---------------------------------------------------------------- MH1 symbols, relocations
R32, R26, HI16, LO16, GPREL = 2, 4, 5, 6, 7
RMASK = {R26: 0x03FFFFFF, HI16: 0xFFFF, LO16: 0xFFFF, GPREL: 0xFFFF, R32: 0xFFFFFFFF}


def read_mh1_elf():
    """symtab [(name, value, size, type, module)], relocs {module: {addr: (type, symidx)}}"""
    d = open(P("disc/mh1/SLPM_654.95"), "rb").read()
    shoff, = struct.unpack_from("<I", d, 32)
    shnum, shstr = struct.unpack_from("<HH", d, 48)
    S = [struct.unpack_from("<10I", d, shoff + i * 40) for i in range(shnum)]
    shs = d[S[shstr][4]:S[shstr][4] + S[shstr][5]]

    def nm(t, o):
        return t[o:t.index(b"\0", o)].decode("latin-1")
    secname = [nm(shs, s[0]) for s in S]
    modof = {}
    for i, n in enumerate(secname):
        if n == "main":
            modof[i] = "main"
        elif n.endswith(".bin"):
            modof[i] = n[:-4]
    sy = next(s for s in S if s[1] == 2)
    st = S[sy[6]]
    strs = d[st[4]:st[4] + st[5]]
    syms = []
    for k in range(sy[5] // 16):
        no, val, size, info, _o, sh = struct.unpack_from("<IIIBBH", d, sy[4] + k * 16)
        syms.append((nm(strs, no), val, size, info & 0xF, modof.get(sh)))
    rel = collections.defaultdict(dict)
    for s in S:
        if s[1] == 9 and s[7] in modof:
            mod = modof[s[7]]
            for k in range(s[5] // 8):
                off, info = struct.unpack_from("<II", d, s[4] + k * 8)
                rel[mod][off] = (info & 0xFF, info >> 8)
    return syms, rel


def read_split_names():
    """{module: {addr: name}} and {module: sorted [(addr, size, name, isfunc)]} from
    config/symbols/*.txt (the unique, assembler-safe names the C uses)."""
    names, lists = {}, {}
    pat = re.compile(r"^(\S+) = 0x([0-9A-F]+);(?: // (.*))?$")
    for mod in ["main"] + MH1_OVL:
        nmap, lst = {}, []
        for l in open(P("config/symbols/%s.txt" % mod)):
            m = pat.match(l.strip())
            if not m:
                continue
            a = int(m.group(2), 16)
            attrs = m.group(3) or ""
            sz = re.search(r"size:0x([0-9A-F]+)", attrs)
            sz = int(sz.group(1), 16) if sz else 0
            nmap.setdefault(a, m.group(1))
            lst.append((a, sz, m.group(1), "type:func" in attrs))
        lst.sort()
        names[mod], lists[mod] = nmap, lst
    return names, lists


# ---------------------------------------------------------------- instruction helpers
MASKLO = {8, 9, 0xD, 0xF, 0x1E, 0x1F} | set(range(0x20, 0x30)) | {0x31, 0x35, 0x39, 0x3D, 0x37, 0x3F} \
    | {0xA, 0xB, 0xC, 0xE}


def shape(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op in MASKLO:
        return w & 0xFFFF0000
    return w


def token(w):
    """opcode class of an instruction, registers and immediates dropped"""
    op = w >> 26
    if op == 0:
        return w & 0x3F if w else 0x7F
    if op == 1:
        return 0x100 | ((w >> 16) & 0x1F)
    if op in (0x10, 0x11, 0x12):
        return (op << 12) | (((w >> 21) & 0x1F) << 6) | (w & 0x3F)
    if op == 0x1C:
        return 0x80000 | (w & 0x7FF)
    return 0x200 | op


def is_end(m, a):
    """a 16-aligned address that follows a function end: jr ra / j + delay slot + nop padding"""
    k = a
    for _ in range(4):
        if k - 4 < m.text_lo or m.word(k - 4) != 0:
            break
        k -= 4
    for back in ((4, 8) if k < a else (8,)):     # never start in a delay slot
        p = k - back
        if p >= m.text_lo:
            w = m.word(p)
            if w == 0x03E00008 or (w >> 26) == 2:
                return True
    return False


class Func:
    __slots__ = ("mod", "addr", "size", "name", "tok", "shp", "pair", "cls", "sim", "how", "calls")

    def __init__(self, mod, addr, size, name=None):
        self.mod, self.addr, self.size, self.name = mod, addr, size, name
        self.pair = None
        self.cls = None
        self.sim = 0.0
        self.how = ""
        self.tok = self.shp = self.calls = None

    def key(self):
        return (self.mod, self.addr)


def g_functions(G, extra):
    """Function starts in G: module text starts, jal targets, ends of jr-ra blocks, and
    `extra` (starts of exact MH1 twins). Sizes up to the next start, minus nop padding."""
    starts = collections.defaultdict(set)
    for m in G.values():
        starts[m.name].add(m.text_lo)
    for m in G.values():
        for a in range(m.text_lo, m.text_hi, 4):
            w = m.word(a)
            if (w >> 26) == 3:
                t = ((a + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
                for cand in (m.name, "main", "sub_main"):
                    c = G[cand]
                    if c.text_lo <= t < c.text_hi:
                        starts[cand].add(t)
                        break
        for a in range(m.text_lo + 16 - (m.text_lo & 15) if m.text_lo & 15 else m.text_lo, m.text_hi, 16):
            if m.word(a) != 0 and is_end(m, a):
                starts[m.name].add(a)
    for mod, a in extra:
        starts[mod].add(a)
    funcs = []
    for mod, ss in starts.items():
        m = G[mod]
        ss = sorted(ss)
        for i, a in enumerate(ss):
            e = ss[i + 1] if i + 1 < len(ss) else m.text_hi
            k = e
            while k - 4 > a and m.word(k - 4) == 0:
                k -= 4
            if k - 4 >= a and (m.word(k - 4) == 0x03E00008 or (m.word(k - 4) >> 26) == 2) and k < e:
                k += 4          # keep the delay-slot nop
            if k - a > 0:
                funcs.append(Func(mod, a, k - a))
    return funcs


# ---------------------------------------------------------------- matching
def similarity(a, b):
    la, lb = len(a.tok), len(b.tok)
    if not la or not lb or min(la, lb) * 3 < max(la, lb):
        return 0.0
    s = difflib.SequenceMatcher(None, a.tok, b.tok, autojunk=False)
    if s.real_quick_ratio() < 0.4 or s.quick_ratio() < 0.4:
        return 0.0
    return s.ratio()


def lis(pairs):
    """longest chain increasing in both coordinates; pairs sorted by first"""
    import bisect
    tails, tidx, prev = [], [], [None] * len(pairs)
    for i, (_, g) in enumerate(pairs):
        k = bisect.bisect_left(tails, g)
        if k == len(tails):
            tails.append(g)
            tidx.append(i)
        else:
            tails[k] = g
            tidx[k] = i
        prev[i] = tidx[k - 1] if k else None
    out, i = [], tidx[-1] if tidx else None
    while i is not None:
        out.append(pairs[i])
        i = prev[i]
    return out[::-1]


def main():
    write = "--no-write" not in sys.argv
    M1, G = load_mh1(), load_g()
    syms, rel = read_mh1_elf()
    snames, slists = read_split_names()

    # MH1 functions (CSV order; names from the split symbol files)
    f1 = []
    seen = set()
    for r in csv.DictReader(open(P("docs/survey/mh1_symbols.csv"), encoding="utf-8")):
        if r["type"] != "FUNC":
            continue
        sec = r["section"]
        mod = "main" if sec == "main" else sec[:-4] if sec.endswith(".bin") else None
        if mod not in M1:
            continue
        a, sz = int(r["addr"], 16), int(r["size"])
        m = M1[mod]
        if sz < 4 or not (m.text_lo <= a < m.text_hi) or (mod, a) in seen:
            continue
        seen.add((mod, a))
        f = Func(mod, a, sz, snames[mod].get(a, r["name"]))
        w = m.words(a, sz // 4)
        n = len(w)
        while n > 1 and w[n - 1] == 0:      # MH1 sizes can include a trailing nop
            n -= 1
        if n < len(w) and (w[n - 1] == 0x03E00008 or (w[n - 1] >> 26) == 2):
            n += 1
        w = w[:n]
        f.tok = [token(x) for x in w]
        f.shp = tuple(shape(x) for x in w)
        f1.append(f)
    f1.sort(key=lambda f: (f.mod, f.addr))

    # ---- exact shape matches (anywhere in G text)
    K = 6
    gidx = collections.defaultdict(list)
    gshape = {}
    for m in G.values():
        sh = [shape(x) for x in m.words(m.text_lo, (m.text_hi - m.text_lo) // 4)]
        gshape[m.name] = sh
        for i in range(0, len(sh) - K + 1):
            if ((m.text_lo + 4 * i) & 15) == 0:
                gidx[tuple(sh[i:i + K])].append((m.name, i))
    cands = {}
    for f in f1:
        n = len(f.shp)
        k = min(K, n)
        hits = []
        if k == K:
            for (gm, i) in gidx.get(f.shp[:K], ()):
                if tuple(gshape[gm][i:i + n]) == f.shp:
                    hits.append((gm, G[gm].text_lo + 4 * i))
        cands[f.key()] = hits

    def reloc_equal(f, gm, ga):
        """identical except MH1's relocated fields"""
        rl = rel[f.mod]
        a = M1[f.mod].words(f.addr, f.size // 4)
        b = G[gm].words(ga, len(f.shp))
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                r = rl.get(f.addr + 4 * i)
                msk = RMASK.get(r[0], 0) if r else 0
                if (x & ~msk) != (y & ~msk):
                    return False
        return True

    # unique hits -> anchors; ambiguous ones decided later by order
    extra = set()
    uniq = {}
    used_g = collections.Counter()
    for f in f1:
        for h in cands[f.key()]:
            used_g[h] += 1
    for f in f1:
        h = cands[f.key()]
        if len(h) == 1 and used_g[h[0]] == 1:
            uniq[f.key()] = h[0]
            extra.add(h[0])
    gfuncs = g_functions(G, extra)
    gby = {f.key(): f for f in gfuncs}
    for gf in gfuncs:
        gw = G[gf.mod].words(gf.addr, gf.size // 4)
        gf.tok = [token(x) for x in gw]
        gf.shp = tuple(shape(x) for x in gw)
    gsorted = collections.defaultdict(list)
    for gf in gfuncs:
        gsorted[gf.mod].append(gf)
    for v in gsorted.values():
        v.sort(key=lambda g: g.addr)

    def gfunc_at(gm, a):
        return gby.get((gm, a))

    def split_g(gf, n):
        """gf really ends after n bytes (a start the boundary scan missed): cut the rest
        (after nop padding) into a new G function"""
        m = G[gf.mod]
        a, end = gf.addr + n, gf.addr + gf.size
        while a < end and m.word(a) == 0:
            a += 4
        gf.size = n
        gw = m.words(gf.addr, n // 4)
        gf.tok = [token(x) for x in gw]
        gf.shp = tuple(shape(x) for x in gw)
        if a < end:
            nf = Func(gf.mod, a, end - a)
            w = m.words(a, (end - a) // 4)
            nf.tok = [token(x) for x in w]
            nf.shp = tuple(shape(x) for x in w)
            gfuncs.append(nf)
            gby[nf.key()] = nf
            lst = gsorted[gf.mod]
            lst.insert(lst.index(gf) + 1, nf)

    ALLOWED = {("main", "main"), ("main", "sub_main"), ("main", "game"), ("game", "game"),
               ("lobby", "lobby"), ("lobby", "nethttp"), ("main", "nethttp"), ("select", "select"), ("yn", "yn")}

    def merge_g(gf, n):
        """a false start cut gf short: absorb the unpaired G functions inside its true extent"""
        m = G[gf.mod]
        lst = gsorted[gf.mod]
        k = lst.index(gf) + 1
        while k < len(lst) and lst[k].addr < gf.addr + n:
            if lst[k].pair:
                return
            k += 1
        for x in lst[lst.index(gf) + 1:k]:
            gfuncs.remove(x)
            del gby[x.key()]
        end = lst[k - 1].addr + lst[k - 1].size
        del lst[lst.index(gf) + 1:k]
        gf.size = max(n, end - gf.addr)
        gw = m.words(gf.addr, gf.size // 4)
        gf.tok = [token(x) for x in gw]
        gf.shp = tuple(shape(x) for x in gw)
        if gf.size > n:
            split_g(gf, n)

    def pair(f, gf, how, sim=None):
        if f.pair or gf.pair:
            return False
        if (f.mod, gf.mod) not in ALLOWED and gf.shp[:len(f.shp)] != f.shp:
            return False        # only exact twins may cross other module boundaries
        if gf.size > 4 * len(f.shp) and gf.shp[:len(f.shp)] == f.shp:
            split_g(gf, 4 * len(f.shp))
        elif gf.size < 4 * len(f.shp):
            m = G[gf.mod]
            i0 = (gf.addr - m.text_lo) >> 2
            if tuple(gshape[gf.mod][i0:i0 + len(f.shp)]) == f.shp:
                merge_g(gf, 4 * len(f.shp))
        f.pair, gf.pair = gf, f
        if sim is None:
            sim = 1.0 if f.shp == gf.shp else similarity(f, gf)
        if f.shp == gf.shp:
            f.cls = "identical" if reloc_equal(f, gf.mod, gf.addr) else "shape"
        else:
            f.cls = "edited" if sim >= 0.5 else "rewritten"
        f.sim, f.how = sim, how
        gf.cls = f.cls
        return True

    # anchors: LIS per (mh1 module, g module); big unique matches stay even off-chain
    by_pair = collections.defaultdict(list)
    for f in f1:
        h = uniq.get(f.key())
        if h:
            by_pair[(f.mod, h[0])].append((f.addr, h[1], f))
    for (mm, gm), lst in by_pair.items():
        lst.sort(key=lambda t: t[0])
        ch = lis([(t[0], t[1]) for t in lst])
        chs = set(ch)
        for a, ga, f in lst:
            gf = gfunc_at(gm, ga)
            if gf is None or gf.size < 4 * len(f.shp):
                continue
            if (a, ga) in chs or f.size >= 48:
                pair(f, gf, "exact")

    def neighbours_resolve():
        """ambiguous exact hits: take the one between the paired neighbours"""
        n = 0
        bym = collections.defaultdict(list)
        for f in f1:
            bym[f.mod].append(f)
        for mod, lst in bym.items():
            for i, f in enumerate(lst):
                if f.pair or not cands[f.key()]:
                    continue
                lo = next((x.pair for x in reversed(lst[:i]) if x.pair), None)
                hi = next((x.pair for x in lst[i + 1:] if x.pair), None)
                ok = []
                for gm, ga in cands[f.key()]:
                    gf = gfunc_at(gm, ga)
                    if gf is None or gf.pair:
                        continue
                    if lo and lo.mod == gm and ga <= lo.addr:
                        continue
                    if hi and hi.mod == gm and ga >= hi.addr:
                        continue
                    if (lo and lo.mod == gm) or (hi and hi.mod == gm):
                        ok.append(gf)
                if len(ok) == 1 and pair(f, ok[0], "exact-order"):
                    n += 1
        return n

    while neighbours_resolve():
        pass

    # ---- gap alignment between paired neighbours (same module pair)
    def gap_align():
        n = 0
        bym = collections.defaultdict(list)
        for f in f1:
            bym[f.mod].append(f)
        for mod, lst in bym.items():
            i = 0
            while i < len(lst):
                if lst[i].pair:
                    i += 1
                    continue
                j = i
                while j < len(lst) and not lst[j].pair:
                    j += 1
                lo = lst[i - 1].pair if i > 0 else None
                hi = lst[j].pair if j < len(lst) else None
                gaps = []
                if lo is not None and (hi is None or hi.mod == lo.mod) and (hi is None or hi.addr > lo.addr):
                    gm = lo.mod
                    gl = [g for g in gsorted[gm] if g.addr > lo.addr and (hi is None or g.addr < hi.addr)
                          and not g.pair]
                    if hi is None:
                        gl = gl[:max(8, 2 * (j - i))]
                    gaps.append(gl)
                elif hi is not None:
                    gm = hi.mod
                    gl = [g for g in gsorted[gm] if g.addr < hi.addr and (lo is None or lo.mod != gm or
                                                                         g.addr > lo.addr) and not g.pair]
                    gl = gl[-max(8, 2 * (j - i)):]
                    gaps.append(gl)
                ml = lst[i:j]
                for gl in gaps:
                    if not gl or len(ml) * len(gl) > 60000:
                        continue
                    n += align(ml, gl)
                i = j
        return n

    def align(ml, gl):
        """Needleman-Wunsch on function similarity; pairs need sim >= 0.5"""
        R, C = len(ml), len(gl)
        S = [[0.0] * (C + 1) for _ in range(R + 1)]
        B = [[0] * (C + 1) for _ in range(R + 1)]
        sims = {}
        for r in range(1, R + 1):
            for c in range(1, C + 1):
                best, b = S[r - 1][c], 1
                if S[r][c - 1] > best:
                    best, b = S[r][c - 1], 2
                s = sims.get((r, c))
                if s is None:
                    s = similarity(ml[r - 1], gl[c - 1])
                    sims[(r, c)] = s
                if s >= 0.5:
                    v = S[r - 1][c - 1] + s * (ml[r - 1].size + gl[c - 1].size)
                    if v > best:
                        best, b = v, 3
                S[r][c], B[r][c] = best, b
        r, c, n = R, C, 0
        while r and c:
            b = B[r][c]
            if b == 3:
                if pair(ml[r - 1], gl[c - 1], "order", sims[(r, c)]):
                    n += 1
                r, c = r - 1, c - 1
            elif b == 1:
                r -= 1
            else:
                c -= 1
        return n

    # ---- call sites: a call in a twin names its callee's twin
    def calls_of(mod, addr, size, M):
        out = []
        m = M[mod]
        for i, w in enumerate(m.words(addr, size // 4)):
            if (w >> 26) == 3:
                a = addr + 4 * i
                out.append(((a + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
        return out

    f1by = {f.key(): f for f in f1}

    def mh1_func_at(mod, t):
        for mm in (mod, "main"):
            f = f1by.get((mm, t))
            if f:
                return f
        return None

    def g_func_at(mod, t):
        for mm in (mod, "main", "sub_main"):
            f = gby.get((mm, t))
            if f:
                return f
        return None

    def callsite_pass():
        n = 0
        for f in f1:
            gf = f.pair
            if not gf:
                continue
            c1 = calls_of(f.mod, f.addr, f.size, M1)
            c2 = calls_of(gf.mod, gf.addr, gf.size, G)
            if f.cls in ("identical", "shape"):
                pairs = list(zip(c1, c2))
            else:
                # align the call lists: known callees map, unknown ones are wildcards
                k1 = []
                for t in c1:
                    x = mh1_func_at(f.mod, t)
                    k1.append(("g", x.pair.mod, x.pair.addr) if x and x.pair else ("m", t))
                k2 = []
                for t in c2:
                    y = g_func_at(gf.mod, t)
                    k2.append(("g", y.mod, y.addr) if y else ("x", t))
                pairs = []
                sm = difflib.SequenceMatcher(None, k1, k2, autojunk=False)
                for tag, i1, i2, j1, j2 in sm.get_opcodes():
                    if tag == "replace" and i2 - i1 == j2 - j1:
                        pairs += list(zip(c1[i1:i2], c2[j1:j2]))
            for t1, t2 in pairs:
                x = mh1_func_at(f.mod, t1)
                y = g_func_at(gf.mod, t2)
                if not x or not y or x.pair or y.pair:
                    continue
                s = 1.0 if x.shp == y.shp else similarity(x, y)
                if f.cls in ("identical", "shape") or s >= 0.4:
                    if pair(x, y, "call", s):
                        n += 1
        return n

    # ---- n-gram search for what is left
    def ngram_pass(th=0.6):
        N = 5
        def grams(t):
            return set(tuple(t[i:i + N]) for i in range(len(t) - N + 1))
        inv = collections.defaultdict(list)
        gg = {}
        for gf in gfuncs:
            if gf.pair or len(gf.tok) < 12:
                continue
            s = grams(gf.tok)
            gg[gf.key()] = s
            for x in s:
                inv[x].append(gf)
        best = {}
        for f in f1:
            if f.pair or len(f.tok) < 12:
                continue
            s = grams(f.tok)
            cnt = collections.Counter()
            for x in s:
                lst = inv.get(x)
                if lst and len(lst) < 200:
                    for gf in lst:
                        cnt[gf] += 1
            for gf, c in cnt.most_common(5):
                j = c / (len(s) + len(gg[gf.key()]) - c)
                if j < 0.3:
                    break
                sim = similarity(f, gf)
                if sim >= th and sim > best.get(f.key(), (0,))[0]:
                    best[f.key()] = (sim, gf)
        # mutual best only
        gbest = {}
        for k, (s, gf) in best.items():
            if s > gbest.get(gf.key(), (0,))[0]:
                gbest[gf.key()] = (s, k)
        n = 0
        for gk, (s, k) in gbest.items():
            if gk in gby and pair(f1by[k], gby[gk], "ngram", s):
                n += 1
        return n

    rounds = []
    for it in range(6):
        a = gap_align()
        b = callsite_pass()
        c = neighbours_resolve()
        rounds.append((a, b, c))
        if not (a or b or c):
            break
    d = ngram_pass()
    for it in range(4):
        a = gap_align()
        b = callsite_pass()
        if not (a or b):
            break
    print("passes (gap, call, order):", rounds, "ngram:", d, file=sys.stderr)

    for f in f1:
        if not f.pair:
            f.cls = "missing"
    nh = [f for f in f1 if cands[f.key()]]
    print("MH1 functions with an exact-shape hit somewhere in G: %d (%d bytes); paired as identical/shape: %d"
          % (len(nh), sum(f.size for f in nh), sum(1 for f in f1 if f.cls in ("identical", "shape"))),
          file=sys.stderr)
    for f in nh:
        if f.cls not in ("identical", "shape") and f.size >= 64 and "--why" in sys.argv:
            print("  hit but %s: %s %s 0x%X size %d hits %s pair %s" % (
                f.cls, f.mod, f.name, f.addr, f.size, [(g, hex(a)) for g, a in cands[f.key()][:3]],
                (f.pair.mod, hex(f.pair.addr), f.pair.size) if f.pair else None), file=sys.stderr)
    gonly = [g for g in gfuncs if not g.pair]

    # ---------------------------------------------------------- data names from twins
    data_votes = collections.defaultdict(collections.Counter)   # (gmod, gaddr) -> name votes
    func_names = {}

    def sym_name(mod, idx, target):
        """split name + offset for an MH1 relocation target"""
        name, val, size, typ, smod = syms[idx]
        smod = smod or mod
        if typ in (1, 2) and smod in snames and val in snames[smod]:
            return smod, snames[smod][val], target - val
        lst = slists.get(smod) or []
        import bisect
        k = bisect.bisect_right(lst, (target, 1 << 40)) - 1
        while k >= 0 and lst[k][0] <= target:
            a, sz, nm_, isf = lst[k]
            if target < a + max(sz, 1):
                return smod, nm_, target - a
            if a < target - 0x100000:
                break
            k -= 1
        return smod, None, 0

    def g_mod_of(gmod, a):
        for mm in (gmod, "main", "sub_main"):
            if G[mm].vram <= a < G[mm].end:
                return mm
        return None

    for f in f1:
        gf = f.pair
        if not gf or f.cls not in ("identical", "shape"):
            continue
        rl = rel[f.mod]
        a1 = M1[f.mod].words(f.addr, len(f.shp))
        a2 = G[gf.mod].words(gf.addr, len(f.shp))
        his = {}
        for i in range(len(a1)):
            r = rl.get(f.addr + 4 * i)
            if not r:
                continue
            t, idx = r
            if t == HI16:
                his[idx] = i
                continue
            if t == LO16 and idx in his:
                h = his[idx]
                tg1 = ((a1[h] & 0xFFFF) << 16) + (((a1[i] & 0xFFFF) ^ 0x8000) - 0x8000)
                tg2 = ((a2[h] & 0xFFFF) << 16) + (((a2[i] & 0xFFFF) ^ 0x8000) - 0x8000)
            elif t == GPREL:
                tg1 = MH1_GP + (((a1[i] & 0xFFFF) ^ 0x8000) - 0x8000)
                tg2 = G_GP + (((a2[i] & 0xFFFF) ^ 0x8000) - 0x8000)
            else:
                continue
            smod, name, off = sym_name(f.mod, idx, tg1 & 0xFFFFFFFF)
            gm = g_mod_of(gf.mod, (tg2 - off) & 0xFFFFFFFF)
            if name and gm:
                data_votes[(gm, (tg2 - off) & 0xFFFFFFFF)][name] += 1

    # ---------------------------------------------------------- reports
    def table(title, rows_by):
        print("\n" + title)
        print("| | funcs | identical | shape | edited | rewritten | missing | twin bytes (ident+shape) | any counterpart |")
        print("|---|---|---|---|---|---|---|---|---|")
        tot = collections.Counter()
        totb = collections.Counter()
        for k, fl in rows_by:
            c = collections.Counter(f.cls for f in fl)
            b = collections.Counter()
            for f in fl:
                b[f.cls] += f.size
            B = sum(b.values()) or 1
            tot.update(c)
            totb.update(b)
            print("| %s | %d | %d | %d | %d | %d | %d | %.1f%% | %.1f%% |" % (
                k, len(fl), c["identical"], c["shape"], c["edited"], c["rewritten"], c["missing"],
                100 * (b["identical"] + b["shape"]) / B, 100 * (B - b["missing"]) / B))
        B = sum(totb.values()) or 1
        print("| **all** | %d | %d | %d | %d | %d | %d | %.1f%% | %.1f%% |" % (
            sum(tot.values()), tot["identical"], tot["shape"], tot["edited"], tot["rewritten"],
            tot["missing"], 100 * (totb["identical"] + totb["shape"]) / B, 100 * (B - totb["missing"]) / B))
        print("bytes: " + ", ".join("%s %.1f%%" % (k, 100 * totb[k] / B)
                                    for k in ("identical", "shape", "edited", "rewritten", "missing")))

    bymod = collections.OrderedDict((m, [f for f in f1 if f.mod == m]) for m in ["main"] + MH1_OVL)
    table("By MH1 module (counts; %% of MH1 code bytes)", bymod.items())
    where = collections.Counter((f.mod, f.pair.mod) for f in f1 if f.pair)
    print("\nWhere MH1 functions went (mh1 module -> G module):",
          ", ".join("%s->%s %d" % (a, b, n) for (a, b), n in sorted(where.items())))
    gb = collections.Counter()
    gt = collections.Counter()
    for g in gfuncs:
        gt[g.mod] += g.size
        if not g.pair:
            gb[g.mod] += g.size
    print("\nG-only functions (no MH1 counterpart), by G module:")
    for m in ["main"] + G_OVL:
        n = sum(1 for g in gonly if g.mod == m)
        print("  %-9s %5d funcs %8d bytes = %5.1f%% of its %d text bytes" % (
            m, n, gb[m], 100 * gb[m] / max(gt[m], 1), gt[m]))

    # PC port: game-C functions linked into build/pc/mhview
    pcnames = pc_port_names()
    if pcnames:
        sel = [f for f in f1 if f.name in pcnames]
        table("Functions the PC port runs as game C (%d names found in build/pc/*.o of src/game, src/main, src/lobby)"
              % len(pcnames), [("pc-port", sel)])
    # matched C
    mr = matched_ranges()
    raw = raw_funcs()
    sel = [f for f in f1 if (f.mod, f.addr) not in raw and any(lo <= f.addr < hi for lo, hi, _ in mr.get(f.mod, []))]
    table("MH1 functions with matched C (config/c_files.txt, without c_rawfuncs)", collections.OrderedDict(
        (m, [f for f in sel if f.mod == m]) for m in ["main"] + MH1_OVL).items())

    # clean twins: identical, and every call goes to the G twin of the MH1 callee
    def clean(f):
        if f.cls != "identical":
            return False
        gf = f.pair
        for t1, t2 in zip(calls_of(f.mod, f.addr, 4 * len(f.shp), M1), calls_of(gf.mod, gf.addr, 4 * len(f.shp), G)):
            x = mh1_func_at(f.mod, t1)
            if x is None or x.pair is None or x.pair.addr != t2:
                return False
        return True
    cl = [f for f in sel if clean(f)]
    print("\nMatched C functions that are clean identical twins (calls resolve to the callee's twin): %d of %d "
          "(%d of %d bytes)" % (len(cl), len(sel), sum(f.size for f in cl), sum(f.size for f in sel)))
    # whole translation units: every function a clean twin, laid out in G in the same order and spacing
    tu_ok = tu_n = tu_b = tu_okb = 0
    tus = []
    for mod, lst in mr.items():
        for lo, hi, name in lst:
            fs = [f for f in f1 if f.mod == mod and lo <= f.addr < hi]
            if not fs:
                continue
            tu_n += 1
            b = sum(f.size for f in fs)
            tu_b += b
            ok = all(clean(f) for f in fs) and all(f.pair.mod == fs[0].pair.mod and
                                                    f.pair.addr - fs[0].pair.addr == f.addr - fs[0].addr
                                                    for f in fs)
            if ok:
                tu_ok += 1
                tu_okb += b
            tus.append((mod, name, lo, hi, ok, fs[0].pair.mod if fs[0].pair else "", fs[0].pair.addr if fs[0].pair else 0))
    print("Matched C files whose every function is a clean twin, same layout in G: %d of %d files "
          "(%d of %d bytes)" % (tu_ok, tu_n, tu_okb, tu_b))
    if write:
        with open(P("build/mhg_tu_reuse.txt"), "w") as fh:
            for t in tus:
                fh.write("%s %s 0x%08X 0x%08X %s %s 0x%08X\n" % t)

    if not write:
        return
    with open(P("docs/survey/mhg_map.csv"), "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["mh1_module", "mh1_addr", "size", "name", "class", "how", "sim", "g_module", "g_addr", "g_size"])
        for f in f1:
            gf = f.pair
            w.writerow([f.mod, "0x%08X" % f.addr, f.size, f.name, f.cls, f.how, "%.2f" % f.sim,
                        gf.mod if gf else "", "0x%08X" % gf.addr if gf else "", gf.size if gf else ""])
        for g in sorted(gonly, key=lambda g: (g.mod, g.addr)):
            w.writerow(["", "", "", "", "g-only", "", "", g.mod, "0x%08X" % g.addr, g.size])
    # G symbol files
    os.makedirs(P("config/mhg/symbols"), exist_ok=True)
    out = collections.defaultdict(dict)    # gmod -> addr -> (name, size, func)
    used = set()
    for f in f1:
        gf = f.pair
        if gf and f.name not in used:
            out[gf.mod][gf.addr] = (f.name, gf.size, True)
            used.add(f.name)
    nconf = 0
    for (gm, a), votes in data_votes.items():
        (name, n), = votes.most_common(1)
        if len(votes) > 1:
            nconf += 1
        if name in used or a in out[gm]:
            continue
        out[gm][a] = (name, 0, False)
        used.add(name)
    for g in gonly:
        if g.addr not in out[g.mod]:
            out[g.mod][g.addr] = ("func_%08X" % g.addr if g.mod == "main" else
                                  "%s_func_%08X" % (g.mod, g.addr), g.size, True)
    for gm, d in out.items():
        with open(P("config/mhg/symbols/%s.txt" % gm), "w") as fh:
            fh.write("// Generated by tools/mhg_map.py: MH1 names at PS2 Monster Hunter G addresses.\n"
                     "// Address suffixes in names (foo_00533A00) are MH1 addresses, kept so shared C links.\n")
            for a in sorted(d):
                name, sz, isf = d[a]
                attrs = (["type:func"] if isf else []) + (["size:0x%X" % sz] if sz else [])
                fh.write("%s = 0x%08X;%s\n" % (name, a, (" // " + " ".join(attrs)) if attrs else ""))
    print("\nwrote docs/survey/mhg_map.csv, config/mhg/symbols/*.txt (%d data names; %d addresses with "
          "conflicting votes, majority kept)" % (sum(1 for d in out.values() for v in d.values() if not v[2]), nconf))


def matched_ranges():
    out = collections.defaultdict(list)
    for l in open(P("config/c_files.txt")):
        p = l.split("#", 1)[0].split()
        if len(p) == 4 and ":" not in p[0]:
            out[p[0]].append((int(p[1], 16), int(p[2], 16), p[3]))
    return out


def raw_funcs():
    out = set()
    if os.path.exists(P("config/c_rawfuncs.txt")):
        for l in open(P("config/c_rawfuncs.txt")):
            p = l.split("#", 1)[0].split()
            if len(p) >= 4:
                out.add((p[0], int(p[1], 16)))
    return out


def pc_port_names():
    """defined text symbols of the game C objects linked into the PC port"""
    import glob
    import subprocess
    sh = open(P("tools/build_pc.sh")).read()
    srcs = set(re.findall(r"src/(?:game|main|lobby|select|yn)/[\w/]+\.c", sh))
    srcs |= set(os.path.relpath(p, ROOT) for p in glob.glob(P("src/main/pl/pl[0-9][0-9].c")))
    names = set()
    for s in srcs:
        o = P("build/pc", os.path.basename(s)[:-2] + ".o")
        if not os.path.exists(o):
            continue
        r = subprocess.run(["nm", "--defined-only", o], capture_output=True, text=True)
        for l in r.stdout.splitlines():
            p = l.split()
            if len(p) == 3 and p[1] in "TtWw":
                names.add(p[2])
    return names


if __name__ == "__main__":
    main()
