#!/usr/bin/env python3
"""ref_match.py MODULE OUT.csv SRC.c... [-- extra mwcc flags]

Fingerprint search: compile reference C (e.g. OpenSSL 0.9.6, which G's SSL-C 2.1 library derives from) with the
project's compiler (mwccps2 3.0b52 via wibo, -O4,p) and look for each compiled function's bytes in a G module,
with relocated fields masked (R_MIPS_26 low 26 bits, HI16 / LO16 / GPREL16 low 16, 32 whole word). A hit means
that source compiles to G's code as is; it still has to go through the real build (tools/build.py) to count.

MODULE is a G overlay name (disc/mhg/overlays/MODULE.bin, text at file offset 0x40). Sources must lie under the
repo (wibo hands relative paths to the Windows compiler): link a reference tree into build/ref first.
Writes OUT.csv: src, func, size, g_addr (blank when not found), hits.
"""
import collections, csv, os, re, struct, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BU = os.path.join(ROOT, "tools/binutils/bin/mips64r5900el-ps2-elf-")
WIBO = "tools/compilers/wibo"
MWCC = "tools/compilers/mwcps2-3.0b52-030722/mwccps2.exe"
CFLAGS = ["-c", "-O4,p", "-nostdinc", "-stderr", "-pragma", "divbyzerocheck on"]
VRAM = {"nethttp": 0xA06200, "lobby": None}
# REF_CC overrides the compiler command (e.g. "tools/compilers/ee/ee-gcc2.96/bin/ee-gcc -c -O2 -G0"); SSL-C was built with ee-gcc
CC = os.environ["REF_CC"].split() if os.environ.get("REF_CC") else [WIBO, MWCC] + CFLAGS
MASK = {"R_MIPS_26": 0x03FFFFFF, "R_MIPS_HI16": 0xFFFF, "R_MIPS_LO16": 0xFFFF, "R_MIPS_GPREL16": 0xFFFF, "R_MIPS_32": 0xFFFFFFFF}


def module_words(mod):
    d = open(os.path.join(ROOT, "disc/mhg/overlays/%s.bin" % mod), "rb").read()
    text_size = struct.unpack_from("<I", d, 0x0C)[0] if False else len(d) - 0x40
    t = d[0x40:0x40 + text_size]
    return list(struct.unpack_from("<%dI" % (len(t) // 4), t))


def funcs_of(obj):
    syms = []
    for l in subprocess.run([BU + "objdump", "-t", obj], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r"^([0-9a-f]+)\s+[gl]\s+F\s+\.text\s+([0-9a-f]+)\s+(\S+)$", l)
        if m and int(m.group(2), 16):
            syms.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
    rel = {}
    for l in subprocess.run([BU + "objdump", "-r", "-j", ".text", obj], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r"^([0-9a-f]+)\s+(R_MIPS_\w+)", l)
        if m:
            rel[int(m.group(1), 16)] = MASK.get(m.group(2), 0xFFFFFFFF)
    with tempfile.NamedTemporaryFile(suffix=".bin") as tf:
        subprocess.run([BU + "objcopy", "-O", "binary", "-j", ".text", obj, tf.name], check=True)
        text = open(tf.name, "rb").read()
    out = []
    for off, size, name in syms:
        words = struct.unpack_from("<%dI" % (size // 4), text, off)
        masks = [0xFFFFFFFF & ~rel.get(off + 4 * i, 0) for i in range(size // 4)]
        out.append((name, size, words, masks))
    return out


def main():
    args = sys.argv[1:]
    extra = []
    if "--" in args:
        i = args.index("--"); extra = args[i + 1:]; args = args[:i]
    mod, outp, srcs = args[0], args[1], args[2:]
    os.chdir(ROOT)
    W = module_words(mod)
    index = collections.defaultdict(list)
    for i, w in enumerate(W):
        index[w].append(i)
    base = VRAM.get(mod) or 0
    rows = []
    found = 0
    with tempfile.TemporaryDirectory(dir=os.path.join(ROOT, "build")) as td:
        for src in srcs:
            obj = os.path.join(td, "o.o")
            # gcc's cpp does the preprocessing (mwcc's -nostdinc keeps <...> headers off the -I paths)
            pp = os.path.join(td, "pp.c")
            r = subprocess.run(["cpp", "-P", "-undef", "-nostdinc", "-D__MWERKS__=1", "-D__mips__", "-D_MIPSEL"] + extra + [src, pp],
                               capture_output=True, text=True)
            if r.returncode:
                print("PREPROCESS FAIL", src, r.stderr.strip().splitlines()[:1])
                continue
            r = subprocess.run(CC + [os.path.relpath(pp, ROOT), "-o", os.path.relpath(obj, ROOT)],
                               capture_output=True, text=True, timeout=180)
            if r.returncode or not os.path.exists(obj):
                print("COMPILE FAIL", src, (r.stdout + r.stderr).strip().splitlines()[-1:] if (r.stdout + r.stderr).strip() else "")
                continue
            for name, size, words, masks in funcs_of(obj):
                k = next((j for j, m in enumerate(masks) if m == 0xFFFFFFFF), None)
                hits = []
                if k is not None and size >= 16:
                    for p in index.get(words[k], []):
                        s = p - k
                        if s < 0 or s + len(words) > len(W):
                            continue
                        if all((W[s + j] ^ words[j]) & masks[j] == 0 for j in range(len(words))):
                            hits.append(base + 0x40 * 0 + s * 4)
                if hits:
                    found += 1
                rows.append([os.path.relpath(src, ROOT), name, size, "0x%X" % hits[0] if hits else "", len(hits)])
            os.remove(obj)
    with open(outp, "w", newline="") as fh:
        w = csv.writer(fh); w.writerow(["src", "func", "size", "g_addr", "hits"]); w.writerows(rows)
    print("%d of %d functions found in %s" % (found, len(rows), mod))


if __name__ == "__main__":
    main()
