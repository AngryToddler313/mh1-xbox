#!/usr/bin/env python3
"""put.py TU.c NM.c NAME VARIANT.c "COMMENT": replace NAME's raw asm stub in TU.c by the C in VARIANT.c (static when
the symbol is LOCAL), make its prototype static, drop it from config/c_rawfuncs.txt and its near-match from NM.c."""
import re, sys, csv, subprocess
tu, nm, name, var, com = sys.argv[1:6]
local = any(r['name'] == name and r['bind'] == 'LOCAL' and r['type'] == 'FUNC'
            for r in csv.DictReader(open('docs/survey/mh1_symbols.csv')))
s = open(tu).read()
pat = re.compile(r'/\* %s: original bytes kept.*?\*/\n#ifdef __MWERKS__\nasm [^\n]*\n\{\n#include "%s\.inc"\n\}\n#endif\n' % (name, name), re.S)
m = pat.search(s); assert m, 'no stub'
c = open(var).read().strip('\n') + '\n'
if local:
    c = re.sub(r'^((?:#[^\n]*\n)*)(?!static)', r'\1static ', c, count=1)
    s = re.sub(r'^void %s\(' % name, 'static void %s(' % name, s, count=1, flags=re.M)
    s = re.sub(r'^(\w[\w \*]*?) %s\(([^)]*)\);' % name, lambda mm: mm.group(0) if mm.group(0).startswith('static') else 'static ' + mm.group(0), s, count=1, flags=re.M)
m = pat.search(s)
s = s[:m.start()] + ('/* %s */\n' % com if com else '') + c + s[m.end():]
open(tu, 'w').write(s)
r = [l for l in open('config/c_rawfuncs.txt') if not re.match(r'^\w+ 0x[0-9A-Fa-f]+ 0x[0-9A-Fa-f]+ %s(\s|$)' % name, l)]
open('config/c_rawfuncs.txt', 'w').write(''.join(r))
subprocess.run(['python3', 'tools/rmfn.py', nm, name])
