#!/usr/bin/env python3
"""mhg_guard.py FILE.c NAME... : wrap the named top-level functions of an MH1 C file in `#if !defined(G_PART)` ... `#endif`
(NAME:N instead wraps in `#if !defined(G_PART) || G_PART == N`, for a file the G build compiles in several parts).

For files whose G twin lacks some functions (G dropped them): a G part wrapper (src/mhg/.../x_a.c with
`#define G_PART 1` + `#include` of the file) then compiles only the functions G has, while the normal (PC / MH1)
build still gets all of them. A function ends at the first line that is exactly `}` after its start.
"""
import re
import sys

path, names = sys.argv[1], sys.argv[2:]
lines = open(path).read().split('\n')
out = []
i = 0
spec = {n.split(':')[0]: (n.split(':')[1] if ':' in n else None) for n in names}
want = set(spec)
while i < len(lines):
    m = re.match(r'^(?:[A-Za-z_][\w \*]*?[ \*])?\**(\w+)\s*\(', lines[i])
    if m and m.group(1) in want and not lines[i].rstrip().endswith(';'):
        j = i
        while lines[j] != '}':
            j += 1
        out.append('#if !defined(G_PART)' + (' || G_PART == %s' % spec[m.group(1)] if spec[m.group(1)] else ''))
        out += lines[i:j + 1]
        out.append('#endif')
        i = j + 1
        want.discard(m.group(1))
        continue
    out.append(lines[i])
    i += 1
if want:
    print('not found:', sorted(want))
    sys.exit(1)
open(path, 'w').write('\n'.join(out))
