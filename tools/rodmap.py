#!/usr/bin/env python3
"""rodmap.py OBJ MODULE: for every .rodata section of OBJ (in section order), the original address of the data it
holds, found from the HI16/LO16 pair that references it in a byte-matched function. Prints c_files rodata lines."""
import sys, struct, csv, os
sys.path.insert(0, 'tools')
import check
obj, mod = sys.argv[1], sys.argv[2]
d = open(obj, 'rb').read()
shoff, = struct.unpack_from("<I", d, 32); shnum, shstr = struct.unpack_from("<HH", d, 48)
S = [struct.unpack_from("<10I", d, shoff + i * 40) for i in range(shnum)]
st = S[shstr]; names = d[st[4]:st[4] + st[5]]
def nm(o, t): return t[o:t.index(b"\0", o)].decode()
secname = [nm(s[0], names) for s in S]
sy = next(s for s in S if s[1] == 2); strt = S[sy[6]]; strs = d[strt[4]:strt[4] + strt[5]]
syms = []
for k in range(sy[5] // 16):
    no, val, size, info, _o, sh = struct.unpack_from("<IIIBBH", d, sy[4] + k * 16)
    syms.append((nm(no, strs), val, size, info & 0xF, sh))
funcs = [(n, v, sz, sh) for n, v, sz, t, sh in syms if t == 2]
orig = {}
for r in csv.DictReader(open('docs/survey/mh1_symbols.csv')):
    if r['type'] == 'FUNC' and r['section'] == check.SECTIONS[mod]:
        orig.setdefault(r['name'], int(r['addr'], 16))
base, img = check.module_image(mod)
rod = [i for i, n in enumerate(secname) if n == '.rodata']
found = {}
for i, s in enumerate(S):
    if s[1] != 9: continue
    tsec = s[7]
    rels = [struct.unpack_from("<II", d, s[4] + k * 8) for k in range(s[5] // 8)]
    for off, info in rels:
        typ = info & 0xFF; symi = info >> 8
        if typ != 5: continue
        n, v, sz, t, sh = syms[symi]
        if sh not in rod: continue
        f = next(((fn, fv) for fn, fv, fsz, fsh in funcs if fsh == tsec and fv <= off < fv + fsz), None)
        if not f or f[0] not in orig: continue
        a = orig[f[0]] + off - f[1]
        hi = struct.unpack_from("<I", img, a - base)[0] & 0xFFFF
        # find the LO16 partner: next addiu/lw using the same register within 8 instructions
        rt = (struct.unpack_from("<I", img, a - base)[0] >> 16) & 31
        lo = None
        for k in range(1, 12):
            w = struct.unpack_from("<I", img, a - base + 4 * k)[0]
            if (w >> 26) in (9, 0x23) and ((w >> 21) & 31) == rt:
                lo = w & 0xFFFF; lo = lo - 0x10000 if lo & 0x8000 else lo; break
        if lo is None: continue
        addr = (hi << 16) + lo
        # addend of the reloc in our object (offset into the section)
        ow = struct.unpack_from("<I", d, S[tsec][4] + off)[0] & 0xFFFF
        found.setdefault(sh, set()).add((addr, f[0]))
for k, sh in enumerate(rod):
    sz = S[sh][5]
    v = sorted(found.get(sh, []))
    if not v: print('#', k, 'size', hex(sz), 'NO REF'); continue
    a = v[0][0]
    print('%s:rodata 0x%08X 0x%08X   # %d %s' % (mod, a, a + sz, k, ','.join(sorted(set(x[1] for x in v)))))
