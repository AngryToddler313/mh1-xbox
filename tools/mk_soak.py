#!/usr/bin/env python3
"""Pad script and test aids for a memory soak: LOOPS village -> quest 131 -> village loops in
one run from power-on (tools/test_soak.sh on the PC, tools/xemu_soak.sh in xemu).

Loop 1 is tools/test_quest_loop.sh's run (NEW GAME, the Elder's first-visit talk, quest 131,
two Aptonoth carved, delivered at the camp box, reward, village). Every later loop accepts
quest 131 again at the Elder (11 circles walk through his talk and take the first card of the
1-star list), leaves through the gate and plays the same hunt. A quest can end a few ticks
earlier or later from loop to loop (the clear check), so the script waits for the game:
"@villageN" / "@questN" (pad_sdl.c) hold it until the N-th village entry / quest start, and
RT_AID_PER_QUEST=1 makes the RT_PL_* aids count from each quest's start, so loop 1's aids
repeat. RT_LB_WARP counts village ticks over all visits: 2240 in visit 1, 790 in each later
one (the gate warp at visit tick 760, both anchored to the visit's start).

usage: mk_soak.py LOOPS OUTDIR [DL]
  DL: host ticks the boot part is shifted by (xemu: 240, the opening movie skipped with start;
      the PC test runs with RT_NOMOVIE). Writes OUTDIR/input.txt and OUTDIR/env.txt."""
import os, subprocess, sys

loops = int(sys.argv[1])
out = sys.argv[2]
dl = int(sys.argv[3]) if len(sys.argv) > 3 else 0
tools = os.path.dirname(os.path.abspath(__file__))
D = os.path.join(tools, 'pc_scripts')
Q1 = 3787               # loop 1's quest start (host tick, RT_NOMOVIE)
HUNT = 4260             # the hunt script up to its last press on the reward screen (the village comes at ~4273)
LB1, LBV = 2240, 790    # village ticks of visit 1 / of each later visit


def rd(n):
    return open(os.path.join(D, n)).read().strip()


def cut(script, n):     # the first n ticks of a script
    keep, acc = [], 0
    for p in script.split(','):
        k = int(p.split('*')[1]) if '*' in p else 1
        if acc + k >= n:
            if n - acc > 0:
                keep.append(p.split('*')[0] + '*%d' % (n - acc))
            break
        keep.append(p)
        acc += k
    return ','.join(keep)


def rel(evs, end):      # (tick, action) from an anchor -> a script of end ticks
    out, t = [], 0
    for at, a in evs:
        if at > t:
            out.append('idle*%d' % (at - t))
        out.append(a)
        t = at + 2
    if end > t:
        out.append('idle*%d' % (end - t))
    return ','.join(out)


os.makedirs(out, exist_ok=True)
ng = rd('newgame.txt')
if dl:
    ng = ng.replace('idle*600,start*2,idle*60,', 'idle*700,start*2,idle*200,', 1)
open(os.path.join(out, 'newgame.txt'), 'w').write(ng + '\n')
# loop 1 up to its quest: NEW GAME, the first-visit talk with the Elder (as test_quest_loop.sh)
ev = ['%d:square*2' % (2860 + dl)] + ['%d:circle*2' % (2930 + dl + 27 * i) for i in range(30)] + ['%d:square*2' % (3770 + dl)]
r = subprocess.run([sys.executable, os.path.join(tools, 'mk_input.py'), os.path.join(out, 'newgame.txt'), ';'.join(ev),
                    str(Q1 + dl - 10)], stdout=subprocess.PIPE, universal_newlines=True)
if r.returncode:
    sys.exit(r.returncode)
hunt = cut(rd('quest131_hunt.txt'), HUNT)
# a later village visit (ticks from its start): the Elder's talk, quest 131 accepted, the gate
visit = rel([(68, 'square*2')] + [(139 + 50 * i, 'circle*2') for i in range(11)] + [(729, 'square*2'), (772, 'square*2')], 780)
parts = [r.stdout.strip(), '@quest1', hunt]
for k in range(2, loops + 1):
    parts += ['@village%d' % k, visit, '@quest%d' % k, hunt]
parts += ['@village%d' % (loops + 1), rel([(68, 'square*2')], 600)]
open(os.path.join(out, 'input.txt'), 'w').write(','.join(parts) + '\n')

lb = ['1300,2290,1000,4000', '1360,10901,12409,38AB', '2210,10650,15225']
for k in range(2, loops + 1):
    a = LB1 + LBV * (k - 2)
    lb += ['%d,10901,12409,38AB' % (a + 60), '%d,10650,15225' % (a + 760)]    # the Elder; the gate
with open(os.path.join(out, 'env.txt'), 'w') as f:
    f.write('RT_NAME=TEST\nRT_QUEST_TRACE=1\nRT_AID_PER_QUEST=1\n')
    f.write('RT_LB_WARP=%s\n' % ';'.join(lb))
    f.write('RT_PL_TARGET=0:0,1250:1\n')
    f.write('RT_PL_WARP=10,12250,10000;2400,6750,11900;2500,10350,10500\n')
    f.write('RT_PL_WARP_EM=90-520,600,1270-1700,1780\n')
    f.write('RT_DMG_MUL=40\n')
    f.write('RT_PL_ITEMS_MIN=2450:18:2\n')
