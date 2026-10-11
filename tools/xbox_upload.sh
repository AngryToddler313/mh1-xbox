#!/bin/bash
# xbox_upload.sh [--data] : copy build/xbox/default.xbe to the real Xbox over FTP
# (F:\Games\MH1, or E:\Games\MH1 when the Xbox has no F: partition). --data also
# copies the game files from disc/mh1 into its data folder (about 925 MB, skipped when the Xbox already has the same size).
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
# Evolution X's FTP server has no EPSV; use plain PASV, or active mode (XBOX_FTP_ACTIVE=1)
AUTH+=(--disable-epsv)
[ -n "$XBOX_FTP_ACTIVE" ] && AUTH+=(--ftp-port -)
drives=$(curl -sS "${AUTH[@]}" --list-only "ftp://$XBOX/" | tr -d "\r")
echo "Xbox drives: $(echo $drives)"
if echo "$drives" | grep -qxE 'F:?'; then URL=ftp://$XBOX/F/Games/MH1; else URL=ftp://$XBOX/E/Games/MH1; fi
echo "copying to $URL"
put() { echo "-> $2"; curl -sS "${AUTH[@]}" --ftp-create-dirs -T "$1" "$URL/$2"; }
put build/xbox/default.xbe default.xbe
if [ "$1" = --data ]; then
	for f in AFS00.AFS AFS01.AFS AFS_DATA.AFS SLPM_654.95; do
		have=$(curl -sSI "${AUTH[@]}" "$URL/data/$f" 2>/dev/null | tr -d '\r' | awk '/Content-Length/{print $2}')
		if [ "$have" = "$(stat -c %s disc/mh1/$f)" ]; then echo "== data/$f already there"; continue; fi
		curl -# "${AUTH[@]}" --ftp-create-dirs -T disc/mh1/$f "$URL/data/$f"
	done
fi
echo done
