#!/usr/bin/env python3
"""Laptop side of the Xbox network loader (src/pc/xbox/loader, docs/xbox.md "Network loader").

    tools/xbox_serve.py [--port 8765] [--xbe build/xbox/nv2a/default.xbe] [--input FILE] [--env FILE]
    tools/xbox_serve.py --data dist/xbox/MHG        (or dist/xbox/MH1: install a game folder, see below)

Serves build/xbox/default.xbe (the null-graphics build; --xbe for another) to the loader as /mh1.xbe, with /version
(size and time of that file, plus build/xbox/rerun.txt when present, so
touching that file makes a waiting loader start the same build again).
--input / --env: a test run's scripted pad input (the PC's --input format) and
RT_* switches (NAME=VALUE lines), sent as GET /test; the loader puts them in
E:\\Games\\MH1\\data (input.txt, env.txt; xbox_env.c, viewer.c) and removes
them when none are given. Logs the Xbox sends (POST /log) are saved to build/xbox/logs/<time>.txt and
printed. A log that starts with "MH1DUMP <name>" (src/pc/xbox/dumper: the BIOS
flash, for xemu) is saved to build/xbox/dump/<name> instead; Microsoft's file, so it stays out of git
(build/ is ignored). Only these paths are served; nothing else in the repo is reachable.

--data DIR: install a game folder (./mh1 xbox's dist/xbox/MHG or dist/xbox/MH1: default.xbe + data/) on
the Xbox through the loader instead of FTP (docs/xbox.md "Installing game data through the loader").
/version then ends in a "DATA" line; the loader fetches GET /manifest ("MH1DATA <folder> <chunk> <n>",
then per file "F <size> <crc32> <path>" and "C <crc32 of each chunk> ..."), and each missing or short
file with GET /data/<path> and a Range header, writing it to E:\\Games\\<folder> (MHG when the folder
holds SLPM_658.69, else MH1). For MH1 the folder's default.xbe is not installed over the loader; it is
served as the build (/mh1.xbe) unless --xbe names another.
"""
import argparse
import http.server
import os
import re
import sys
import time
import zlib

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build', 'xbox')
XBE = os.path.join(ROOT, 'default.xbe')
RERUN = os.path.join(ROOT, 'rerun.txt')
LOGS = os.path.join(ROOT, 'logs')
DUMPS = os.path.join(ROOT, 'dump')
TEST = {'input': None, 'env': None}
CHUNK = 4 << 20          # the loader checks and writes the data in pieces of this size
DATA = {'dir': None, 'files': {}, 'manifest': b''}


def version():
    if not os.path.isfile(XBE):
        v = 'none'       # no build to send (a data-only install); the loader keeps what it has
    else:
        st = os.stat(XBE)
        v = '%d-%d' % (st.st_size, int(st.st_mtime))
        if os.path.exists(RERUN):
            v += '-' + open(RERUN).read().strip()
    if DATA['dir']:
        v += '\nDATA'
    return v


def data_fresh():
    """the folder changed since the manifest was made (a new default.xbe copied in): make it again"""
    if DATA['dir'] and any(os.path.getsize(f) != n or os.path.getmtime(f) != t for f, n, t in DATA['stamp']):
        print('== the data folder changed: checksums again')
        load_data(DATA['src'])


def load_data(src):
    """the manifest of a game folder: G or MH1 by its program file; data/ subfolder or the bare disc files"""
    src = os.path.abspath(src)
    sub = os.path.join(src, 'data')
    base = sub if os.path.isdir(sub) else src
    game = 'MHG' if os.path.exists(os.path.join(base, 'SLPM_658.69')) else 'MH1'
    elf = 'SLPM_658.69' if game == 'MHG' else 'SLPM_654.95'
    names = [elf, 'AFS01.AFS', 'AFS_DATA.AFS', 'AFS00.AFS']    # small first: one bad link shows early
    files = [('data/' + n, os.path.join(base, n)) for n in names]
    for _, f in files:
        if not os.path.exists(f):
            sys.exit('%s missing' % f)
    xbe = os.path.join(src, 'default.xbe')
    if game == 'MHG' and os.path.exists(xbe):
        files.insert(0, ('default.xbe', xbe))         # G runs as its own game, E:\Games\MHG\default.xbe
    if XBE_DEFAULT:     # no --xbe: MH1's build is the folder's XBE (E:\Games\MH1\default.xbe is the loader
        set_xbe(xbe if game == 'MH1' and os.path.exists(xbe) else '')   # itself, so it goes in as mh1.xbe);
        # for G no build, so the loader starts G once its folder is complete
    out = [b'MH1DATA %s %d %d' % (game.encode(), CHUNK, len(files))]
    files_new = {}
    for rel, f in files:
        size, crc, chunks = 0, 0, []
        print('checksums: %s ...' % f, end=' ')
        sys.stdout.flush()
        with open(f, 'rb') as fh:
            while True:
                b = fh.read(CHUNK)
                if not b:
                    break
                size += len(b)
                crc = zlib.crc32(b, crc)
                chunks.append(zlib.crc32(b))
        print('%d bytes, crc %08x' % (size, crc))
        out.append(b'F %d %08x %s' % (size, crc, rel.encode()))
        out.append(b'C ' + b' '.join(b'%08x' % c for c in chunks))
        files_new[rel] = (f, size)
    DATA['files'] = files_new
    DATA['dir'] = game
    DATA['src'] = src
    DATA['stamp'] = [(f, n, os.path.getmtime(f)) for f, n in DATA['files'].values()]
    DATA['manifest'] = b'\n'.join(out) + b'\n'


XBE_DEFAULT = True


def set_xbe(path):
    global XBE, XBE_DEFAULT
    XBE = os.path.abspath(path) if path else ''
    XBE_DEFAULT = False


class Handler(http.server.BaseHTTPRequestHandler):
    def reply(self, body, ctype='application/octet-stream'):
        self.send_response(200)
        self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path in ('/version', '/manifest'):
            data_fresh()
        if self.path == '/version':
            self.reply(version().encode(), 'text/plain')
        elif self.path == '/test':
            ins = open(TEST['input'], 'rb').read() if TEST['input'] else b''
            env = open(TEST['env'], 'rb').read() if TEST['env'] else b''
            self.reply(b'MH1TEST %d %d\n' % (len(ins), len(env)) + ins + env, 'text/plain')
        elif self.path == '/mh1.xbe' and os.path.isfile(XBE):
            self.reply(open(XBE, 'rb').read())
        elif self.path == '/manifest' and DATA['dir']:
            self.reply(DATA['manifest'], 'text/plain')
        elif self.path.startswith('/data/') and self.path[6:] in DATA['files']:
            self.send_data(*DATA['files'][self.path[6:]])
        else:
            self.send_error(404)

    def send_data(self, f, size):
        """the file from the Range start (bytes=N- or bytes=N-M) on, in 1 MB pieces"""
        start, end = 0, size - 1
        m = re.fullmatch(r'bytes=(\d+)-(\d*)', self.headers.get('Range', '') or '')
        if m:
            start = int(m.group(1))
            if m.group(2):
                end = min(end, int(m.group(2)))
            if start > end:
                self.send_error(416)
                return
            self.send_response(206)
            self.send_header('Content-Range', 'bytes %d-%d/%d' % (start, end, size))
        else:
            self.send_response(200)
        self.send_header('Content-Type', 'application/octet-stream')
        self.send_header('Content-Length', str(end + 1 - start))
        self.end_headers()
        t0, left = time.time(), end + 1 - start
        try:
            with open(f, 'rb') as fh:
                fh.seek(start)
                while left > 0:
                    b = fh.read(min(left, 1 << 20))
                    if not b:
                        break
                    self.wfile.write(b)
                    left -= len(b)
        except (BrokenPipeError, ConnectionResetError) as e:
            print('== %s cut at %d MB (%s)' % (self.path, (end + 1 - start - left) >> 20, e.__class__.__name__))
            return
        dt = max(time.time() - t0, 0.001)
        print('== %s: %d MB from %d MB in %.0f s (%.1f MB/s)' % (self.path, (end + 1 - start) >> 20, start >> 20,
                                                             dt, (end + 1 - start) / dt / 1e6))

    def do_POST(self):
        body = self.rfile.read(int(self.headers.get('Content-Length', 0)))
        if self.path == '/log' and body.startswith(b'MH1DUMP '):    # src/pc/xbox/dumper: name line + data
            head, _, data = body.partition(b'\n')
            name = head[8:].decode('ascii', 'replace').strip()
            if not re.fullmatch(r'[a-z0-9_]{1,24}\.bin', name):
                self.send_error(400)
                return
            os.makedirs(DUMPS, exist_ok=True)
            open(os.path.join(DUMPS, name), 'wb').write(data)
            print('== Xbox dump -> %s (%d bytes)' % (os.path.join(DUMPS, name), len(data)))
        elif self.path == '/log':
            os.makedirs(LOGS, exist_ok=True)
            name = os.path.join(LOGS, time.strftime('%Y%m%d-%H%M%S') + '.txt')
            open(name, 'wb').write(body)
            print('== Xbox log -> %s (%d bytes)' % (name, len(body)))
            sys.stdout.write(body.decode('utf-8', 'replace')[-3000:] + '\n')
        elif self.path == '/note':
            print('== Xbox: %s' % body.decode('utf-8', 'replace'))
        else:
            self.send_error(404)
            return
        self.reply(b'ok', 'text/plain')

    def log_message(self, fmt, *args):
        print('%s %s' % (time.strftime('%H:%M:%S'), fmt % args))
        sys.stdout.flush()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--port', type=int, default=8765)
    ap.add_argument('--xbe', help='the build to serve (default build/xbox/default.xbe)')
    ap.add_argument('--input', help="a test run's scripted input (PC --input format)")
    ap.add_argument('--env', help="a test run's RT_* switches (NAME=VALUE lines)")
    ap.add_argument('--data', help='a game folder to install through the loader (dist/xbox/MHG, dist/xbox/MH1)')
    a = ap.parse_args()
    TEST['input'] = a.input and os.path.abspath(a.input)
    TEST['env'] = a.env and os.path.abspath(a.env)
    if a.xbe:
        set_xbe(a.xbe)
    if a.data:
        load_data(a.data)
        print('installing %s to E:\\Games\\%s (%d files) through the loader: start Monster Hunter on the Xbox'
              % (os.path.abspath(a.data), DATA['dir'], len(DATA['files'])))
    print('serving %s on port %d (version %s)' % (
        os.path.normpath(XBE) + ' as /mh1.xbe' if os.path.isfile(XBE) else 'no build', a.port,
        version().replace('\n', ' ')))
    sys.stdout.flush()
    http.server.ThreadingHTTPServer(('0.0.0.0', a.port), Handler).serve_forever()


if __name__ == '__main__':
    main()
