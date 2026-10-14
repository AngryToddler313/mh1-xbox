#!/usr/bin/env python3
"""mhg_offdiff.py NAME... : how load/store offsets changed between MH1 functions and their G twins.

NAME is an MH1 function name or a prefix ending in '*'. The two bodies are aligned on (opcode, rs, rt); for every
aligned load/store whose offset differs the pair (MH1 offset, G offset) gets a vote. Offsets that move
consistently across functions belong to one struct (pointer-passed work areas that tools/mhg_fields.py cannot
see). Prints MH1 offset -> G offsets with vote counts and the functions that voted.
"""
import collections, csv, difflib, os, struct, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def img(path, main):
    d = open(os.path.join(ROOT, path), 'rb').read()
    return (0x100000, d) if main else (struct.unpack_from('<I', d, 8)[0], d)


def main():
    pats = sys.argv[1:]
    im1 = {m: img('disc/mh1/split/%s.bin' % m, m == 'main') for m in ('main', 'game', 'lobby', 'select', 'yn')}
    im2 = {m: img('build/mhg/split/%s.bin' % m, m == 'main') for m in ('main', 'sub_main', 'game', 'lobby', 'select', 'yn', 'nethttp')}
    votes = collections.defaultdict(collections.Counter)
    who = collections.defaultdict(set)
    for r in csv.DictReader(open(os.path.join(ROOT, 'docs/survey/mhg_map.csv'))):
        n = r['name']
        if not r['g_addr'] or r['mh1_module'] not in im1 or not n:
            continue
        if not any((n == p) or (p.endswith('*') and n.startswith(p[:-1])) for p in pats):
            continue
        b1, d1 = im1[r['mh1_module']]
        b2, d2 = im2[r['g_module']]
        a1, a2 = int(r['mh1_addr'], 16), int(r['g_addr'], 16)
        W1 = struct.unpack_from('<%dI' % (int(r['size']) // 4), d1, a1 - b1)
        W2 = struct.unpack_from('<%dI' % (int(r['g_size']) // 4), d2, a2 - b2)

        def tok(w):
            op = w >> 26
            return (op, (w >> 21) & 31, (w >> 16) & 31) if op not in (2, 3) else (op,)
        sm = difflib.SequenceMatcher(None, [tok(w) for w in W1], [tok(w) for w in W2], autojunk=False)
        for i, j, k in sm.get_matching_blocks():
            for x, y in zip(W1[i:i + k], W2[j:j + k]):
                op = x >> 26
                if (0x20 <= op <= 0x2F or 0x30 <= op <= 0x3F or op == 9) and (x & 0xFFFF) != (y & 0xFFFF):
                    o1 = x & 0xFFFF; o2 = y & 0xFFFF
                    rs = (x >> 21) & 31
                    if rs in (28, 29, 0) or op == 9:
                        continue
                    votes[o1][o2] += 1
                    who[(o1, o2)].add(n)
    for o1 in sorted(votes):
        print('0x%X -> %s' % (o1, '  '.join('0x%X(%d: %s)' % (k, v, ','.join(sorted(who[(o1, k)])[:3])) for k, v in votes[o1].most_common(3))))


main()
