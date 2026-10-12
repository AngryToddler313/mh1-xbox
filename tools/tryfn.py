#!/usr/bin/env python3
"""tryfn.py BASE.c FUNC VARIANT.c...: replace FUNC's definition in BASE.c with each variant file's text and print
the number of real differing lines (realdiff) of FUNC."""
import re, sys, subprocess, os
base, func = sys.argv[1], sys.argv[2]
s = open(base).read()
m = re.search(r'^[^\n;]*\b%s\([^;{)]*\)\s*(?:[^{;()]*;\s*)*\{\n' % func, s, re.M)
a = m.start(); b = s.index('\n}\n', a) + 3
for v in sys.argv[3:]:
    t = open(v).read().strip('\n') + '\n'
    if s[a:b].startswith('static') and not t.startswith('static'): t = 'static ' + t
    out = 'build/_tryfn.c'   # kept: tools/align.py build/_tryfn.c FUNC shows the last variant's differences
    open(out, 'w').write(s[:a] + t + s[b:])
    r = subprocess.run(['python3', 'tools/check.py', '-v', out], capture_output=True, text=True).stdout.split('\n')
    n = None; on = False
    for l in r:
        mm = re.match(r'^(OK|--)  (\S+)', l)
        if mm: on = mm.group(2) == func; n = 0 if on and n is None else n; continue
        if on and l.strip().startswith('>>') and 'original calls ?' not in l: n += 1
    print(v, 'DIFF' if n is None else n)
