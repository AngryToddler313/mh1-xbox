#!/bin/bash
# linkerr.sh MODULE: full ld errors for a G module (same args as build.py)
cd ${ROOT:-/root/mh1-xbox}; m=$1; C=config/mhg; B=build/mhg
ld=$B/$m.ld; [ -f $B/$m.rn.ld ] && ld=$B/$m.rn.ld
al=""; [ -f $C/${m}_aliases.txt ] && al="-T $C/${m}_aliases.txt"
tools/binutils/bin/mips64r5900el-ps2-elf-ld -EL -nostdlib --no-warn-mismatch -T $ld -T $C/${m}_undefined_syms_auto.txt -T $C/${m}_undefined_funcs_auto.txt $al -T $C/g_names.txt -T $C/g_funcnames.txt -o /tmp/le_$m.elf 2>&1
