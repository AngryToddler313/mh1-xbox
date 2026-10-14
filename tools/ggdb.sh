#!/bin/sh
# ggdb.sh TAG "pad script" SECS GDBCMDFILE [ENV=val ...] : like gvil.sh under gdb with a command file (agent D tooling);
# the file's last commands should include `run` ... then bt / print ...; the output goes to build/D/g/TAG.gdb
cd "$(dirname "$0")/.." || exit 1
TAG=$1; IN=$2; SECS=$3; CMD=$4; shift 4
mkdir -p build/D/g
( ulimit -v 3000000
  [ -z "$INTRO" ] && export RT_VILLAGE_SKIP_INTRO=1
  export MH1_SAVE_DIR="$PWD/build/D/g/card" RT_NOMOVIE=1 RT_QUEST_TRACE=1 RT_VILLAGE_START=1 RT_NAME=TEST RT_STEP=1
  for kv in "$@"; do export "$kv"; done
  gdb -q -batch -x "$CMD" \
    --args build/mhg/pc/mhview_g disc/mhg --quest 1 --play --input "$IN" --shot build/D/g/$TAG.png --time "$SECS" > build/D/g/$TAG.gdb 2>&1 )
tail -${TAILN:-40} build/D/g/$TAG.gdb
