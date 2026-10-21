#!/usr/bin/env python3
"""shiftfind.py MODULE: walk build/mhg/MODULE.map; symbols named with their G address (D_XXXXXX, func_XXXXXXXX, ..._XXXXXXXX)
must sit at that address. Print each point where the offset changes, with the C objects placed since the last good symbol."""
import re, sys, signal
signal.signal(signal.SIGPIPE, signal.SIG_DFL)
m = sys.argv[1]
cur = None; since = []; lastc = None; last = 0; lastsym = None
sec = None
for l in open('build/mhg/%s.map' % m):
    h = re.match(r'^ (\.\S+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+)', l)
    if h:
        sec, obj = h.group(1), h.group(4)
        if int(h.group(3), 16) and '/src/' in obj:
            if sec != '.text': lastc = (sec, obj)
            if not since or since[-1] != (sec, obj): since.append((sec, obj))
        continue
    s = re.match(r'^\s+0x([0-9a-f]{8,16})\s+(\S+)\s*$', l)
    if not s: continue
    a, name = int(s.group(1), 16), s.group(2)
    t = re.match(r'^(?:D_|jtbl_|func_|lit_\d+_)([0-9A-Fa-f]{6,8})$', name)
    if not t or a == 0: continue
    want = int(t.group(1), 16)
    if abs(want - a) > 0x10000: continue
    d = a - want
    if d != last:
        print('%s: offset %+d -> %+d at %s 0x%X (prev ok %s); C objects since: %s' % (m, last, d, name, a, lastsym,
              sorted(set('%s %s' % (sc, o.replace('build/mhg/src/', '')) for sc, o in (since or ([lastc] if lastc else []))))[-8:]))
        last = d
    lastsym = name; since = []
