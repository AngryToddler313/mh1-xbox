#!/bin/sh
# Play-test Monster Hunter G on the PC (owner's hands-on runs; shows a window).
#   tools/play_g.sh            from power-on: logos, title, new game / continue, Kokoto village
#   tools/play_g.sh quest [N]  straight into G quest N (default 10, Rathian), normal rules
#   tools/play_g.sh easy       quest 10, the hunter can't faint
# Saves go to G's own folder (separate from MH1's). Controls as tools/play.sh (Xbox pad or
# keyboard: W/A/S/D move, arrows attack, K roll, L sheathe/confirm, J item, E guard,
# Enter start/pause). Esc or closing the window quits. A log of the run goes to
# build/mhg/play.log: send it along with any bug you see.
cd "$(dirname "$0")/.."
RUN=build/mhg/pc/mhview_g
[ -x $RUN ] || tools/build_pc_mhg.sh || exit 1
export MH1_SHOW=1
case "$1" in
    quest) set -- --quest "${2:-10}" ;;
    easy)  export RT_PL_GOD=1; set -- --quest 10 ;;
    *)     set -- --boot ;;
esac
$RUN disc/mhg --play --size 1024x768 "$@" 2>&1 | tee build/mhg/play.log
