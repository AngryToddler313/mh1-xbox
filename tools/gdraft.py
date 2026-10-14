#!/usr/bin/env python3
"""gdraft.py FUNC... : m2c first draft of functions of PS2 Monster Hunter G (asm/mhg/MODULE/text, glabel or alabel), agent D tooling.
usage: gdraft.py [-m lobby] FUNC [FUNC...]     (also: --names appends the D_<addr> data names it sees to config/mhg/pc_names.txt)
gp-relative accesses (`OFF($28)`) are rewritten to %gp_rel(NAME) with NAME from G's main symbols (else gp_<addr>); the printed
C is only a draft: field offsets of moved structs, G addresses of data and MH1-layout buffers need the same care as any G copy."""
import argparse, glob, os, re, subprocess, sys, tempfile, bisect
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
M2C = os.path.join(ROOT, "tools/m2c/m2c.py")
PY = os.path.join(ROOT, ".venv/bin/python")
GP = 0x2F2B70
GPR = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
       "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]

def named_regs(t):
    return re.sub(r"\$(\d+)\b", lambda m: "$" + (GPR[int(m.group(1))] if int(m.group(1)) < 32 else m.group(1)), t)

def gp_syms():
    d = {}
    for mod in ("main", "sub_main"):
        p = os.path.join(ROOT, "build/mhg/pc/symbols/%s.txt" % mod)
        if os.path.exists(p):
            for l in open(p):
                m = re.match(r"(\w+) = 0x([0-9A-Fa-f]+);", l)
                if m:
                    d.setdefault(int(m[2], 16), m[1])
    return d

def block(module, name):
    for path in glob.glob(os.path.join(ROOT, "asm/mhg", module, "text", "*.s")):
        t = open(path).read()
        m = re.search(r"^ *[ga]label %s\n(.*?)(?=^ *(?:[ga]label|endlabel|nonmatching) |\Z)" % re.escape(name), t, re.M | re.S)
        if m:
            return "glabel %s\n%s" % (name, m.group(1))
    return None

def jump_tables(module, text):
    out = ""
    names = sorted(set(re.findall(r"%hi\(((?:lit_\d+(?:_[0-9A-F]+)?|jtbl_[0-9A-F]+))\)", text)))
    labels = set(re.findall(r"\.L([0-9A-F]{8}):", text))
    for n in names:
        for path in glob.glob(os.path.join(ROOT, "asm/mhg", module, "data", "data", "*.s")):
            t = open(path).read()
            m = re.search(r"^dlabel %s\n(.*?)^enddlabel" % n, t, re.M | re.S)
            if not m:
                continue
            words = re.findall(r"\.word (?:0x|\.L)([0-9A-Fa-f]{8})\s*$", m.group(1), re.M)
            while words and int(words[-1], 16) == 0:     # the table is followed by padding words
                words.pop()
            for w in set(x.upper() for x in words):
                if w not in labels:
                    text, cnt = re.subn(r"^(\s*/\* [0-9A-F]+ %s )" % w, ".L%s:\n\\1" % w, text, count=1, flags=re.M)
                    if cnt:
                        labels.add(w)
            if words and all(w.upper() in labels for w in words):
                jt = n.replace("lit_", "jtbl_")
                if jt != n:
                    text = text.replace(n, jt)
                out += ".section .rodata\nglabel %s\n" % jt + "".join("    .word .L%s\n" % w.upper() for w in words) + "\n"
            break
    return text, out

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("funcs", nargs="+")
    ap.add_argument("-m", default="lobby")
    ap.add_argument("--names", action="store_true")
    a = ap.parse_args()
    gs = gp_syms()
    ks = sorted(gs)
    allD = set()
    for fn in a.funcs:
        b = block(a.m, fn)
        if not b:
            print("/* %s: not found */" % fn); continue
        b, jt = jump_tables(a.m, b)
        def gp(m):
            off = int(m.group(1), 16)
            off = off - 0x10000 if off & 0x8000 and off > 0x7FFF and m.group(1).startswith("0x") and False else off
            return m.group(0)
        def rep(m):
            sign = -1 if m.group(1) == "-" else 1
            addr = GP + sign * int(m.group(2), 16)
            n = gs.get(addr)
            if n is None:
                i = bisect.bisect_right(ks, addr) - 1
                n = "gp_%06X" % addr
            return "%%gp_rel(%s)($28)" % n
        b = re.sub(r"(-?)0x([0-9A-Fa-f]+)\(\$28\)", lambda m: rep(m) if True else m.group(0), b)
        allD |= set(re.findall(r"\b(D_[0-9A-Fa-f]{8})\b", b))
        with tempfile.NamedTemporaryFile("w", suffix=".s", delete=False) as t:
            t.write('.include "macro.inc"\n.set noat\n.set noreorder\n.section .text, "ax"\n\n' + named_regs(b) + jt)
        p = subprocess.run([PY, M2C, "-t", "mips-mwcc-c", "--valid-syntax", t.name], capture_output=True, text=True)
        os.unlink(t.name)
        print(p.stdout.strip() or p.stderr.strip()); print()
    if a.names and allD:
        known = set(re.findall(r"^\S+ (\S+) ", open(os.path.join(ROOT, "config/mhg/pc_names.txt")).read(), re.M))
        with open(os.path.join(ROOT, "config/mhg/pc_names.txt"), "a") as f:
            for d in sorted(allD - known):
                f.write("%s %s 0x%s  # G data referenced by an agent-D G copy (gdraft.py)\n" % (a.m, d, d[2:]))
                print("name added:", d, file=sys.stderr)

if __name__ == "__main__":
    sys.exit(main())
