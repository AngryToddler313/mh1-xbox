#!/usr/bin/env python3
"""subnm.py TU.c NM.c OUT.c [NAME...]: copy TU.c with the raw `asm` stubs (all, or the named ones) replaced by the
near-match C of the same function from NM.c (made static when the TU's prototype is static)."""
import re, sys
tu, nm, out = sys.argv[1:4]; names = set(sys.argv[4:])
s = open(tu).read(); n = open(nm).read()
def nmfunc(name):
    m = re.search(r'^[^\n;]*\b%s\([^;{)]*\)[ \t]*\n?(?:[^{;()\n]+;[ \t]*(?:/\*[^\n]*\*/)?[ \t]*\n)*[ \t]*\{\n' % name, n, re.M)
    if not m: raise SystemExit('no C for ' + name)
    b = n.index('\n}\n', m.start()) + 3
    return n[m.start():b]
def rep(m):
    name = m.group(2)
    if names and name not in names: return m.group(0)
    f = nmfunc(name)
    if re.search(r'^static [^\n]*\b%s\(' % name, s, re.M) and not f.startswith('static'): f = 'static ' + f
    return f
pat = re.compile(r'#ifdef __MWERKS__\nasm ([^\n]*?\b(\w+)\([^\n]*)\n\{\n#include "\w+\.inc"\n\}\n#endif\n', re.M)
s2 = pat.sub(rep, s)
# prototypes of substituted functions: make them match static-ness of nothing (leave as is)
open(out, 'w').write(s2)
