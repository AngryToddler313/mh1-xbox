#!/bin/sh
# The PS2 online patch (tools/ps2_patch.py, docs/network.md "PS2 patch"), checked without a PS2 or PCSX2:
#  1. a dry run on the disc folder: the rebuild is identical to the disc, the DNAS call and the browser calls are gone
#     from the patched functions, the server and login are in the patched lobby.bin, nothing outside the two patched
#     functions changes (the tool refuses otherwise);
#  2. the tool's refusals (MH Oldschool's address and names, a bad login);
#  3. with the .iso (disc/mh1/*.iso): a PPF patch for the image, and patched files from the folder; the PPF applied
#     to the image's SLPM_654.95 / AFS_DATA.AFS gives exactly the patched files, and those differ from the originals
#     only inside ms_network_bb_authentication and internet_browser.
# The server side of the same code (mh1p_fill_server_table, linked into the PC online build) is tested by
# tools/test_online_server.sh: the PC client logs in to mh1-server with an account through that server table.
# Needs the PS2 rebuild (tools/rebuild.sh main lobby OK). Run it under the brief's memory cap (wibo needs ~3 GB
# of address space): (ulimit -v 3000000; tools/test_ps2_patch.sh)
cd "$(dirname "$0")/.."
DISC=${DISC:-disc/mh1}
OUT=build/ps2patch/test; rm -rf $OUT; mkdir -p $OUT
fail() { echo "ps2 patch test FAILED: $1 (see $OUT)"; exit 1; }

python3 tools/ps2_patch.py $DISC --server 127.0.0.1:10200 --login 12345678 --password ABCDEFGH12345678 \
    --dry-run --disasm > $OUT/dry.txt 2>&1 || fail "dry run"
grep -q "^main   rebuild OK" $OUT/dry.txt || fail "main rebuild"
grep -q "^lobby  rebuild OK" $OUT/dry.txt || fail "lobby rebuild"
grep -q "^dnas .*calls gone: InetDNAS2ConnectNetStart" $OUT/dry.txt || fail "DNAS call"
grep -q "^portal .*calls gone: MainBrowser, MainBsDispose, MainBsInitialize" $OUT/dry.txt || fail "browser calls"
grep -q "settings .*'127.0.0.1:10200', '12345678', 'ABCDEFGH12345678'" $OUT/dry.txt || fail "settings in lobby.bin"
grep -q "jal	0xa769c0" $OUT/dry.txt && fail "the patched code still calls InetDNAS2ConnectNetStart"
echo "dry run: OK"

for bad in "34.75.107.68:10200" "viper.capcom.co.jp:8200" "www01.kddi-mmbb.jp:443" "nohostport"; do
    python3 tools/ps2_patch.py $DISC --server "$bad" --dry-run > $OUT/refuse.txt 2>&1 && fail "accepted $bad"
done
python3 tools/ps2_patch.py $DISC --server 127.0.0.1:10200 --login 12ab --dry-run > $OUT/refuse.txt 2>&1 && fail "accepted a bad login"
echo "refusals: OK"

ISO=$(ls $DISC/*.iso 2>/dev/null | head -1)
if [ -z "$ISO" ]; then echo "no .iso in $DISC: image checks skipped"; echo "ps2 patch test: OK"; exit 0; fi
python3 tools/ps2_patch.py "$ISO" --server 127.0.0.1:10200 --login 12345678 --password ABCDEFGH12345678 \
    --ppf $OUT/mh1.ppf > $OUT/ppf.txt 2>&1 || fail "ppf"
python3 tools/ps2_patch.py $DISC --server 127.0.0.1:10200 --login 12345678 --password ABCDEFGH12345678 \
    --out $OUT/files > $OUT/files.txt 2>&1 || fail "patched files"
python3 - "$ISO" $OUT $DISC <<'EOF' || fail "image check"
import struct, sys
sys.path.insert(0, "tools")
import disc_files
iso_path, out, disc = sys.argv[1:4]
iso = disc_files.Iso(iso_path)
root = iso.root()
p = open(out + "/mh1.ppf", "rb").read()
assert p[:5] == b"PPF30" and p[5] == 2, "not a PPF 3.0 file"
recs, i = [], 60
while i < len(p):
    off, n = struct.unpack_from("<QB", p, i)
    recs.append((off, p[i + 9:i + 9 + n]))
    i += 9 + n
# the functions allowed to change: main's ms_network_bb_authentication (file offset from the ELF), lobby's
# internet_browser (offset in AFS_DATA.AFS)
allowed = {"SLPM_654.95": [], "AFS_DATA.AFS": []}
for name in allowed:
    size, lba = root[name]
    orig = b"".join(iso.read_file(lba, size))
    new = bytearray(orig)
    for off, data in recs:
        rel = off - lba * 2048
        if 0 <= rel < size:
            new[rel:rel + len(data)] = data
    patched = open("%s/files/%s" % (out, name), "rb").read()
    assert bytes(new) == patched, "%s: the PPF on the image differs from the patched file" % name
    if name == "SLPM_654.95":
        d = [k for k in range(size) if orig[k] != patched[k]]
        e = orig
        shoff, = struct.unpack_from("<I", e, 32)
        shnum, shstrndx = struct.unpack_from("<HH", e, 48)
        secs = [struct.unpack_from("<10I", e, shoff + j * 40) for j in range(shnum)]
        names = e[secs[shstrndx][4]:]
        main = [s for s in secs if names[s[0]:names.index(b"\0", s[0])] == b"main"][0]
        lo, hi = main[4] + 0x268040 - main[3], main[4] + 0x2685B4 - main[3]
        assert d and lo <= d[0] and d[-1] < hi, "SLPM_654.95 changed outside ms_network_bb_authentication"
        print("SLPM_654.95: %d bytes differ, all in ms_network_bb_authentication" % len(d))
    else:
        cnt, = struct.unpack_from("<I", orig, 4)
        for j in range(cnt):
            o, s = struct.unpack_from("<II", orig, 8 + 8 * j)
            if orig[o + 0x20:o + 0x29] == b"lobby.bin":
                load, = struct.unpack_from("<I", orig, o + 8)
                lo, hi = o + 0x5B5980 - load, o + 0x5B5C50 - load
        before = orig[:lo] + orig[hi:]
        after = patched[:lo] + patched[hi:]
        assert before == after, "AFS_DATA.AFS changed outside internet_browser"
        assert b"127.0.0.1:10200" in patched[lo:hi]
        print("AFS_DATA.AFS: changes only in lobby.bin's internet_browser, the server address is there")
print("PPF records: %d, they turn the image into the patched files" % len(recs))
EOF
echo "ps2 patch test: OK"
