#!/usr/bin/env python3
"""pool_align.py - candidate G -> US string pairs by aligning two string pools in address order (agent E, 15 Oct 2026).

For tables the structural pairing of tools/text_map.py cannot reach (the US functions differ from the JP ones, so no code
reference leads to them) the US build's strings often lie in the same order as G's. This prints the NUL-terminated strings
of a G range next to the best order-preserving alignment with a US range (Needleman-Wunsch with a gap cost; the score
rewards equal printf conversions, colour codes and line counts, a plausible length ratio and a few UI words). It is a
candidate list for a human: about one pair in three is wrong where G edited a line, so every pair that goes into
tools/text_pairs_manual.txt must be read on both sides first. Nothing is written; the output holds game text (do not commit it).

    python3 tools/pool_align.py sub_main 0x537f90 0x539600 sub_main 0x5BA7A0 0x5BB960
    python3 tools/pool_align.py lobby 0x63EB28 0x641100 lobby 0x699580 0x69B540

A line starts with * when the G string has no English in build/text/en_mhg.txt yet. Output: g ADDR TEXT => x ADDR TEXT [score].
Standard library only (plus text_map, which reads the discs under disc/).
"""
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import text_map as tm  # noqa: E402

CODE = re.compile(rb"~[A-Z]\d\d")
WORDS = {"ハンター": ("hunter",), "ギルド": ("guild",), "クエスト": ("quest",), "アイテム": ("item",), "ボックス": ("box",),
         "武器": ("weapon",), "防具": ("armor",), "調合": ("combin",), "報酬": ("reward",), "購入": ("buy",), "売": ("sell", "sold"),
         "選択": ("select",), "名前": ("name",), "装備": ("equip",), "残り": ("remain", "left"), "時間": ("time",),
         "ポイント": ("point",), "終了": ("end", "quit"), "失敗": ("fail",), "成功": ("clear", "success"), "回復": ("health", "recover")}


def chunks(img, space, lo, hi):
    base, b = img.mod[space]
    out, i = [], lo - base
    while i < hi - base:
        j = b.find(b"\0", i)
        if j < 0:
            break
        if j > i + 1 and all(c >= 0x20 or c == 10 for c in b[i:j]) and texty(b[i:j]):
            out.append((base + i, b[i:j]))
        i = j + 1
    return out


def texty(b):
    """mostly letters, kana, kanji: not a pointer table or a number"""
    n = k = 0
    while k < len(b):
        c = b[k]
        if 0x41 <= c <= 0x5A or 0x61 <= c <= 0x7A or c in (0x20, 0x7E, 0x25, 0x3A, 0x2E, 0x21, 0x3F, 0x0A):
            n += 1
            k += 1
        elif tm.td.dbl(b, k) and b[k] >= 0x81:
            n += 2
            k += 2
        else:
            k += 1
    return n >= 0.7 * len(b)


def jp_len(b):
    n = i = 0
    while i < len(b):
        if tm.td.dbl(b, i):
            n += 1
            i += 2
        else:
            n += 0.5
            i += 1
    return max(n, 1)


def score(gb, xb):
    if tm.conv_sig(gb) != tm.conv_sig(xb):
        return None
    cg, cx = CODE.findall(gb), CODE.findall(xb)
    s = 1.5 if cg == cx else (-1.0 if cg and cx else (-1.5 if cg or cx else 0))
    d = abs(gb.count(b"\n") - xb.count(b"\n"))
    s += 0.7 if not d else -0.4 * min(d, 3)
    s += 1.0 - 1.2 * abs(math.log(max(len(xb), 1) / jp_len(gb) / 2.3))
    t, xl = gb.decode("cp932", "replace"), xb.decode("latin1").lower()
    hit = miss = 0
    for w, ens in WORDS.items():
        if w in t:
            if any(e in xl for e in ens):
                hit += 1
            else:
                miss += 0.4
    return s + min(hit, 4) * 1.4 - min(miss, 3) * 0.4


def align(gl, xl, gap=-0.9):
    n, m = len(gl), len(xl)
    D = [[0.0] * (m + 1) for _ in range(n + 1)]
    B = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(1, n + 1):
        D[i][0], B[i][0] = D[i - 1][0] + gap, 1
    for j in range(1, m + 1):
        D[0][j], B[0][j] = D[0][j - 1] + gap, 2
    for i in range(1, n + 1):
        for j in range(1, m + 1):
            best, bk = D[i - 1][j] + gap, 1
            if D[i][j - 1] + gap > best:
                best, bk = D[i][j - 1] + gap, 2
            sc = score(gl[i - 1][1], xl[j - 1][1])
            if sc is not None and D[i - 1][j - 1] + sc > best:
                best, bk = D[i - 1][j - 1] + sc, 3
            D[i][j], B[i][j] = best, bk
    i, j, out = n, m, {}
    while i > 0 or j > 0:
        k = B[i][j]
        if k == 3:
            out[i - 1] = (j - 1, score(gl[i - 1][1], xl[j - 1][1]))
            i, j = i - 1, j - 1
        elif k == 1:
            i -= 1
        else:
            j -= 1
    return out


def dec(b):
    return b.decode("cp932", "replace").replace("\n", "\\n")


def main():
    a = sys.argv[1:]
    if len(a) != 6:
        sys.exit(__doc__)
    g = tm.Img("g", os.path.join(tm.ROOT, "disc", "mhg"), "SLPM_658.69")
    us = tm.Img("us", os.path.join(tm.ROOT, "disc", "mh1_us"), "SLUS_208.96")
    have = set()
    p = os.path.join(tm.ROOT, "build", "text", "en_mhg.txt")
    if os.path.exists(p):
        for line in open(p, "rb"):
            if line[:1] != b"#" and b" = " in line:
                pp = line.split(b" = ")[0].decode().split(":")
                if len(pp) == 2 and pp[1].startswith("0x"):
                    have.add((pp[0], int(pp[1], 16)))
    gl = chunks(g, a[0], int(a[1], 16), int(a[2], 16))
    xl = chunks(us, a[3], int(a[4], 16), int(a[5], 16))
    al = align(gl, xl)
    used = {j for j, _s in al.values()}
    for i, (ga, gb) in enumerate(gl):
        star = " " if (("main" if a[0] == "sub_main" else a[0]), ga) in have else "*"
        if i in al:
            j, sc = al[i]
            print("%s g%X %s => x%X %s [%.1f]" % (star, ga, dec(gb)[:36], xl[j][0], dec(xl[j][1])[:46], sc))
        else:
            print("%s g%X %s => ---" % (star, ga, dec(gb)[:36]))
    for j, (xa, xb) in enumerate(xl):
        if j not in used:
            print("    US-only x%X %s" % (xa, dec(xb)[:50]))


if __name__ == "__main__":
    main()
