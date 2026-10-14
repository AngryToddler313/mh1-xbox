#!/usr/bin/env python3
"""mhg_parts.py FILE.c... : register the functions of an MH1 C file that already match G, one G part per contiguous run.

For a file where some functions differ in G (or G dropped them), the functions that match are still byte-identical G
code. This guards every top-level function with `#if !defined(G_PART) || G_PART == k` (k = its run of contiguous G
addresses; functions that do not match get `#if !defined(G_PART)`), writes src/mhg/<dir>/<name>_<k>.c wrappers
(`#define G_PART k` + `#include` of the file) and appends their G ranges to config/mhg/c_files.txt (HAND section end).
The normal build (MH1, PC) never defines G_PART, so it still compiles every function.
Each part is checked with VERSION=mhg tools/check.py; a function that fails in its part (typically a static function
called across parts) is dropped from its run and the file is redone. A file is left untouched when it already has
G_PART guards or something does not add up. Run `VERSION=mhg tools/rebuild.sh MODULE` afterwards: check.py cannot see
data (a part with its own tables), so tools/mhg_cdiff.py names the parts that break the link.

Usage: python3 tools/mhg_parts.py [--dry] src/main/.../file.c ...
"""
import os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIST = os.path.join(ROOT, 'config/mhg/c_files.txt')
ENV = dict(os.environ, VERSION='mhg')


def check(path):
    r = subprocess.run(['python3', 'tools/check.py', path], cwd=ROOT, capture_output=True, text=True, env=ENV, timeout=300)
    rows = []
    for l in r.stdout.splitlines():
        m = re.match(r'(OK|--)\s+(\S+)\s+(\S+)\s+0x([0-9A-F]+)\s+(\d+) bytes', l)
        if m:
            rows.append((m[1], m[2], m[3], int(m[4], 16), int(m[5])))
        elif l.startswith('??'):
            rows.append(('??', l.split()[1], '', 0, 0))
    return rows


def covered():
    cov = {}
    for l in open(LIST):
        p = l.split()
        if len(p) == 4 and not l.startswith('#') and ':' not in p[0]:
            try:
                cov.setdefault(p[0], []).append((int(p[1], 16), int(p[2], 16)))
            except ValueError:
                pass
    return cov


def spans(lines, names):
    """name -> (first line, last line) of the first definition of each function"""
    out = {}
    for n in names:
        pat = re.compile(r'^[A-Za-z_][^;{}=]*?[ \*]%s\s*\(' % re.escape(n))
        for i, l in enumerate(lines):
            if pat.match(l) and not l.rstrip().endswith(';'):
                j = i
                while j < len(lines) and lines[j] != '}':
                    j += 1
                if j < len(lines):
                    out[n] = (i, j)
                break
    return out


def runs_of(oks):
    oks = sorted(oks, key=lambda r: (r[2], r[3]))
    runs, cur = [], []
    for r in oks:
        if cur and cur[-1][2] == r[2] and 0 <= r[3] - (cur[-1][3] + cur[-1][4]) <= 15:
            cur.append(r)
        else:
            if cur:
                runs.append(cur)
            cur = [r]
    if cur:
        runs.append(cur)
    return runs


def strip_parts(path):
    """undo an earlier run on this file: guards, wrappers, list lines (only for files whose parts this tool made)"""
    base = re.sub(r'\.c$', '', os.path.basename(path))
    rel = os.path.relpath(os.path.dirname(path), 'src')
    wdir = os.path.join(ROOT, 'src/mhg', rel)
    ws = [f for f in (os.listdir(wdir) if os.path.isdir(wdir) else []) if re.fullmatch(re.escape(base) + r'_p\d+\.c', f)]
    if not ws:
        return False
    src = open(os.path.join(ROOT, path)).read().split('\n')
    out, i = [], 0
    while i < len(src):
        if re.fullmatch(r'#if !defined\(G_PART\)( \|\| G_PART == \d+)?', src[i]):
            j = i + 1
            while src[j] != '}':
                j += 1
            if src[j + 1] != '#endif':
                raise RuntimeError('unexpected guard layout in %s' % path)
            out += src[i + 1:j + 1]
            i = j + 2
            continue
        out.append(src[i])
        i += 1
    open(os.path.join(ROOT, path), 'w').write('\n'.join(out))
    names = {'mhg/%s/%s' % (rel, f[:-2]) for f in ws}
    lines = open(LIST).read().split('\n')
    open(LIST, 'w').write('\n'.join(l for l in lines if not (len(l.split()) == 4 and l.split()[3] in names)))
    for f in ws:
        os.remove(os.path.join(wdir, f))
    return True


def process(path, dry):
    if '--redo' in sys.argv:
        strip_parts(path)
    src = open(os.path.join(ROOT, path)).read()
    if 'G_PART' in src:
        return 'has G_PART guards'
    rows = check(path)
    if not rows:
        return 'no rows'
    cov = covered()
    skipf = set()
    if os.path.exists(os.path.join(ROOT, 'config/mhg/c_skip_funcs.txt')):
        for l in open(os.path.join(ROOT, 'config/mhg/c_skip_funcs.txt')):
            q = l.split('#')[0].split()
            if len(q) == 2 and q[0] == path:
                skipf.add(q[1])
    oks = [r for r in rows if r[0] == 'OK' and r[1] not in skipf and not any(lo <= r[3] and r[3] + r[4] <= hi for lo, hi in cov.get(r[2], []))]
    if not oks:
        return 'nothing to add'
    if all(r[0] == 'OK' for r in rows) and len(runs_of(rows)) == 1:
        return 'whole file matches: use mhg_addc.py'
    names = [r[1] for r in rows]
    lines = src.split('\n')
    sp = spans(lines, names)
    if len(set(names)) != len(names) or any(n not in sp for n in names):
        return 'cannot find every function (%s)' % ','.join(n for n in names if n not in sp)[:80]
    base = re.sub(r'\.c$', '', os.path.basename(path))
    rel = os.path.relpath(os.path.dirname(path), 'src')
    wdir = os.path.join('src/mhg', rel)
    for attempt in range(int(os.environ.get('PARTS_TRIES', 4))):
        runs = runs_of(oks)
        guard = {}
        for k, run in enumerate(runs, 1):
            for r in run:
                guard[r[1]] = k
        new = list(lines)
        # insert from the bottom so indexes stay valid
        for n, (a, b) in sorted(sp.items(), key=lambda kv: -kv[1][0]):
            k = guard.get(n)
            new.insert(b + 1, '#endif')
            new.insert(a, '#if !defined(G_PART) || G_PART == %d' % k if k else '#if !defined(G_PART)')
        if dry:
            return '%d runs, %d functions' % (len(runs), len(oks))
        open(os.path.join(ROOT, path), 'w').write('\n'.join(new))
        os.makedirs(os.path.join(ROOT, wdir), exist_ok=True)
        bad = []
        for k, run in enumerate(runs, 1):
            w = os.path.join(wdir, '%s_p%d.c' % (base, k))
            open(os.path.join(ROOT, w), 'w').write(
                '/* G: part %d of %s (the functions of this file that match G; see tools/mhg_parts.py) */\n'
                '#define G_PART %d\n#include "../%s"\n' % (k, path, k, path))
            got = check(w)
            want = [r[1] for r in run]
            ok = {r[1] for r in got if r[0] == 'OK'}
            for n in want:
                if n not in ok:
                    bad.append(n)
            if [r[1] for r in got] != [r[1] for r in run] and not bad:
                bad += [n for n in want]       # part holds other functions than planned
        if os.environ.get('PARTS_DEBUG'):
            print('attempt', attempt, 'bad:', ','.join(bad))
        if not bad:
            break
        open(os.path.join(ROOT, path), 'w').write(src)
        oks = [r for r in oks if r[1] not in bad]
        for f in os.listdir(os.path.join(ROOT, wdir)):
            if f.startswith(base + '_p'):
                os.remove(os.path.join(ROOT, wdir, f))
        if not oks:
            return 'no part survives (%s)' % ','.join(bad)[:80]
    else:
        open(os.path.join(ROOT, path), 'w').write(src)
        return 'did not converge'
    with open(LIST, 'a') as fh:
        for k, run in enumerate(runs, 1):
            fh.write('%s 0x%08X 0x%08X mhg/%s/%s_p%d\n' % (run[0][2], run[0][3], run[-1][3] + run[-1][4], rel, base, k))
    return '%d parts, %d functions, %d bytes' % (len(runs), sum(len(r) for r in runs), sum(x[4] for r in runs for x in r))


def main():
    args = sys.argv[1:]
    dry = '--dry' in args
    for p in [a for a in args if not a.startswith('--')]:
        try:
            print('%-45s %s' % (p, process(p, dry)))
        except Exception as e:  # noqa
            print('%-45s ERROR %s' % (p, e))


main()
