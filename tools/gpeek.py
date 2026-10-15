#!/usr/bin/env python3
"""gpeek.py MODULE ADDR [N=16] [-b|-h] : words (or bytes -b, halves -h) of PS2 G data at a G address, from build/mhg/split
(module main|sub_main|lobby|game|select|yn|nethttp; offset = ADDR - the module's vram in config/mhg/MODULE.yaml). Agent D tool."""
import re, struct, sys
ROOT = __file__.rsplit('/tools/', 1)[0]
a = [x for x in sys.argv[1:] if not x.startswith('-')]
mod, addr = a[0], int(a[1], 0)
n = int(a[2], 0) if len(a) > 2 else 16
y = open('%s/config/mhg/%s.yaml' % (ROOT, mod)).read()
vram = int(re.search(r'vram:\s*(0x[0-9A-Fa-f]+)', y).group(1), 16)
d = open('%s/build/mhg/split/%s.bin' % (ROOT, mod), 'rb').read()
o = addr - vram
if '-b' in sys.argv:
    for i in range(0, n, 16):
        print('%08X ' % (addr + i) + ' '.join('%02x' % x for x in d[o + i:o + min(i + 16, n)]))
elif '-h' in sys.argv:
    for i in range(0, n * 2, 16):
        print('%08X ' % (addr + i) + ' '.join('%04x' % x for x in struct.unpack_from('<%dH' % (min(16, n * 2 - i) // 2), d, o + i)))
else:
    for i in range(0, n * 4, 16):
        print('%08X ' % (addr + i) + ' '.join('%08x' % x for x in struct.unpack_from('<%dI' % (min(16, n * 4 - i) // 4), d, o + i)))
