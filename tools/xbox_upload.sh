#!/bin/bash
# xbox_upload.sh [--loader] [--data] : install the network loaders on the real Xbox over FTP, one per game:
# E:\Games\MH1\default.xbe (listed as "Monster Hunter") and E:\Games\MHG\default.xbe ("Monster Hunter G"),
# each with a loader.txt holding this laptop's address (found automatically) and its game. Each loader then
# fetches its game's builds (mh1.xbe / mhg.xbe) and data itself from tools/xbox_serve.py (docs/xbox.md
# "Network loader"), so this runs once. An older G game in E:\Games\MHG\default.xbe is replaced by the loader
# (G now lives in mhg.xbe, which the loader downloads). XBOX_GAMES=F/Games for another parent folder.
# --data also copies the MH1 game files from disc/mh1 into E:\Games\MH1\data (about 925 MB, skipped when the
# Xbox already has the same size).
# Big files over Evolution X's FTP stall: rather install game data through the loader itself
# (tools/xbox_serve.py --data dist/xbox/MHG or dist/xbox/MH1, docs/xbox.md "Installing game data through the loader").
# --loader (or nothing): only the two loaders and their loader.txt, four small files.
# --settings FILE: only FILE, as settings.ini in the MH1 folder (XBOX_DIR=E/Games/MHG for G's) (the online build's
# online_server / online_login / online_password lines, docs/PLAYING.md "Online on the Xbox"); nothing is built.
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
GAMES=${XBOX_GAMES:-E/Games}
URL=ftp://$XBOX/${XBOX_DIR:-$GAMES/MH1}
put() { echo "-> $2"; curl -sS "${AUTH[@]}" --ftp-create-dirs -T "$1" "$2"; }
if [ "$1" = --settings ]; then
	[ -f "$2" ] || { echo "--settings FILE: no file '$2'"; exit 1; }
	put "$2" "$URL/settings.ini"
	echo done
	exit 0
fi
# always a fresh build with the default address: objects left by a test build for xemu
# (XH_HOST=10.0.2.2, docs/xbox.md) would otherwise end up in the installed loader.
# One program, two titles (the dashboard lists each folder by its XBE's title).
rm -f src/pc/xbox/loader/*.obj src/pc/xbox/xnet_http.obj
(. ~/xboxdev/env.sh && make -C src/pc/xbox/loader -s &&
	make -C src/pc/xbox/loader -s OUTPUT_DIR=../../../../build/xbox/loader_g XBE_TITLE='Monster\ Hunter\ G')
me=$(ip -4 route get "$XBOX" | sed -n 's/.* src \([0-9.]*\).*/\1/p')
[ -n "$me" ] || { echo "could not find this laptop's address towards $XBOX"; exit 1; }
echo "this laptop: $me:8765"
lt=$(mktemp); trap 'rm -f "$lt" ${NETRC:-}' EXIT
# one loader: default.xbe + loader.txt into ftp://.../<folder>
install_loader() {
	local xbe=$1 dir=ftp://$XBOX/$GAMES/$2 game=$3 have want
	echo "copying the $2 loader to $dir"
	put "$xbe" "$dir/default.xbe"
	# Evolution X's FTP can end a transfer early without an error: compare the size it lists
	have=$(curl -sS "${AUTH[@]}" "$dir/" 2>/dev/null | tr -d '\r' |
		awk '$NF == "default.xbe" && NF >= 9 && $1 ~ /^-/ { s = $5 } $NF == "default.xbe" && NF == 4 { s = $3 } END { print s }')
	want=$(stat -c %s "$xbe")
	if [ -z "$have" ]; then echo "   (could not read the size back; the listing gave nothing)"
	elif [ "$have" != "$want" ]; then echo "   default.xbe is $have bytes on the Xbox, should be $want: run this again"; exit 1
	else echo "   default.xbe: $have bytes, complete"; fi
	printf '%s:8765\ngame=%s\n' "$me" "$game" > "$lt"
	put "$lt" "$dir/loader.txt"
}
install_loader build/xbox/loader/default.xbe MH1 mh1
install_loader build/xbox/loader_g/default.xbe MHG mhg
if [ "$1" = --data ]; then
	for f in AFS00.AFS AFS01.AFS AFS_DATA.AFS SLPM_654.95; do
		have=$(curl -sSI "${AUTH[@]}" "$URL/data/$f" 2>/dev/null | tr -d '\r' | awk '/Content-Length/{print $2}')
		if [ "$have" = "$(stat -c %s disc/mh1/$f)" ]; then echo "== data/$f already there"; continue; fi
		curl -# "${AUTH[@]}" --ftp-create-dirs -T disc/mh1/$f "$URL/data/$f"
	done
fi
echo done
