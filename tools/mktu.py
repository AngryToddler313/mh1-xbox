#!/usr/bin/env python3
"""mktu.py MODULE START END OUT.c [NMFILE...]: assemble a whole-file TU for [START,END) from the registered run
files (config/c_files.txt) plus, for unmatched functions, the given near-match files. Top-level items (declarations,
typedefs, data, functions) are split by brace depth; identical declarations are kept once. LOCAL functions get
`static`. Prints the function list with its source."""
import sys, re, csv, os
mod, lo, hi, out = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16), sys.argv[4]
nmfiles = sys.argv[5:]
sec = {'main': 'main'}.get(mod, mod + '.bin')
funcs = sorted([(int(r['addr'], 16), r['name'], r['bind']) for r in csv.DictReader(open('docs/survey/mh1_symbols.csv'))
                if r['type'] == 'FUNC' and r['section'] == sec and lo <= int(r['addr'], 16) < hi])
runs = []
for l in open('config/c_files.txt'):
    p = l.split()
    if len(p) == 4 and p[0] == mod: runs.append((int(p[1], 16), int(p[2], 16), 'src/%s/%s.c' % (mod, p[3])))
def items(path):
    s = open(path).read()
    # strip comments but keep them out of the item text
    res = []; i = 0; n = len(s); depth = 0; start = 0; cur = ''
    out_items = []
    buf = []
    j = 0
    while j < n:
        c = s[j]
        if s.startswith('/*', j):
            k = s.index('*/', j) + 2; buf.append(s[j:k]); j = k; continue
        if s.startswith('//', j):
            k = s.find('\n', j); k = n if k < 0 else k; buf.append(s[j:k]); j = k; continue
        if c == '"' or c == "'":
            k = j + 1
            while s[k] != c: k += 2 if s[k] == '\\' else 1
            buf.append(s[j:k + 1]); j = k + 1; continue
        if c == '#' and depth == 0 and (j == 0 or s[j - 1] == '\n'):
            k = j
            while True:
                e = s.find('\n', k); e = n if e < 0 else e
                if s[e - 1] == '\\': k = e + 1; continue
                break
            out_items.append(('pp', s[j:e].strip())); j = e; continue
        buf.append(c)
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                t = ''.join(buf).strip()
                # function definition if followed by no ';' (struct/union/enum/init ends with ;)
                k = j + 1
                while k < n and s[k] in ' \t': k += 1
                if k < n and (s[k] in ';,' or (re.match(r'\w', s[k]) and not re.search(r'\)\s*(?:\w[^;{]*;\s*)*\{', t))):
                    j += 1; continue
                if re.search(r'\)\s*(?:\w[^;{]*;\s*)*\{', t) and not re.match(r'^(typedef|struct|union|enum)\b', t.split('\n')[-1] if False else re.sub(r'/\*.*?\*/', '', t, flags=re.S).strip()):
                    out_items.append(('fn', t)); buf = []
        elif c == ';' and depth == 0:
            cl = re.sub(r'/\*.*?\*/', '', ''.join(buf), flags=re.S).strip()
            if re.search(r'^[\w \t\*]*\b\w+\s*\(\s*\w+(\s*,\s*\w+)*\s*\)\s*\n', cl, re.M) and not re.match(r'^(extern|typedef)\b', cl):
                j += 1; continue   # K&R parameter declarations
            out_items.append(('decl', ''.join(buf).strip())); buf = []
        j += 1
    return out_items
def fname(t):
    t2 = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
    m = re.search(r'(\w+)\s*\([^{]*\{', t2)
    return m.group(1) if m else None
src_of = {}
for a, nm, b in funcs:
    f = None
    for x, y, p in runs:
        if x <= a < y: f = p
    src_of[nm] = f
cache = {}
def get(path):
    if path not in cache: cache[path] = items(path)
    return cache[path]
raw = set(os.environ.get('TU_RAW', '').split()); rawhead = {}
addarg = set(os.environ.get('TU_ADDARG', '').split())
pp = []; decls = []; seen = set(); body = []
order = []
for a, nm, b in funcs:
    paths = [src_of[nm]] if src_of[nm] else nmfiles
    found = None
    for p in paths:
        for k, t in get(p):
            if k == 'fn' and (fname(t) == nm or fname(t) == '%s_%08X' % (nm, a)): found = (p, t); break
        if found: break
    if not found:
        print('MISSING', hex(a), nm); body.append('/* MISSING %s */' % nm); continue
    p, t = found
    if p not in order:
        order.append(p)
        for k, x in get(p):
            if k == 'pp':
                if x not in seen and not x.startswith('#include "') or (x.startswith('#include') and x not in seen):
                    if x not in seen: seen.add(x); (pp if x.startswith('#include') else decls).append(x)
            elif k == 'decl':
                key = re.sub(r'/\*.*?\*/', '', x, flags=re.S); key = re.sub(r'\s+', ' ', key).strip()
                if key not in seen: seen.add(key); decls.append(x)
    t2 = t
    if b == 'LOCAL':
        tt = re.sub(r'^(\s*(?:/\*.*?\*/\s*)*)', lambda m: m.group(1) + ('' if re.match(r'\s*(?:/\*.*?\*/\s*)*static', t, re.S) else 'static '), t, count=1, flags=re.S)
        t2 = tt
    if nm in raw:
        t3 = re.sub(r'/\*.*?\*/', '', t, flags=re.S).strip()
        head = re.sub(r'^static\s+', '', t3[:t3.index('{')].strip())
        t2 = ('/* %s: original bytes kept (near-match C in %s, linked weak by the PC build): build/raw/%s.inc,\n   config/c_rawfuncs.txt */\n'
              '#ifdef __MWERKS__\nasm %s\n{\n#include "%s.inc"\n}\n#endif' % (nm, p, nm, head, nm))
        rawhead[nm] = head
    if nm in addarg:   # called with one more argument than the definition takes: (void) -> (), else one unused int
        if re.search(r'\b%s\s*\(\s*void\s*\)' % nm, t2):
            t2 = re.sub(r'(\b%s\s*\()\s*void\s*\)' % nm, r'\1)', t2, count=1)
        else:
            t2 = re.sub(r'(\b%s\s*\([^)]*\S)\s*\)' % nm, r'\1, s32 unused_arg)', t2, count=1)
    body.append(t2)
    print('%06X %-6s %-28s %s' % (a, b[0], nm, p))
defined = {}
for t in body:
    n = fname(t) if '#ifdef __MWERKS__\nasm' not in t else None
    if n: defined[n] = t
def proto(t):
    t2 = re.sub(r'/\*.*?\*/', '', t, flags=re.S).strip()
    head = t2[:t2.index('{')].strip()
    m = re.match(r'^(.*?\b\w+\s*\()(.*?)\)\s*(.*)$', head, re.S)
    if m.group(3).strip():      # K&R
        return re.sub(r'\s+', ' ', m.group(1)) + ');'
    return re.sub(r'\s+', ' ', head) + ';'
def isproto(x):
    c = re.sub(r'/\*.*?\*/', '', x, flags=re.S).strip()
    if c.startswith('typedef') or '{' in c: return None
    m = re.match(r'^(?:extern\s+)?[\w\s\*]*?\b(\w+)\s*\(', c)
    return m.group(1) if m else None
drops = [x for x in os.environ.get('TU_DROP', '').split('|') if x]
renames = [x for x in os.environ.get('TU_RENAME', '').split() if x]
decls2 = []
protoseen = set()
for x in decls:
    n = isproto(x)
    if n and (n in defined or n in raw): continue
    if any(re.search(r, x) for r in drops): continue
    if n and n in protoseen:
        print('# dropped second declaration of %s: %s' % (n, re.sub(r'\s+', ' ', x)[:100])); continue
    if n: protoseen.add(n)
    decls2.append(x)
pp2 = [x for x in pp if x.startswith('#include')]
other_pp = [x for x in pp if not x.startswith('#include') and not re.match(r'#(define|undef) (\w+)( \2_proto_unused)?$', x)]
pre = ''.join('#define %s %s_proto_unused\n' % (r, r) for r in renames) + '\n'.join(pp2) + '\n' + ''.join('#undef %s\n' % r for r in renames)
protos = [proto(t) for n, t in defined.items() if n not in raw] + [rawhead[n] + ';' for n in rawhead]
open(out, 'w').write(pre + '\n'.join(other_pp) + '\n\n' + '\n'.join(decls2) + '\n\n/* prototypes of this file */\n' + '\n'.join(protos) + '\n\n' + '\n\n'.join(body) + '\n')
