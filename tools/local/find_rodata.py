#!/usr/bin/env python3
"""find_rodata.py [PATH...]: for C files dropped as "rodata-unslotted" (docs/handoff/dropped_registrations.txt, or the
PATHs given, as written in config/mhg/c_files.txt e.g. mhg/game/em/em16a_p2), compile the file like tools/build.py and
search its .rodata (R_MIPS_32 words masked) in the G module image (build/mhg/split/MODULE.bin). Prints the
`MODULE:rodata START END PATH` line for a unique hit, else why not. Does not edit c_files.txt."""
import os, re, struct, subprocess, sys, tempfile
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)
WIBO = "tools/compilers/wibo"; MWCC = "tools/compilers/mwcps2-3.0b52-030722/mwccps2.exe"
CFLAGS = ["-c", "-O4,p", "-nostdinc", "-stderr", "-pragma", "divbyzerocheck on", "-DVERSION_MHG", "-Iinclude", "-Ibuild/mhg/raw", "-Ibuild/raw"]

def image(mod):
    d = open("build/mhg/split/%s.bin" % mod, "rb").read()
    return (0x100000 if mod == "main" else struct.unpack_from("<I", d, 8)[0]), d

GN = None
def gnames():
    """name -> G address from config/mhg/symbols/*.txt and the PROVIDE files the G link uses"""
    global GN
    if GN is None:
        GN = {}
        import glob
        for f in glob.glob("config/mhg/symbols/*.txt"):
            for l in open(f):
                m = re.match(r"(\w+) = 0x([0-9A-Fa-f]+);", l)
                if m: GN.setdefault(m.group(1), int(m.group(2), 16))
        for f in glob.glob("config/mhg/*_aliases.txt") + ["config/mhg/g_names.txt", "config/mhg/g_funcnames.txt"]:
            for l in open(f):
                m = re.match(r"PROVIDE\((\w+) = 0x([0-9A-Fa-f]+)\)", l)
                if m: GN.setdefault(m.group(1), int(m.group(2), 16))
    return GN

def rodata(obj, text_g=None):
    d = open(obj, "rb").read()
    shoff, = struct.unpack_from("<I", d, 32); shnum, shstr = struct.unpack_from("<HH", d, 48)
    S = [struct.unpack_from("<10I", d, shoff + i * 40) for i in range(shnum)]
    names = d[S[shstr][4]:S[shstr][4] + S[shstr][5]]
    nm = lambda o: names[o:names.index(b"\0", o)].decode()
    out = []
    for i, s in enumerate(S):
        if nm(s[0]) == ".rodata" and s[5]:
            data = bytearray(d[s[4]:s[4] + s[5]]); mask = [0xFF] * len(data)
            for r in S:
                if r[1] == 9 and r[7] == i:
                    for k in range(r[5] // 8):
                        off, info = struct.unpack_from("<II", d, r[4] + k * 8)
                        if info & 0xFF == 2:          # R_MIPS_32
                            sym = info >> 8
                            st = next(x for x in S if x[1] == 2)
                            sn, sval, _sz, sinfo, _o, sh = struct.unpack_from("<IIIBBH", d, st[4] + sym * 16)
                            strt = S[st[6]]; strs = d[strt[4]:strt[4] + strt[5]]
                            name = strs[sn:strs.index(b"\0", sn)].decode()
                            if sinfo & 0xF == 3 and sh < len(S):
                                # section symbol: name it by the function defined in that section (one .text per function)
                                for k2 in range(st[5] // 16):
                                    n2, v2, z2, i2, o2, h2 = struct.unpack_from("<IIIBBH", d, st[4] + k2 * 16)
                                    if h2 == sh and i2 & 0xF == 2 and v2 == 0:
                                        name = strs[n2:strs.index(b"\0", n2)].decode(); break
                            g = gnames().get(name)
                            if g is not None:
                                add, = struct.unpack_from("<I", data, off)
                                struct.pack_into("<I", data, off, (g + add) & 0xFFFFFFFF)
                            else:
                                for j in range(4): mask[off + j] = 0
            out.append((bytes(data), mask, s[8] or 1))
    return out

def find(blob, data, mask, align):
    anchor = next((i for i in range(0, len(data) - 3, 4) if all(mask[i:i + 4]) ), None)
    hits = []
    if anchor is None:
        return hits
    key = data[anchor:anchor + 4]; p = blob.find(key)
    while p != -1:
        s = p - anchor
        if s >= 0 and s % align == 0 and s + len(data) <= len(blob) and all(m == 0 or blob[s + j] == data[j] for j, m in enumerate(mask)):
            hits.append(s)
        p = blob.find(key, p + 1)
    return hits

def main():
    if len(sys.argv) > 1:
        todo = [(None, p, None) for p in sys.argv[1:]]
    else:
        todo = []
        for l in open("docs/handoff/dropped_registrations.txt"):
            if l.startswith("rodata-unslotted\t"):
                f = l.split("\t")[1].split()
                if ":" not in f[0]:
                    todo.append((f[0], f[3], int(f[1], 16)))
    seen = set()
    for mod, path, text_g in todo:
        if path in seen: continue
        seen.add(path)
        mod = mod or "game"
        src = "src/%s.c" % path
        with tempfile.TemporaryDirectory(dir="build") as td:
            obj = os.path.join(td, "x.o")
            r = subprocess.run([WIBO, MWCC] + CFLAGS + [src, "-o", os.path.relpath(obj)], capture_output=True, text=True, timeout=180)
            if r.returncode or not os.path.exists(obj):
                print("#COMPILE", path); continue
            secs = rodata(obj, text_g)
        if not secs:
            print("#NORODATA", mod, path); continue
        base, blob = image(mod)
        for data, mask, align in secs:
            h = find(blob, data, mask, max(4, align))
            if len(h) == 1:
                a = base + h[0]
                print("%s:rodata 0x%08X 0x%08X %s" % (mod, a, a + len(data), path))
            else:
                print("#%s %s %s rodata %d bytes: %d hits" % ("NOTFOUND" if not h else "AMBIGUOUS", mod, path, len(data), len(h)))

main()
