#!/bin/sh
# Download the two things tools/build_win.sh needs, into ~/mh1win (WINROOT overrides). No root, nothing is
# installed system-wide. About 100 MB download, 400 MB unpacked:
#   llvm-mingw (clang for 32-bit Windows)   github.com/mstorsjo/llvm-mingw
#   SDL2 development files for MinGW        github.com/libsdl-org/SDL
set -e
W=${WINROOT:-$HOME/mh1win}
LLVM_REL=20261006
SDL_VER=2.32.10
mkdir -p "$W"
cd "$W"
command -v curl >/dev/null || { echo "curl is needed"; exit 1; }
command -v tar >/dev/null && command -v xz >/dev/null || { echo "tar and xz are needed (apt install xz-utils)"; exit 1; }
L=llvm-mingw-$LLVM_REL-msvcrt-ubuntu-22.04-x86_64
if [ ! -x "$L/bin/i686-w64-mingw32-clang" ]; then
    [ -f $L.tar.xz ] || curl -fL --retry 3 -o $L.tar.xz.part https://github.com/mstorsjo/llvm-mingw/releases/download/$LLVM_REL/$L.tar.xz
    [ -f $L.tar.xz ] || mv $L.tar.xz.part $L.tar.xz
    tar xf $L.tar.xz
fi
if [ ! -f SDL2-$SDL_VER/i686-w64-mingw32/lib/libSDL2.dll.a ]; then
    S=SDL2-devel-$SDL_VER-mingw.tar.gz
    [ -f $S ] || curl -fL --retry 3 -o $S.part https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VER/$S
    [ -f $S ] || mv $S.part $S
    tar xf $S
fi
echo "Windows toolchain ready in $W"
