#!/usr/bin/env python3
"""carry_match.py [mhg|us] [OUT.csv]: how much of MH1's matched (byte-exact) C appears with the same
instruction shape in another build: PS2 Monster Hunter G (disc/mhg) or the US MH1 (disc/mh1_us; its
overlays must first be unpacked with `tools/mwo_unpack.py disc/mh1_us/AFS_DATA.AFS -o build/us_ovl`).
Masking as tools/mhg_match.py (addresses, jump targets, 16-bit immediates). Splits the count into
functions we have matched (config/c_files.txt minus c_rawfuncs) and the rest, and counts the functions
found at exactly one place (their name can be carried over directly). OUT.csv lists every function
(names and sizes only, no game bytes). Run from the repository root. Standard library only."""
import struct, csv, sys, collections, os, glob
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
def words(b): return list(struct.unpack('<%dI' % (len(b)//4), b[:len(b)//4*4]))
MASKLO = {8,9,0xd,0xf,0x1e,0x1f} | set(range(0x20,0x30)) | {0x31,0x35,0x39,0x3d,0x37,0x3f}
def m(w):
    op = w >> 26
    if op in (2,3): return w & 0xfc000000
    if op in MASKLO: return w & 0xffff0000
    return w
SEC2MOD = {'main':'main','game.bin':'game','lobby.bin':'lobby','select.bin':'select','yn.bin':'yn'}
mh1 = {'main': (open(R+'disc/mh1/split/main.bin','rb').read(), 0x100000)}
for k in ('game.bin','lobby.bin','select.bin','yn.bin'):
    mh1[k] = (open(R+'disc/mh1/split/'+k,'rb').read(), 0x533980)   # whole file: the MWo3 header is part of the load, code VA = base + file offset
TGT = sys.argv[1] if len(sys.argv) > 1 else 'mhg'
if TGT == 'us':
    ge = open(R+'disc/mh1_us/SLUS_208.96','rb').read(); G = {'main': ge[0x180:0x180+2724224]}; OVD = R+'build/us_ovl/*.bin'
else:
    ge = open(R+'disc/mhg/SLPM_658.69','rb').read(); G = {'main': ge[0x200:0x200+0x1ef200]}; OVD = R+'disc/mhg/overlays/*.bin'
for f in sorted(glob.glob(OVD)):
    k = os.path.basename(f)
    if k.startswith(('dnas','nethttp')): continue
    G[k] = open(f,'rb').read()[0x40:]
K = 6
gw = {}; idx = collections.defaultdict(list); graw = {}
for k,b in G.items():
    w = words(b); graw[k] = w; mw = [m(x) for x in w]; gw[k] = mw
    for i in range(len(mw)-K): idx[tuple(mw[i:i+K])].append((k,i))
ranges = []
for line in open(R+'config/c_files.txt'):
    f = line.split('#',1)[0].split()
    if f and ':' not in f[0]: ranges.append((f[0], int(f[1],0), int(f[2],0)))
raw = set()
for line in open(R+'config/c_rawfuncs.txt'):
    f = line.split('#',1)[0].split()
    if len(f) >= 4: raw.add((f[0], int(f[1],0)))
# stats[mod][matched?] = [n, bytes, found_n, found_bytes, exact_n]
st = collections.defaultdict(lambda: {True:[0,0,0,0,0], False:[0,0,0,0,0]})
gmod = collections.Counter(); uniq = collections.Counter()
out = open(sys.argv[2],'w') if len(sys.argv) > 2 else None
if out: out.write('mh1_module,name,size,matched,found,exact,target_module\n')
for r in csv.DictReader(open(R+'docs/survey/mh1_symbols.csv')):
    if r['type'] != 'FUNC' or r['section'] not in mh1 or r['class'] != 'game': continue
    sz = int(r['size']); a = int(r['addr'],16)
    if sz < 4: continue
    mod = SEC2MOD[r['section']]
    matched = (mod,a) not in raw and any(mm == mod and lo <= a < hi for mm,lo,hi in ranges)
    b, base = mh1[r['section']]; fw = words(b[a-base:a-base+sz])
    s = st[mod][matched]; s[0] += 1; s[1] += sz
    hit = None; exact = False
    if len(fw) >= K:
        mf = [m(x) for x in fw]
        for (k,i) in idx.get(tuple(mf[:K]),()):
            if gw[k][i:i+len(mf)] == mf: hit = (k,i); break
    nh = 0
    if len(fw) >= K:
        nh = sum(1 for (k,i) in idx.get(tuple(mf[:K]),()) if gw[k][i:i+len(mf)] == mf)
    if hit:
        uniq[mod] += (nh == 1)
        s[2] += 1; s[3] += sz
        exact = graw[hit[0]][hit[1]:hit[1]+len(fw)] == fw
        s[4] += exact
        if matched: gmod[(mod,hit[0])] += 1
    if out: out.write('%s,%s,%d,%d,%d,%d,%s\n' % (mod, r['name'], sz, matched, bool(hit), exact, hit[0] if hit else ''))
print('%-7s %-9s %6s %9s %6s %9s %7s %6s' % ('module','kind','funcs','bytes','found','fbytes','%bytes','exact'))
T = {True:[0]*5, False:[0]*5}
for mod in ('main','game','lobby','select','yn'):
    for mt in (True, False):
        s = st[mod][mt]
        for i in range(5): T[mt][i] += s[i]
        print('%-7s %-9s %6d %9d %6d %9d %6.1f%% %6d' % (mod, 'matched' if mt else 'unmatched', s[0], s[1], s[2], s[3], 100.0*s[3]/max(1,s[1]), s[4]))
for mt in (True, False):
    s = T[mt]
    print('%-7s %-9s %6d %9d %6d %9d %6.1f%% %6d' % ('total', 'matched' if mt else 'unmatched', s[0], s[1], s[2], s[3], 100.0*s[3]/max(1,s[1]), s[4]))
print('matched MH1 functions found in the target, by (MH1 module, target module):')
for k,v in sorted(gmod.items()): print(' ', k, v)

print('found at exactly one place (name recoverable directly):', dict(uniq), sum(uniq.values()))
