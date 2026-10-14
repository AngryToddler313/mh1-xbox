#!/usr/bin/env python3
"""text_map.py - build an English text table for MH1 JP and Monster Hunter G from the player's own discs.

    python3 tools/text_map.py [--jp disc/mh1] [--us disc/mh1_us] [--g disc/mhg] [-o build/text]

Everything is derived on the player's machine from the three discs; the output (the text tables) goes to
build/text/ (git-ignored) and is never committed. The repository holds only this tool and a report of counts
and string ids (docs/text_coverage.md, made by --report).

How a US string is tied to a JP string, and then to a G string, "by structure":
  1. code reference: every JP function is searched in the other build by masked instruction n-grams (addresses
     and 16-bit immediates masked, as tools/carry_match.py). Where the code is the same, the instruction at the
     same place loads the matching string: lui/addiu pairs (and gp-relative loads) give (JP string, other string).
     Edited functions are aligned on the n-grams that still match, and the words between them one to one.
  2. table index: a pointer table (item names, weapons, armour, monsters, skills, menus ...) is a JP data object
     (symbol table). Its address in the other build comes from the same code references (an address loaded by
     matched code), or for G from the MH1 symbol names that tools/mhg_map.py put at G addresses. The k-th
     pointer word of the JP object and the k-th word at the other build's object are the same table slot. An
     object is accepted only if its string pointers line up (>= 90% of them), so a reshaped table is left out.
  3. quest number: m001.. (JP), u001.. (US), mNNNNN (G) are tied by their header fields (stage, monsters,
     rewards ...), not by number alone.
A pair is dropped if the printf-style conversions (%d %s ...) of the two strings differ, or the "US" string
is still Japanese text. English goes JP id -> US text; the G table re-keys it to G addresses through (1) and (2)
applied to G the same way. Strings with no counterpart are listed in the coverage report, grouped by area.

Output ids follow tools/text_dump.py / src/pc/rt/rt_text.c: space:0xADDRESS (main, game, lobby, select, yn,
and sub_main for G) and quest:N:0xFILEOFFSET. Standard library only.
"""
import argparse
import bisect
import collections
import csv
import difflib
import os
import re
import struct
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import mwo_unpack  # noqa: E402
import text_dump as td  # noqa: E402
from clay_dump import melt  # noqa: E402

K = 6                      # n-gram length for locating code
MASKLO = {8, 9, 0xd, 0xf, 0x1e, 0x1f} | set(range(0x20, 0x30)) | {0x31, 0x35, 0x39, 0x3d, 0x37, 0x3f}
OVL_NAMES = ("game", "lobby", "select", "yn", "sub_main", "nethttp")


def mask(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xfc000000
    if op in MASKLO:
        return w & 0xffff0000
    return w


def log(*a):
    print(*a, file=sys.stderr, flush=True)


# ---------------------------------------------------------------- images
class Img:
    """One game build: modules {space: (va, bytes)}. main is the ELF load segment; overlays are the whole MWo3
    file loaded at its segment start (the header is part of the load)."""

    def __init__(self, tag, disc, main_name):
        self.tag = tag
        self.disc = disc
        elf = open(os.path.join(disc, main_name), "rb").read()
        va, seg = td.load_elf(os.path.join(disc, main_name))
        self.mod = {"main": (va, seg)}
        self.gp = None
        shoff, = struct.unpack_from("<I", elf, 0x20)
        shentsize, shnum, shstr = struct.unpack_from("<HHH", elf, 0x2E)
        if shoff and shnum:
            stroff = struct.unpack_from("<I", elf, shoff + shstr * shentsize + 0x10)[0]
            for i in range(shnum):
                nm, _t, _f, _a, off, size = struct.unpack_from("<6I", elf, shoff + i * shentsize)
                name = elf[stroff + nm: elf.index(b"\0", stroff + nm)]
                if name == b".reginfo" and size >= 24:
                    self.gp, = struct.unpack_from("<I", elf, off + 20)
        afs = os.path.join(disc, "AFS_DATA.AFS")
        self.afs = afs
        self.text_end = {}
        for blob in mwo_unpack.afs_entries(afs):
            if b"MWo3" not in blob[:4] and blob[2:6] != b"MWo3":
                continue
            m, _packed = mwo_unpack.unpack(blob)
            h = mwo_unpack.describe(m)
            nm = h["name"].replace(".bin", "")
            if nm in OVL_NAMES:
                self.mod[nm] = (h["load"], bytes(m))
                self.text_end[nm] = h["load"] + 0x40 + h["text"]

    def span(self, space):
        va, b = self.mod[space]
        return va, va + len(b)

    def u32(self, space, va):
        base, b = self.mod[space]
        o = va - base
        if 0 <= o <= len(b) - 4:
            return struct.unpack_from("<I", b, o)[0]
        return None

    def cstr(self, space, va, limit=4096):
        base, b = self.mod[space]
        o = va - base
        if not 0 <= o < len(b):
            return None
        e = b.find(b"\0", o, o + limit)
        return None if e < 0 else b[o:e]

    def resolve(self, va, own):
        """space an address belongs to, seen from code in module `own`"""
        m = self.span("main")
        if m[0] <= va < m[1]:
            return "main"
        if "sub_main" in self.mod:
            s = self.span("sub_main")
            if s[0] <= va < s[1]:
                return "sub_main"
        if own != "main" and own in self.mod:
            s = self.span(own)
            if s[0] <= va < s[1]:
                return own
        return None


def code_words(img, space):
    """masked word list and raw word list of the code part of a module"""
    va, b = img.mod[space]
    if space == "main":
        end = min(len(b), 0x150000)          # text part (JP 0x23A280, G same, US a little more)
        end = min(len(b), 0x252000 - va)
    else:
        end = min(len(b), 0x40 + (img.text_end[space] - va - 0x40))
    n = end // 4
    raw = struct.unpack("<%dI" % n, b[:n * 4])
    return list(raw)


def refs_of(raw, base, gp):
    """{(index of lui or gp-use, index of the using instruction): address}: lui + use pairs (one lui may feed
    several uses), and gp + imm"""
    out = {}
    n = len(raw)
    for i in range(n):
        w = raw[i]
        op = w >> 26
        if op == 0xF:
            rt = (w >> 16) & 31
            hi = (w & 0xFFFF) << 16
            for j in range(i + 1, min(n, i + 48)):
                v = raw[j]
                vop = v >> 26
                rs = (v >> 21) & 31
                vt = (v >> 16) & 31
                imm = v & 0xFFFF
                if rs == rt and vop in (9, 0x23, 0x24, 0x25, 0x21, 0x20, 0x2B, 0x28, 0x29, 0x27, 0x37, 0x1E, 0x31, 0x39):
                    out[(i, j)] = (hi + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
                elif rs == rt and vop == 0xD:
                    out[(i, j)] = hi | imm
                # the register is overwritten: stop (a use that also writes it is still counted above)
                if vop in (9, 0xD, 0x23, 0x24, 0x25, 0xF, 0x21, 0x20, 0x37, 0x1E) and vt == rt:
                    break
                if vop == 0 and (v & 0x3F) not in (8, 9, 0x18, 0x19, 0x1A, 0x1B) and ((v >> 11) & 31) == rt:
                    break
                if vop == 0 and (v & 0x3F) == 8:        # jr: end of the straight-line code
                    break
        elif op == 9 and gp and ((w >> 21) & 31) == 28:
            imm = w & 0xFFFF
            out[(i, i)] = (gp + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
    return out


class Index:
    """K-gram index of a build's code. Unique grams only (value -1 for repeats)."""

    def __init__(self, img):
        self.img = img
        self.mods = []
        self.h = {}
        for si, space in enumerate(s for s in img.mod if s in ("main", "game", "lobby", "select", "yn", "sub_main")):
            raw = code_words(img, space)
            mw = [mask(x) for x in raw]
            self.mods.append((space, raw, mw))
            h = self.h
            for i in range(len(mw) - K):
                key = hash(tuple(mw[i:i + K]))
                h[key] = (si << 24) | i if key not in h else -1
        log("index %s: %d grams" % (img.tag, len(self.h)))


# ---------------------------------------------------------------- JP side data
def jp_symbols():
    funcs, objs = [], []
    for r in csv.DictReader(open(os.path.join(ROOT, "docs/survey/mh1_symbols.csv"))):
        sec = r["section"].replace(".bin", "")
        if sec not in ("main", "game", "lobby", "select", "yn"):
            continue
        a, sz = int(r["addr"], 16), int(r["size"])
        if r["type"] == "FUNC" and sz >= 8:
            funcs.append((sec, a, sz, r["name"]))
        elif r["type"] == "OBJECT" and sz > 0:
            objs.append((sec, a, sz, r["name"]))
    return funcs, objs


def jp_strings(img):
    """{(space, va): bytes} the Shift-JIS strings text_dump finds (same rules, so the ids agree)"""
    out = {}
    va0, seg = img.mod["main"]
    for off, s in td.runs(seg, 1, td.POOL[0] - va0, min(td.POOL[1] - va0, len(seg)), 2):
        out[("main", va0 + off)] = s
    for sp in ("game", "lobby", "select", "yn"):
        va, d = img.mod[sp]
        for off, s in td.runs(d, 1, 0, None, 4):
            out[(sp, va + off)] = s
    return out


def g_strings(img):
    """heuristic scan of a build with no symbols: Shift-JIS runs in main's data part and in the overlays"""
    out = {}
    va0, seg = img.mod["main"]
    lo = 0x240000
    for off, s in td.runs(seg, 1, lo - va0, len(seg), 2):
        out[("main", va0 + off)] = s
    for sp in img.mod:
        if sp == "main" or sp in ("nethttp",):
            continue
        va, d = img.mod[sp]
        for off, s in td.runs(d, 1, 0, None, 4):
            out[(sp, va + off)] = s
    return out


# ---------------------------------------------------------------- pairing by code
def pair_code(jp, jpi, other, funcs):
    """Vote pairs (jp space, jp addr) -> {(space, addr): count} through matched/aligned code."""
    idx = Index(other)
    modinfo = {s: (raw, mw) for s, raw, mw in idx.mods}
    spaces = [s for s, _r, _m in idx.mods]
    jrefs = {}
    pairs = collections.defaultdict(collections.Counter)
    stat = collections.Counter()
    gpj = jp.gp
    for sec, a, sz, name in funcs:
        va, b = jp.mod[sec]
        o = a - va
        if o < 0 or o + sz > len(b):
            continue
        n = sz // 4
        fw = list(struct.unpack_from("<%dI" % n, b, o))
        if n < K + 2:
            continue
        mf = [mask(x) for x in fw]
        votes = collections.Counter()
        hits = []
        for k in range(n - K + 1):
            p = idx.h.get(hash(tuple(mf[k:k + K])))
            if p is not None and p >= 0:
                si, pos = p >> 24, p & 0xFFFFFF
                votes[(si, pos - k)] += 1
                hits.append((k, si, pos))
        if not votes:
            stat["nomatch"] += 1
            continue
        (si, diag), v = votes.most_common(1)[0]
        if v < 2 and n > 12:
            stat["weak"] += 1
            continue
        space, traw, tmw = idx.mods[si]
        # map JP word index -> other word index on anchors near the winning diagonal
        wmap = {}
        for k, hsi, pos in hits:
            if hsi == si and abs(pos - k - diag) <= max(8, n // 2):
                for t in range(K):
                    if mf[k + t] == tmw[pos + t]:
                        wmap[k + t] = pos + t
        # fill gaps of equal size between mapped words
        ks = sorted(wmap)
        for x, y in zip(ks, ks[1:]):
            if y - x > 1 and wmap[y] - wmap[x] == y - x:
                for t in range(x + 1, y):
                    if mf[t] == tmw[wmap[x] + (t - x)]:
                        wmap[t] = wmap[x] + (t - x)
        stat["func_exact" if len(wmap) == n else "func_partial"] += 1
        jr = refs_of(fw, a, gpj)
        lo = min(wmap.values())
        hi = max(wmap.values()) + 1
        tr = refs_of(traw[lo:hi], 0, other.gp)
        for (i, ju), ja in jr.items():
            if i not in wmap or ju not in wmap:
                continue
            key = (wmap[i] - lo, wmap[ju] - lo)
            if key not in tr:
                continue
            xa = tr[key]
            js = jp.resolve(ja, sec)
            xs = other.resolve(xa, space)
            if js is None or xs is None:
                continue
            pairs[(js, ja)][(xs, xa)] += 1
    log("code pairing %s: %s, %d JP addresses" % (other.tag, dict(stat), len(pairs)))
    return pairs


# ---------------------------------------------------------------- pointer tables
def jp_cells(jp, S, objs):
    """{(space, obj name): (obj addr, [(offset, jp string key)])}: the pointer cells of each data object that
    point at a known JP string"""
    byspace = collections.defaultdict(list)
    for sec, a, sz, n in objs:
        byspace[sec].append((a, sz, n))
    out = {}
    for sp in byspace:
        byspace[sp].sort()
        starts = [x[0] for x in byspace[sp]]
        base, b = jp.mod[sp]
        for o in range(0, len(b) - 3, 4):
            v, = struct.unpack_from("<I", b, o)
            if v < 0x100000:
                continue
            tsp = jp.resolve(v, sp)
            if tsp is None or (tsp, v) not in S:
                continue
            a = base + o
            k = bisect.bisect_right(starts, a) - 1
            if k >= 0 and byspace[sp][k][0] <= a < byspace[sp][k][0] + byspace[sp][k][1]:
                oa, _sz, name = byspace[sp][k]
                out.setdefault((sp, name), (oa, []))[1].append((a - oa, (tsp, v)))
    return out


def x_string_ok(img, space, v):
    """is v (a pointer word seen in module `space`) the start of a plausible text string?"""
    sp = img.resolve(v, space)
    if sp is None:
        return None
    s = img.cstr(sp, v, 2000)
    if not s:
        return None
    bad = sum(1 for c in s if c < 0x20 and c not in (9, 10))
    if bad:
        return None
    return sp


def x_ptr_cells(img, space):
    """positions (VA) of data words in module `space` that are plausible string pointers"""
    va, b = img.mod[space]
    start = 0x252000 - va if space == "main" else img.text_end.get(space, va) - va
    out = {}
    for o in range(max(0, start), len(b) - 3, 4):
        v, = struct.unpack_from("<I", b, o)
        if v >= 0x100000:
            sp = x_string_ok(img, space, v)
            if sp:
                out[va + o] = (sp, v)
    return out


class ObjMap:
    """JP data objects found in another build. Evidence, strongest first: name (G symbols), code reference
    (matched code that loads the object's address), pointer graph (a parent table already found holds a pointer
    to it), layout (same word classes in the same layout as the neighbour). Every candidate is checked by the
    word-class profile of the whole object before it is accepted."""

    def __init__(self, jp, other, S, objs, pairs, byname=None):
        self.jp, self.o, self.S, self.objs = jp, other, S, objs
        self.byspace = collections.defaultdict(list)
        for sec, a, sz, n in objs:
            self.byspace[sec].append((a, sz, n))
        for sp in self.byspace:
            self.byspace[sp].sort()
        self.starts = {sp: [x[0] for x in v] for sp, v in self.byspace.items()}
        self.info = {(sec, n): (a, sz) for sec, a, sz, n in objs}
        self.xs = {}                 # cache of x_string_ok
        self.found = {}              # (sp, name) -> (xspace, delta, how)
        self.votes = collections.defaultdict(collections.Counter)
        self.byname = byname or {}
        for (js, ja), c in pairs.items():
            t = self.obj_at(js, ja)
            if t:
                for (xs, xa), n in c.items():
                    self.votes[t][(xs, xa - ja, "code")] += n

    def S_key(self, sp, v):
        return (self.jp.resolve(v, sp), v)

    def obj_at(self, sp, va):
        v = self.byspace.get(sp)
        if not v:
            return None
        k = bisect.bisect_right(self.starts[sp], va) - 1
        if k >= 0 and v[k][0] <= va < v[k][0] + max(v[k][1], 1):
            return (sp, v[k][2])
        return None

    def jp_class(self, sp, v):
        if v == 0:
            return "Z"
        if v >= 0x100000:
            ts = self.jp.resolve(v, sp)
            if ts and (ts, v) in self.S:
                return "S"
            if ts:
                return "P"
        return "I"

    def x_class(self, sp, v):
        if v == 0:
            return "Z"
        if v >= 0x100000:
            ck = (sp, v)
            if ck not in self.xs:
                self.xs[ck] = x_string_ok(self.o, sp, v)
            if self.xs[ck]:
                return "S"
            if self.o.resolve(v, sp):
                return "P"
        return "I"

    def profile(self, key, xs, delta):
        """(agreeing words, words) between the JP object and the other build's words at +delta"""
        a, sz = self.info[key]
        sp = key[0]
        base, b = self.jp.mod[sp]
        n = min(sz // 4, (len(b) - (a - base)) // 4)
        xb, xbuf = self.o.mod[xs]
        agree = tot = 0
        for k in range(n):
            jv, = struct.unpack_from("<I", b, a - base + 4 * k)
            xo = a + delta + 4 * k - xb
            if not 0 <= xo <= len(xbuf) - 4:
                return 0, n
            xv, = struct.unpack_from("<I", xbuf, xo)
            jc, xc = self.jp_class(sp, jv), self.x_class(xs, xv)
            tot += 1
            if jc == xc or (jc in "SP" and xc in "SP"):
                agree += 1
                if jc == "I" and jv != xv:
                    agree -= 0.5
        return agree, tot

    def accept(self, key, xs, delta, how, need=None):
        if need is None:
            need = 0.8 if how in ("name", "code", "graph") else 0.95
        if key in self.found:
            return False
        a, sz = self.info[key]
        if sz < 4:
            return False
        ag, tot = self.profile(key, xs, delta)
        if tot and ag >= need * tot:
            self.found[key] = (xs, delta, how)
            return True
        return False

    def run(self):
        keys = set(self.votes) | set(self.byname)
        for key in sorted(keys, key=lambda k: -sum(self.votes[k].values()) if k in self.votes else 0):
            if key in self.found or key not in self.info:
                continue
            cands = collections.defaultdict(lambda: [0, 0])      # (xs, delta) -> [name?, code votes]
            if key in self.byname:
                cands[tuple(self.byname[key])][0] = 1
            for (xs, d, _h), n in self.votes.get(key, collections.Counter()).items():
                cands[(xs, d)][1] += n
            ranked = sorted(cands.items(), key=lambda kv: (-(kv[1][0] + (kv[1][1] > 0)), -kv[1][1]))
            for (xs, d), (nm, cv) in ranked[:3]:
                if nm and cv:
                    need, how = 0.25, "name+code"
                elif cv >= 2:
                    need, how = 0.25, "code"
                elif cv:
                    need, how = 0.5, "code"
                else:
                    need, how = 0.6, "name"
                if self.accept(key, xs, d, how, need):
                    break
        for rnd in range(6):
            before = len(self.found)
            self.graph()
            self.layout()
            if len(self.found) == before:
                break
        return self.found

    def graph(self):
        """follow pointer words of found objects to the objects they point at"""
        queue = list(self.found)
        seen = set(queue)
        while queue:
            key = queue.pop()
            xs, delta, _h = self.found[key]
            a, sz = self.info[key]
            sp = key[0]
            base, b = self.jp.mod[sp]
            xb, xbuf = self.o.mod[xs]
            for k in range(min(sz // 4, (len(b) - (a - base)) // 4)):
                jv, = struct.unpack_from("<I", b, a - base + 4 * k)
                if jv < 0x100000:
                    continue
                ts = self.jp.resolve(jv, sp)
                t = self.obj_at(ts, jv) if ts else None
                if not t or t in self.found:
                    continue
                xo = a + delta + 4 * k - xb
                if not 0 <= xo <= len(xbuf) - 4:
                    continue
                xv, = struct.unpack_from("<I", xbuf, xo)
                xts = self.o.resolve(xv, xs)
                if not xts:
                    continue
                if self.accept(t, xts, xv - jv, "graph"):
                    queue.append(t)

    def layout(self):
        """an object between two found neighbours of the same shift takes that shift if its profile fits"""
        for sp, lst in self.byspace.items():
            keys = [(sp, n) for _a, _s, n in lst]
            for i, key in enumerate(keys):
                if key in self.found:
                    continue
                a, sz = self.info[key]
                if sz < 48:
                    continue
                prev = next((self.found[keys[j]] for j in range(i - 1, -1, -1) if keys[j] in self.found), None)
                nxt = next((self.found[keys[j]] for j in range(i + 1, len(keys)) if keys[j] in self.found), None)
                if prev and nxt and prev[:2] == nxt[:2]:
                    self.accept(key, prev[0], prev[1], "layout", 0.97)
                    continue
                # directly adjacent neighbour (gap of padding only): same shift
                for j in (i - 1, i + 1):
                    if 0 <= j < len(keys) and keys[j] in self.found and sz >= 24:
                        na, nsz = self.info[keys[j]]
                        gap = (a - (na + nsz)) if j < i else (na - (a + sz))
                        if 0 <= gap <= 16:
                            f = self.found[keys[j]]
                            if self.accept(key, f[0], f[1], "adjacent", 0.97):
                                break

    def pairs(self, cells):
        """{jp string key: Counter(other string key)} from the found objects' string cells; rows for the report"""
        out = collections.defaultdict(collections.Counter)
        rows = []
        for key, (oa, lst) in cells.items():
            if key not in self.found:
                rows.append((key, len(lst), 0, "unfound"))
                continue
            xs, d, how = self.found[key]
            xb, xbuf = self.o.mod[xs]
            n = 0
            for off, jk in lst:
                xo = oa + off + d - xb
                if not 0 <= xo <= len(xbuf) - 4:
                    continue
                xv, = struct.unpack_from("<I", xbuf, xo)
                sp = self.o.resolve(xv, xs) if xv >= 0x100000 else None
                if sp and self.x_class(xs, xv) == "S":
                    out[jk][(sp, xv)] += 1
                    n += 1
            rows.append((key, len(lst), n, how))
        return out, rows


# ---------------------------------------------------------------- string pool order
def ascii_chunks(img, space, lo, hi):
    """NUL-terminated chunks of printable text in [lo, hi) of a module: [(va, bytes)] (English builds)"""
    base, b = img.mod[space]
    out = []
    i = max(lo - base, 0)
    end = min(hi - base, len(b))
    while i < end:
        if b[i] == 0:
            i += 1
            continue
        j = b.find(b"\0", i, end + 1)
        if j < 0:
            break
        s = b[i:j]
        if all(c >= 0x20 or c in (9, 10) for c in s):
            out.append((base + i, s))
        i = j + 1
    return out


def lis_pairs(prs):
    """longest increasing subsequence of (jp index, x index) pairs, both increasing"""
    prs = sorted(prs)
    tails, idx, prev = [], [], [-1] * len(prs)
    for k, (_a, x) in enumerate(prs):
        p = bisect.bisect_left([prs[t][1] for t in tails], x)
        if p == len(tails):
            tails.append(k)
        else:
            tails[p] = k
        prev[k] = tails[p - 1] if p else -1
    out = []
    k = tails[-1] if tails else -1
    while k >= 0:
        out.append(prs[k])
        k = prev[k]
    return out[::-1]


def pool_fill(jp, S, pairs1, other, span=0x4000):
    """Between paired strings of one module, strings in pool order. pairs1: {jp key: x key} already known.
    Both builds' pools are cut into NUL-terminated chunks (any text, ASCII-only ones too); where two paired
    strings enclose the same number of chunks on both sides, they correspond one to one.
    Returns {jp key: x key} for translatable JP strings (those in S) found that way."""
    out = {}
    stat = collections.Counter()
    for sp in ("main", "game", "lobby", "select", "yn"):
        for xsp in sorted(set(v[0] for k, v in pairs1.items() if k[0] == sp)):
            prs = [(k[1], v[1]) for k, v in pairs1.items() if k[0] == sp and v[0] == xsp]
            if len(prs) < 2:
                continue
            jl = ascii_chunks(jp, sp, min(a for a, _ in prs) - span, max(a for a, _ in prs) + span)
            xl = ascii_chunks(other, xsp, min(b for _, b in prs) - span, max(b for _, b in prs) + span)
            ji = {va: i for i, (va, _b) in enumerate(jl)}
            xi = {va: i for i, (va, _b) in enumerate(xl)}
            anchors = lis_pairs([(ji[a], xi[b]) for a, b in prs if a in ji and b in xi])
            for (j0, x0), (j1, x1) in zip(anchors, anchors[1:]):
                nj, nx = j1 - j0 - 1, x1 - x0 - 1
                if nj > 0 and nj == nx:
                    for t in range(1, nj + 1):
                        key = (sp, jl[j0 + t][0])
                        if key in S:
                            out[key] = (xsp, xl[x0 + t][0])
                            stat["fill_ok"] += 1
                elif nj > 0:
                    stat["gap_mismatch_chunks"] += nj
                    stat["gaps_mismatch"] += 1
    log("pool fill:", dict(stat))
    return out


def g_name_map(img, objs):
    """{(jp section, name): (g space, delta)} from config/mhg/symbols (MH1 names at G addresses)"""
    jpa = {n: (sec, a) for sec, a, sz, n in objs}
    out = {}
    for sp in ("main", "sub_main", "game", "lobby", "select", "yn"):
        p = os.path.join(ROOT, "config", "mhg", "symbols", sp + ".txt")
        if not os.path.exists(p):
            continue
        for line in open(p):
            m = re.match(r"^(\S+) = 0x([0-9A-Fa-f]+);", line)
            if not m or m.group(1) not in jpa:
                continue
            sec, a = jpa[m.group(1)]
            out[(sec, m.group(1))] = (sp, int(m.group(2), 16) - a)
    return out


# ---------------------------------------------------------------- tables as rows, row alignment
CONV = re.compile(rb"%[-+ #0-9.]*[a-zA-Z]")


def conv_sig(b):
    return tuple(sorted(CONV.findall(b)))


def digits_sig(b, sjis):
    """the ASCII digit runs of a string (full-width digits too for Shift-JIS text)"""
    if sjis:
        t = b.decode("cp932", "replace")
        t = "".join(chr(ord(c) - 0xFEE0) if "０" <= c <= "９" else c for c in t)
    else:
        t = b.decode("latin-1")
    return tuple(re.findall(r"\d+", t))


def gcd(a, b):
    while b:
        a, b = b, a % b
    return a


class Table:
    """rows of one data object (JP) or of the other build's object at a shift"""

    def __init__(self, om, key):
        self.om, self.key = om, key
        a, sz = om.info[key]
        sp = key[0]
        base, b = om.jp.mod[sp]
        n = min(sz // 4, (len(b) - (a - base)) // 4)
        words = struct.unpack_from("<%dI" % n, b, a - base)
        cl = [om.jp_class(sp, w) for w in words]
        cellw = [i for i, c in enumerate(cl) if c == "S"]
        if len(cellw) >= 2:
            d = 0
            for x, y in zip(cellw, cellw[1:]):
                d = gcd(d, y - x)
            self.R = max(d, 1)
        else:
            self.R = n or 1
        R = self.R
        self.nrows = (n + R - 1) // R
        cols = collections.Counter(i % R for i in cellw)
        self.cols = sorted(c for c, k in cols.items() if k * 2 >= self.nrows or k >= 2)
        self.words = words
        self.cl = cl
        self.rows = []
        for r in range(self.nrows):
            cells = []
            ints = []
            for c in range(R):
                i = r * R + c
                if i >= n:
                    break
                if c in self.cols:
                    cells.append(self.om.S_key(sp, words[i]) if cl[i] == "S" else None)
                elif cl[i] in ("I", "Z"):
                    ints.append(words[i])
                else:
                    ints.append(0)
            self.rows.append((tuple(ints), tuple(cells)))

    def x_rows(self, xs, xstart, cap):
        """the other build's rows from xstart: [(ints, cells)] cells = x string key or None (zero)"""
        om = self.om
        xb, xbuf = om.o.mod[xs]
        R = self.R
        rows = []
        o = xstart - xb
        while len(rows) < cap and 0 <= o and o + 4 * R <= len(xbuf):
            w = struct.unpack_from("<%dI" % R, xbuf, o)
            cells, ints, ok = [], [], True
            for c in range(R):
                cc = om.x_class(xs, w[c])
                if c in self.cols:
                    if cc == "S":
                        sp2 = om.o.resolve(w[c], xs)
                        cells.append((sp2, w[c]))
                    elif cc in ("Z", "P"):      # zero, or a pointer to an empty string / non-text data
                        cells.append(None)
                    else:
                        ok = False
                        break
                elif cc in ("I", "Z"):
                    ints.append(w[c])
                else:
                    ints.append(0)
            if not ok:
                break
            rows.append((tuple(ints), tuple(cells)))
            o += 4 * R
        while rows and self.cols and all(c is None for c in rows[-1][1]):
            rows.pop()
        return rows


def similar(a, b):
    """0..1 similarity of two row keys' int tuples (fraction of equal words)"""
    if not a and not b:
        return 1.0
    n = max(len(a), len(b))
    return sum(1 for x, y in zip(a, b) if x == y) / n


def text_sim(a, b):
    if a == b:
        return 1.0
    if not a or not b:
        return 0.0
    return difflib.SequenceMatcher(None, a, b, autojunk=False).ratio()


def align_rows(keysj, keysx, sim, thresh, moved=False):
    """exact matches by SequenceMatcher on the keys, gaps filled 1:1 where sizes agree and every pair is
    similar enough, else by a small global alignment. Returns [(j index, x index)]"""
    sm = difflib.SequenceMatcher(None, keysj, keysx, autojunk=False)
    pairs = []
    for tag, i0, i1, j0, j1 in sm.get_opcodes():
        if tag == "equal":
            pairs.extend((i0 + t, j0 + t) for t in range(i1 - i0))
        elif tag == "replace":
            na, nb = i1 - i0, j1 - j0
            if na * nb > 250000:
                continue
            # Needleman-Wunsch with similarity scores; gaps cost 0.25
            sc = [[0.0] * (nb + 1) for _ in range(na + 1)]
            bk = [[0] * (nb + 1) for _ in range(na + 1)]
            for p in range(1, na + 1):
                sc[p][0] = -0.25 * p
                bk[p][0] = 1
            for q in range(1, nb + 1):
                sc[0][q] = -0.25 * q
                bk[0][q] = 2
            for p in range(1, na + 1):
                for q in range(1, nb + 1):
                    s = sim(keysj[i0 + p - 1], keysx[j0 + q - 1])
                    m = sc[p - 1][q - 1] + (s if s >= thresh else -1.0)
                    u = sc[p - 1][q] - 0.25
                    l = sc[p][q - 1] - 0.25
                    best = max(m, u, l)
                    sc[p][q] = best
                    bk[p][q] = 0 if best == m else (1 if best == u else 2)
            p, q = na, nb
            tmp = []
            while p > 0 or q > 0:
                mv = bk[p][q] if p > 0 and q > 0 else (1 if p > 0 else 2)
                if mv == 0:
                    if sim(keysj[i0 + p - 1], keysx[j0 + q - 1]) >= thresh:
                        tmp.append((i0 + p - 1, j0 + q - 1))
                    p -= 1
                    q -= 1
                elif mv == 1:
                    p -= 1
                else:
                    q -= 1
            pairs.extend(reversed(tmp))
    if moved:
        pairs.extend(moved_rows(keysj, keysx, pairs, sim, thresh))
        pairs.sort()
    return pairs


def moved_rows(keysj, keysx, pairs, sim, thresh):
    """Rows the ordered alignment left out because the other build reordered or moved them (the US reordered
    the item ids and inserted weapon / armour rows): identical keys first (a group only when both sides hold the
    same number of copies, paired in order), then the most similar unpaired rows, mutually best, at least `thresh`."""
    pj = {p for p, _ in pairs}
    px = {q for _, q in pairs}
    uj = [i for i in range(len(keysj)) if i not in pj]
    ux = [i for i in range(len(keysx)) if i not in px]
    out = []
    gj, gx = collections.defaultdict(list), collections.defaultdict(list)
    for i in uj:
        gj[keysj[i]].append(i)
    for i in ux:
        gx[keysx[i]].append(i)
    for k, a in gj.items():
        b = gx.get(k)
        if b and len(a) == len(b):
            out.extend(zip(a, b))
    dj = {p for p, _ in out}
    dx = {q for _, q in out}
    uj = [i for i in uj if i not in dj]
    ux = [i for i in ux if i not in dx]
    if len(uj) * len(ux) <= 400000:
        best_j, best_x = {}, {}
        for i in uj:
            for q in ux:
                s = sim(keysj[i], keysx[q])
                if s >= thresh:
                    if s > best_j.get(i, (0, None))[0]:
                        best_j[i] = (s, q)
                    if s > best_x.get(q, (0, None))[0]:
                        best_x[q] = (s, i)
        for i, (s, q) in best_j.items():
            if best_x.get(q, (0, None))[1] == i:
                out.append((i, q))
    return out


SLAVES = {"item_str": "Item_data", "pit_help_item_str": "Item_data"}     # id-indexed arrays: master record table
SLAVE_OFF = {"pit_help_item_str": 24}      # leading rows (UI prompts) before item id 0 in an id-indexed array


def pair_tables(om, cells, mode, jtext, xtext, slaves=SLAVES, masters=None):
    """Pair the string cells of every found object, row by row.
    mode "text": rows are aligned by their (Japanese) text, for a Japanese other build (G);
    mode "ints": rows are aligned by their non-pointer words, for the English build (US).
    Returns ({jp key: Counter(x key)}, report rows)."""
    out = collections.defaultdict(collections.Counter)
    rows_report = []
    master_map = dict(masters or {})
    order = sorted(cells, key=lambda k: (k[1] not in slaves.values(), k))
    for key in order:
        oa, lst = cells[key]
        if key not in om.found:
            rows_report.append((key, len(lst), 0, "unfound"))
            continue
        xs, d, how = om.found[key]
        t = Table(om, key)
        xrows = t.x_rows(xs, oa + d, 3 * t.nrows + 40)
        if mode == "text":
            kj = [(tuple(jtext(c) if c else b"" for c in cr), ir) for ir, cr in t.rows]
            kx = [(tuple(xtext(c) if c else b"" for c in cr), ir) for ir, cr in xrows]

            def sim(a, b):
                ta = [text_sim(x, y) for x, y in zip(a[0], b[0])]
                tv = sum(ta) / len(ta) if ta else 1.0
                return 0.8 * tv + 0.2 * similar(a[1], b[1])
            pairs = align_rows(kj, kx, sim, 0.5)
        else:
            kj = [ir for ir, cr in t.rows]
            kx = [ir for ir, cr in xrows]
            m = slaves.get(key[1])
            if m and ("main", m) in master_map:
                pairs = master_map[("main", m)]
                off = SLAVE_OFF.get(key[1], 0)
                pairs = [(j + off, x + off) for j, x in pairs if j + off < len(kj) and x + off < len(kx)]
                pairs += [(i, i) for i in range(min(off, len(kj), len(kx)))]
            else:
                pairs = align_rows(kj, kx, similar, 0.6, moved=True)
                master_map[key] = pairs
        n = 0
        for j, x in pairs:
            for c, (jc, xc) in enumerate(zip(t.rows[j][1], xrows[x][1])):
                if jc and xc:
                    out[jc][xc] += 1
                    n += 1
        rows_report.append((key, len(lst), n, how + "/rows%d-%d" % (len(t.rows), len(xrows))))
    return out, rows_report


# ---------------------------------------------------------------- Japanese other build (G): content anchors, flat alignment
def content_votes(om, cells, jtext, xstrings, minlen=12):
    """Votes for object shifts from strings whose bytes are identical and unique in both builds (G only).
    xstrings: {(space, va): bytes} the other build's strings."""
    byc = collections.defaultdict(list)
    for k, b in xstrings.items():
        byc[b].append(k)
    jbyc = collections.defaultdict(list)
    for k in om.S:
        jbyc[jtext(k)].append(k)
    # pointer cells of the other build, by target
    xcell_of = collections.defaultdict(list)
    for sp in om.o.mod:
        if sp == "nethttp":
            continue
        for pos, (tsp, v) in x_ptr_cells(om.o, sp).items():
            xcell_of[(tsp, v)].append((sp, pos))
    jcell_of = collections.defaultdict(list)
    for key, (oa, lst) in cells.items():
        for off, jk in lst:
            jcell_of[jk].append((key, oa + off))
    n = 0
    for b, jks in jbyc.items():
        if len(b) < minlen or len(jks) != 1 or len(byc.get(b, ())) != 1:
            continue
        jk, xk = jks[0], byc[b][0]
        jc, xc = jcell_of.get(jk, []), xcell_of.get(xk, [])
        if len(jc) == 1 and len(xc) == 1:
            key, ja = jc[0]
            xs, xa = xc[0]
            om.votes[key][(xs, xa - ja, "content")] += 1
            n += 1
    log("content anchors:", n)


def flat_pairs_text(om, key, cells, jtext, xtext, x_run):
    """Align the string cells of a found object, in table order, with the other build's by their text.
    x_run: list of other-build string keys in order starting at the object. Returns [(jp key, x key)]"""
    oa, lst = cells[key]
    jl = [jk for off, jk in sorted(lst)]
    jt = [jtext(k) for k in jl]
    xt = [xtext(k) for k in x_run]
    sm = difflib.SequenceMatcher(None, jt, xt, autojunk=False)
    pairs = []
    stat = collections.Counter()
    for tag, i0, i1, j0, j1 in sm.get_opcodes():
        if tag == "equal":
            pairs.extend((i0 + t, j0 + t) for t in range(i1 - i0))
        elif tag == "replace":
            na, nb = i1 - i0, j1 - j0
            if na * nb > 160000:
                stat["skipped"] += na
                continue
            sub = align_rows(jt[i0:i1], xt[j0:j1], text_sim, 0.5)
            pairs.extend((i0 + p, j0 + q) for p, q in sub)
    # order-free pass: identical text that is unique among the still-unpaired cells on both sides
    done_j = {i for i, _ in pairs}
    done_x = {j for _, j in pairs}
    rest_x = collections.defaultdict(list)
    for j in range(len(xt)):
        if j not in done_x:
            rest_x[xt[j]].append(j)
    rest_j = collections.Counter(jt[i] for i in range(len(jt)) if i not in done_j)
    for i in range(len(jt)):
        if i not in done_j and rest_j[jt[i]] == 1 and len(rest_x.get(jt[i], ())) == 1 and len(jt[i]) >= 3:
            pairs.append((i, rest_x[jt[i]][0]))
    return [(jl[i], x_run[j]) for i, j in pairs]


def x_flat_run(om, key, cells, xs, xstart):
    """the other build's string pointers in table order from xstart, as many words as 2.5x the JP object plus slack"""
    a, sz = om.info[key]
    n = int(sz / 4 * 2.5) + 32
    xb, xbuf = om.o.mod[xs]
    out = []
    o = xstart - xb
    for k in range(n):
        if not 0 <= o + 4 * k <= len(xbuf) - 4:
            break
        v, = struct.unpack_from("<I", xbuf, o + 4 * k)
        if v >= 0x100000 and om.x_class(xs, v) == "S":
            out.append((om.o.resolve(v, xs), v))
    return out


def master_pairs(om, name, rec_words, nmax=400):
    """Row alignment of a record table that holds no strings itself (Item_data: ids shared by name tables).
    Returns {("main", name): [(jp row, x row)]} or {} when the table was not found."""
    key = ("main", name)
    if key not in om.found:
        return {}
    xs, d, _how = om.found[key]
    a, sz = om.info[key]
    base, b = om.jp.mod["main"]
    J = [struct.unpack_from("<%dI" % rec_words, b, a - base + 4 * rec_words * i) for i in range(sz // (4 * rec_words))]
    xb, xbuf = om.o.mod[xs]
    xo = a + d - xb
    X = []
    for i in range(nmax):
        if xo + 4 * rec_words * (i + 1) > len(xbuf):
            break
        X.append(struct.unpack_from("<%dI" % rec_words, xbuf, xo + 4 * rec_words * i))
    return {key: align_rows(J, X, similar, 0.7, moved=True)}


# ---------------------------------------------------------------- quests
def read_quests(disc, pat):
    afs = os.path.join(disc, "AFS_DATA.AFS")
    e, names = td.afs_entries(afs)
    out = {}
    for i, nm in enumerate(names):
        m = nm and re.match(pat, nm)
        if m:
            out[int(m.group(1))] = melt(td.afs_read(afs, *e[i]))
    return out


def cstr_at(d, o):
    e = d.find(b"\0", o)
    return d[o:e] if e >= 0 else b""


def quest_fields_jp(d):
    """[(file offset, bytes)] title, goal, failure, details of an MH1 quest file (US the same layout)"""
    if len(d) < 0x70:
        return []
    return [(o, cstr_at(d, o)) for o in struct.unpack_from("<4I", d, 0x60) if 0 < o < len(d)]


def quest_fields_g(d):
    """G quest file (0x48 header): the text strings sit one after the other from 0x68"""
    if len(d) < 0x70:
        return []
    end = struct.unpack_from("<I", d, 0x60)[0]
    o, out = 0x68, []
    while o < end:
        e = d.find(b"\0", o)
        if e < 0 or e >= end:
            break
        out.append((o, d[o:e]))
        o = e + 1
    return out[:4]


def quest_nz(d):
    t0 = struct.unpack_from("<I", d, 0x60)[0]
    n = max((t0 - 0x70) // 4, 0)
    w = struct.unpack_from("<%dI" % n, d, 0x70)
    return {i: x for i, x in enumerate(w) if x}


def quest_match_us(J, U, thresh=0.9):
    """{JP quest no: (US quest no, score)} by the non-text data of the file (same position, same value)"""
    jn = {n: quest_nz(d) for n, d in J.items()}
    un = {n: quest_nz(d) for n, d in U.items()}
    out = {}
    for n in J:
        sc = []
        for u in U:
            a, b = jn[n], un[u]
            if a and b:
                sc.append((sum(1 for i, x in a.items() if b.get(i) == x) / max(len(a), len(b)), u))
        sc.sort(reverse=True)
        if not sc or sc[0][0] < thresh:
            continue
        best = sc[0]
        for s, u in sc:
            if u == n and s >= best[0] - 0.02:
                best = (s, u)
                break
        out[n] = (best[1], best[0])
    return out


def quest_pairs_us(J, U, match):
    """{(\"quest\", N, offset): english bytes} for JP quests tied to a US file (US fields in the same order)"""
    out = {}
    for n, (u, _s) in match.items():
        fj, fu = quest_fields_jp(J[n]), quest_fields_jp(U[u])
        for (oj, bj), (ou, bu) in zip(fj, fu):
            out[("quest", n, oj)] = bu
    return out


def quest_pairs_g(J, G):
    """{JP quest key: G quest key} for G quests that share at least two identical fields with exactly one JP quest.
    Returns (pairs, equal) where equal says the field text is identical."""
    jf = {n: quest_fields_jp(d) for n, d in J.items()}
    idx = collections.defaultdict(set)
    for n, f in jf.items():
        for i, (o, b) in enumerate(f):
            if b:
                idx[(i, b)].add(n)
    pairs = {}
    gmatch = {}
    for g, d in G.items():
        f = quest_fields_g(d)
        votes = collections.Counter()
        for i, (o, b) in enumerate(f):
            if b:
                for n in idx.get((i, b), ()):
                    votes[n] += 1
        if votes:
            (n, c), = votes.most_common(1)
            if c >= 2 or (c == 1 and f and f[0][1] and len(idx.get((0, f[0][1]), ())) == 1):
                if sum(1 for _n, k in votes.items() if k == c) == 1:
                    gmatch[g] = n
                    for i, ((oj, bj), (og, bg)) in enumerate(zip(jf[n], f)):
                        pairs.setdefault(("quest", n, oj), []).append((("quest", g, og), bj == bg))
    return pairs, gmatch


# ---------------------------------------------------------------- filters, ids, output
def is_japanese(b):
    """two or more plausible Shift-JIS double-byte characters: still Japanese (or icon art), not English"""
    i = n = 0
    while i < len(b):
        if td.dbl(b, i):
            n += 1
            i += 2
        else:
            i += 1
    return n >= 2


# messages the game cuts into several lines (one pointer per line): the US build breaks them at other places, so
# pairing line by line is unreliable; left out until the lines of one message are aligned as a group
LINESPLIT = re.compile(r"^(mc_msg|ms_err|ncm_|McardMsg|yn_dialog|yn_mess|yn_help)")


def good_english(jb, xb):
    """(ok, reason) for a JP string and its candidate English text"""
    if not xb:
        return False, "empty"
    if xb == jb:
        return False, "same as JP"
    if is_japanese(xb):
        return False, "still Japanese"
    if conv_sig(jb) != conv_sig(xb):
        return False, "format mismatch"
    return True, ""


def key_id(k):
    if k[0] == "quest":
        return "quest:%d:0x%X" % (k[1], k[2])
    return "%s:0x%X" % k


def esc(b):
    return b.replace(b"\\", b"\\\\").replace(b"\n", b"\\n")


def write_table(path, entries, header):
    """entries {key: bytes}; bytes are written as they are (ASCII, a few single high bytes)"""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    order = {"main": 0, "sub_main": 1, "game": 2, "lobby": 3, "select": 4, "yn": 5, "quest": 6}
    with open(path, "wb") as f:
        for h in header:
            f.write(("# %s\n" % h).encode())
        for k in sorted(entries, key=lambda k: (order.get(k[0], 9),) + tuple(k[1:])):
            f.write(key_id(k).encode() + b" = " + esc(entries[k]) + b"\n")


# ---------------------------------------------------------------- areas for the coverage report
AREAS = [
    (r"^(item_str|pit_help_item_str|Item_|item_)", "items (names, help)"),
    (r"^(Ken_|Gun_|buki_|weapon_|gun_)", "weapons"),
    (r"^(Armor_|armor_)", "armour"),
    (r"^(Skill_|skill_)", "skills"),
    (r"^(enemy_name|em_name|monster_|mon_)", "monsters"),
    (r"^(irai_|quest_|lb_quest|lb_board|qst)", "quest board / quest text tables"),
    (r"^(npc_|talk_|tl_|eat_)", "village NPC lines and talk"),
    (r"^(tuto|tutorial)", "tutorial pages"),
    (r"^(plaza|unique_act|lm_|lb_menu|lb_set|lb_guild|lb_shop|shop_|strButton|lb_|login_)", "village / plaza / guild / shop UI"),
    (r"^(mc_|ms_|ncm_|yn_|McardMsg)", "memory card and yes/no dialogs"),
    (r"^(net_|bs_|netDialog|ng_word|kbd_|softkey|roma_)", "online, keyboard, word filter"),
    (r"^(menu_|option_|info_|pit_help_menu|main_menu|omake|edit_msg|hunter_|title)", "menus, options, HUD"),
]


def area_of_name(name):
    name = re.sub(r"_?\d+$", "", name or "")
    for pat, label in AREAS:
        if re.match(pat, name):
            return label
    return "other tables (%s)" % name if name else "code literals"


# ---------------------------------------------------------------- main
def cached(path, fn):
    import pickle
    if path and os.path.exists(path):
        try:
            return pickle.load(open(path, "rb"))
        except Exception:
            pass
    v = fn()
    if path:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        pickle.dump(v, open(path, "wb"))
    return v


def run_us(jp, us, funcs, objs, S, cells, cache):
    pr = cached(cache and cache + "_us_code.pkl", lambda: {k: dict(v) for k, v in pair_code(jp, None, us, funcs).items()})
    pr = {k: collections.Counter(v) for k, v in pr.items()}
    om = ObjMap(jp, us, S, objs, pr, None)
    om.run()
    masters = master_pairs(om, "Item_data", 4)
    out, rows = pair_tables(om, cells, "ints", lambda k: jp.cstr(*k), lambda k: us.cstr(*k), masters=masters)
    for exp, ms in EXP_TABLES.items():
        # a description table is paired per item through its record tables, not by line position
        ex, row = exp_pairs(om, cells, exp, ms)
        for jk in [k for k in out if owner_of(cells, ("main", exp), k)]:
            del out[jk]
        for jk, c in ex.items():
            out[jk] = c
        rows = [r for r in rows if r[0] != ("main", exp)] + [row]
    bad = length_check(jp, us, cells, out)
    for key in bad:
        for off, jk in cells[key][1]:
            out.pop(jk, None)
    rows = [(r[0], r[1], 0, "length check failed") if r[0] in bad else r for r in rows]
    # strings between two paired strings of the pool, same count on both sides
    p1 = {k: c.most_common(1)[0][0] for k, c in out.items()}
    fill = pool_fill(jp, S, p1, us)
    for jk, xk in fill.items():
        if jk not in out:
            out[jk] = collections.Counter({xk: 1})
    rows.append((("pool", "order"), len(fill), len(fill), "pool order"))
    return om, out, rows


def owner_of(cells, key, jk):
    return any(k == jk for _o, k in cells[key][1])


def run_g(jp, g, funcs, objs, S, cells, GS, cache):
    pr = cached(cache and cache + "_g_code.pkl", lambda: {k: dict(v) for k, v in pair_code(jp, None, g, funcs).items()})
    pr = {k: collections.Counter(v) for k, v in pr.items()}
    byname = g_name_map(g, objs)
    om = ObjMap(jp, g, S, objs, pr, byname)
    jt = lambda k: jp.cstr(*k)
    xt = lambda k: g.cstr(*k)
    content_votes(om, cells, jt, GS)
    om.run()
    out = collections.defaultdict(collections.Counter)
    rows = []
    for key in sorted(cells):
        if key not in om.found:
            rows.append((key, len(cells[key][1]), 0, "unfound"))
            continue
        xs, d, how = om.found[key]
        run = x_flat_run(om, key, cells, xs, cells[key][0] + d)
        prs = flat_pairs_text(om, key, cells, jt, xt, run)
        for j, x in prs:
            out[j][x] += 1
        rows.append((key, len(cells[key][1]), len(prs), how))
    return om, out, rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--jp", default=os.path.join(ROOT, "disc", "mh1"))
    ap.add_argument("--us", default=os.path.join(ROOT, "disc", "mh1_us"))
    ap.add_argument("--g", default=os.path.join(ROOT, "disc", "mhg"))
    ap.add_argument("-o", "--out", default=os.path.join(ROOT, "build", "text"))
    ap.add_argument("--report", help="write the coverage report (counts and ids, no text) to this file")
    ap.add_argument("--ids", help="write the list of G strings without English (ids and areas only) to this file")
    ap.add_argument("--no-cache", action="store_true")
    args = ap.parse_args()
    t0 = time.time()
    cache = None if args.no_cache else os.path.join(args.out, "cache")
    jp = Img("jp", args.jp, "SLPM_654.95")
    us = Img("us", args.us, "SLUS_208.96")
    g = Img("g", args.g, "SLPM_658.69")
    funcs, objs = jp_symbols()
    S = jp_strings(jp)
    GS = g_strings(g)
    cells = jp_cells(jp, S, objs)
    owner = {}
    for key, (oa, lst) in cells.items():
        for off, jk in lst:
            owner.setdefault(jk, key)
    jt = lambda k: jp.cstr(*k)

    # ---- US -> JP
    omu, outu, rowsu = run_us(jp, us, funcs, objs, S, cells, cache)
    en_jp = {}
    drop = collections.Counter()
    for jk, cnt in outu.items():
        xk = cnt.most_common(1)[0][0]
        xb = us.cstr(*xk)
        ow = owner.get(jk)
        if (ow and LINESPLIT.match(ow[1])) or (not ow and jk[0] == "yn"):
            drop["line-split dialog (needs per-message alignment)"] += 1
            continue
        ok, why = good_english(jt(jk), xb)
        if ok:
            en_jp[jk] = xb
        else:
            drop[why] += 1
    # quests
    Jq = read_quests(args.jp, r"m(\d{3})\.mib$")
    Uq = read_quests(args.us, r"u(\d{3})\.mib$")
    Gq = read_quests(args.g, r"m(\d{5})\.mib$")
    qm = quest_match_us(Jq, Uq)
    qp = quest_pairs_us(Jq, Uq, qm)
    nq = 0
    for k, xb in qp.items():
        jb = cstr_at(Jq[k[1]], k[2])
        ok, why = good_english(jb, xb)
        if ok:
            en_jp[k] = xb
            nq += 1
        else:
            drop["quest " + why] += 1
    log("US: %d JP strings with English (%d quest fields); dropped %s; %.0fs" % (len(en_jp), nq, dict(drop), time.time() - t0))
    hdr = ["English text table for MH1 JP, made by tools/text_map.py from your own discs. Do not share.",
           "ids are the Japanese build's (rt_text.c); source: the US disc"]
    write_table(os.path.join(args.out, "en_mh1.txt"), en_jp, hdr)

    # ---- G
    omg, outg, rowsg = run_g(jp, g, funcs, objs, S, cells, GS, cache)
    gq, gqmatch = quest_pairs_g(Jq, Gq)
    en_g = {}
    gclass = collections.Counter()
    gmap = {}                                   # jp key -> list of (g key, same text?)
    for jk, cnt in outg.items():
        for gk, _n in cnt.most_common(2):
            gmap.setdefault(jk, []).append((gk, jt(jk) == g.cstr(*gk)))
    for jk, lst in gq.items():
        gmap.setdefault(jk, []).extend(lst)
    gstat = collections.Counter()
    for jk, lst in gmap.items():
        for gk, same in lst:
            if jk not in en_jp:
                gstat["no US English for the JP string"] += 1
                continue
            if same:
                en_g[gk] = en_jp[jk]
                gstat["identical text"] += 1
            else:
                a = jt(jk) if jk[0] != "quest" else cstr_at(Jq[jk[1]], jk[2])
                b = g.cstr(*gk) if gk[0] != "quest" else cstr_at(Gq[gk[1]], gk[2])
                if text_sim(a, b) >= 0.8 and digits_sig(a, True) == digits_sig(b, True) and conv_sig(a) == conv_sig(b):
                    en_g[gk] = en_jp[jk]
                    gstat["near text (edited a little)"] += 1
                else:
                    gstat["text edited in G (left out)"] += 1
    log("G: %d G strings with English; %s; %.0fs" % (len(en_g), dict(gstat), time.time() - t0))
    hdr = ["English text table for Monster Hunter G, made by tools/text_map.py from your own discs. Do not share.",
           "ids are G's (main incl. sub_main, game, lobby, select, yn addresses; quest:N:offset of mNNNNN.mib); source: the US disc via MH1 JP"]
    # G's resident sub_main (0x4A0C80..) lies below the overlay load address and above main: the PC text layer
    # (rt_data.c: v < LB_VRAM -> main space) already treats it as part of main, so its ids are written as main:
    write_table(os.path.join(args.out, "en_mhg.txt"), {(("main",) + k[1:]) if k[0] == "sub_main" else k: v
                                                      for k, v in en_g.items()}, hdr)

    summary = dict(jp_strings=len(S), jp_en=len(en_jp), g_strings=len(GS), g_en=len(en_g), gstat=gstat, drop=drop)
    if args.report or args.ids:
        write_report(args, jp, us, g, S, GS, cells, owner, omu, rowsu, omg, rowsg, en_jp, en_g, gmap, gstat, drop,
                     Jq, Uq, Gq, qm, gqmatch, funcs, objs)
    return summary


def g_data_symbols():
    """{module: sorted [(address, name)]} MH1 names at G data addresses (config/mhg/symbols)"""
    out = {}
    for sp in ("main", "sub_main", "game", "lobby", "select", "yn"):
        p = os.path.join(ROOT, "config", "mhg", "symbols", sp + ".txt")
        lst = []
        if os.path.exists(p):
            for line in open(p):
                m = re.match(r"^(\S+) = 0x([0-9A-Fa-f]+);(.*)$", line)
                if m and "type:func" not in m.group(3):
                    lst.append((int(m.group(2), 16), m.group(1)))
        out[sp] = sorted(lst)
    return out


def write_report(args, jp, us, g, S, GS, cells, owner, omu, rowsu, omg, rowsg, en_jp, en_g, gmap, gstat, drop,
                 Jq, Uq, Gq, qm, gqmatch, funcs, objs):
    gtwin = {}
    for jk, lst in gmap.items():
        for gk, same in lst:
            gtwin.setdefault(gk, (jk, same))
    gsyms = g_data_symbols()
    gcell = {}
    for sp in g.mod:
        if sp == "nethttp":
            continue
        for pos, (tsp, v) in x_ptr_cells(g, sp).items():
            gcell.setdefault((tsp, v), (sp, pos))

    def g_area(gk):
        if gk in gtwin and gtwin[gk][0][0] != "quest":
            jo = owner.get(gtwin[gk][0])
            return area_of_name(jo[1]) if jo else "code literals"
        if gk[0] == "quest":
            return "quests"
        c = gcell.get(gk)
        if c:
            lst = gsyms.get(c[0]) or []
            k = bisect.bisect_right([a for a, _n in lst], c[1]) - 1
            if k >= 0 and c[1] - lst[k][0] < 0x8000:
                return area_of_name(lst[k][1]) + (" [G table]" if gk not in gtwin else "")
            return "unnamed G tables (%s)" % c[0]
        return "code literals (%s)" % gk[0]

    # all G strings: scanned ones plus quest fields
    gall = {}
    for gk, b in GS.items():
        gall[gk] = b
    for gn, d in Gq.items():
        for o, b in quest_fields_g(d):
            if b:
                gall[("quest", gn, o)] = b
    rows = collections.defaultdict(collections.Counter)
    notlist = []
    for gk in sorted(gall, key=lambda k: (str(k[0]),) + tuple(k[1:])):
        area = g_area(gk)
        if gk in en_g:
            st = "english"
        elif gk in gtwin:
            jk, same = gtwin[gk]
            if jk not in en_jp:
                st = "twin without US English"
            elif not same:
                st = "edited in G"
            else:
                st = "english"
        else:
            st = "G only"
        rows[area][st] += 1
        if st != "english":
            notlist.append((key_id(gk), area, st))
    # MH1 JP side
    jrows = collections.defaultdict(collections.Counter)
    for jk in S:
        o = owner.get(jk)
        area = area_of_name(o[1]) if o else "code literals"
        jrows[area]["english" if jk in en_jp else "none"] += 1
    for n, d in Jq.items():
        for o, b in quest_fields_jp(d):
            if b:
                jrows["quests"]["english" if ("quest", n, o) in en_jp else "none"] += 1

    def table(rws, cols):
        tot = collections.Counter()
        lines = ["| area | strings | official English | %s |" % " | ".join(cols), "|---|---:|---:|%s" % ("---:|" * len(cols))]
        for a in sorted(rws, key=lambda a: -sum(rws[a].values())):
            c = rws[a]
            n = sum(c.values())
            tot.update(c)
            lines.append("| %s | %d | %d (%d%%) | %s |" % (a, n, c["english"], 100 * c["english"] // max(n, 1),
                                                         " | ".join(str(c[x]) for x in cols)))
        n = sum(tot.values())
        lines.append("| **total** | **%d** | **%d (%d%%)** | %s |" % (n, tot["english"], 100 * tot["english"] // max(n, 1),
                                                                 " | ".join(str(tot[x]) for x in cols)))
        return "\n".join(lines), tot

    gtab, gtot = table(rows, ["twin without US English", "edited in G", "G only"])
    jtab, jtot = table(jrows, ["none"])
    unf_us = collections.Counter()
    for key, n, p, how in rowsu:
        if how == "unfound":
            unf_us[key[0]] += n
    unf_g = collections.Counter()
    for key, n, p, how in rowsg:
        if how == "unfound":
            unf_g[key[0]] += n
    hows = collections.Counter(v[2] for v in omu.found.values())
    howg = collections.Counter(v[2] for v in omg.found.values())
    q_exact = sum(1 for n, (u, s) in qm.items() if s >= 0.99)
    txt = ["# Text coverage: official English for MH1 JP and Monster Hunter G",
           "",
           "Made by `tools/text_map.py --report` from the three discs (disc/mh1, disc/mh1_us, disc/mhg). Counts and ids only; "
           "no game text is quoted. Re-run it to refresh the numbers (about 3 minutes, 1 GB of RAM). Ids are `space:0xADDRESS` "
           "(PS2 address) and `quest:N:0xFILEOFFSET`, as in docs/english.md.",
           "",
           "## How the English is tied to the Japanese (by structure)",
           "",
           "1. **Code reference.** Each MH1 JP function is looked up in the other build by its masked instruction n-grams "
           "(addresses and 16-bit immediates masked). Where the code is the same, the same `lui/addiu` pair loads the same "
           "object, so JP address -> other address (%d US and %d G addresses). Edited functions are aligned on the n-grams that still match." %
           (len(omu.votes), len(omg.votes)),
           "2. **Table index.** A JP pointer table is a symbol of the JP build. Its place in the other build comes from (1), "
           "from the MH1 names that tools/mhg_map.py put at G addresses, from pointers inside tables already found, or from "
           "the neighbouring table's shift; an object is accepted only if the word classes (string pointer / zero / pointer / "
           "integer) of the whole object agree with the JP layout (95% for layout guesses, 25-80% for code or name hits because "
           "the tables were edited). Rows are then paired by position, or, where rows were inserted, by their number fields (US) "
           "or by their Japanese text (G); the item-id arrays follow the Item_data record alignment.",
           "3. **Quest number.** MH1 JP quest files are tied to US files by their non-text data (%d of %d JP quests, %d with an identical "
           "data block; the village quests 131-177 and a few others were rebalanced and are left out). G quests are tied to JP "
           "quests by identical title/goal/failure/details fields (%d of %d G quests)." % (len(qm), len(Jq), q_exact, len(gqmatch), len(Gq)),
           "4. **Content anchor (G only).** A JP string at least 12 bytes long whose bytes are identical and unique in JP and in G "
           "gives its table a shift vote (3211 such anchors); nothing is paired by content alone.",
           "",
           "A pair is dropped when its printf conversions differ, when the 'English' is still Japanese, or when it equals the JP bytes.",
           "G strings whose Japanese text changed from MH1 get the English of the old text only if the change is small (text "
           "similarity >= 0.8, same digits and conversions); otherwise they are listed as *edited in G*.",
           "",
           "Objects located in the other build: US %d of %d string tables (%s); G %d (%s). Not located: US %d string cells, G %d."
           % (len([k for k in cells if k in omu.found]), len(cells), ", ".join("%s %d" % kv for kv in hows.most_common()),
              len([k for k in cells if k in omg.found]), ", ".join("%s %d" % kv for kv in howg.most_common()),
              sum(unf_us.values()), sum(unf_g.values())),
           "",
           "## MH1 JP strings with official US English",
           "",
           jtab,
           "",
           "(%d of the %d JP strings found in the executables and overlays, plus %d quest fields.)" %
           (sum(1 for k in S if k in en_jp), len(S), sum(1 for k in en_jp if k[0] == "quest")),
           "",
           "Dropped pairs: " + ", ".join("%s %d" % kv for kv in drop.most_common()) + ".",
           "",
           "## Monster Hunter G strings with official US English",
           "",
           gtab,
           "",
           "G strings are counted from a scan of main and the overlays (Shift-JIS runs, as tools/text_dump.py; a few hundred false "
           "hits are possible) plus the four text fields of each quest file. Status of the pairs between a JP string and its G twin: " +
           ", ".join("%s %d" % kv for kv in gstat.most_common()) + ".",
           "",
           "### What is left, and where it can come from",
           "",
           "- **G only** (no MH1 twin: G's new quests, items, monsters, the plaza/gathering hall code and tables): no official "
           "English exists on the US disc. Candidate source: the MHG 'English Remix' patch (Amaillo, Rev. 15; the author allows reuse). "
           "Not downloaded or used; list only.",
           "- **edited in G** and **twin without US English**: the Remix again, or the US text if a person checks it.",
           "- **Code literals** that no table points to (HUD banners, result lines): not paired in this version (the "
           "code-reference pairs reach few of them because most literals are reached through pointer cells in data).",
           "- The full list of ids without English, with area and reason: `docs/survey/mhg_text_untranslated.txt`.",
           ""]
    if args.report:
        open(args.report, "w").write("\n".join(txt))
    if args.ids:
        with open(args.ids, "w") as f:
            f.write("# G string ids without official US English: id, area, reason. Generated by tools/text_map.py --ids.\n")
            for i, a, s_ in notlist:
                f.write("%s\t%s\t%s\n" % (i, a, s_))
    log("report written")



# ---------------------------------------------------------------- description tables (runs of lines per item)
EXP_TABLES = {"weapon_exp": ["Ken_data", "Gun_data"],
              "armor_exp": ["Armor_Head_Data", "Armor_Body_Data", "Armor_Arm_Data", "Armor_Waist_Data", "Armor_Leg_Data"]}


def exp_runs_jp(cells, name):
    oa, lst = cells[("main", name)]
    byw = {o // 4: k for o, k in lst}
    out, cur, prev = [], [], None
    for w in sorted(byw):
        if prev is not None and w != prev + 1:
            out.append(cur)
            cur = []
        cur.append(byw[w])
        prev = w
    out.append(cur)
    return out


def exp_runs_x(om, key):
    xs, d, _h = om.found[key]
    a, sz = om.info[key]
    xb, xbuf = om.o.mod[xs]
    out, cur = [], []
    for i in range(int(sz / 4 * 1.6) + 16):
        o = a + d + 4 * i - xb
        if not 0 <= o <= len(xbuf) - 4:
            break
        v, = struct.unpack_from("<I", xbuf, o)
        if v >= 0x100000 and om.x_class(xs, v) == "S":
            cur.append((om.o.resolve(v, xs), v))
        else:
            if cur:
                out.append(cur)
            cur = []
    if cur:
        out.append(cur)
    return out


def exp_pairs(om, cells, exp, masters):
    """Pair the line groups of a description table through the alignment of the record tables it follows
    (blade, bowgun / five armour slots): group (slot base + row) of the JP table <-> same row of the other build.
    Returns {jp key: Counter(x key)} and a report row."""
    key = ("main", exp)
    out = collections.defaultdict(collections.Counter)
    if key not in om.found or any(("main", m) not in om.found for m in masters):
        return out, (key, len(cells[key][1]), 0, "unfound")
    jr = exp_runs_jp(cells, exp)
    xr = exp_runs_x(om, key)
    # rows of each master in the JP build, starts in the other
    tabs = [Table(om, ("main", m)) for m in masters]
    starts = [om.info[("main", m)][0] + om.found[("main", m)][1] for m in masters]
    jbase = xbase = 0
    n = 0
    for i, (m, t) in enumerate(zip(masters, tabs)):
        nj = t.nrows
        xs = om.found[("main", m)][0]
        if i + 1 < len(masters):
            nx = (starts[i + 1] - starts[i]) // (4 * t.R)
        else:
            nx = len(xr) - xbase
        xrows = t.x_rows(xs, starts[i], max(nx, 0))
        kj = [ir for ir, cr in t.rows]
        kx = [ir for ir, cr in xrows]
        for j, x in align_rows(kj, kx, similar, 0.6, moved=True):
            if jbase + j < len(jr) and xbase + x < len(xr):
                for lj, lx in zip(jr[jbase + j], xr[xbase + x]):
                    out[lj][lx] += 1
                    n += 1
        jbase += nj
        xbase += nx
    return out, (key, len(cells[key][1]), n, "runs")


# tables whose rows are tied by the number fields of their own records (name + stats): checked by the records,
# a prose-length correlation says nothing about them (English names are not as long as Japanese ones)
RECORD_ALIGNED = {"Ken_data", "Gun_data", "Armor_Head_Data", "Armor_Body_Data", "Armor_Arm_Data", "Armor_Waist_Data",
                  "Armor_Leg_Data", "monster_data"}


def length_check(jp, us, cells, out, minmean=14, minpairs=8, rmin=0.3):
    """Prose tables (mean JP length >= minmean bytes) whose paired lengths do not correlate are dropped:
    returns the set of object keys that fail"""
    bad = set()
    for key, (oa, lst) in cells.items():
        a, b = [], []
        for off, jk in lst:
            if jk in out:
                a.append(len(jp.cstr(*jk)))
                b.append(len(us.cstr(*out[jk].most_common(1)[0][0])))
        if len(a) < minpairs or sum(a) / len(a) < minmean or key[1] in EXP_TABLES or key[1] in RECORD_ALIGNED:
            continue
        n = len(a)
        ma, mb = sum(a) / n, sum(b) / n
        sa = sum((x - ma) ** 2 for x in a) ** 0.5
        sb = sum((y - mb) ** 2 for y in b) ** 0.5
        r = sum((x - ma) * (y - mb) for x, y in zip(a, b)) / (sa * sb) if sa and sb else 0
        if r < rmin:
            bad.add(key)
    return bad


if __name__ == "__main__":
    main()
