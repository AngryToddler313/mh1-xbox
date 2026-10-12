#!/usr/bin/env python3
"""tryfn.py BASE.c FUNC VARIANT.c...: replace FUNC's definition in BASE.c with each variant file's text and print
the number of real differing lines of FUNC (tools/align.py: relocations and branch shifts ignored; 0 = match)."""
import re, sys, subprocess, os
base, func = sys.argv[1], sys.argv[2]
s = open(base).read()
m = re.search(r'^[^\n;]*\b%s\([^;{)]*\)[ \t]*\n?(?:[^{;()\n]+;[ \t]*(?:/\*[^\n]*\*/)?[ \t]*\n)*[ \t]*\{\n' % func, s, re.M)
a = m.start(); b = s.index('\n}\n', a) + 3
for v in sys.argv[3:]:
    t = open(v).read().strip('\n') + '\n'
    if s[a:b].startswith('static') and not t.startswith('static'): t = 'static ' + t
    out = 'build/_tryfn_%d.c' % os.getpid()
    open(out, 'w').write(s[:a] + t + s[b:])
    r = subprocess.run(['python3', 'tools/check.py', out], capture_output=True, text=True).stdout
    st = [l for l in r.split('\n') if re.match(r'^(OK|--)  %s ' % re.escape(func), l)]
    if not st:
        print(v, 'DIFF'); continue
    if st[0].startswith('OK'):
        print(v, 0); continue
    a2 = subprocess.run(['python3', 'tools/align.py', out, func], capture_output=True, text=True).stdout
    n = sum(1 for l in a2.split('\n') if l.startswith('   ') and '(reloc)' not in l)
    print(v, n if n else 'calls')   # 'calls': only call-name differences (check.py cannot name overlay targets)
import shutil
shutil.move(out, 'build/_tryfn.c')   # kept: tools/align.py build/_tryfn.c FUNC shows the last variant's differences
