#!/bin/bash
# xbox_install.sh XBOX_IP [FOLDER]: copy the finished game folder (./mh1 xbox: dist/xbox/MH1, holding default.xbe and
# data/) to the Xbox hard disk as E:\Games\MH1 over FTP, with curl. Files already there with the right size are
# skipped, so an interrupted copy can simply be started again. Your Xbox's FTP login is asked for here (or taken
# from ~/.netrc) and is never stored. About 925 MB: a home Wi-Fi link can need an hour.
#   XBOX_DIR=F/Games/MH1   another drive / folder (FTP path: drive letter, then folders)
#   XBOX_PASSIVE=1         use passive FTP (the default is active mode, which some dashboards need)
# Any other FTP program (FileZilla ...) does the same job: copy the folder's contents to E:\Games\MH1.
set -e
cd "$(dirname "$0")/.."
XBOX=$1
[ -n "$XBOX" ] || { echo "usage: $0 XBOX_IP [FOLDER]   (FOLDER defaults to dist/xbox/MH1)"; exit 2; }
SRC=${2:-dist/xbox/MH1}
command -v curl >/dev/null || { echo "curl is needed (sudo apt install curl), or use an FTP program"; exit 1; }
# a Monster Hunter G folder (./mh1 xbox with the G disc: dist/xbox/MHG) goes to E:\Games\MHG
ELF=SLPM_654.95; DIR=MH1; NAME="Monster Hunter"
[ -f "$SRC/data/SLPM_658.69" ] && { ELF=SLPM_658.69; DIR=MHG; NAME="Monster Hunter G"; }
for f in default.xbe data/AFS_DATA.AFS data/AFS00.AFS data/AFS01.AFS data/$ELF; do
	[ -f "$SRC/$f" ] || { echo "$SRC/$f is missing: run ./mh1 xbox DISC first"; exit 1; }
done
if grep -q "machine $XBOX" ~/.netrc 2>/dev/null; then
	AUTH=(--netrc)
else
	read -rp "Xbox FTP login: " u
	read -rsp "Xbox FTP password: " p; echo
	NETRC=$(mktemp); trap 'rm -f "$NETRC"' EXIT; chmod 600 "$NETRC"
	printf 'machine %s login %s password %s\n' "$XBOX" "$u" "$p" > "$NETRC"
	AUTH=(--netrc-file "$NETRC")
fi
[ -n "$XBOX_PASSIVE" ] || AUTH+=(--ftp-port -)
URL=ftp://$XBOX/${XBOX_DIR:-E/Games/$DIR}
echo "copying to $URL"
for f in default.xbe data/$ELF data/AFS01.AFS data/AFS_DATA.AFS data/AFS00.AFS; do
	want=$(stat -c %s "$SRC/$f")
	have=$(curl -sSI "${AUTH[@]}" "$URL/$f" 2>/dev/null | tr -d '\r' | awk '/Content-Length/{print $2}')
	if [ "$have" = "$want" ]; then echo "== $f already there"; continue; fi
	echo "-> $f ($((want >> 20)) MB)"
	curl -# "${AUTH[@]}" --ftp-create-dirs -T "$SRC/$f" "$URL/$f"
done
echo "done. On the Xbox start '$NAME' from the games list (E:\\Games\\$DIR\\default.xbe)."
