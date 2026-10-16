#!/bin/sh
# Monster Hunter G: every offline quest (village 1-50, training 10001-10048) on the PC build, always hidden, with the
# test aids of tools/test_all_quests.sh (warp to the monster, damage x40, no damage to the hunter, area walking):
# the monster(s) spawn, the quest clears (reward screen + list), and a second run with the quest time set to 20 s
# fails it on time-out. Without quest numbers also a carts check (quest 9, no GOD/warp: the Rathian faints the hunter
# three times -> quest failed). Village and money after the reward screen are not checked.
# Also a Dual Swords hunt (quest 10, weapon 234: demon mode, the class's own attacks; DUAL=1 with quest numbers).
# usage: tools/test_all_quests_g.sh [quest numbers, decimal]   Logs: build/show/allq_g/qN.log, qN_to.log, q10_dual.log
cd "$(dirname "$0")/.."
[ -x "${BIN:-build/mhg/pc/mhview_g}" ] || tools/build_pc_mhg.sh >/dev/null || exit 1
G=1 exec python3 tools/test_all_quests.py "$@"
