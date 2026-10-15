#!/usr/bin/env python3
"""mhg_dropparts.py [--sections] [--cdiff]: remove parts (lines `mhg/.../x_pN` of config/mhg/c_files.txt) that break the G link.

--sections: parts whose object has data sections (.data .sdata .sbss .bss .rodata .lit*): they would add a second copy of
            the file's tables to the module. Needs the objects of a VERSION=mhg build.
--cdiff:    parts that tools/mhg_cdiff.py reports as differing from G's bytes (reads its output from stdin).
--since REV: only consider parts whose list line is not in config/mhg/c_files.txt of git revision REV (the new ones).
Prints what it dropped. The part files stay in src/mhg; only the list line goes.
"""
import os, re, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIST = os.path.join(ROOT, 'config/mhg/c_files.txt')
SIZE = '/home/james/.local/opt/ps2-binutils-2.47/bin/mips64r5900el-ps2-elf-size'
OKSEC = {'.comment', '.text', '.mwcats', '.reginfo', '.pdr', '.mdebug', '.gnu.attributes', '.MIPS.abiflags', '.sdata_unused'}


def data_sections(name):
    obj = os.path.join(ROOT, 'build/mhg/src/%s.c.o' % name)
    if not os.path.exists(obj):
        return None
    out = subprocess.run([SIZE, '-A', obj], capture_output=True, text=True).stdout
    bad = []
    for l in out.splitlines():
        m = re.match(r'(\.\S+)\s+(\d+)\s+', l)
        if m and m[1] not in OKSEC and int(m[2]) > 0:
            bad.append('%s:%s' % (m[1], m[2]))
    return bad


def main():
    lines = open(LIST).read().split('\n')
    drop = set()
    old = None
    if '--since' in sys.argv:
        rev = sys.argv[sys.argv.index('--since') + 1]
        old = set(subprocess.run(['git', 'show', '%s:config/mhg/c_files.txt' % rev], cwd=ROOT, capture_output=True, text=True).stdout.split('\n'))
    if '--sections' in sys.argv:
        for l in lines:
            p = l.split()
            if old is not None and l in old:
                continue
            if len(p) == 4 and re.search(r'_p\d+$', p[3]):
                b = data_sections(p[3])
                if b:
                    drop.add(p[3])
                    print('data  %-40s %s' % (p[3], ' '.join(b)))
    if '--cdiff' in sys.argv:
        for l in sys.stdin:
            m = re.match(r'\S+\s+(mhg/\S+_p\d+)\s', l)
            if m:
                drop.add(m[1])
                print('cdiff %s' % m[1])
    if drop:
        # remember the functions of the dropped parts (tools/mhg_parts.py leaves them out next time)
        skip = os.path.join(ROOT, 'config/mhg/c_skip_funcs.txt')
        with open(skip, 'a') as fh:
            for part in sorted(drop):
                w = 'src/%s.c' % part
                m = re.match(r'(.*)/(\w+)_p\d+$', part)
                src = open(os.path.join(ROOT, w)).read()
                inc = re.search(r'#include "\.\./(.*)"', src)[1]
                r = subprocess.run(['python3', 'tools/check.py', w], cwd=ROOT, capture_output=True, text=True,
                                   env=dict(os.environ, VERSION='mhg')).stdout
                for l in r.splitlines():
                    if l.startswith('OK'):
                        fh.write('%s %s # part %s did not link\n' % (inc, l.split()[1], part))
        open(LIST, 'w').write('\n'.join(l for l in lines if not (len(l.split()) == 4 and l.split()[3] in drop)))


main()
