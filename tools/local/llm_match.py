#!/usr/bin/env python3
"""llm_match.py FILE FUNC [--model M] [--rounds N] [--write]

Local-model matching loop for PS2 Monster Hunter G (VERSION=mhg). Asks a local Ollama model to
rewrite one C function so it compiles to G's bytes, checks every attempt with tools/check.py
(Metrowerks compile + relocation-masked compare), and keeps the best attempt.

The G version is inserted as `#if defined(VERSION_MHG) ... #else <MH1 original> #endif`, so the
MH1 build keeps its own code (there is no JP MH1 disc here to re-verify it).

--write keeps a full match in FILE; registering it in config/mhg (and the 7/7 link check) is
docs/handoff/reg.py's job. Every attempt is appended to build/mhg/llm/log.csv.
Standard library only; Ollama at $OLLAMA_URL (default http://localhost:11434, from WSL the
Windows host is reached through curl.exe interop).
"""
import argparse, csv, json, os, re, subprocess, sys, time, urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ENV = dict(os.environ, VERSION="mhg")


def ollama_url():
    if os.environ.get("OLLAMA_URL"):
        return os.environ["OLLAMA_URL"]
    for host in ["localhost"] + [l.split()[1] for l in open("/etc/resolv.conf") if l.startswith("nameserver")]:
        try:
            urllib.request.urlopen("http://%s:11434/api/version" % host, timeout=2)
            return "http://%s:11434" % host
        except Exception:
            pass
    return "interop"   # WSL NAT: reach the Windows host's localhost through curl.exe


def check(path, func):
    """(ndiff, ninsn, diff text) of FUNC in PATH; ndiff None when it does not compile."""
    r = subprocess.run(["python3", "tools/check.py", path, "-v"], cwd=ROOT, env=ENV,
                       capture_output=True, text=True, timeout=240)
    out = r.stdout + r.stderr
    block, keep = [], False
    for l in out.splitlines():
        m = re.match(r"^(OK|--|\?\?)\s+(\S+)", l)
        if m:
            keep = m.group(2) == func
            if keep and m.group(1) == "OK":
                return 0, 0, l
        if keep:
            block.append(l)
    if not block:
        return None, 0, out[-1500:]
    m = re.search(r"\((\d+)/(\d+) instructions differ\)", block[0])
    if not m:
        return None, 0, "\n".join(block)[:1500]
    return int(m.group(1)), int(m.group(2)), "\n".join(block)


def find_func(src, func):
    """(start, end) of FUNC's definition (return type line through closing brace)."""
    m = re.search(r"^[^\n;{}#]*\b%s\s*\([^;{]*\)\s*\{" % re.escape(func), src, re.M)
    if not m:
        return None
    i, depth = m.end() - 1, 0
    while i < len(src):
        depth += {"{": 1, "}": -1}.get(src[i], 0)
        i += 1
        if depth == 0:
            return m.start(), i
    return None


def ask(url, model, prompt):
    body = json.dumps({"model": model, "prompt": prompt, "stream": False, "think": False,
                       "options": {"num_ctx": 16384, "temperature": 0.4}}).encode()
    t = time.time()
    if url == "interop":
        bf = os.path.join(ROOT, "build/mhg/llm/req_%d.json" % os.getpid())
        open(bf, "wb").write(body)
        win = subprocess.run(["wslpath", "-w", bf], capture_output=True, text=True).stdout.strip()
        out = subprocess.run(["curl.exe", "-s", "-m", "900", "-H", "Content-Type: application/json",
                              "--data-binary", "@" + win, "http://localhost:11434/api/generate"],
                             capture_output=True, timeout=960).stdout
        r = json.loads(out)
    else:
        r = json.load(urllib.request.urlopen(urllib.request.Request(url + "/api/generate", body,
                      {"Content-Type": "application/json"}), timeout=900))
    return r["response"], time.time() - t


def extract_c(text, func):
    m = re.search(r"```(?:c|C)?\n(.*?)```", text, re.S)
    code = m.group(1) if m else text
    span = find_func(code, func)
    return code[span[0]:span[1]] if span else None


PROMPT = """You are matching a decompilation of a PS2 game built with Metrowerks CodeWarrior (mwccps2, -O4,p).
The C function below compiles to the RIGHT column of the listing; the original game (Monster Hunter G) has
the LEFT column. Lines marked ">>" differ ("(reloc)" lines are addresses and are fine).
Rewrite ONLY this function so it compiles to the left column exactly. Keep its name and signature. Use the
same types, macros and names already in the code; do not invent new globals or headers. Small changes
(statement order, temporaries, casts, signedness, loop form, constants, struct offsets) are usually enough.
Reply with the complete function in one ```c block and nothing else.

Current C:
```c
{code}
```

Listing (original | ours):
{diff}
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("file"); ap.add_argument("func")
    ap.add_argument("--model", default="qwen2.5-coder:7b")
    ap.add_argument("--rounds", type=int, default=4)
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--save", help="on a match, save the G function to SAVE/FUNC.c")
    a = ap.parse_args()
    os.chdir(ROOT)
    url = ollama_url()
    orig = open(a.file).read()
    span = find_func(orig, a.func)
    if not span:
        sys.exit("function %s not found in %s" % (a.func, a.file))
    mh1 = orig[span[0]:span[1]]
    base, ninsn, diff = check(a.file, a.func)
    if base == 0:
        print("ALREADY", a.file, a.func); return
    if base is None:
        sys.exit("baseline does not compile/compare:\n" + diff)
    tmp = os.path.join(os.path.dirname(a.file), ".llm_%s_%s" % (a.func, os.path.basename(a.file)))
    best, best_code, cur_code, cur_diff = base, None, mh1, diff
    os.makedirs("build/mhg/llm", exist_ok=True)
    logf = open("build/mhg/llm/log.csv", "a", newline="")
    log = csv.writer(logf)
    try:
        for rnd in range(1, a.rounds + 1):
            open("build/mhg/llm/model_current.txt", "w").write("%s  %s  %s  round %d/%d  (start: %d of %d instructions differ, best so far %d)\n" % (
                a.model, a.file, a.func, rnd, a.rounds, base, ninsn, best))
            reply, secs = ask(url, a.model, PROMPT.format(code=cur_code, diff=cur_diff[:9000]))
            cand = extract_c(reply, a.func)
            nd = None
            if cand:
                g = "#if defined(VERSION_MHG)\n%s\n#else\n%s\n#endif" % (cand, mh1)
                open(tmp, "w").write(orig[:span[0]] + g + orig[span[1]:])
                nd, _, d = check(tmp, a.func)
            print("round %d: %s diff (baseline %d/%d) %.0fs" % (rnd, "no code" if not cand else ("compile error" if nd is None else nd), base, ninsn, secs))
            log.writerow([time.strftime("%Y-%m-%dT%H:%M:%S"), a.model, a.file, a.func, ninsn, base, rnd,
                          "" if nd is None else nd, round(secs, 1)])
            logf.flush()
            if nd is not None and nd < best:
                best, best_code = nd, cand
                cur_code, cur_diff = cand, d
            if best == 0:
                break
    finally:
        if os.path.exists(tmp):
            os.remove(tmp)
    if best == 0:
        print("MATCH", a.file, a.func, a.model)
        if a.save:
            os.makedirs(a.save, exist_ok=True)
            open(os.path.join(a.save, a.func + ".c"), "w").write("// %s  (%s)\n%s\n" % (a.file, a.model, best_code))
        if a.write:
            open(a.file, "w").write(orig[:span[0]] + "#if defined(VERSION_MHG)\n%s\n#else\n%s\n#endif" % (best_code, mh1) + orig[span[1]:])
    else:
        print("BEST %d of %d (baseline %d)" % (best, ninsn, base), a.file, a.func, a.model)


if __name__ == "__main__":
    main()
