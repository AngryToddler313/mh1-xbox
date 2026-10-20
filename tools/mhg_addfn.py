#!/usr/bin/env python3
"""mhg_addfn.py FILE.c [FUNC ...] : add functions of an already-partitioned file (G_PART guards, see mhg_parts.py) that match G now.

For every FUNC (default: every function that checks OK against G and is not covered yet) whose guard is the plain
`#if !defined(G_PART)`, give it a part of its own: guard `|| G_PART == N`, wrapper src/mhg/<dir>/<name>_pN.c, a line in
config/mhg/c_files.txt for its G range. A part that does not check OK on its own (it calls a static of another part) is undone.
Usage: python3 tools/mhg_addfn.py src/main/chat/f_chat.c [Disp_help_mess ...]
"""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mhg_parts as P

def main():
    path = sys.argv[1]
    want = set(sys.argv[2:])
    src = open(os.path.join(P.ROOT, path)).read()
    if 'G_PART' not in src:
        print('no G_PART guards: use mhg_parts.py'); return
    rows = P.check(path)
    cov = P.covered()
    base = re.sub(r'\.c$', '', os.path.basename(path))
    rel = os.path.relpath(os.path.dirname(path), 'src')
    wdir = os.path.join('src/mhg', rel)
    os.makedirs(os.path.join(P.ROOT, wdir), exist_ok=True)
    cand = [r for r in rows if r[0] == 'OK' and (not want or r[1] in want)
            and not any(lo <= r[3] and r[3] + r[4] <= hi for lo, hi in cov.get(r[2], []))]
    done = 0
    for r in sorted(cand, key=lambda r: r[3]):
        lines = open(os.path.join(P.ROOT, path)).read().split('\n')
        sp = P.spans(lines, [r[1]])
        if r[1] not in sp:
            print('  %s: not found' % r[1]); continue
        a, b = sp[r[1]]
        if lines[a].startswith('#if !defined(G_PART)'):   # spans() gave the whole guard block
            a += 1
        if lines[a - 1] != '#if !defined(G_PART)':
            print('  %s: guard is %r' % (r[1], lines[a - 1])); continue
        ns = [int(x) for x in re.findall(r'G_PART == (\d+)', '\n'.join(lines))]
        for f in os.listdir(os.path.join(P.ROOT, wdir)):
            m = re.fullmatch(re.escape(base) + r'_p(\d+)\.c', f)
            if m: ns.append(int(m[1]))
        n = max(ns + [0]) + 1
        lines[a - 1] = '#if !defined(G_PART) || G_PART == %d' % n
        open(os.path.join(P.ROOT, path), 'w').write('\n'.join(lines))
        w = os.path.join(wdir, '%s_p%d.c' % (base, n))
        open(os.path.join(P.ROOT, w), 'w').write(
            '/* G: part %d of %s (the functions of this file that match G; see tools/mhg_parts.py) */\n'
            '#define G_PART %d\n#include "../%s"\n' % (n, path, n, path))
        got = P.check(w)
        if [x[1] for x in got] != [r[1]] or got[0][0] != 'OK':
            open(os.path.join(P.ROOT, path), 'w').write('\n'.join(lines[:a - 1] + ['#if !defined(G_PART)'] + lines[a:]))
            os.remove(os.path.join(P.ROOT, w))
            print('  %s: does not check OK as its own part (%s)' % (r[1], [x[:2] for x in got])); continue
        with open(P.LIST, 'a') as fh:
            fh.write('%s 0x%08X 0x%08X mhg/%s/%s_p%d\n' % (r[2], r[3], r[3] + r[4], rel, base, n))
        print('  %s: part %d' % (r[1], n)); done += 1
    print('%s: %d added' % (path, done))

main()
