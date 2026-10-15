#!/usr/bin/env python3
"""test_gvillage.py - the Monster Hunter G village on the PC (build/mhg/pc/mhview_g), scripted and hidden: shops, forge, item box, elder.
Each case warps to an NPC (RT_LB_WARP) and presses the pad at the tick numbers of tools/test_activities.py (the MH1 timing works for G
too); pass/fail is read from the trace line "ud money N pouch: .. | box: .. | ware: .. | wear: .." that RT_QUEST_TRACE prints when the saved
hunter changes. Pictures: build/D/g/<case>_<tick>.png. Usage: tools/test_gvillage.py [case ...]   (about 10 minutes for all).
G item ids: herb 129, antidote herb 130, iron ore 155, item 218 (the head piece's second material), 449 the elder's gift."""
import os, re, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
SHOP, WSHOP, FORGE, ELDER = '10000,13425', '9860,12076', '9960,11920', '11100,12150'
HOUSE = '30,11225,14400,0;150,1990,1160,4001'

def circles(first, n, step=30): return {first + step * i: 'circle*2' for i in range(n)}
def evs(d): return ','.join('%d:%s' % kv for kv in sorted(d.items()))

def run(tag, end, ev, env):
    e = dict(os.environ); e.update({k: str(v) for k, v in env.items()})
    r = subprocess.run([sys.executable, 'tools/gvil_ev.py', tag, str(end), evs(ev)] + ['%s=%s' % kv for kv in env.items() if kv[0] == 'W'],
                       env=e, capture_output=True, text=True, timeout=900)
    log = open('build/D/g/%s.log' % tag, 'rb').read().decode('latin1')
    UD = re.compile(r'ud money (-?\d+) pouch:(.*?) \| box:(.*?) \| ware:(.*?) \| wear: w(\d+)/(\d+)/')
    def kv(s):
        d = {}
        for x in s.split():
            a, b = x.split(':'); d[int(a)] = d.get(int(a), 0) + int(b)
        return d
    ud = [(int(m.group(1)), kv(m.group(2)), kv(m.group(3)), m.group(4).split(), int(m.group(6))) for m in UD.finditer(log)]
    return log, ud

def env(**k):
    k.setdefault('RT_SHOTS', '')
    return {a: b for a, b in k.items() if b != ''}

CASES = {}
def case(f): CASES[f.__name__] = f; return f

@case
def wshop_buy():
    log, ud = run('t_wb', 330, circles(45, 8), env(W=WSHOP, RT_MONEY=50000))
    return len(ud) > 1 and ud[-1][0] == 47900 and '6/1/0' in ud[-1][3], 'first weapon bought for 2100z: money %d ware %s' % (ud[-1][0], ud[-1][3])
@case
def wshop_sell():
    ev = {45: 'circle*2', 75: 'circle*2', 105: 'circle*2', 120: 'ddown*2', 135: 'circle*2', 200: 'dright*2'}; ev.update(circles(230, 3))
    log, ud = run('t_ws', 400, ev, env(W=WSHOP, RT_MONEY=1000, RT_WARE='6:1,6:2,2:1'))
    return ud[-1][0] == 2350 and len(ud[-1][3]) == 2, 'sold the second sword for 1350z: money %d, ware %s' % (ud[-1][0], ud[-1][3])
@case
def ashop_buy():
    ev = {45: 'circle*2', 75: 'circle*2', 105: 'circle*2', 135: 'circle*2', 150: 'ddown*2'}; ev.update(circles(165, 5))
    log, ud = run('t_ab', 420, ev, env(W=WSHOP, RT_MONEY=50000))
    return 'ware' and any(x.startswith('2/') or x.startswith('3/') or x.startswith('0/') for x in ud[-1][3]) and ud[-1][0] < 50000, 'armour piece bought: money %d ware %s' % (ud[-1][0], ud[-1][3])
@case
def ashop_sell():
    ev = {45: 'circle*2', 75: 'circle*2', 105: 'circle*2', 120: 'ddown*2', 135: 'circle*2', 200: 'dright*2', 215: 'dright*2'}; ev.update(circles(240, 3))
    log, ud = run('t_as', 420, ev, env(W=WSHOP, RT_MONEY=1000, RT_WARE='6:1,6:2,2:1'))
    return ud[-1][0] > 1000 and len(ud[-1][3]) == 2, 'head piece sold: money %d ware %s' % (ud[-1][0], ud[-1][3])
@case
def shop_buy():
    log, ud = run('t_sb', 300, circles(45, 7), env(W=SHOP, RT_MONEY=500))
    return ud[-1][0] == 480 and ud[-1][1] == {129: 1}, 'a herb for 20z: money %d pouch %s' % (ud[-1][0], ud[-1][1])
@case
def shop_qty():
    ev = circles(45, 5); ev.update({180 + 12 * i: 'dup*2' for i in range(2)}); ev[224] = 'circle*2'; ev[254] = 'circle*2'
    log, ud = run('t_sq', 330, ev, env(W=SHOP, RT_MONEY=500))
    return ud[-1][0] == 440 and ud[-1][1] == {129: 3}, 'quantity picker: 3 herbs, money %d pouch %s' % (ud[-1][0], ud[-1][1])
@case
def shop_sell():
    ev = {45: 'circle*2', 75: 'circle*2', 105: 'circle*2', 120: 'ddown*2'}; ev.update(circles(135, 4))
    log, ud = run('t_ss', 300, ev, env(W=SHOP, RT_MONEY=500, RT_POUCH='129:5,1:3'))
    return ud[-1][0] > 500 and ud[-1][1].get(129) == 4, 'sold a herb: money %d pouch %s' % (ud[-1][0], ud[-1][1])
def forge_ev(tab):
    ev = {45: 'circle*2', 75: 'circle*2', 105: 'circle*2', 135: 'circle*2'}
    if tab == 'armour': ev[120] = 'ddown*2'
    if tab == 'upgrade': ev[150] = 'ddown*2'
    ev.update(circles(165, 4)); return ev
@case
def forge_weapon():
    log, ud = run('t_fw', 350, forge_ev('weapon'), env(W=FORGE, RT_MONEY=50000, RT_POUCH='155:5'))
    return ud[-1][0] == 48950 and ud[-1][1] == {155: 2} and '6/1/0' in ud[-1][3], 'Iron Sword: 1050z + 3 ore: money %d pouch %s ware %s' % (ud[-1][0], ud[-1][1], ud[-1][3])
@case
def forge_armour():
    log, ud = run('t_fa', 350, forge_ev('armour'), env(W=FORGE, RT_MONEY=50000, RT_POUCH='155:5,218:3'))
    return ud[-1][0] == 49850 and ud[-1][1] == {155: 4, 218: 2} and '2/1/0' in ud[-1][3], 'head piece: 150z + ore + 218: money %d pouch %s' % (ud[-1][0], ud[-1][1])
@case
def forge_upgrade():
    log, ud = run('t_fu', 350, forge_ev('upgrade'), env(W=FORGE, RT_MONEY=50000, RT_POUCH='155:5', RT_WARE='6:1'))
    return ud[-1][0] == 48650 and ud[-1][1] == {155: 3} and ud[-1][3] == ['6/2/0'], 'sword 1 -> 2: 1350z + 2 ore: money %d pouch %s ware %s' % (ud[-1][0], ud[-1][1], ud[-1][3])
def house(ev0, end, **k):
    ev = {45: 'square*2', 200: 'square*2'}; ev.update(ev0)
    return run(k.pop('tag'), end, ev, env(RT_LB_WARP=HOUSE, RT_MONEY=500, **k))
@case
def box_store():
    log, ud = house({260: 'circle*2', 300: 'circle*2', 340: 'circle*2', 380: 'circle*2'}, 500, tag='t_bs', RT_POUCH='129:5,1:3', RT_BOX_ITEMS='130:4')
    return ud[-1][1] == {1: 3} and ud[-1][2] == {130: 4, 129: 5}, 'herbs pouch -> box: pouch %s box %s' % (ud[-1][1], ud[-1][2])
@case
def box_take():
    log, ud = house({250: 'ddown*2', 270: 'circle*2', 320: 'circle*2', 370: 'circle*2', 420: 'circle*2'}, 520, tag='t_bt', RT_POUCH='1:3', RT_BOX_ITEMS='130:4,131:2')
    return ud[-1][1].get(130) == 1 and ud[-1][2].get(130) == 3, 'took one: pouch %s box %s' % (ud[-1][1], ud[-1][2])
@case
def box_equip():
    ev = {250: 'ddown*2', 265: 'ddown*2', 290: 'circle*2', 340: 'dright*2', 370: 'circle*2', 420: 'circle*2', 470: 'circle*2'}
    log, ud = house(ev, 560, tag='t_be', RT_POUCH='129:5', RT_WARE='6:1,6:2,2:1,2:5')
    return ud[-1][4] == 2, 'wielded weapon 1 -> 2: %d' % ud[-1][4]
@case
def box_tidy():
    log, ud = house({260: 'circle*2', 300: 'select*2', 340: 'circle*2'}, 450, tag='t_bz', RT_POUCH='130:2,0:0,129:3,1:1', RT_BOX_ITEMS='131:2,130:3,130:4,1:5')
    return list(ud[-1][1]) == [1, 129, 130], 'tidy prompt, yes: pouch sorted %s' % ud[-1][1]
@case
def box_sell():
    ev = {250: 'ddown*2', 265: 'ddown*2', 280: 'ddown*2', 295: 'ddown*2', 320: 'circle*2', 360: 'circle*2', 400: 'circle*2', 440: 'circle*2', 480: 'circle*2'}
    log, ud = house(ev, 560, tag='t_bl', RT_POUCH='129:5', RT_BOX_ITEMS='130:4,131:7')
    return ud[-1][0] == 502 and ud[-1][2].get(130) == 3, 'sold an item from the box: money %d box %s' % (ud[-1][0], ud[-1][2])
@case
def elder():
    log, ud = run('t_el', 700, circles(45, 22), env(W=ELDER, RT_MONEY=500))
    ok = 'talk state 0 (x68 47) page 12' in log and ud[-1][1].get(449) == 1
    return ok, 'the elder\'s first talk (story event 1): gift item 449 %s' % ud[-1][1]

if __name__ == '__main__':
    names = sys.argv[1:] or list(CASES)
    fail = 0
    for n in names:
        try:
            ok, msg = CASES[n]()
        except Exception as ex:
            ok, msg = False, 'error %r' % (ex,)
        print('%-14s %s  %s' % (n, 'OK  ' if ok else 'FAIL', msg)); sys.stdout.flush()
        fail += not ok
    sys.exit(1 if fail else 0)
