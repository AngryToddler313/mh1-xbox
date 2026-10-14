#!/bin/sh
# Monster Hunter G: every offline quest (village 1-50, training 10001-10048) on the PC build, always hidden, with the
# test aids of tools/test_all_quests.sh (warp to the monster, damage x40, no damage to the hunter, area walking):
# the monster(s) spawn, the quest clears (reward screen + list), and a second run with the quest time set to 20 s
# fails it on time-out. Village and money after the reward screen are not checked (G's village is not ported).
# usage: tools/test_all_quests_g.sh [quest numbers, decimal]   Logs: build/show/allq_g/qN.log, qN_to.log
cd "$(dirname "$0")/.."
[ -x "${BIN:-build/mhg/pc/mhview_g}" ] || tools/build_pc_mhg.sh >/dev/null || exit 1
G=1 exec python3 tools/test_all_quests.py "$@"
