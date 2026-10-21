#!/usr/bin/env python3
"""readd.py SLOTS.txt: re-register dropped files (docs/handoff/dropped_registrations.txt, reason rodata-unslotted)
whose .rodata slots tools/local/find_rodata.py found, then `VERSION=mhg tools/rebuild.sh`; keeps only what links 7/7
(bisects a failing batch). Edits config/mhg/c_files.txt; does not commit."""
import os, re, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)
CF = "config/mhg/c_files.txt"

def build_ok():
    out = subprocess.run(["sh", "tools/rebuild.sh"], env=dict(os.environ, VERSION="mhg"), capture_output=True, text=True).stdout
    ok = [l for l in out.splitlines() if re.match(r"^\w+\s+OK\s", l)]
    return len(ok) == 7

slots = {}
for l in open(sys.argv[1]):
    if not l.startswith("#") and ":rodata " in l:
        f = l.split(); slots.setdefault(f[3], []).append(l.strip())
missing = {l.split()[3] for l in open(sys.argv[1]) if l.startswith("#")}
text = {}
for l in open("docs/handoff/dropped_registrations.txt"):
    if l.startswith("rodata-unslotted\t"):
        e = l.split("\t")[1].strip(); f = e.split()
        if ":" not in f[0]: text.setdefault(f[3], []).append(e)
groups = [p for p in text if p in slots and p not in missing]
base = open(CF).read()
print(len(groups), "files to re-add")

def try_set(gs):
    open(CF, "w").write(base_now() + "".join("\n".join(text[p] + slots[p]) + "\n" for p in gs))
    return build_ok()

kept = []
def base_now():
    return base + "".join("\n".join(text[p] + slots[p]) + "\n" for p in kept)

def solve(gs):
    if not gs: return
    if try_set(gs):
        kept.extend(gs); print("OK", len(gs), "files", flush=True); return
    if len(gs) == 1:
        print("FAIL", gs[0], flush=True); return
    h = len(gs) // 2
    solve(gs[:h]); solve(gs[h:])

solve(groups)
open(CF, "w").write(base_now())
build_ok()
print("kept", len(kept), "of", len(groups))
