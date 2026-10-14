#!/usr/bin/env python3
"""mhg_addrmap.py MODULE|all LO HI [--min N] : where do the data addresses MH1's code uses in [LO,HI) live in G?

Pairs the twin functions of MODULE (docs/survey/mhg_map.csv, any class that has a counterpart), aligns their
instruction opcodes (difflib) and, for every load/store/addiu whose base is a `lui` or gp, resolves the
absolute address on both sides. Prints MH1 address -> G address with the number of votes. This shows how a
struct (lb_sys, ...) was re-laid out: fields are separate G addresses, not lb_sys+off.
Needs disc/mh1/split/MODULE.bin and build/mhg/split/MODULE.bin (VERSION=mhg tools/rebuild.sh)."""
import collections, csv, difflib, os, re, struct, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GP1, GP2 = 0x38EB70, 0x2F2B70
MEM = {0x20, 0x21, 0x23, 0x24, 0x25, 0x27, 0x28, 0x29, 0x2B, 0x31, 0x39, 0x37, 0x3F, 0x1E, 0x1F, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x3A, 0x36, 0x3E}

def vram(path):
    return int(re.search(r'vram:\s*(0x[0-9A-Fa-f]+|\d+)', open(path).read()).group(1), 0)

class Img:
    def __init__(self, bin_, base, gp):
        self.d, self.base, self.gp = open(bin_, 'rb').read(), base, gp
    def words(self, a, n):
        o = a - self.base
        return list(struct.unpack('<%dI' % n, self.d[o:o + 4 * n]))

def sx(v):
    return v - 0x10000 if v & 0x8000 else v

def refs(words, gp, ptr=None):
    """[(index, absolute address)] for every memory access / addiu through a lui or gp base. With ptr = the
    address of a pointer global (pNet, cw), accesses through a register loaded from it give (ptr, offset)."""
    hi, out, preg = {}, [], {}
    for i, w in enumerate(words):
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if ptr is not None:
            if op in MEM and rs in preg:
                out.append((i, (ptr, sx(imm))))
            if op == 0x23 and rs == 28 and gp + sx(imm) == ptr:
                preg[rt] = 1
                continue
            if op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x27, 0x31, 0x37, 0x1E, 0x22, 0x26, 0x09, 0x0F, 0x0D, 0x0C, 0x0A, 0x0B):
                preg.pop(rt, None)
            elif op == 0:
                preg.pop((w >> 11) & 31, None)
            continue
        if op == 0x0F:
            hi[rt] = imm << 16
            continue
        if op in MEM or op == 0x09:
            if rs == 28:
                out.append((i, gp + sx(imm)))
            elif rs in hi:
                out.append((i, (hi[rs] + sx(imm)) & 0xFFFFFFFF))
            if op not in MEM or op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x27, 0x31, 0x37, 0x1E, 0x22, 0x26):
                hi.pop(rt, None)
        elif op == 0:
            hi.pop((w >> 11) & 31, None)
    return out

def tok(w):
    op = w >> 26
    return (op, w & 63) if op == 0 else op

def main():
    mod, lo, hi_ = sys.argv[1], int(sys.argv[2], 0), int(sys.argv[3], 0)
    mn = int(sys.argv[sys.argv.index('--min') + 1]) if '--min' in sys.argv else 1
    P1 = P2 = None
    if '--ptr' in sys.argv:
        nm = sys.argv[sys.argv.index('--ptr') + 1]
        def look(f):
            for l in open(os.path.join(ROOT, f)):
                m = re.match(r'%s = 0x([0-9A-Fa-f]+);' % nm, l)
                if m: return int(m[1], 16)
        P1, P2 = look('config/symbols/main.txt'), look('config/mhg/symbols/main.txt')
    imgs = {}
    def img(m):
        if m not in imgs:
            f1 = os.path.join(ROOT, 'disc/mh1/split/%s.bin' % ('main' if m == 'main' else m))
            imgs[m] = (Img(f1, vram(os.path.join(ROOT, 'config/%s.yaml' % m)), GP1),
                       Img(os.path.join(ROOT, 'build/mhg/split/%s.bin' % m), vram(os.path.join(ROOT, 'config/mhg/%s.yaml' % m)), GP2))
        return imgs[m]
    votes = collections.defaultdict(collections.Counter)
    for r in csv.DictReader(open(os.path.join(ROOT, 'docs/survey/mhg_map.csv'))):
        if (mod != 'all' and r['mh1_module'] != mod) or r['mh1_module'] != r['g_module'] or not r['g_addr']:
            continue
        if r['mh1_module'] == 'sub_main':
            continue
        A, B = img(r['mh1_module'])
        a1, s1, a2, s2 = int(r['mh1_addr'], 16), int(r['size']), int(r['g_addr'], 16), int(r['g_size'])
        w1, w2 = A.words(a1, s1 // 4), B.words(a2, s2 // 4)
        r1, r2 = dict(refs(w1, GP1, P1)), dict(refs(w2, GP2, P2))
        sm = difflib.SequenceMatcher(None, [tok(w) for w in w1], [tok(w) for w in w2], autojunk=False)
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag != 'equal':
                continue
            for k in range(i2 - i1):
                x, y = r1.get(i1 + k), r2.get(j1 + k)
                if x is not None and y is not None and (P1 is not None or lo <= x < hi_):
                    votes[x][y] += 1
    emit = '--emit' in sys.argv
    for x in sorted(votes, key=lambda k: (k if isinstance(k, tuple) else (0, k))):
        tot = sum(votes[x].values())
        if emit:     # the winner, if it has >= mn votes and > 60% of them
            y, n = votes[x].most_common(1)[0]
            if n >= mn and n * 10 > tot * 6:
                print('%08X %08X %d' % (x, y, n))
            continue
        if tot >= mn:
            if P1 is not None:
                print('%s+0x%X -> +0x%X' % (nm, x[1], 0) if False else '+0x%X -> %s' % (x[1], '  '.join('+0x%X(%d)' % (y[1], n) for y, n in votes[x].most_common(3))))
            else:
                print('%08X -> %s' % (x, '  '.join('%08X(%d)' % (y, n) for y, n in votes[x].most_common(3))))
main()
