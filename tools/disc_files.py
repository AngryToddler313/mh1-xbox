#!/usr/bin/env python3
"""disc_files.py - check the player's own Monster Hunter (PS2, Japan) disc and copy out the files the port needs.

    tools/disc_files.py check   DISC [--quick]
    tools/disc_files.py extract DISC DEST [--files NAME,...]
    tools/disc_files.py game    DISC          prints mh1 or mhg (which of the two supported discs it is)

DISC is a .iso (2048-byte sectors, or 2352-byte raw sectors) of the Japanese disc (SLPM-65495), or a folder
holding the files already extracted from it. The Japanese Monster Hunter G disc (SLPM-65869) is accepted too
(its Xbox build: ./mh1 xbox, docs/mhg.md). `check` verifies every needed file by size and CRC32; `extract`
copies them into DEST (a file that is already there with the right size and CRC32 is kept). Nothing of
Capcom's is stored in this repository: the table below holds only sizes and checksums (the same numbers as
src/pc/install.c uses). Exit status 0 = fine, 1 = wrong or damaged disc, 2 = usage / file problem.
Standard library only.
"""
import os
import re
import struct
import sys
import zlib

# name, size, crc32, required (the PC / Xbox builds read these; SYSTEM.CNF is only recorded)
WANT = [
    ('AFS_DATA.AFS', 121866240, 0x3BD5B15A, True),
    ('AFS00.AFS', 775729152, 0xE2312A4D, True),
    ('AFS01.AFS', 20463616, 0xD7B1D61F, True),
    ('SLPM_654.95', 5648908, 0xCF1A62C0, True),
    ('SYSTEM.CNF', 76, 0x9CF85397, False),
]

# Monster Hunter G (Japan, SLPM-65869; VERSION=mhg): the files its PC / Xbox builds read
WANT_G = [
    ('AFS_DATA.AFS', 150759424, 0xE397237C, True),
    ('AFS00.AFS', 926939136, 0x434FA486, True),
    ('AFS01.AFS', 25186304, 0x3AFA8E6F, True),
    ('SLPM_658.69', 2029336, 0xB25E8415, True),
    ('SYSTEM.CNF', 77, 0x71A2CE13, False),
]
# game -> (boot file, files, name)
GAMES = {
    'mh1': ('SLPM_654.95', WANT, 'the Japanese Monster Hunter disc (SLPM-65495)'),
    'mhg': ('SLPM_658.69', WANT_G, 'the Japanese Monster Hunter G disc (SLPM-65869)'),
}

# boot ids of other discs, for a helpful message when the wrong one is given (SYSTEM.CNF: BOOT2 = cdrom0:\SLxx_nnn.nn)
OTHER = {
    'SLUS_': 'a North American disc', 'SLES_': 'a European disc', 'SCUS_': 'a North American disc',
    'SCES_': 'a European disc', 'SLKA_': 'a Korean disc', 'SCAJ_': 'an Asian disc',
}


class DiscError(Exception):
    pass


class Source:
    """a list of files (name -> (size, reader)) from an ISO or a folder"""

    def __init__(self):
        self.files = {}      # upper-case name -> (size, opener)
        self.kind = ''

    def names(self):
        return sorted(self.files)


class Iso:
    def __init__(self, path):
        self.f = open(path, 'rb')
        self.size = os.path.getsize(path)
        self.secsz, self.off = 2048, 0
        for secsz, off in ((2048, 0), (2352, 24), (2352, 16)):
            if self._pvd(secsz, off):
                self.secsz, self.off = secsz, off
                break
        else:
            raise DiscError('%s is not a CD/DVD image (no ISO9660 volume found). Use the .iso file of the '
                            'disc (BIN/CUE images with 2048 or 2352 byte sectors are accepted; for a CUE '
                            'give the .bin).' % path)

    def _pvd(self, secsz, off):
        try:
            self.f.seek(16 * secsz + off)
            b = self.f.read(2048)
        except OSError:
            return False
        return len(b) == 2048 and b[0] == 1 and b[1:6] == b'CD001'

    def sector(self, lba, n=1):
        out = []
        for i in range(n):
            self.f.seek((lba + i) * self.secsz + self.off)
            b = self.f.read(2048)
            if len(b) != 2048:
                raise DiscError('the image ends early (damaged or incomplete download?)')
            out.append(b)
        return b''.join(out)

    def root(self):
        pvd = self.sector(16)
        lba = struct.unpack_from('<I', pvd, 156 + 2)[0]
        size = struct.unpack_from('<I', pvd, 156 + 10)[0]
        d = self.sector(lba, (size + 2047) // 2048)[:size]
        res, p = {}, 0
        while p < len(d):
            n = d[p]
            if n == 0:                          # records do not cross a sector: skip to the next one
                p = (p // 2048 + 1) * 2048
                continue
            ext = struct.unpack_from('<I', d, p + 2)[0]
            fsz = struct.unpack_from('<I', d, p + 10)[0]
            flags, nl = d[p + 25], d[p + 32]
            name = d[p + 33:p + 33 + nl].decode('latin-1')
            p += n
            if name in ('\x00', '\x01') or flags & 2:
                continue
            res[re.sub(r';\d+$', '', name).upper().rstrip('.')] = (fsz, ext)
        return res

    def read_file(self, ext, size, chunk=1 << 20):
        left, lba = size, ext
        while left > 0:
            n = min(chunk // 2048, (left + 2047) // 2048)
            b = self.sector(lba, n)
            lba += n
            yield b[:left]
            left -= len(b)


def open_source(path):
    if os.path.isdir(path):
        s = Source()
        s.kind = 'folder'
        for n in os.listdir(path):
            p = os.path.join(path, n)
            if os.path.isfile(p):
                s.files[n.upper()] = (os.path.getsize(p), p)
        return s, None
    if not os.path.isfile(path):
        raise DiscError('%s does not exist.' % path)
    iso = Iso(path)
    s = Source()
    s.kind = 'iso'
    for n, (sz, ext) in iso.root().items():
        s.files[n] = (sz, ext)
    return s, iso


def stream(s, iso, name, chunk=1 << 20):
    sz, ref = s.files[name]
    if iso is None:
        with open(ref, 'rb') as f:
            while True:
                b = f.read(chunk)
                if not b:
                    return
                yield b
    else:
        yield from iso.read_file(ref, sz, chunk)


def identify(s, iso):
    """the boot file named by SYSTEM.CNF, for the 'wrong disc' message"""
    if 'SYSTEM.CNF' in s.files and s.files['SYSTEM.CNF'][0] < 4096:
        txt = b''.join(stream(s, iso, 'SYSTEM.CNF')).decode('latin-1')
        m = re.search(r'BOOT2?\s*=\s*cdrom0:\\?([A-Z0-9_.]+)', txt, re.I)
        if m:
            return m.group(1).upper()
    return None


def which_game(s):
    """'mh1' / 'mhg' by the boot file on the disc, else None"""
    for g, (boot, _, _) in GAMES.items():
        if boot in s.files:
            return g
    return None


def wrong_disc_message(s, iso):
    boot = identify(s, iso)
    if boot and boot not in ('SLPM_654.95', 'SLPM_658.69'):
        what = 'a different PS2 game or region'
        for pre, d in OTHER.items():
            if boot.startswith(pre):
                what = d
        return ('This disc boots %s, which is %s. The port needs the Japanese Monster Hunter for PS2 '
                '(boot file SLPM_654.95, product code SLPM-65495) or the Japanese Monster Hunter G (SLPM_658.69). '
                'The US / European releases are different games for this purpose.' % (boot, what))
    if not any(n.startswith(('SLPM_', 'SLUS_', 'SLES_', 'SCPS_')) for n in s.files):
        return ('This does not look like a PS2 disc image (no boot file such as SLPM_654.95 in its root). '
                'Use the .iso of the Japanese Monster Hunter disc, or a folder with its files.')
    return None


def crc_of(s, iso, name, quick=False, progress=True):
    sz, _ = s.files[name]
    crc, done = 0, 0
    for b in stream(s, iso, name):
        crc = zlib.crc32(b, crc)
        done += len(b)
        if progress and sys.stderr.isatty():
            sys.stderr.write('\r  checking %-14s %3d%%' % (name, 100 * done // max(sz, 1)))
    if progress and sys.stderr.isatty():
        sys.stderr.write('\r%40s\r' % '')
    return crc & 0xFFFFFFFF


def verify(s, iso, names=None):
    """raise DiscError unless every wanted file is present with the known size and CRC32; returns the game"""
    game = which_game(s)
    if not game:
        raise DiscError(wrong_disc_message(s, iso) or 'SLPM_654.95 is not on this disc: it is not the Japanese '
                        'Monster Hunter (SLPM-65495).')
    want, label = GAMES[game][1], GAMES[game][2]
    for name, size, crc, req in want:
        if names and name not in names:
            continue
        if name not in s.files:
            if req:
                raise DiscError('%s is missing from the disc.' % name)
            continue
        if s.files[name][0] != size:
            raise DiscError('%s has the wrong size (%d bytes, expected %d). This is not %s, '
                            'or the image is damaged / modified.' % (name, s.files[name][0], size, label))
    for name, size, crc, req in want:
        if names and name not in names:
            continue
        if name in s.files and crc_of(s, iso, name) != crc:
            raise DiscError('%s has the right size but a wrong checksum: the image is damaged or modified '
                            '(a re-dump or a patched / translated image does not work). Dump the disc again.' % name)
    return game


def cmd_check(path, names=None):
    s, iso = open_source(path)
    print('%s: %s, files found: %s' % (path, s.kind, ', '.join(s.names()[:8]) + (', ...' if len(s.files) > 8 else '')))
    game = verify(s, iso, names)
    print('OK: %s; all needed files match.' % GAMES[game][2])


def cmd_extract(path, dest, names=None):
    s, iso = open_source(path)
    game = verify(s, iso, names)        # first: nothing is written for a wrong disc
    os.makedirs(dest, exist_ok=True)
    for name, size, crc, req in GAMES[game][1]:
        if names and name not in names:
            continue
        if name not in s.files:
            continue
        out = os.path.join(dest, name)
        if os.path.isfile(out) and os.path.getsize(out) == size:
            c = 0
            with open(out, 'rb') as f:
                for b in iter(lambda: f.read(1 << 20), b''):
                    c = zlib.crc32(b, c)
            if c & 0xFFFFFFFF == crc:
                print('  %-14s already there' % name)
                continue
        part = out + '.part'
        c, done = 0, 0
        with open(part, 'wb') as f:
            for b in stream(s, iso, name):
                f.write(b)
                c = zlib.crc32(b, c)
                done += len(b)
                if sys.stderr.isatty():
                    sys.stderr.write('\r  copying %-14s %3d%%' % (name, 100 * done // max(size, 1)))
        if sys.stderr.isatty():
            sys.stderr.write('\r%40s\r' % '')
        if c & 0xFFFFFFFF != crc or done != size:
            os.remove(part)
            raise DiscError('%s changed while copying (read error?)' % name)
        os.replace(part, out)
        print('  %-14s copied (%d MB)' % (name, size >> 20))


def main(argv):
    names = None
    if '--files' in argv:
        i = argv.index('--files')
        names = set(argv[i + 1].split(','))
        del argv[i:i + 2]
    try:
        if len(argv) >= 3 and argv[1] == 'check':
            cmd_check(argv[2], names)
        elif len(argv) >= 3 and argv[1] == 'game':
            s, iso = open_source(argv[2])
            g = which_game(s)
            if not g:
                raise DiscError(wrong_disc_message(s, iso) or 'not a supported Monster Hunter disc')
            print(g)
        elif len(argv) >= 4 and argv[1] == 'extract':
            cmd_extract(argv[2], argv[3], names)
        else:
            print(__doc__)
            return 2
    except DiscError as e:
        print('error: %s' % e, file=sys.stderr)
        return 1
    except OSError as e:
        print('error: %s' % e, file=sys.stderr)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
