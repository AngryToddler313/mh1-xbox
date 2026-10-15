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
# XBOX_RATE=2M (or 500K ...): cap the copy speed; some Xbox FTP servers stall when fed at full speed
[ -n "$XBOX_RATE" ] && AUTH+=(--limit-rate "$XBOX_RATE")
URL=ftp://$XBOX/${XBOX_DIR:-E/Games/$DIR}
echo "copying to $URL"
# Evolution X's FTP server drops long transfers at random points. Each file is tried up to
# XBOX_TRIES times (default 5); a retry resumes from what arrived (curl -C -, the server's SIZE /
# APPE) and falls back to a fresh copy if the server can't resume. If a file keeps failing at once
# (426 on start), the Xbox's FTP server is still holding the broken file: restart the Xbox, rerun.
remote_size() {   # bytes on the Xbox, empty if unknown. Evolution X does not answer SIZE, so this
	# reads the directory listing (LIST) and takes the size column of the file's line
	local dir=${1%/*} name=${1##*/}
	[ "$dir" = "$1" ] && dir=
	curl -sS "${AUTH[@]}" "$URL/${dir:+$dir/}" 2>/dev/null | tr -d '\r' |
		awk -v n="$name" '$NF == n && NF >= 9 && $1 ~ /^-/ { s = $5 }        # unix style: -rw-r--r-- 1 u g SIZE Mon D T name
		$NF == n && NF == 4 && $3 ~ /^[0-9]+$/ { s = $3 }                  # DOS style: date time SIZE name
		END { if (s != "") print s }'
}
for f in default.xbe data/$ELF data/AFS01.AFS data/AFS_DATA.AFS data/AFS00.AFS; do
	want=$(stat -c %s "$SRC/$f")
	have=$(remote_size "$f")
	if [ "$have" = "$want" ]; then echo "== $f already there"; continue; fi
	echo "-> $f ($((want >> 20)) MB)"
	ok=
	for try in $(seq "${XBOX_TRIES:-5}"); do
		have=$(remote_size "$f")
		if [ -n "$have" ] && [ "$have" -gt 0 ] && [ "$have" -lt "$want" ]; then
			echo "   resuming at $((have >> 20)) MB (try $try)"
			curl -# "${AUTH[@]}" --ftp-create-dirs -C "$have" -T "$SRC/$f" "$URL/$f" && { ok=1; break; }
		else
			[ "$try" -gt 1 ] && echo "   starting again (try $try)"
			curl -# "${AUTH[@]}" --ftp-create-dirs -T "$SRC/$f" "$URL/$f" && { ok=1; break; }
		fi
		sleep 5
	done
	[ -n "$ok" ] || { echo "copy of $f failed ${XBOX_TRIES:-5} times: restart the Xbox and rerun (finished files are skipped)"; exit 1; }
	have=$(remote_size "$f"); [ -z "$have" ] || [ "$have" = "$want" ] || echo "   warning: $f is $have bytes on the Xbox, expected $want"
done
echo "done. On the Xbox start '$NAME' from the games list (E:\\Games\\$DIR\\default.xbe)."
