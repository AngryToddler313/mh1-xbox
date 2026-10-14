#!/usr/bin/env python3
"""wrapg.py FILE FUNC... : wrap the definitions of FUNC in FILE with `#ifndef VERSION_MHG ... #endif` (the MH1 copy is compiled out of
the G build, where a G copy exists). Brace matching on the text, K&R and ANSI headers; leaves other lines alone; idempotent."""
import re, sys
f = sys.argv[1]
s = open(f).read()
for fn in sys.argv[2:]:
    pat = re.compile(r"^(?:[A-Za-z_][\w \*]*?[ \*])?%s\s*\(" % re.escape(fn), re.M)
    done = False
    for m in pat.finditer(s):
        a = m.start()
        # skip declarations: find the next '{' or ';' after the parens
        i = m.end()
        depth = 1
        while depth and i < len(s):
            depth += (s[i] == '(') - (s[i] == ')')
            i += 1
        j = i
        while j < len(s) and s[j] not in '{;':
            j += 1
        if j >= len(s) or s[j] == ';' and s[i:j].strip() == '':
            continue            # a prototype
        if s[j] != '{':
            # K&R parameter declarations end at the first '{'
            j = s.index('{', i)
        depth = 0
        k = j
        while True:
            depth += (s[k] == '{') - (s[k] == '}')
            k += 1
            if depth == 0:
                break
        if s[max(0, a - 25):a].count('#ifndef VERSION_MHG'):
            done = True
            break
        s = s[:a] + '#ifndef VERSION_MHG\n' + s[a:k] + '\n#endif' + s[k:]
        done = True
        break
    print('%s %s: %s' % (f, fn, 'wrapped' if done else 'NOT FOUND'))
open(f, 'w').write(s)
