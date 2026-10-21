#!/usr/bin/env python3
"""perm_setup.py FILE FUNC [MODULE]: make a decomp-permuter directory build/mhg/perm/FUNC for one G function.

target.o is G's own asm for FUNC (asm/mhg/<module>/text/*.s, assembled with the build's flags); base.c is FILE
preprocessed for G (VERSION_MHG) with every other function stripped; compile.sh runs mwccps2 through wibo with
tools/check.py's flags. Then: tools/permuter/permuter.py build/mhg/perm/FUNC -j8 --stop-on-zero
"""
import glob, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(ROOT)
BU = "tools/binutils/bin/mips64r5900el-ps2-elf-"
AS_FLAGS = ["-EL", "-march=r5900", "-mabi=eabi", "-G0", "-no-pad-sections", "-Iinclude"]
CFLAGS = "-c -O4,p -nostdinc -stderr -Iinclude -Ibuild/mhg/raw -Ibuild/raw -pragma \"divbyzerocheck on\" -DVERSION_MHG"


def main():
    path, func = sys.argv[1], sys.argv[2]
    mods = [sys.argv[3]] if len(sys.argv) > 3 else ["main", "sub_main", "select", "game", "yn", "lobby", "nethttp"]
    src = None
    for m in mods:
        for f in glob.glob("asm/mhg/%s/**/*.s" % m, recursive=True):
            t = open(f).read()
            if re.search(r"^glabel %s\b" % re.escape(func), t, re.M):
                src = t
                break
        if src:
            break
    if not src:
        sys.exit("no G asm for %s" % func)
    lines = src.splitlines()
    head = [l for l in lines[:8] if l.startswith(".")]
    i = next(k for k, l in enumerate(lines) if re.match(r"glabel %s\b" % re.escape(func), l))
    j = next(k for k in range(i, len(lines)) if re.match(r"endlabel %s\b" % re.escape(func), lines[k]))
    d = "build/mhg/perm/%s" % func
    os.makedirs(d, exist_ok=True)
    open(d + "/target.s", "w").write("\n".join(head + lines[i:j + 1]) + "\n")
    r = subprocess.run([BU + "as"] + AS_FLAGS + [d + "/target.s", "-o", d + "/target.o"], capture_output=True, text=True)
    if r.returncode:
        sys.exit("assembling target failed:\n" + r.stderr[-2000:])
    pp = subprocess.run(["cpp", "-P", "-undef", "-nostdinc", "-Iinclude", "-Ibuild/mhg/raw", "-Ibuild/raw",
                         "-DVERSION_MHG", "-D__MWERKS__=1", path], capture_output=True, text=True)
    if pp.returncode:
        sys.exit("preprocess failed:\n" + pp.stderr[-2000:])
    open(d + "/base.c", "w").write(pp.stdout)
    subprocess.run([sys.executable, "tools/permuter/strip_other_fns.py", d + "/base.c", func], check=True)
    open(d + "/compile.sh", "w").write(
        "#!/bin/bash\n# compile.sh IN.c -o OUT.o (called by decomp-permuter)\n"
        "in=$(realpath \"$1\"); out=$(realpath -m \"$3\")\n"
        "cd %s && exec timeout 60 tools/compilers/wibo tools/compilers/mwcps2-3.0b52-030722/mwccps2.exe %s "
        "\"$(realpath --relative-to=. \"$in\")\" -o \"$(realpath -m --relative-to=. \"$out\")\"\n" % (ROOT, CFLAGS))
    os.chmod(d + "/compile.sh", 0o755)
    open(d + "/settings.toml", "w").write(
        'func_name = "%s"\ncompiler_type = "mwcc"\nobjdump_command = "%sobjdump -drz -m mips:5900"\n' % (func, os.path.join(ROOT, BU)))
    print(d)


if __name__ == "__main__":
    main()
