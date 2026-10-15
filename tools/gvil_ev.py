#!/usr/bin/env python3
"""gvil_ev.py - G village run with a pad script given as tick events (the timing of tools/test_activities.py):
    python3 tools/gvil_ev.py TAG END_TICK "45:circle*2,75:circle*2,150:ddown*2" [ENV=val ...]
Warp (RT_LB_WARP) is given by W=x,z in the environment list (put at village tick 30). Shots: RT_SHOTS=... as env.
Output: build/D/g/TAG.log, TAG.png (+ TAG_<tick>.png)."""
import os, subprocess, sys
tag, end, evs = sys.argv[1], int(sys.argv[2]), sys.argv[3]
env = dict(os.environ)
for a in sys.argv[4:]:
    k, v = a.split("=", 1)
    env[k] = v
steps, t = [], 0
for e in sorted((int(x.split(":")[0]), x.split(":", 1)[1]) for x in evs.split(",") if x):
    if e[0] > t:
        steps.append("idle*%d" % (e[0] - t)); t = e[0]
    steps.append(e[1]); t += int(e[1].split("*")[1]) if "*" in e[1] else 1
steps.append("idle*%d" % max(1, end - t + 60))
if "W" in env:
    env["RT_LB_WARP"] = "30,%s,0" % env.pop("W")
if env.get("GDB"):      # GDB=1: run under gdb (tools/ggdb.sh), backtrace at the crash -> build/D/g/TAG.gdb
    open("build/D/g/gdbcmd.txt", "w").write(open(env["GDBCMD"]).read() if env.get("GDBCMD") else "run\nbt 14\ninfo registers\n")
    args = ["tools/ggdb.sh", tag, ",".join(["@village1"] + steps), str(end / 30.0 + 2), "build/D/g/gdbcmd.txt"]
    os.execvpe("sh", ["sh"] + args, env)
args = ["tools/gvil.sh", tag, ",".join(["@village1"] + steps), str(end / 30.0 + 2)]
os.execvpe("sh", ["sh"] + args, env)
