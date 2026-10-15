#!/usr/bin/env python3
"""lbfields.py: rewrite the CNET_SYS struct in include/lbnet.h from config/lbnet_fields.txt (MH1 layout) and
config/mhg/lbnet_fields_g.txt (G layout, #ifdef MHG_PS2).
MH1 line: OFFSET TYPE NAME [COMMENT]   (OFFSET hex; TYPE is a C type, arrays as NAME[N]).
G line: NAME GOFFSET [DIM [SIZE]]  (see that file). A MH1 field with no G line goes behind the end of the G struct."""
import re, os, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SIZE = {'s8': 1, 'u8': 1, 's16': 2, 'u16': 2, 's32': 4, 'u32': 4, 'f32': 4, 'ptr': 4}
rows = []
for l in open(os.path.join(ROOT, 'config/lbnet_fields.txt')):
    l = l.rstrip('\n')
    if not l.strip() or l.startswith('#'):
        continue
    m = re.match(r'(0x[0-9A-Fa-f]+)\s+(\S+)\s+(\S+)\s*(.*)', l)
    off = int(m.group(1), 16)
    ty, name, cm = m.group(2), m.group(3), m.group(4)
    if ty.endswith('*') or ty == 'ptr':
        sz = 4
    else:
        sz = SIZE.get(ty)
    am = re.match(r'(\w+?)\[(0x[0-9A-Fa-f]+|\d+)\]$', name)
    dim = None
    if am:
        dim = int(am.group(2), 0)
        esz = SIZE.get(ty, None)
        sz = dim * (esz if esz else 1)
    ms = re.search(r'size=(0x[0-9A-Fa-f]+)', cm)
    if ms:
        sz = int(ms.group(1), 16)
    rows.append([off, ty, name, cm, sz, dim])
rows.sort(key=lambda r: r[0])


def base(n):
    return re.sub(r'\[.*', '', n)


def emit(rows, label):
    out = []
    pos = 0
    for off, ty, name, cm, sz, dim in sorted(rows, key=lambda r: r[0]):
        if off < pos:
            sys.exit('%s: overlap at 0x%X (%s)' % (label, off, name))
        if off > pos:
            out.append('    u8 _pad%03X[0x%X];' % (pos, off - pos))
        c = re.sub(r'size=\S+\s*', '', cm)
        out.append('    %s %s;  /* 0x%03X %s */' % (ty, name, off, c))
        pos = off + sz
    return out, pos


mh1, end1 = emit(rows, 'MH1')

# G layout
g = {}
for l in open(os.path.join(ROOT, 'config/mhg/lbnet_fields_g.txt')):
    l = l.split('#')[0].strip()
    if not l:
        continue
    t = l.split()
    g[t[0]] = t[1:]
grows = []
end = 0
for off, ty, name, cm, sz, dim in rows:
    b = base(name)
    if b in g:
        a = g[b]
        goff = int(a[0], 0)
        nm, nsz = name, sz
        if len(a) > 1 and a[1].startswith('='):
            nsz = int(a[1][1:], 0)          # the type is a different size in G (the include/lbnet.h typedef changes too)
        elif len(a) > 1:
            nd = int(a[1].strip('[]'), 0)
            if len(a) > 2:
                nsz = int(a[2], 0)
            else:
                nsz = sz // dim * nd
            nm = '%s[%s]' % (b, a[1].strip('[]'))
        grows.append([goff, ty, nm, cm, nsz, None])
        end = max(end, goff + nsz)
end = (end + 0x1F) & ~0xF
for off, ty, name, cm, sz, dim in rows:
    if base(name) not in g:
        a = 4 if sz >= 4 else (2 if sz == 2 else 1)
        end = (end + a - 1) & ~(a - 1)
        grows.append([end, ty, name, cm + ' [G offset unknown]', sz, None])
        end += sz
gl, endg = emit(grows, 'G')

t = open(os.path.join(ROOT, 'include/lbnet.h')).read()
new = '#ifdef MHG_PS2\ntypedef struct CNET_SYS {\n' + '\n'.join(gl) + '\n} CNET_SYS;\n#else\ntypedef struct CNET_SYS {\n' + '\n'.join(mh1) + '\n} CNET_SYS;\n#endif'
t = re.sub(r'(#ifdef MHG_PS2\n)?typedef struct CNET_SYS \{.*?\} CNET_SYS;(\n#else\ntypedef struct CNET_SYS \{.*?\} CNET_SYS;\n#endif)?', lambda m: new, t, count=1, flags=re.S)
open(os.path.join(ROOT, 'include/lbnet.h'), 'w').write(t)
print('CNET_SYS: %d fields, MH1 ends at 0x%X, G ends at 0x%X' % (len(rows), end1, endg))
