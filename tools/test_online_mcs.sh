#!/bin/sh
# Two headless ONLINE=1 PC clients hunt together on the game's OWN in-hunt protocol (docs/network.md 3.10): mh1-server
# with --lobby-mcs (6914 "mh1-mcs", 6916 = its mcs relay, tools/server/mcs.py), the PCs run the PS2's session code
# (rt_mcs.c: cmcs_* login, mcsls sync / ping / start, the AQ packets) instead of net_peer frames. The same scripted
# screens as test_online_server.sh: login, town, ANNA posts quest 1 (a room for 2 with a password), BOB joins, the
# match, both load the quest, ANNA clears it (delivery, RT_PL_ITEMS), both see the clear and go back to the town.
# 127.0.0.1 only. About 4 minutes.
cd "$(dirname "$0")/.."
BIN=${BIN:-build/pc/mhview_online}
RUN=${RUN:-}
OUT=build/show/online_mcs; rm -rf $OUT; mkdir -p $OUT
SP=""; PA=""
fail() { echo "mcs hunt test FAILED: $1 (see $OUT)"; [ -n "$PA" ] && kill $PA 2>/dev/null; [ -n "$SP" ] && kill $SP 2>/dev/null; exit 1; }
[ -f "$BIN" ] || fail "$BIN is missing (ONLINE=1 tools/build_pc.sh)"
for nm in ANNA BOB; do
    [ -f build/coop/save_$nm/BISLPM-65495MH/BISLPM-65495MH ] || tools/test_coop.sh saves $nm >/dev/null || fail "could not make $nm's card"
    rm -rf $OUT/card_$nm; mkdir -p $OUT/card_$nm; cp -r build/coop/save_$nm/. $OUT/card_$nm/
done
MCSPORT=${MCSPORT:-10297}
cat > $OUT/mh1-server.conf <<CONF
[server]
bind = 127.0.0.1
lobby-port = 0
lobby-mcs = yes
mcs-port = $MCSPORT
db = accounts.sqlite3
verbose = yes
CONF
for nm in ANNA BOB; do
    python3 tools/server/mh1_server.py account add --db $OUT/accounts.sqlite3 --note "test $nm" > $OUT/account_$nm.txt || fail "account add"
done
id_of() { sed -n 's/^login \([0-9]*\) password .*/\1/p' $OUT/account_$1.txt; }
pw_of() { sed -n 's/^login [0-9]* password \(.*\)/\1/p' $OUT/account_$1.txt; }
python3 tools/server/mh1_server.py serve --config $OUT/mh1-server.conf > $OUT/server.log 2>&1 &
SP=$!
i=0; PORT=""
while [ $i -lt 50 ] && [ -z "$PORT" ]; do
    PORT=$(sed -n 's/^mh1-server lobby on 127.0.0.1:\([0-9]*\).*/\1/p' $OUT/server.log)
    [ -n "$PORT" ] || { sleep 0.1; i=$((i + 1)); }
done
[ -n "$PORT" ] || fail "mh1-server did not start"
LOGIN="200:circle*2;302:circle*2;404:circle*2;556:circle*2;738:square*3"
TALK=""; t=1041; while [ $t -le 1441 ]; do TALK="$TALK;$t:circle*2"; t=$((t + 40)); done
REW=""; t=3500; while [ $t -le 7000 ]; do
    REW="$REW;$t:circle*2;$((t+30)):circle*2;$((t+60)):circle*2;$((t+90)):circle*2;$((t+120)):circle*2;$((t+150)):circle*2;$((t+180)):cross*2;$((t+200)):ddown*2;$((t+220)):circle*2"
    t=$((t + 250))
done
IA=$(python3 tools/mk_input.py - "$LOGIN$TALK;1521:dleft*2;1543:dleft*2;1565:ddown*2;1600:dright*2;1625:ddown*2;1650:circle*2;1720:ddown*2;1745:ddown*2;1770:circle*2;1830:circle*2;1890:circle*2;2800:square*3;2860:circle*2;2920:circle*2;2980:circle*2$REW" 7600)
JOIN=""; t=2000; while [ $t -le 2450 ]; do JOIN="$JOIN;$t:circle*2"; t=$((t + 50)); done
IB=$(python3 tools/mk_input.py - "$LOGIN;1950:square*3$JOIN;2700:square*3;2760:circle*2;2820:circle*2$REW" 7600)
run() {     # name seconds input warps [env...]
    nm=$1; t=$2; in=$3; w=$4; shift 4
    env "$@" MH1_SAVE_DIR="$PWD/$OUT/card_$nm" RT_VILLAGE_START=1 RT_VILLAGE_SKIP_INTRO=1 RT_ONLINE_TRACE=1 RT_QUEST_TRACE=1 \
        RT_NOMOVIE=1 RT_NET_REGISTERED=1 RT_NET_PORT=$PORT RT_NET_ID=$(id_of $nm) RT_NET_PASS=$(pw_of $nm) RT_LB_WARP="$w" \
        RT_NP_POS=150 RT_NP_EM=150 RT_SHOTS=${SHOTS:-3300,3420} timeout ${TMO:-400} $RUN $BIN \
        disc/mh1 --quest 10 --play --online --mute --input "$in" --shot $OUT/$nm.png --time $t --size 640x480 > $OUT/$nm.log 2>&1
}
run ANNA 270 "$IA" "60,5545,2750,8000;300,1300,1600,C000;2200,3100,2100,8000" RT_PL_ITEMS=77:15 RT_PL_WARP="${WARP_A:-150,10350,10640,7000}" \
    RT_SK_TEXT=MH1PASS7 RT_SK_TRACE=1 $ENVA &
PA=$!
sleep 2
run BOB 268 "$IB" "60,5545,2750,8000;300,1850,1500,8000;2100,3100,2100,8000" RT_SK_TEXT=MH1PASS7 RT_SK_TRACE=1 $ENVB \
    || fail "client BOB stopped early"
wait $PA || { PA=""; fail "client ANNA stopped early"; }
PA=""
kill $SP 2>/dev/null; wait $SP 2>/dev/null; SP=""
grep -q "mcs: match room 1" $OUT/server.log || fail "the lobby did not hand the match to the mcs relay"
grep -q "mcs: login .*: player 0 of match" $OUT/server.log || fail "ANNA did not log in to the mcs relay"
grep -q "mcs: login .*: player 1 of match" $OUT/server.log || fail "BOB did not log in to the mcs relay"
for nm in ANNA BOB; do
    grep -aq "mcs: logged in to the game server: 2 players" $OUT/$nm.log || fail "$nm: no game-server login"
    grep -aq "mcs: every player is in" $OUT/$nm.log || fail "$nm: the players' select did not complete"
    grep -aq "mcs: start agreed" $OUT/$nm.log || fail "$nm: net_start_ck did not agree on the start"
    grep -aq "rt_flow: tick [0-9]* mode 2 step 0 D5 3" $OUT/$nm.log || fail "$nm did not see the quest cleared"
    grep -aq "online: back to the town after the quest" $OUT/$nm.log || fail "$nm did not go back to the town"
done
echo "mcs hunt OK: two PCs on the game's own in-hunt protocol through mh1-server's mcs relay, login, select, start, the clear, back to the town"
