#!/usr/bin/env python3
"""dbg_keys.py - a --input pad script from short menu words, for driving the debug menu (src/pc/dbgmenu.c) in
scripted runs (tools/test_dbgmenu.sh). Each press is held 2 ticks, then 2 idle ticks (the menu reads the pad once
per drawn frame and acts on a new press).

    python3 tools/dbg_keys.py "w60 open d5 x x r1 o w300"

Words: open (Back + R3), uN / dN / lN / rN (N presses of the d-pad, N default 1), x (cross), o (circle),
sq (square), l1, r1, st (start), wN (N idle ticks). Standard library only."""
import sys


def build(words):
    out = []

    def press(b):
        out.append(b + "*2")
        out.append("idle*2")
    for t in words.split():
        if t == "open":
            press("select+r3")
        elif t in ("l1", "r1"):
            press(t)
        elif t[0] == "w" and t[1:].isdigit():
            out.append("idle*" + t[1:])
        elif t[0] in "udlr" and (t[1:] == "" or t[1:].isdigit()):
            for _ in range(int(t[1:] or 1)):
                press({"u": "up", "d": "down", "l": "left", "r": "right"}[t[0]])
        elif t in ("x", "o", "sq", "st"):
            press({"x": "cross", "o": "circle", "sq": "square", "st": "start"}[t])
        else:
            sys.exit("dbg_keys: unknown word " + t)
    return ",".join(out)


if __name__ == "__main__":
    print(build(" ".join(sys.argv[1:])))
