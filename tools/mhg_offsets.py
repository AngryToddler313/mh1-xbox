#!/usr/bin/env python3
"""mhg_offsets.py SYMBOL... : how MH1 field offsets of a global struct (game_w, quest_w, ...) moved in G.

Pairs same-named functions of asm/<mod>/text (MH1) and asm/mhg/<mod>/text (G) with equal instruction
counts, and reads the `%lo(SYM + 0xOFF)` operands of the MH1 side against the G side, which spells the
same place as `D_XXXXXXXX` or `NAME + 0xOFF`. Prints MH1 offset -> G offset with the number of votes.
Needs asm/ and asm/mhg/ (made by VERSION=mhg tools/rebuild.sh and tools/rebuild.sh).
"""
import collections, glob, os, re, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
syms = {}
for f in glob.glob(os.path.join(ROOT, 'config/mhg/symbols/*.txt')):
    for l in open(f):
        m = re.match(r'(\S+) = 0x([0-9A-Fa-f]+);', l)
        if m: syms[m[1]] = int(m[2], 16)
want = sys.argv[1:]
INS = re.compile(r'^\s*/\* [0-9A-F]+ [0-9A-F]+ [0-9A-F]+ \*/\s+(\S+)\s+(.*)$')
def funcs(base):
    out = {}
    for f in glob.glob(os.path.join(base, '*/text/*.s')):
        cur = None
        for l in open(f):
            m = re.match(r'glabel (\S+)', l)
            if m: cur = m[1]; out[cur] = []; continue
            if l.startswith('endlabel'): cur = None; continue
            m = INS.match(l)
            if m and cur: out[cur].append((m[1], m[2]))
    return out
def gaddr(tok):
    m = re.match(r'%(?:lo|hi|gp_rel)\((\w+)(?: \+ (0x[0-9A-Fa-f]+))?\)', tok)
    if not m: return None
    n, o = m[1], int(m[2], 16) if m[2] else 0
    mm = re.match(r'D_([0-9A-F]{8})$', n)
    if mm: return int(mm[1], 16) + o
    if n in syms: return syms[n] + o
    mm = re.match(r'(?:lit|jtbl)_.*_([0-9A-F]{8})$', n)
    return None
A = funcs(os.path.join(ROOT, 'asm')); B = funcs(os.path.join(ROOT, 'asm/mhg'))
votes = collections.defaultdict(collections.Counter)
for n, a in A.items():
    b = B.get(n)
    if not b or len(a) != len(b): continue
    for (oa, xa), (ob, xb) in zip(a, b):
        if oa != ob: continue
        for s in want:
            m = re.search(r'%%lo\(%s \+ (0x[0-9A-Fa-f]+)\)' % re.escape(s), xa) or re.search(r'%%lo\((%s)\)' % re.escape(s), xa)
            if not m: continue
            off = int(m[1], 16) if m[1].startswith('0x') else 0
            tb = re.search(r'%lo\([^)]*\)', xb)
            g = gaddr(tb[0]) if tb else None
            if g is not None and s in syms: votes[(s, off)][g - syms[s]] += 1
for (s, off), c in sorted(votes.items()):
    print('%s +0x%X -> %s' % (s, off, ' '.join('+0x%X(%d)' % (k, v) for k, v in c.most_common(3))))
