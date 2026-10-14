#!/usr/bin/env python3
"""mhg_names.py NAME... : find the G address of MH1 data symbols that config/mhg/*_aliases.txt lacks.

Pairs same-named functions of asm/<mod>/text (MH1) and asm/mhg/<mod>/text (G) with equal instruction
counts; where the MH1 instruction names `NAME + off` the G instruction's operand gives G's address
(`D_XXXXXXXX + off2`, minus off). Prints `NAME = 0x...;` lines (most votes first, with the vote count)
for config/mhg/g_names.txt. Needs asm/ and asm/mhg/ (VERSION=mhg tools/rebuild.sh and tools/rebuild.sh).
"""
import collections, glob, os, re, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
syms = {}
for f in glob.glob(os.path.join(ROOT, 'config/mhg/symbols/*.txt')):
    for l in open(f):
        m = re.match(r'(\S+) = 0x([0-9A-Fa-f]+);', l)
        if m: syms[m[1]] = int(m[2], 16)
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
def gaddr(x):
    m = re.search(r'%(?:lo|gp_rel)\((\w+)(?: \+ (0x[0-9A-Fa-f]+))?\)', x)
    if not m: return None
    n, o = m[1], int(m[2], 16) if m[2] else 0
    mm = re.match(r'D_([0-9A-F]{8})$', n)
    if mm: return int(mm[1], 16) + o
    if n in syms: return syms[n] + o
def main():
    want = set(sys.argv[1:])
    A = funcs(os.path.join(ROOT, 'asm')); B = funcs(os.path.join(ROOT, 'asm/mhg'))
    votes = collections.defaultdict(collections.Counter)
    for n, a in A.items():
        b = B.get(n)
        if not b or len(a) != len(b): continue
        for (oa, xa), (ob, xb) in zip(a, b):
            if oa != ob: continue
            m = re.search(r'%(?:lo|gp_rel)\((\w+)(?: \+ (0x[0-9A-Fa-f]+))?\)', xa)
            if not m or m[1] not in want: continue
            g = gaddr(xb)
            if g is not None:
                votes[m[1]][g - (int(m[2], 16) if m[2] else 0)] += 1
    for n in sorted(want):
        c = votes.get(n)
        print('%s = 0x%08X;  // %s' % (n, c.most_common(1)[0][0], dict(c)) if c else '// %s: no vote' % n)
main()
