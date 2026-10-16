#!/usr/bin/env python3
"""ps2_patch.py - patch the player's own Monster Hunter (PS2, Japan, SLPM-65495) disc image so that it goes online
with OUR server (tools/server/mh1_server.py) on a real PS2 or in PCSX2. docs/network.md "PS2 patch".

    tools/ps2_patch.py DISC --server HOST:PORT [--login 12345678 --password XXXXXXXXXXXXXXXX]
                       [--out PATCHED.iso | --ppf FILE.ppf] [--dry-run] [--disasm]

DISC is the .iso of the disc (2048-byte sectors) or a folder with its files (then --out is a folder that gets a patched
SLPM_654.95 and AFS_DATA.AFS). What changes, and nothing else:
  (a) DNAS is skipped: ms_network_bb_authentication (main, src/main/net/ms01.c) takes "authenticated" at once
      instead of asking InetDNAS2ConnectNetStart (Sony's DNAS servers are gone).
  (b) the KDDI portal (the in-game browser over HTTPS, also gone) is replaced by our internet_browser
      (src/ps2patch/online.c), which fills the server table the portal used to leave: HOST:PORT (a name, looked up
      with the console's DNS, or an address), the login and password of the player's mh1-server account.
The game's own lobby client then logs in to that server as before (the same protocol the PC port speaks).

How: the patched code is compiled HERE from this repository's C (-DMH1_ONLINE_PATCH) with the PS2 compiler of the
byte-matching rebuild (README.md Phase 1: tools/compilers, tools/binutils) and linked at the original addresses with
the rebuild's symbols (build/main.elf, build/lobby.elf). The rebuild must be OK (tools/rebuild.sh main lobby) and
identical to DISC's own files; the patched bytes are taken from that rebuild and written into a copy of DISC. So the
patch is made on the player's machine from the player's own disc; no Capcom bytes are in this repository. The
patched image is the player's copy of the game and the .ppf holds his login: keep both to yourself.

Exit status 0 = done, 1 = a check failed (wrong disc, rebuild not matching, patch does not fit), 2 = usage.
Standard library only (plus the rebuild's toolchain).
"""
import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import disc_files  # noqa: E402

BU = os.path.join(ROOT, "tools/binutils/bin/mips64r5900el-ps2-elf-")
WIBO = os.path.join(ROOT, "tools/compilers/wibo")
MWCC = os.path.join(ROOT, "tools/compilers/mwcps2-3.0b52-030722/mwccps2.exe")
CFLAGS = ["-c", "-O4,p", "-nostdinc", "-stderr", "-pragma", "divbyzerocheck on"]   # as tools/build.py
OUT = "build/ps2patch"
GP = 0x0038EB70

# What is replaced. module, C file, range [lo, hi) that the compiled file takes (the file's own original range, or the
# replaced function's), the ranges allowed to differ afterwards (the patched function only), what to check.
UNITS = [
    dict(name="dnas", module="main", src="src/main/net/ms01.c", lo=0x267FD0, hi=0x2685B4,
         changes=[(0x268040, 0x2685B4)], what="ms_network_bb_authentication: DNAS skipped",
         gone={0xA769C0: "InetDNAS2ConnectNetStart"}),
    dict(name="portal", module="lobby", src="src/ps2patch/online.c", lo=0x5B5980, hi=0x5B5C50,
         changes=[(0x5B5980, 0x5B5C50)], what="internet_browser: the KDDI portal replaced by our server table",
         gone={}),     # the browser calls (MainBsInitialize, MainBrowser, MainBsDispose): filled in from the symbols
]
PORTAL_GONE = ["MainBsInitialize", "MainBrowser", "MainBsDispose"]

# never: MH Oldschool's servers (docs/network.md 2.1; the same list as src/pc/net/net_cpinet.c) and the original
# service's names. Their operators serve unpatched discs themselves; this patch is for our own server only.
DENY_IP = {"34.75.107.68", "151.80.238.99", "151.80.238.101", "151.80.238.104"}
DENY_NAME = ("mholdschool.com", "capcom.co.jp", "kddi-mmbb.jp", "dnas.playstation.org")

MAIN_VRAM = 0x00100000


class Fail(Exception):
    pass


def run(cmd, cwd=ROOT):
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if p.returncode:
        raise Fail("%s\n%s%s" % (" ".join(cmd), p.stdout, p.stderr))
    return p.stdout


def sha1(b):
    return hashlib.sha1(b).hexdigest()


# ------------------------------------------------------------------------------------------------- the player's disc
class Disc:
    """SLPM_654.95 and AFS_DATA.AFS of DISC (an ISO or a folder): bytes, and where each file starts in the image"""

    def __init__(self, path):
        self.path = path
        self.src, self.iso = disc_files.open_source(path)
        game = disc_files.verify(self.src, self.iso, ["SLPM_654.95", "AFS_DATA.AFS"])
        if game != "mh1":
            raise Fail("this patch is for Monster Hunter (SLPM-65495) only, not Monster Hunter G")
        if self.iso is not None and self.iso.secsz != 2048:
            raise Fail("only plain .iso images (2048-byte sectors) can be patched; convert the BIN/CUE to .iso first")
        self.files = {}
        for name in ("SLPM_654.95", "AFS_DATA.AFS"):
            sz, ref = self.src.files[name]
            self.files[name] = dict(size=sz, lba=ref if self.iso is not None else None)

    def read(self, name, off, n):
        sz, ref = self.src.files[name]
        if self.iso is None:
            with open(ref, "rb") as f:
                f.seek(off)
                return f.read(n)
        self.iso.f.seek(ref * 2048 + off)
        return self.iso.f.read(n)

    def image_offset(self, name, off):
        """offset in the ISO of byte `off` of file `name` (files are contiguous on the disc)"""
        return self.files[name]["lba"] * 2048 + off


def elf_main(disc):
    """file offset and address of SLPM_654.95's 'main' section (the code the rebuild's main.bin is)"""
    head = disc.read("SLPM_654.95", 0, 0x40)
    shoff, = struct.unpack_from("<I", head, 32)
    shnum, shstrndx = struct.unpack_from("<HH", head, 48)
    secs = [struct.unpack("<10I", disc.read("SLPM_654.95", shoff + i * 40, 40)) for i in range(shnum)]
    st = secs[shstrndx]
    names = disc.read("SLPM_654.95", st[4], st[5])
    for s in secs:
        if names[s[0]:names.index(b"\0", s[0])] == b"main":
            if s[3] != MAIN_VRAM:
                raise Fail("unexpected 'main' section address 0x%X" % s[3])
            return s[4], s[5]
    raise Fail("SLPM_654.95 has no 'main' section")


def afs_lobby(disc):
    """offset in AFS_DATA.AFS and size of the lobby.bin overlay (an uncompressed MWo3 module in MH1)"""
    head = disc.read("AFS_DATA.AFS", 0, 8)
    count, = struct.unpack_from("<I", head, 4)
    table = disc.read("AFS_DATA.AFS", 8, count * 8)
    for i in range(count):
        off, size = struct.unpack_from("<II", table, i * 8)
        h = disc.read("AFS_DATA.AFS", off, 0x40)
        if h[:4] == b"MWo3" and h[0x20:0x30].rstrip(b"\0") == b"lobby.bin":
            return off, size, struct.unpack_from("<I", h, 8)[0]
    raise Fail("lobby.bin not found in AFS_DATA.AFS")


# ------------------------------------------------------------------------------------------------------ the rebuild
def rebuilt(module):
    p = os.path.join(ROOT, "build/%s.bin" % module)
    e = os.path.join(ROOT, "build/%s.elf" % module)
    if not (os.path.exists(p) and os.path.exists(e)):
        raise Fail("build/%s.bin is missing: run the byte-matching rebuild first (README.md Phase 1, then "
                   "tools/rebuild.sh main lobby)" % module)
    return open(p, "rb").read()


def symbols(modules, exclude):
    """name -> address of every symbol of the rebuilt modules (the first module wins), as a linker script"""
    seen, lines = set(exclude), ["_gp = 0x%X;" % GP]
    seen.add("_gp")
    for m in modules:
        for l in run([BU + "nm", os.path.join(ROOT, "build/%s.elf" % m)]).split("\n"):
            f = l.split()
            if len(f) == 3 and f[1] in "TtDdBbRrAaSsVvWG" and re.match(r"^[A-Za-z_.$][\w.$]*$", f[2]) \
                    and f[2] not in seen:
                seen.add(f[2])
                lines.append("%s = 0x%s;" % (f[2], f[0]))
    return "\n".join(lines) + "\n"


def conf_header(a):
    def c(s):
        return '"%s"' % s.replace("\\", "\\\\").replace('"', '\\"')
    return ("/* written by tools/ps2_patch.py: the player's server and login (MH1PCONF, include/ps2patch.h) */\n"
            "#define MH1P_CONF { %s, %s, %s, %s, %s, %s, %s }\n"
            % (c("MH1SERVER000"), c(a.name), c(a.server), c(a.login), c(a.password), c("0"), c("100")))


def build_unit(u, conf):
    """compile the unit's C with -DMH1_ONLINE_PATCH (or u['defs']) and link it at u['lo']; returns its bytes"""
    os.makedirs(os.path.join(ROOT, OUT), exist_ok=True)
    open(os.path.join(ROOT, OUT, "mh1p_conf.h"), "w").write(conf)
    obj = "%s/%s.o" % (OUT, u["name"])
    run([WIBO, MWCC] + CFLAGS + ["-Iinclude", "-I" + OUT, "-Ibuild/raw"] + u.get("defs", ["-DMH1_ONLINE_PATCH"]) + [u["src"], "-o", obj])
    # what the object may hold: code and initialised data (no small data: lobby.bin is out of the gp range)
    for l in run([BU + "objdump", "-h", obj]).split("\n"):
        f = l.split()
        if len(f) >= 7 and f[0].isdigit() and f[1] not in (".text", ".data", ".rodata", ".mwcats", ".comment") \
                and int(f[2], 16):
            raise Fail("%s: section %s is not allowed in a patch" % (u["src"], f[1]))
    defined = [l.split()[2] for l in run([BU + "nm", obj]).split("\n") if len(l.split()) == 3]
    mods = ["main"] if u["module"] == "main" else [u["module"], "main"]
    open(os.path.join(ROOT, OUT, u["name"] + "_syms.ld"), "w").write(symbols(mods, defined))
    open(os.path.join(ROOT, OUT, u["name"] + ".ld"), "w").write(
        "SECTIONS {\n  . = 0x%X;\n  .patch : { %s(.text) . = ALIGN(16); %s(.rodata) %s(.data) }\n"
        "  /DISCARD/ : { *(.mwcats) *(.comment) *(.reginfo) *(.pdr) }\n}\n" % (u["lo"], obj, obj, obj))
    elf = "%s/%s.elf" % (OUT, u["name"])
    run([BU + "ld", "-EL", "-nostdlib", "--no-warn-mismatch", "-T", "%s/%s.ld" % (OUT, u["name"]),
         "-T", "%s/%s_syms.ld" % (OUT, u["name"]), "-Map", "%s/%s.map" % (OUT, u["name"]), "-o", elf])
    binp = "%s/%s.bin" % (OUT, u["name"])
    run([BU + "objcopy", "-O", "binary", "-j", ".patch", elf, binp])
    code = open(os.path.join(ROOT, binp), "rb").read()
    if len(code) > u["hi"] - u["lo"]:
        raise Fail("%s: %d bytes, more than the %d of the original range 0x%X-0x%X"
                   % (u["src"], len(code), u["hi"] - u["lo"], u["lo"], u["hi"]))
    syms = dict((l.split()[2], int(l.split()[0], 16)) for l in run([BU + "nm", elf]).split("\n")
                if len(l.split()) == 3)
    u["code_end"] = syms.get("mh1p_conf", u["lo"] + len(code))    # code, then the player's settings
    return code + bytes(u["hi"] - u["lo"] - len(code))      # the rest: nop (0)


def disasm(module_bin, base, lo, hi):
    tmp = os.path.join(ROOT, OUT, "dis.bin")
    open(tmp, "wb").write(module_bin[lo - base:hi - base])
    out = run([BU + "objdump", "-D", "-b", "binary", "-m", "mips:5900", "-EL", "--adjust-vma=0x%X" % lo, tmp])
    return [l for l in out.split("\n") if re.match(r"^\s+[0-9a-f]+:\t", l)]


def jal_targets(lines):
    t = set()
    for l in lines:
        m = re.search(r"\tjal\s+0x([0-9a-f]+)", l)
        if m:
            t.add(int(m.group(1), 16))
    return t


def ranges(a, b, base):
    """[(lo, hi)] addresses where a and b differ"""
    out, i, n = [], 0, len(a)
    while i < n:
        if a[i] != b[i]:
            j = i
            while j < n and a[j] != b[j]:
                j += 1
            if out and base + i - out[-1][1] < 16:
                out[-1] = (out[-1][0], base + j)
            else:
                out.append((base + i, base + j))
            i = j
        else:
            i += 1
    return out


# -------------------------------------------------------------------------------------------------------- arguments
def check_args(a):
    m = re.match(r"^([A-Za-z0-9.\-]+):(\d+)$", a.server or "")
    if not m or not 0 < int(m.group(2)) < 65536:
        raise SystemExit("--server must be HOST:PORT (a name or an IPv4 address, and the lobby port), e.g. "
                         "192.168.1.20:10200")
    host = m.group(1).lower()
    if len(a.server) > 0x3F:
        raise SystemExit("--server: at most 63 characters")
    if host in DENY_IP or any(host == d or host.endswith("." + d) for d in DENY_NAME):
        raise SystemExit("--server: %s belongs to MH Oldschool or the original service. This patch is for your own "
                         "server (tools/server/mh1_server.py); MH Oldschool serves unpatched discs itself." % host)
    if not re.match(r"^\d{8,10}$", a.login):
        raise SystemExit("--login: the 8 digits of your mh1-server account")
    if not re.match(r"^[\x21-\x7E]{1,16}$", a.password):
        raise SystemExit("--password: your account's password (up to 16 characters, no spaces)")
    if not re.match(r"^[\x20-\x7E]{1,31}$", a.name):
        raise SystemExit("--name: up to 31 plain characters")


def write_ppf(path, changes, desc):
    """PPF 3.0 (no block check, no undo data): offset (u64), length (u8), bytes"""
    with open(path, "wb") as f:
        f.write(b"PPF30" + bytes([2]) + desc.encode("ascii", "replace")[:50].ljust(50, b" ") + bytes([0, 0, 0, 0]))
        for off, data in changes:
            for i in range(0, len(data), 255):
                chunk = data[i:i + 255]
                f.write(struct.pack("<QB", off + i, len(chunk)) + chunk)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("disc", help="the .iso of your own disc, or a folder with its files")
    ap.add_argument("--server", help="HOST:PORT of the mh1-server lobby (required)")
    ap.add_argument("--login", default="00000000", help="your account's 8-digit login (default: 00000000, only for "
                    "a server started with --open on your own machine)")
    ap.add_argument("--password", default="LOCALTEST0000000", help="your account's password")
    ap.add_argument("--name", default="mh1-server", help="the server's name in the server list")
    ap.add_argument("--out", help="the patched image (default: next to DISC, '... (mh1-server).iso'); for a folder: "
                    "a folder")
    ap.add_argument("--ppf", help="write the change as a PPF 3.0 patch for DISC (an .iso); without --out no image "
                    "is written")
    ap.add_argument("--dry-run", action="store_true", help="build and check the patch, write no image")
    ap.add_argument("--disasm", action="store_true", help="print the patched functions' code")
    a = ap.parse_args()
    check_args(a)
    try:
        disc = Disc(a.disc)
        moff, msize = elf_main(disc)
        loff, lsize, lload = afs_lobby(disc)
        base = {"main": (MAIN_VRAM, "SLPM_654.95", moff, msize), "lobby": (lload, "AFS_DATA.AFS", loff, lsize)}
        img = {}
        for m, (vram, fname, foff, size) in base.items():
            have = disc.read(fname, foff, size)
            built = rebuilt(m)
            if built != have[:len(built)] or len(built) != len(have):
                raise Fail("the rebuilt %s (build/%s.bin, sha1 %s) is not this disc's (sha1 %s): rebuild from this disc "
                           "(tools/rebuild.sh main lobby must say OK)" % (m, m, sha1(built), sha1(have)))
            img[m] = bytearray(built)
            print("%-6s rebuild OK, identical to %s (%d bytes)" % (m, fname, size))
        conf = conf_header(a)
        lobby_syms = dict((l.split()[2], int(l.split()[0], 16)) for l in
                          run([BU + "nm", os.path.join(ROOT, "build/lobby.elf")]).split("\n") if len(l.split()) == 3)
        for n in PORTAL_GONE:
            UNITS[1]["gone"][lobby_syms[n]] = n
        changes = []
        for u in UNITS:
            vram, fname, foff, size = base[u["module"]]
            code = build_unit(u, conf)
            old = bytes(img[u["module"]])
            img[u["module"]][u["lo"] - vram:u["hi"] - vram] = code
            diff = ranges(old, img[u["module"]], vram)
            for lo, hi in diff:
                if not any(c0 <= lo and hi <= c1 for c0, c1 in u["changes"]):
                    raise Fail("%s: 0x%X-0x%X changed outside the patched function" % (u["name"], lo, hi))
            was = disasm(old, vram, u["lo"], u["hi"])
            now = disasm(img[u["module"]], vram, u["lo"], u["code_end"])
            for t, n in u["gone"].items():
                if t not in jal_targets(was):
                    raise Fail("%s: the original does not call %s here: wrong disc or wrong symbols" % (u["name"], n))
                if t in jal_targets(now):
                    raise Fail("%s: the patched code still calls %s" % (u["name"], n))
            print("%-6s %s: %d bytes changed in 0x%X-0x%X; calls gone: %s" % (
                u["name"], u["what"], sum(hi - lo for lo, hi in diff), u["lo"], u["hi"],
                ", ".join(sorted(u["gone"].values()))))
            if a.disasm:
                print("\n".join("    " + l.strip() for l in now if not l.rstrip().endswith("nop")))
                if u["code_end"] < u["hi"]:
                    d = bytes(img[u["module"]][u["code_end"] - vram:u["hi"] - vram])
                    print("    %X: settings %s" % (u["code_end"], [x.decode("ascii", "replace")
                                                                    for x in d.split(b"\0") if x]))
            for lo, hi in diff:
                changes.append((fname, foff + lo - vram, bytes(img[u["module"]][lo - vram:hi - vram])))
        if a.server.encode() not in bytes(img["lobby"][0x5B5980 - lload:0x5B5C50 - lload]):
            raise Fail("the server address is not in the patched internet_browser")
        print("server %s, login %s (the patched lobby.bin holds them)" % (a.server, a.login))
        if a.dry_run:
            print("dry run: nothing written")
            return 0
        if disc.iso is not None:
            img_changes = [(disc.image_offset(fn, off), data) for fn, off, data in changes]
            if a.ppf:
                write_ppf(a.ppf, img_changes, "MH1 online patch for mh1-server")
                print("written: %s (%d changes; apply it to %s with any PPF 3.0 tool)" % (
                    a.ppf, len(img_changes), os.path.basename(a.disc)))
            if a.ppf and not a.out:
                return 0
            out = a.out or re.sub(r"(\.iso)?$", " (mh1-server).iso", a.disc, count=1, flags=re.I)
            if os.path.abspath(out) == os.path.abspath(a.disc):
                raise Fail("--out must not be the original image")
            print("copying %s -> %s ..." % (a.disc, out))
            shutil.copyfile(a.disc, out)
            with open(out, "r+b") as f:
                for off, data in img_changes:
                    f.seek(off)
                    f.write(data)
            print("written: %s (%d changes)" % (out, len(img_changes)))
        else:
            out = a.out or os.path.join(a.disc, "mh1-server")
            if os.path.abspath(out) == os.path.abspath(a.disc):
                raise Fail("--out must not be the disc folder")
            os.makedirs(out, exist_ok=True)
            for fname in ("SLPM_654.95", "AFS_DATA.AFS"):
                shutil.copyfile(disc.src.files[fname][1], os.path.join(out, fname))
                with open(os.path.join(out, fname), "r+b") as f:
                    for fn, off, data in changes:
                        if fn == fname:
                            f.seek(off)
                            f.write(data)
            print("written: %s/SLPM_654.95 and AFS_DATA.AFS (copy the other disc files next to them)" % out)
            if a.ppf:
                raise Fail("--ppf needs an .iso")
        return 0
    except (Fail, disc_files.DiscError) as e:
        print("ps2_patch: %s" % e, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
