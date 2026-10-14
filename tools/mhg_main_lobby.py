#!/usr/bin/env python3
"""mhg_main_lobby.py : which words of G's main data point into the lobby overlay (hint_tbl, pit_help_str_tbl, ...)?

MH1's ELF says it (.relmain relocations whose symbol is in the lobby.bin section: the PC's load_main_to_lobby). G's ELF is
stripped, so this tool takes MH1's list (word location + the symbol it sits in), finds the same name in G's symbols
(config/mhg/symbols/main.txt, MH1 names at G addresses) and writes the G locations to config/mhg/pc_main_lobby.txt
(tools/gen_rt_meta.py puts them into rt_meta.c; src/pc/rt/rt_mem.c reads the words' values from G's image).
    python3 tools/mhg_main_lobby.py
Reads disc/mh1/SLPM_654.95 (the owner's disc); writes addresses only."""
import bisect, os, re, struct, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def syms(path):
    out = []
    for l in open(path):
        m = re.match(r"(\w+) = 0x([0-9A-Fa-f]+);", l)
        if m:
            out.append((int(m[2], 16), m[1]))
    out.sort()
    return out

def main():
    elf = open(os.path.join(ROOT, "disc/mh1/SLPM_654.95"), "rb").read()
    shoff = struct.unpack_from("<I", elf, 32)[0]
    shn, shstr = struct.unpack_from("<HH", elf, 48)
    sh = [struct.unpack_from("<10I", elf, shoff + 40 * i) for i in range(shn)]
    so = sh[shstr][4]
    name = lambda s: elf[so + s[0]: elf.index(b"\0", so + s[0])].decode()
    lbsec = next(i for i, s in enumerate(sh) if name(s) == "lobby.bin")
    symt = next(s for s in sh if s[1] == 2)
    # main memory image of MH1 (to read the word's value not needed: only locations)
    m1 = syms(os.path.join(ROOT, "config/symbols/main.txt"))
    k1 = [a for a, _ in m1]
    g = {n: a for a, n in syms(os.path.join(ROOT, "config/mhg/symbols/main.txt"))}
    locs = []
    for s in sh:
        if s[1] != 9 or name(s) != ".relmain":
            continue
        for n in range(s[5] // 8):
            loc, info = struct.unpack_from("<II", elf, s[4] + 8 * n)
            sym = info >> 8
            st = struct.unpack_from("<IIIBBH", elf, symt[4] + 16 * sym)
            if (info & 0xFF) != 2 or st[5] != lbsec or st[1] == 0:
                continue
            locs.append(loc)
    gm = open(os.path.join(ROOT, "build/mhg/split/main.bin"), "rb").read()      # G's main image (value check)
    def gval(a):
        return struct.unpack_from("<I", gm, a - 0x100000)[0] if 0x100000 <= a < 0x100000 + len(gm) - 4 else 0
    out, miss = [], []
    for loc in sorted(set(locs)):
        i = bisect.bisect_right(k1, loc) - 1
        a, n = m1[i]
        cand = None
        if n in g and loc - a < 0x4000:
            cand = g[n] + (loc - a)
        else:       # the symbol has no G name: neighbours keep their order. Nearest named symbols before (X) and after (Y)
            #         with the same distance in G: the block was not re-laid out, use the same delta from X
            j = i
            while j > 0 and m1[j][1] not in g:
                j -= 1
            k = i + 1
            while k < len(m1) - 1 and m1[k][1] not in g:
                k += 1
            xa, xn = m1[j]
            ya, yn = m1[k]
            if g[yn] - g[xn] == ya - xa and loc - xa < ya - xa:
                cand = g[xn] + (loc - xa)
        if cand is not None and 0x542600 <= gval(cand) < 0x700000:
            out.append(cand)
        else:
            miss.append((loc, n))
    with open(os.path.join(ROOT, "config/mhg/pc_main_lobby.txt"), "w") as f:
        f.write("# G main data words that point into lobby.bin (tools/mhg_main_lobby.py): G addresses\n")
        for a in sorted(set(out)):
            f.write("0x%08X\n" % a)
    print("%d words, %d without a G name: %s" % (len(out), len(miss), ", ".join("%X %s" % x for x in miss[:10])))

main()
