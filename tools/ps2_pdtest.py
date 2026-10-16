#!/usr/bin/env python3
"""ps2_pdtest.py - TEST ONLY (never for players): build the two test executables of docs/network.md 3.8, which settle
whether the game's DNAS "personalize" encrypts a plain patch list on a console without the DNAS server step.

    tools/ps2_pdtest.py DISC [--out DIR]

DISC is the owner's disc (.iso or folder, as tools/ps2_patch.py). Writes DIR/install/SLPM_654.95 and
DIR/probe/SLPM_654.95 (default DIR build/ps2patch/pdtest; gitignored, they hold Capcom code): the disc's own
executable with src/ps2patch/pdtest.c (MWCC, -DMH1_PDTEST -DPDTEST_MODE=1 / 2) linked over the body of
ms_net_patch_set (0x2687E0-0x26A2E8) and PatchLoadinDNAS_Main's first two instructions turned into
`j pdtest_main; nop`. Nothing else changes (checked). Run them with tools/run_pcsx2.sh --elf ... and the original
.iso as the disc: tools/test_pdtest.sh does the whole test.
"""
import argparse
import os
import shutil
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import ps2_patch as pp  # noqa: E402

LO, HI = 0x2687E0, 0x26A2E8          # ms_net_patch_set (dead: only an official server's patch download reaches it)
TARGET = 0x26A2E0                    # PDT_TARGET in pdtest.c: the test patch writes "PDTESTOK" here


def nm(elf):
    return dict((l.split()[2], int(l.split()[0], 16)) for l in pp.run([pp.BU + "nm", elf]).split("\n")
                if len(l.split()) == 3)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("disc")
    ap.add_argument("--out", default=os.path.join(ROOT, "build/ps2patch/pdtest"))
    ap.add_argument("--pad", default="0", help="experiment: zero bytes before the test list in install mode (e.g. "
                    "0x580, the header size the DNAS library's parser reads first)")
    a = ap.parse_args()
    try:
        disc = pp.Disc(a.disc)
        moff, msize = pp.elf_main(disc)
        have = disc.read("SLPM_654.95", moff, msize)
        built = pp.rebuilt("main")
        if built != have:
            raise pp.Fail("build/main.bin is not this disc's main section: tools/rebuild.sh main must say OK")
        msyms = nm(os.path.join(ROOT, "build/main.elf"))
        hook = msyms["PatchLoadinDNAS_Main"]
        if msyms["ms_net_patch_set"] != LO:
            raise pp.Fail("ms_net_patch_set is not at 0x%X" % LO)
        elf_bytes = open(disc.src.files["SLPM_654.95"][1], "rb").read() if disc.iso is None else \
            disc.read("SLPM_654.95", 0, disc.files["SLPM_654.95"]["size"])
        for mode, name in ((1, "install"), (2, "probe")):
            u = dict(name="pdtest%d" % mode, module="main", src="src/ps2patch/pdtest.c", lo=LO, hi=HI,
                     defs=["-DMH1_PDTEST", "-DPDTEST_MODE=%d" % mode, "-DPDTEST_PAD=%s" % a.pad])
            code = pp.build_unit(u, pp.conf_header(argparse.Namespace(
                name="x", server="127.0.0.1:1", login="00000000", password="x")))
            usyms = nm(os.path.join(ROOT, pp.OUT, u["name"] + ".elf"))
            end = max(v for k, v in usyms.items() if LO <= v < HI)
            if code[TARGET - LO:] != bytes(HI - TARGET) or end >= TARGET:
                raise pp.Fail("pdtest.c reaches the target 0x%X; make it smaller" % TARGET)
            img = bytearray(built)
            img[LO - pp.MAIN_VRAM:HI - pp.MAIN_VRAM] = code
            j = 0x08000000 | (usyms["pdtest_main"] >> 2) & 0x3FFFFFF
            img[hook - pp.MAIN_VRAM:hook - pp.MAIN_VRAM + 8] = struct.pack("<II", j, 0)
            diff = pp.ranges(built, img, pp.MAIN_VRAM)
            for lo, hi in diff:
                if not (LO <= lo and hi <= HI) and not (hook <= lo and hi <= hook + 8):
                    raise pp.Fail("0x%X-0x%X changed outside the test ranges" % (lo, hi))
            out = bytearray(elf_bytes)
            out[moff:moff + msize] = img
            d = os.path.join(a.out, name)
            os.makedirs(d, exist_ok=True)
            open(os.path.join(d, "SLPM_654.95"), "wb").write(out)
            print("%-7s %s/SLPM_654.95: pdtest_main 0x%X (%d bytes of code+data), PatchLoadinDNAS_Main 0x%X -> j" % (
                name, d, usyms["pdtest_main"], u["code_end"] - LO if u["code_end"] > LO else len(code), hook))
        return 0
    except (pp.Fail, pp.disc_files.DiscError) as e:
        print("ps2_pdtest: %s" % e, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
