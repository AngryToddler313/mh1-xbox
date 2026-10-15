#!/bin/sh
# Cross-build the PC port for 32-bit Windows (i686) on Linux, no root:
# build/win/mhview.exe + SDL2.dll + play.bat (+ the helper scripts), by
# running tools/build_pc.sh with llvm-mingw's clang (the way build_arm.sh
# does for ARM). Needs, outside the repo (docs/pc.md "Windows"):
#   ~/mh1win/llvm-mingw-*-ucrt-*/            github.com/mstorsjo/llvm-mingw releases
#   ~/mh1win/SDL2-2.x.y/i686-w64-mingw32/    SDL2-devel-2.x.y-mingw.tar.gz from libsdl-org/SDL
# (WINROOT overrides ~/mh1win). The build runs in build/win/tree, a tree of
# symlinks to src/ include/ tools/ ..., so that build/pc (the Linux build)
# is left alone.
# VERSION=mhg: Monster Hunter G (docs/mhg.md): tree build/winmhg/tree, result build/winmhg/mhview_g.exe + SDL2.dll + play.bat
# (tools/win/play_g.bat), the same way tools/build_pc_mhg.sh does it for Linux.
set -e
cd "$(dirname "$0")/.."
TOP=$PWD
W=${WINROOT:-$HOME/mh1win}
LLVM=$(ls -d "$W"/llvm-mingw-*${WINCRT:-msvcrt}* 2>/dev/null | grep -v "\.tar" | head -1)
SDL=$(ls -d "$W"/SDL2-[0-9]* 2>/dev/null | grep -v '\.tar' | tail -1)/i686-w64-mingw32
[ -x "$LLVM/bin/i686-w64-mingw32-clang" ] || { echo "no llvm-mingw in $W (docs/pc.md, Windows)"; exit 1; }
[ -f "$SDL/lib/libSDL2.dll.a" ] || { echo "no SDL2 mingw development package in $W (docs/pc.md, Windows)"; exit 1; }
if [ "$VERSION" = mhg ]; then
    WB=build/winmhg; EXE_BASE=mhview_g; SUFFIX=-mhg; PLAYBAT=play_g.bat; BUGBAT=bug_report_g.bat
    python3 tools/mhg_pcmeta.py
else
    WB=build/win; EXE_BASE=mhview; SUFFIX=""; PLAYBAT=play.bat; BUGBAT=""
fi
T=$WB/tree
mkdir -p $T/build
for d in src include tools third_party config disc; do
    [ -e "$d" ] && ln -sfn "$TOP/$d" $T/$d
done
if [ "$VERSION" = mhg ]; then
    ln -sfn "$TOP/build/mhg" $T/build/mhg
    # the last build's generated aliases went stale when G code started defining one itself (as build_pc_mhg.sh)
    rm -f $T/build/pc/rt_gen.defsym $T/build/pc/rt_gen.defsym.*
fi
# the build's version string (git hash) is baked in by build_pc.sh through MH1_VERSION
export MH1_VERSION="$(git rev-parse --short=10 HEAD 2>/dev/null || echo unknown)$(git diff --quiet HEAD 2>/dev/null || echo +)$SUFFIX"
export PATH="$LLVM/bin:$PATH"
# clang 18 makes these gcc warnings errors in C99; the decompiled C relies on them
RELAX="-Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=int-conversion \
 -Wno-error=incompatible-function-pointer-types -Wno-error=return-type -Wno-error=incompatible-pointer-types"
export CC="i686-w64-mingw32-clang"
export M32="" PC_SYS=" " EXE=.exe SYMTAB_ARGS="--prefix _" SYM_PREFIX=_
export LINK1_OPTS="-Wl,--error-limit=0" LINK1_TOLERANT=1
export OBJCOPY=llvm-objcopy NM=llvm-nm
export SDL_CFLAGS="-I$SDL/include/SDL2 -DSDL_MAIN_HANDLED"
export LIBS="-L$SDL/lib -lSDL2 -lopengl32 -lcomdlg32 -lshell32 -lwinmm -lm"
# ONLINE=1: the online build (Winsock; build/win/mhview_online.exe, docs/network.md), passed on to build_pc.sh
[ -n "$ONLINE" ] && LIBS="$LIBS -lws2_32 -lgdi32"
export FPSSE="-msse2 -mfpmath=sse"   # single-precision floats as the PS2 (x87 80-bit temporaries hung the Cephadrome death, quest 154)
export GAME_EXTRA="$RELAX -w -fno-builtin"   # as nxdk-cc: the game headers declare memset() K&R
export EXTRA_CFLAGS="-DMH1_WIN -include src/pc/rt/win_utf8.h" GAME_NOAGG=""
sh $T/tools/build_pc.sh
mkdir -p $WB
EXE_NAME=$EXE_BASE; [ -n "$ONLINE" ] && EXE_NAME=${EXE_BASE}_online
cp $T/build/pc/$([ -n "$ONLINE" ] && echo mhview_online || echo mhview).exe $WB/$EXE_NAME.exe
cp "$SDL/bin/SDL2.dll" $WB/SDL2.dll
for f in $PLAYBAT bug_report.bat bug_report.ps1 $BUGBAT; do [ -f tools/win/$f ] && cp tools/win/$f $WB/$f; done
echo "built $WB/$EXE_NAME.exe (32-bit Windows; SDL2.dll next to it)"
