#!/bin/bash
# xbox_upload.sh [--loader] [--data] : install the network loader on the real Xbox over FTP, as
# E:\Games\MH1\default.xbe plus loader.txt (this laptop's address). The loader then fetches
# each game build itself from tools/xbox_serve.py (docs/xbox.md "Network loader"), so this
# runs once. XBOX_DIR=F/Games/MH1 for another folder. --data
# also copies the game files from disc/mh1 into its data folder (about 925 MB, skipped when the Xbox already has the same size).
# Big files over Evolution X's FTP stall: rather install game data through the loader itself
# (tools/xbox_serve.py --data dist/xbox/MHG or dist/xbox/MH1, docs/xbox.md "Installing game data through the loader").
# --loader (or nothing): only the loader and loader.txt, two small files.
# --settings FILE: only FILE, as settings.ini in the game folder (the online build's online_server / online_login /
# online_password lines, docs/PLAYING.md "Online on the Xbox"); nothing is built.
# Login: ~/.netrc ("machine <ip> login .. password ..") or asked for here; never stored.
set -e
cd "$(dirname "$0")/.."
XBOX=${XBOX_IP:-192.168.1.115}
if grep -q "machine $XBOX" ~/.netrc 2>/dev/null; then
	AUTH=(--netrc)
else
	read -rp "Xbox FTP login: " u
	read -rsp "Xbox FTP password: " p; echo
	NETRC=$(mktemp); trap 'rm -f "$NETRC"' EXIT; chmod 600 "$NETRC"
	printf 'machine %s login %s password %s\n' "$XBOX" "$u" "$p" > "$NETRC"
	AUTH=(--netrc-file "$NETRC")
fi
# Evolution X's FTP server answers 502 to EPSV, PASV and listings: active mode only
AUTH+=(--ftp-port -)
URL=ftp://$XBOX/${XBOX_DIR:-E/Games/MH1}
echo "copying to $URL"
put() { echo "-> $2"; curl -sS "${AUTH[@]}" --ftp-create-dirs -T "$1" "$URL/$2"; }
if [ "$1" = --settings ]; then
	[ -f "$2" ] || { echo "--settings FILE: no file '$2'"; exit 1; }
	put "$2" settings.ini
	echo done
	exit 0
fi
# always a fresh build with the default address: objects left by a test build for xemu
# (XH_HOST=10.0.2.2, docs/xbox.md) would otherwise end up in the installed loader
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj
(. ~/xboxdev/env.sh && make -C src/pc/xbox/loader -s)
me=$(ip -4 route get "$XBOX" | sed -n 's/.* src \([0-9.]*\).*/\1/p')
lt=$(mktemp); printf '%s:8765\n' "$me" > "$lt"
put build/xbox/loader/default.xbe default.xbe
# Evolution X's FTP can end a transfer early without an error: compare the size it lists
have=$(curl -sS "${AUTH[@]}" "$URL/" 2>/dev/null | tr -d '\r' |
	awk '$NF == "default.xbe" && NF >= 9 && $1 ~ /^-/ { s = $5 } $NF == "default.xbe" && NF == 4 { s = $3 } END { print s }')
want=$(stat -c %s build/xbox/loader/default.xbe)
if [ -z "$have" ]; then echo "   (could not read the size back; the listing gave nothing)"
elif [ "$have" != "$want" ]; then echo "   default.xbe is $have bytes on the Xbox, should be $want: run this again"; exit 1
else echo "   default.xbe: $have bytes, complete"; fi
put "$lt" loader.txt; rm -f "$lt"
if [ "$1" = --data ]; then
	for f in AFS00.AFS AFS01.AFS AFS_DATA.AFS SLPM_654.95; do
		have=$(curl -sSI "${AUTH[@]}" "$URL/data/$f" 2>/dev/null | tr -d '\r' | awk '/Content-Length/{print $2}')
		if [ "$have" = "$(stat -c %s disc/mh1/$f)" ]; then echo "== data/$f already there"; continue; fi
		curl -# "${AUTH[@]}" --ftp-create-dirs -T disc/mh1/$f "$URL/data/$f"
	done
fi
echo done
