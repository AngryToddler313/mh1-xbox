#!/usr/bin/env python3
"""rmfn.py FILE NAME...: remove function definitions (and the comment block right above each) from FILE."""
import re, sys
p = sys.argv[1]; names = set(sys.argv[2:])
L = open(p).read().split('\n'); out = []; i = 0; done = []
while i < len(L):
    l = L[i]
    m = re.match(r'^(?:static\s+)?[A-Za-z][^;(]*?\b(\w+)\(', l)
    if m and m.group(1) in names and not l.rstrip().endswith(';'):
        j = i
        while L[j] != '}': j += 1
        # drop the comment block directly above (ends with */ on the previous non-empty line)
        k = len(out) - 1
        if k >= 0 and out[k].rstrip().endswith('*/'):
            while k >= 0 and not out[k].lstrip().startswith('/*'): k -= 1
            if k >= 0: del out[k:]
        done.append(m.group(1)); i = j + 1
        while i < len(L) and L[i].strip() == '': i += 1
        out.append(''); continue
    out.append(l); i += 1
open(p, 'w').write('\n'.join(out))
print('removed', done)
