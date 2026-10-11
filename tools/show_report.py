#!/usr/bin/env python3
"""show_report.py - print an in-game bug report (F8) readably.

    python3 tools/show_report.py REPORT_DIR_OR_report.json [--log] [--replay]

REPORT_DIR is a reports/report_<date>/ folder (next to the logs folder; bug_report.sh packs the newest ones).
--log prints the last log lines, --replay prints the command that replays the session headless to that tick
(needs your own disc folder: DISC=path). Standard library only.
"""
import json, os, sys

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    if not args:
        print(__doc__)
        return 1
    p = args[0]
    d = p if os.path.isdir(p) else os.path.dirname(p) or '.'
    rep = json.load(open(os.path.join(d, 'report.json') if os.path.isdir(p) else p))
    g = rep.get('game') or {}
    print('REPORT %s   build %s   %s' % (rep.get('report'), rep.get('build'), rep.get('created')))
    print('NOTE: %s' % rep.get('note'))
    print('window %sx%s   random seed %s   args: %s' % (rep['window'][0], rep['window'][1], rep.get('random_seed'), rep.get('arguments')))
    if g:
        h = g.get('hunter', {})
        c = g.get('camera', {})
        print('GAME  tick %s  mode %s step %s  stage %s  quest %s  map areas %s' % (
            g.get('tick'), g.get('mode'), g.get('step'), g.get('stage'), g.get('quest'), g.get('map_areas')))
        print('      hunter at %s angle %s  motion %s frame %s  mode %s step %s  hp %s' % (
            h.get('position'), h.get('angle'), h.get('motion_id'), h.get('motion_frame'), h.get('mode'), h.get('step'), h.get('hp')))
        print('      camera eye %s target %s fov %s' % (c.get('eye'), c.get('target'), c.get('fov')))
        for m in g.get('monsters_in_area', []):
            print('      %s slot %2d kind %2d at %s motion %s frame %s mode %s/%s hp %s' % (
                'npc    ' if m.get('npc') else 'monster', m['slot'], m['kind'], m['position'], m['motion_id'], m['motion_frame'], m['mode'], m['step'], m['hp']))
    print('MARKS')
    for m in rep.get('marks', []):
        if m['type'] == 'click':
            print('  %d. click at (%d,%d), %d new object(s)' % (m['n'], m['x'], m['y'], m['picked']))
        else:
            print('  %d. box (%d,%d)-(%d,%d), %d new object(s)' % (m['n'], m['x0'], m['y0'], m['x1'], m['y1'], m['picked']))
    print('PICKED OBJECTS (%d)' % len(rep.get('objects', [])))
    for i, o in enumerate(rep.get('objects', []), 1):
        r = o.get('render', {})
        b = r.get('blend', {})
        tex = r.get('texture')
        print('  %d. [%s] %s' % (i, o['kind'], o['description']))
        print('     ids %s  world %s  mark %s%s' % (o.get('ids'), o.get('world_position'), o.get('picked_by_mark'),
              '  hit %s' % o['hit_world'] if 'hit_world' in o else ''))
        if 'nearest_bone' in o:
            print('     nearest bone %(index)d (%(distance).0f away)' % o['nearest_bone'])
        for k in ('motion_id', 'motion_frame', 'hp', 'slot', 'model_kind', 'model_part', 'hunter_slot'):
            if k in o:
                print('     %s %s' % (k, o[k]), end='')
        if 'motion_id' in o:
            print()
        print('     draw: %s verts%s  blend %s %s/%s op %s  z-test %s z-write %s  alpha %s>%s  filter %s clamp %s  scroll %s' % (
            r.get('vertices'), ' tex %sx%s' % (tex['width'], tex['height']) if tex else ' untextured',
            'on' if b.get('on') else 'off', b.get('src'), b.get('dst'), b.get('operation'), r.get('z_test'), r.get('z_write'),
            r.get('alpha_func'), r.get('alpha_ref'), r.get('filter'), r.get('clamp'), r.get('scroll_matrix')))
        if 'draw_function' in o:
            print('     draw function %s' % o['draw_function'])
    rp = rep.get('replay', {})
    print('REPLAY %d ticks of pad input in input.txt, seed %s' % (rp.get('ticks', 0), rep.get('random_seed')))
    if '--replay' in sys.argv:
        disc = os.environ.get('DISC', 'disc/mh1')
        args = rep.get('arguments', '').split()
        if '--input' in args:       # the original run's own --input (a test script) is replaced by the recording
            k = args.index('--input'); del args[k:k + 2]
        # --shot hides the window and --time T fixes the clock, so the game runs every tick up to
        # 30*T as fast as it can (2 s for a 4-minute hunt) instead of in real time on the owner's
        # screen; RT_PICK_EXIT stops it at the pick tick, so T only has to be past it
        tsec = int(int(g.get('tick', 0)) / 30 * 1.5) + 60
        print('  mkdir -p build/replay && RT_SEED=%s RT_PICK_AT=%s RT_PICK_EXIT=1 MH1_LOG_DIR=build/replay/logs build/pc/mhview %s --input @%s --shot build/replay/end.png --time %d' % (
            rep.get('random_seed'), g.get('tick'), ' '.join(args), os.path.join(d, 'input.txt'), tsec))
        print('  (no window, faster than real time; freezes at game tick %s and writes a new report under build/replay/reports whose game state,' % g.get('tick'))
        print('   hunter and monsters equal this one, screenshot included; RT_PICK_NOTE=text. Without --shot/--time it plays in a window in real time)')
    if '--log' in sys.argv:
        print('LOG TAIL')
        print(open(os.path.join(d, 'log_tail.txt'), errors='replace').read())
    print('files: ' + ', '.join(sorted(os.listdir(d))))
    return 0

if __name__ == '__main__':
    sys.exit(main())
