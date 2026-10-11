#!/usr/bin/env python3
"""xbox_fatx.py - format and read the E: partition of an xemu hard disk image.

xemu's hard disk (xbox_files/hdd.img, a raw image; tools/run_xemu.sh makes a
blank sparse one) starts out unformatted, so the game has no E: drive: no
saves (E:\\UDATA), no log file. The retail Xbox disk has no partition table;
the kernel uses fixed offsets, E: (\\Device\\Harddisk0\\Partition1) is
0x1312D6000 bytes at 0xABE80000. This writes an empty FATX file system there
the way nxdk's nxFormatVolume (lib/nxdk/format.c) does: a 4 KB superblock,
the cluster table (16 KB clusters; 32-bit entries above 65519 clusters),
then the root directory cluster. Nothing else on the image is touched.

    tools/xbox_fatx.py format xbox_files/hdd.img      # E: empty (all data on E: lost)
    tools/xbox_fatx.py ls xbox_files/hdd.img [DIR]     # e.g. Games/MH1
    tools/xbox_fatx.py get xbox_files/hdd.img Games/MH1/mh1log.txt [OUT]

Run them only while xemu is not running (it caches the disk). Standard library only.
"""
import os, struct, sys, time

E_OFFSET, E_SIZE = 0xABE80000, 0x1312D6000
SECTOR, CLUSTER, PAGE = 512, 16384, 4096


def geometry():
    """cluster count, entry size, cluster-table bytes (as nxFormatVolume and the kernel compute them)"""
    n = E_SIZE // CLUSTER + 1
    ent = 2 if n < 65520 else 4
    table = (n * ent + PAGE - 1) & ~(PAGE - 1)
    return n, ent, table


def fmt(img):
    n, ent, table = geometry()
    with open(img, 'r+b') as f:
        f.seek(0, 2)
        if f.tell() < E_OFFSET + E_SIZE:
            sys.exit('%s is smaller than a retail Xbox disk (8 GB); no E: partition' % img)
        sb = bytearray(b'\xff' * PAGE)
        struct.pack_into('<4sIIIH', sb, 0, b'FATX', int(time.time()) & 0xFFFFFFFF, CLUSTER // SECTOR, 1, 0)
        f.seek(E_OFFSET)
        f.write(sb)
        fat = bytearray(table)
        if ent == 2:
            struct.pack_into('<HH', fat, 0, 0xFFF8, 0xFFFF)
        else:
            struct.pack_into('<II', fat, 0, 0xFFFFFFF8, 0xFFFFFFFF)
        f.write(fat)
        f.write(b'\xff' * CLUSTER)     # root directory: no entries
    print('E: formatted (FATX, %d clusters of 16 KB, %d-bit table)' % (n, ent * 8))


class Vol:
    def __init__(self, img):
        self.f = open(img, 'rb')
        self.f.seek(E_OFFSET)
        sig, _vid, spc, root = struct.unpack('<4sIII', self.f.read(16))
        if sig != b'FATX':
            sys.exit('E: is not formatted (no FATX signature); see "format"')
        self.cl = spc * SECTOR
        self.n, self.ent, table = geometry()
        self.f.seek(E_OFFSET + PAGE)
        self.fat = self.f.read(table)
        self.data = E_OFFSET + PAGE + table
        self.root = root

    def chain(self, c):
        end = 0xFFF0 if self.ent == 2 else 0xFFFFFFF0
        while 0 < c < end:
            yield c
            c = struct.unpack_from('<H' if self.ent == 2 else '<I', self.fat, c * self.ent)[0]

    def read(self, first, size=None):
        out = bytearray()
        for c in self.chain(first):
            self.f.seek(self.data + (c - 1) * self.cl)
            out += self.f.read(self.cl)
            if size is not None and len(out) >= size:
                break
        return bytes(out[:size]) if size is not None else bytes(out)

    def entries(self, first):
        d = self.read(first)
        for i in range(0, len(d), 64):
            ln, attr = d[i], d[i + 1]
            if ln in (0x00, 0xFF):
                break
            if ln == 0xE5 or ln > 42:
                continue
            name = d[i + 2:i + 2 + ln].decode('latin-1')
            c, size = struct.unpack_from('<II', d, i + 0x2C)
            yield name, attr, c, size

    def find(self, path):
        c, attr, size = self.root, 0x10, 0
        for part in [p for p in path.replace('\\', '/').split('/') if p]:
            for name, a, fc, sz in self.entries(c) if attr & 0x10 else ():
                if name.lower() == part.lower():
                    c, attr, size = fc, a, sz
                    break
            else:
                sys.exit('E:/%s: not found' % path)
        return c, attr, size


def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ('format', 'ls', 'get'):
        sys.exit(__doc__)
    cmd, img = sys.argv[1], sys.argv[2]
    if cmd == 'format':
        return fmt(img)
    v = Vol(img)
    c, attr, size = v.find(sys.argv[3] if len(sys.argv) > 3 else '')
    if cmd == 'ls':
        if not attr & 0x10:
            sys.exit('not a directory')
        for name, a, _c, sz in v.entries(c):
            print('%s%s' % (name, '/' if a & 0x10 else '  %d' % sz))
        return
    if attr & 0x10:
        sys.exit('a directory')
    data = v.read(c, size)
    if len(sys.argv) > 4:
        open(sys.argv[4], 'wb').write(data)
    else:
        sys.stdout.buffer.write(data)


if __name__ == '__main__':
    main()
