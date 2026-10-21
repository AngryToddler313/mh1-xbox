#!/bin/bash
# dn.sh FILE FUNC: G-vs-ours differing lines of FUNC
cd ${ROOT:-/root/mh1-xbox} && VERSION=mhg python3 tools/check.py "$1" -v 2>&1 | awk -v f="$2" '$2==f{p=1;print;next} /^(OK|--|\?\?) /{p=0} p' | grep -B2 -A2 ">>" | head -${3:-30}
