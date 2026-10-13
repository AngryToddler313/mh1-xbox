#!/usr/bin/env python3
"""mhg_addc.py FILE.c... : register C files in config/mhg/c_files.txt when VERSION=mhg check.py says every
function matches and the file covers a contiguous G range. The MH1 entry's data slots (jump tables) are moved
to G through the symbol at the slot start. New lines go under the HAND marker, which tools/mhg_cfiles.py keeps.
Prints OK/FAIL per file; run `VERSION=mhg python3 tools/build.py` afterwards (check.py cannot see addends/slots).
Usage: python3 tools/mhg_addc.py src/main/pl/pl70.c ...
"""
import os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mhg_cfiles
ROOT = mhg_cfiles.ROOT
HAND = mhg_cfiles.HAND
LIST = os.path.join(ROOT, 'config/mhg/c_files.txt')

def add(path):
    rel = os.path.relpath(os.path.abspath(path), os.path.join(ROOT, 'src'))
    name = re.sub(r'\.c$', '', rel)
    cur = open(LIST).read()
    if re.search(r'\s%s$' % re.escape(name), cur, re.M):
        return 'already listed'
    env = dict(os.environ, VERSION='mhg')
    r = subprocess.run(['python3', 'tools/check.py', path], cwd=ROOT, capture_output=True, text=True, env=env)
    lines = [l for l in r.stdout.splitlines() if l[:2] in ('OK', '--', '??')]
    if not lines or r.returncode:
        return 'FAIL ' + ' | '.join(l[:60] for l in lines if l[:2] != 'OK')[:200]
    mods = {l.split()[2] for l in lines}
    if len(mods) != 1:
        return 'FAIL spans modules %s' % sorted(mods)
    mod = mods.pop()
    r = subprocess.run(['python3', 'tools/check.py', path, '--add', mod, name], cwd=ROOT, capture_output=True, text=True, env=env)
    if 'added' not in r.stdout:
        return 'FAIL ' + (r.stdout + r.stderr).strip().splitlines()[-1][:160]
    # check.py appended the text line at the end of the file; keep it there (after the HAND marker) and add slots
    names, gsym = mhg_cfiles.mh1_names(), mhg_cfiles.g_addrs()
    extra = []
    for l in open(os.path.join(ROOT, 'config/c_files.txt')):
        f = l.split('#', 1)[0].split()
        if len(f) == 4 and ':' in f[0] and f[3] == name.split('/', 1)[1] and f[0].split(':')[0] == name.split('/')[0]:
            m, sec = f[0].split(':')
            slo, shi = int(f[1], 16), int(f[2], 16)
            nm = names[m].get(slo) or names['main'].get(slo)
            if nm is None or nm not in gsym:
                return 'added text, DATA SLOT %s %x not found in G (add by hand)' % (f[0], slo)
            smod, ga = gsym[nm]
            extra.append('%s:%s 0x%08X 0x%08X %s' % (smod or mod, sec, ga, ga + shi - slo, name))
    if extra:
        with open(LIST, 'a') as fh:
            fh.write('\n'.join(extra) + '\n')
    return 'added %s%s' % (mod, ' + %d slots' % len(extra) if extra else '')

if __name__ == '__main__':
    for p in sys.argv[1:]:
        print(p, add(p))
