#!/usr/bin/env python3
"""mhg_dlgmap.py : writes include/lbdlg_g.h, the map of MH1 dialog message ids to G's (netDialogMessage: MH1 65 pointers at 0x616950,
G 109 entries of 8 bytes at 0x636F20, G inserted and reworded messages). Exact text match, else the most similar text.
Needs disc/mh1/split/lobby.bin and build/mhg/split/lobby.bin."""
import re, struct, difflib, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
def vram(p): return int(re.search(r'vram:\s*(0x[0-9A-Fa-f]+|\d+)', open(p).read()).group(1), 0)
b1 = vram('config/lobby.yaml'); b2 = vram('config/mhg/lobby.yaml')
d1 = open('disc/mh1/split/lobby.bin', 'rb').read(); d2 = open('build/mhg/split/lobby.bin', 'rb').read()
def cstr(d, base, a):
    o = a - base; e = d.index(b'\0', o); return d[o:e]
m1 = [cstr(d1, b1, struct.unpack_from('<I', d1, 0x616950 - b1 + 4 * i)[0]) for i in range(65)]
m2 = [cstr(d2, b2, struct.unpack_from('<I', d2, 0x636F24 - b2 + 8 * i)[0]) for i in range(109)]
mp = []
for i, s in enumerate(m1):
    ex = [j for j, t in enumerate(m2) if t == s]
    mp.append(ex[0] if ex else max(range(len(m2)), key=lambda j: difflib.SequenceMatcher(None, s, m2[j]).ratio()))
txt = ", ".join(str(x) for x in mp)
open('include/lbdlg_g.h', 'w').write('''#ifndef LBDLG_G_H
#define LBDLG_G_H
/* PS2 Monster Hunter G: dialog message numbers. G's table (netDialogMessage, G 0x636F20) has 109 entries of 8 bytes
 * {u16 kind flags, pad, char *msg} where MH1's has 65 pointers; G inserted and reworded messages. The map MH1 id -> G id was
 * made by exact text match, else the most similar text (tools/mhg_dlgmap.py; 14 of 65 are fuzzy: online / server wording). */
static const unsigned char g_dlg_map[65] = { %s };
#define NETDLG_MSG(id) (*(char **)((unsigned char *)netDialogMessage + 8 * g_dlg_map[id] + 4))
#endif
''' % txt)
