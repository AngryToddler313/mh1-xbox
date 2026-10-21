#!/usr/bin/env python3
"""diffmap.py MODULE: differing bytes of build/mhg/MODULE.bin vs split, grouped by c_files entry (or 'asm/data')."""
import sys, collections
m = sys.argv[1]
base = next(int(l.split()[-1], 16) for l in open('config/mhg/%s.yaml' % m) if 'vram:' in l)
a = open('build/mhg/%s.bin' % m, 'rb').read(); b = open('build/mhg/split/%s.bin' % m, 'rb').read()
ents = [(int(p[1], 16), int(p[2], 16), p[0], p[3]) for p in (l.split() for l in open('config/mhg/c_files.txt')) if len(p) >= 4 and p[0].split(':')[0] == m]
ents.sort()
import bisect
starts = [e[0] for e in ents]
hits = collections.Counter(); first = {}
for i in range(min(len(a), len(b))):
    if a[i] != b[i]:
        v = base + i; k = bisect.bisect_right(starts, v) - 1
        key = '%s %s' % (ents[k][2], ents[k][3]) if k >= 0 and ents[k][0] <= v < ents[k][1] else 'outside'
        hits[key] += 1; first.setdefault(key, v)
for k, n in sorted(hits.items(), key=lambda x: first[x[0]]): print('%6d %08X %s' % (n, first[k], k))
