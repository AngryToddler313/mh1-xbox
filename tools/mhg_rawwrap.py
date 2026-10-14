#!/usr/bin/env python3
"""mhg_rawwrap.py FILE FUNC...: for G, keep FUNC (a function in the shared C FILE that does not match G yet) as
the ORIGINAL bytes inside the TU: wraps its C in
    #if defined(VERSION_MHG) && defined(__MWERKS__)  asm stub (build/mhg/raw/FUNC.inc)  #else <C> #endif
and adds 'MODULE VRAM SIZE FUNC' to config/mhg/c_rawfuncs.txt (G address/size from VERSION=mhg tools/check.py).
MH1 and the PC builds still see the C. A file whose other functions all match can then be registered with
tools/mhg_addc.py (raw functions are not counted as decompiled by VERSION=mhg tools/progress.py)."""
import os, re, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
f = sys.argv[1]
env = dict(os.environ, VERSION='mhg')
chk = subprocess.run(['python3', 'tools/check.py', f], cwd=ROOT, capture_output=True, text=True, env=env).stdout
for fn in sys.argv[2:]:
    m = re.search(r'^(?:--|OK)\s+%s\s+(\w+)\s+(0x[0-9A-Fa-f]+)\s+(\d+) bytes' % re.escape(fn), chk, re.M)
    if not m:
        print('no G address for', fn); continue
    mod, addr, size = m.group(1), int(m.group(2), 16), int(m.group(3))
    s = open(f).read()
    if '%s.inc' % fn in s:
        print(fn, 'already wrapped'); continue
    mm = re.search(r'^((?:static )?[\w][\w \*]*?)\b%s\(([^;{]*)\)\s*\{\n' % re.escape(fn), s, re.M)
    if not mm:
        print('definition of', fn, 'not found'); continue
    en = s.index('\n}\n', mm.end()) + 3
    rt, par, body = mm.group(1).strip(), mm.group(2), s[mm.start():en]
    stub = ('/* G: original bytes (build/mhg/raw/%s.inc, config/mhg/c_rawfuncs.txt); the C below is the MH1 version */\n'
            '#if defined(VERSION_MHG) && defined(__MWERKS__)\n%s %s(%s)\n{\n#include "%s.inc"\n}\n#else\n%s#endif\n') % (fn, ('static asm ' + rt[7:]) if rt.startswith('static ') else 'asm ' + rt, fn, par, fn, body)
    open(f, 'w').write(s[:mm.start()] + stub + s[en:])
    with open(os.path.join(ROOT, 'config/mhg/c_rawfuncs.txt'), 'a') as o:
        o.write('%s 0x%08X 0x%X %s\n' % (mod, addr, size, fn))
    print('wrapped', fn, mod, hex(addr), size)
subprocess.run(['python3', '-c', 'import sys; sys.path.insert(0, "tools"); import build; build.gen_raw()'], cwd=ROOT, env=env)
