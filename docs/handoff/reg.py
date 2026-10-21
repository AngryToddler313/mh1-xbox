#!/usr/bin/env python3
"""reg.py FILE... : register what now matches G in each file (mhg_addfn for split files, mhg_parts / mhg_addc otherwise),
then build all G modules; keep only if all seven are OK. Does not commit."""
import os, re, subprocess, sys
ROOT = os.environ.get('ROOT', '/root/mh1-xbox'); os.chdir(ROOT)
ENV = dict(os.environ, VERSION='mhg')
def build_ok():
    b = subprocess.run(['python3', 'tools/build.py'], env=ENV, capture_output=True, text=True, timeout=3600).stdout
    return len([l for l in b.splitlines() if re.match(r'^\w+\s+OK\s', l)]) == 7, b
for path in sys.argv[1:]:
    snap = subprocess.run(['git', 'stash', 'create'], capture_output=True, text=True).stdout.strip()
    cfg = open('config/mhg/c_files.txt').read(); raw = open('config/mhg/c_rawfuncs.txt').read()
    src = open(path).read()
    if 'G_PART' in src:
        out = subprocess.run(['python3', 'tools/mhg_addfn.py', path], capture_output=True, text=True).stdout.strip().splitlines()[-1]
    else:
        out = subprocess.run(['python3', 'tools/mhg_parts.py', path], capture_output=True, text=True).stdout.strip()
        if 'whole file matches' in out:
            out = subprocess.run(['python3', 'tools/mhg_addc.py', path], capture_output=True, text=True).stdout.strip()
    if open('config/mhg/c_files.txt').read() == cfg:
        print('NONE', path, out[-90:]); continue
    ok, b = build_ok()
    if not ok:
        open('config/mhg/c_files.txt', 'w').write(cfg); open('config/mhg/c_rawfuncs.txt', 'w').write(raw)
        if 'G_PART' not in src:
            open(path, 'w').write(src)
        subprocess.run(['git', 'clean', '-fdq', 'src/mhg'])
        print('UNDO', path, ' / '.join(l for l in b.splitlines() if 'bytes' in l and ' OK ' not in l)[:150]); continue
    print('ADD', path, out[-90:])
r = subprocess.run(['python3', 'tools/progress.py'], env=ENV, capture_output=True, text=True).stdout
print([l for l in r.splitlines() if l.startswith('total')][0])
