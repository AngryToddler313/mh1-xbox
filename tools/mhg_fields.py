#!/usr/bin/env python3
"""mhg_fields.py SYMBOL... : where did the fields of an MH1 global (game_w, select_w, quest_w, ...) go in G?

Aligns every MH1 function with its G twin (docs/survey/mhg_map.csv; same size, class identical/shape or edited
with sim >= 0.8), follows lui/addiu and gp-relative addressing on both sides, and for each memory access with
the same opcode/registers pairs the MH1 address with the G address. MH1 addresses inside SYMBOL are printed as
MH1 offset -> G address (+offset from the G symbol of the same name if it exists), with vote counts.
Usage: python3 tools/mhg_fields.py game_w select_w
"""
import collections, csv, difflib, os, re, struct, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SIM = float(os.environ.get('SIM', '0.8'))
GP1, GP2 = 0x38EB70, 0x2F2B70


def load_syms(pat):
    out = {}
    import glob
    for f in glob.glob(os.path.join(ROOT, pat)):
        for l in open(f):
            m = re.match(r'(\S+) = 0x([0-9A-Fa-f]+);(?:.*size:0x([0-9A-Fa-f]+))?', l)
            if m:
                out[m[1]] = (int(m[2], 16), int(m[3], 16) if m[3] else 0)
    return out


def img(path, main):
    d = open(os.path.join(ROOT, path), 'rb').read()
    return (0x100000, d) if main else (struct.unpack_from('<I', d, 8)[0], d)


def main():
    want = sys.argv[1:]
    s1 = load_syms('config/symbols/*.txt')
    s2 = load_syms('config/mhg/symbols/*.txt')
    ranges = {w: s1[w] for w in want if w in s1}
    im1 = {m: img('disc/mh1/split/%s.bin' % m, m == 'main') for m in ('main', 'game', 'lobby', 'select', 'yn')}
    im2 = {m: img('build/mhg/split/%s.bin' % m, m in ('main',)) for m in ('main', 'sub_main', 'game', 'lobby', 'select', 'yn', 'nethttp')}
    votes = collections.defaultdict(collections.Counter)
    for r in csv.DictReader(open(os.path.join(ROOT, 'docs/survey/mhg_map.csv'))):
        if not r['g_addr'] or r['mh1_module'] not in im1:
            continue
        if r['class'] not in ('identical', 'shape') and not (r['class'] == 'edited' and float(r['sim'] or 0) >= SIM):
            continue
        b1, d1 = im1[r['mh1_module']]
        b2, d2 = im2[r['g_module']]
        a1, a2 = int(r['mh1_addr'], 16), int(r['g_addr'], 16)
        n1, n2 = int(r['size']) // 4, int(r['g_size']) // 4
        W1 = struct.unpack_from('<%dI' % n1, d1, a1 - b1)
        W2 = struct.unpack_from('<%dI' % n2, d2, a2 - b2)
        if n1 == n2:
            pairs = list(zip(W1, W2))
        else:
            def tok(w):
                op = w >> 26
                return (op, (w >> 21) & 31, (w >> 16) & 31) if op not in (2, 3) else (op,)
            sm = difflib.SequenceMatcher(None, [tok(w) for w in W1], [tok(w) for w in W2], autojunk=False)
            pairs = []
            for i, j, k in sm.get_matching_blocks():
                pairs += list(zip(W1[i:i + k], W2[j:j + k])) + [(0, 1 << 31)]   # separator resets lui state
        h1, h2 = {}, {}
        for x, y in pairs:
            op = x >> 26
            if op != (y >> 26):
                h1.clear(); h2.clear(); continue
            rs, rt = (x >> 21) & 31, (x >> 16) & 31
            if op == 0x0F:
                h1[rt] = (x & 0xFFFF) << 16; h2[rt] = (y & 0xFFFF) << 16
                continue
            if (0x20 <= op <= 0x2F) or (0x30 <= op <= 0x3F) or op == 0x09:
                if rs == ((y >> 21) & 31) and rt == ((y >> 16) & 31):
                    o1 = x & 0xFFFF; o1 -= (o1 & 0x8000) << 1
                    o2 = y & 0xFFFF; o2 -= (o2 & 0x8000) << 1
                    if rs == 28:
                        v1, v2 = GP1 + o1, GP2 + o2
                    elif rs in h1 and rs in h2:
                        v1, v2 = h1[rs] + o1, h2[rs] + o2
                    else:
                        v1 = None
                    if v1 is not None:
                        for w, (sa, sz) in ranges.items():
                            if sa <= v1 < sa + max(sz, 1):
                                votes[(w, v1 - sa)][v2] += 1
                if op != 0x2B and not (0x28 <= op <= 0x2F) and not (0x38 <= op <= 0x3F):   # loads/addiu write rt
                    h1.pop(rt, None); h2.pop(rt, None)
            elif op == 0 :
                rd = (x >> 11) & 31
                h1.pop(rd, None); h2.pop(rd, None)
            elif op in (4, 5, 6, 7, 1, 2, 3):
                pass
            else:
                h1.pop(rt, None); h2.pop(rt, None)
    for (w, off), c in sorted(votes.items()):
        base2 = s2.get(w, (None,))[0]
        print('%s +0x%03X -> %s' % (w, off, '  '.join('0x%X%s(%d)' % (k, ('=+0x%X' % (k - base2)) if base2 is not None else '', v) for k, v in c.most_common(3))))


main()
