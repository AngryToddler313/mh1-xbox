#!/usr/bin/env python3
"""show_report.py - print an in-game bug report (F8) readably.

    python3 tools/show_report.py REPORT_DIR_OR_report.json [--log] [--replay [--build-match]]

REPORT_DIR is a reports/report_<date>/ folder (next to the logs folder; bug_report.sh packs the newest ones).
--log prints the last log lines, --replay prints the command that replays the session headless to that tick
(needs your own disc folder: DISC=path). Standard library only.
--replay --build-match runs the replay twice, headless: with the build that made the report (its "build" commit,
checked out into a temporary git worktree under build/, built with tools/build_pc.sh, the binary kept in
build/replay/bin/<commit>/ so the next replay of that build starts at once) and with the current build/pc/mhview
(HEAD). Each run freezes at the report's tick; its game state (stage, quest, hunter position / angle / motion / hp,
the monsters in the area) is compared with report.json. The first says whether the recording replays exactly, the
second whether the same inputs still reach the same state today (the bug is still reachable that way).
"""
import json, os, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

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
    if '--replay' in sys.argv and '--build-match' in sys.argv:
        ok = build_match(rep, d)
        if ok is None:
            return 2
    if '--log' in sys.argv:
        print('LOG TAIL')
        print(open(os.path.join(d, 'log_tail.txt'), errors='replace').read())
    print('files: ' + ', '.join(sorted(os.listdir(d))))
    return 0

def git(*a, cwd=ROOT):
    return subprocess.run(('git',) + a, cwd=cwd, capture_output=True, text=True)


def binary_for(commit):
    """mhview built at that commit: build/replay/bin/<full hash>/mhview, made once in a temporary worktree"""
    out = os.path.join(ROOT, 'build', 'replay', 'bin', commit)
    exe = os.path.join(out, 'mhview')
    if os.path.exists(exe):
        print('  build %s: cached (%s)' % (commit[:10], os.path.relpath(exe, ROOT)))
        return exe
    wt = os.path.join(ROOT, 'build', 'replay', 'wt_' + commit[:10])
    if os.path.exists(wt):
        git('worktree', 'remove', '--force', wt)
        shutil.rmtree(wt, ignore_errors=True)
    r = git('worktree', 'add', '--detach', wt, commit)
    if r.returncode:
        print('  cannot check out %s: %s' % (commit[:10], r.stderr.strip()))
        return None
    try:
        os.makedirs(os.path.join(wt, 'build'), exist_ok=True)
        for link in ('disc', 'tools/compilers', 'tools/binutils', '.venv', 'build/sysroot32'):   # untracked folders the build reads
            src = os.path.realpath(os.path.join(ROOT, link))
            if os.path.exists(src) and not os.path.exists(os.path.join(wt, link)):
                os.symlink(src, os.path.join(wt, link))
        print('  build %s: building in %s (a few minutes, once)' % (commit[:10], os.path.relpath(wt, ROOT)))
        log = os.path.join(ROOT, 'build', 'replay', 'build_%s.log' % commit[:10])
        with open(log, 'w') as f:
            r = subprocess.run(['sh', 'tools/build_pc.sh'], cwd=wt, stdout=f, stderr=subprocess.STDOUT,
                               env=dict(os.environ, MH1_VERSION=commit[:10]))
        built = os.path.join(wt, 'build', 'pc', 'mhview')
        if r.returncode or not os.path.exists(built):
            print('  build %s failed: see %s' % (commit[:10], os.path.relpath(log, ROOT)))
            return None
        os.makedirs(out, exist_ok=True)
        shutil.copy2(built, exe)
        return exe
    finally:
        git('worktree', 'remove', '--force', wt)
        shutil.rmtree(wt, ignore_errors=True)
        git('worktree', 'prune')


def replay(exe, rep, d, tag):
    """the report's session replayed headless with exe to its tick: the new report's game section (or None)"""
    g = rep.get('game') or {}
    args = rep.get('arguments', '').split()
    if '--input' in args:
        k = args.index('--input'); del args[k:k + 2]
    if args and not os.path.isabs(args[0]):     # the disc folder: DISC= or the report's own
        args[0] = os.environ.get('DISC', args[0])
    base = os.path.join(ROOT, 'build', 'replay', 'run_' + tag)
    shutil.rmtree(base, ignore_errors=True)
    os.makedirs(base)
    tsec = int(int(g.get('tick', 0)) / 30 * 1.5) + 60
    env = dict(os.environ, RT_SEED=str(rep.get('random_seed')), RT_PICK_AT=str(g.get('tick')), RT_PICK_EXIT='1',
               MH1_LOG_DIR=os.path.join(base, 'logs'), MH1_SAVE_DIR=os.path.join(base, 'card'), RT_PICK_NOTE='replay ' + tag)
    cmd = [exe] + args + ['--input', '@' + os.path.abspath(os.path.join(d, 'input.txt')), '--shot', os.path.join(base, 'end.png'),
                          '--time', str(tsec)]
    with open(os.path.join(base, 'run.log'), 'w') as f:
        subprocess.run(cmd, cwd=ROOT, env=env, stdout=f, stderr=subprocess.STDOUT, timeout=1800)
    reps = os.path.join(base, 'reports')         # (next to the logs folder)
    found = sorted(os.listdir(reps)) if os.path.isdir(reps) else []
    if not found:
        return None, base
    return json.load(open(os.path.join(reps, found[-1], 'report.json'))).get('game'), os.path.join(reps, found[-1])


def same_state(a, b):
    """the parts of the game section a replay must reproduce: [] if equal, else the differences"""
    diff = []
    for k in ('tick', 'stage', 'quest', 'mode'):
        if a.get(k) != b.get(k):
            diff.append('%s %s != %s' % (k, a.get(k), b.get(k)))
    ha, hb = a.get('hunter', {}), b.get('hunter', {})
    for k in ('angle', 'motion_id', 'hp'):
        if ha.get(k) != hb.get(k):
            diff.append('hunter %s %s != %s' % (k, ha.get(k), hb.get(k)))
    pa, pb = ha.get('position') or [0, 0, 0], hb.get('position') or [0, 0, 0]
    if max(abs(x - y) for x, y in zip(pa, pb)) > 0.5:
        diff.append('hunter position %s != %s' % (pa, pb))
    ma = {(m['slot'], m['kind']): m for m in a.get('monsters_in_area', [])}
    mb = {(m['slot'], m['kind']): m for m in b.get('monsters_in_area', [])}
    if set(ma) != set(mb):
        diff.append('monsters in the area %s != %s' % (sorted(ma), sorted(mb)))
    for key in set(ma) & set(mb):
        x, y = ma[key], mb[key]
        if x.get('hp') != y.get('hp') or x.get('motion_id') != y.get('motion_id') or \
                max(abs(p - q) for p, q in zip(x['position'], y['position'])) > 0.5:
            diff.append('monster slot %d: %s/%s/%s != %s/%s/%s' % (key[0], x['position'], x.get('motion_id'), x.get('hp'),
                                                                   y['position'], y.get('motion_id'), y.get('hp')))
    return diff


def build_match(rep, d):
    want = (rep.get('build') or '').rstrip('+')
    r = git('rev-parse', '--verify', '--quiet', want + '^{commit}')
    head = git('rev-parse', 'HEAD').stdout.strip()
    if r.returncode or not want:
        print('BUILD MATCH: the report\'s build %r is not a commit of this repository' % rep.get('build'))
        return None
    commit = r.stdout.strip()
    if (rep.get('build') or '').endswith('+'):
        print('BUILD MATCH: note: the report was made by a build with uncommitted changes (%s); its commit is used' % rep['build'])
    g = rep.get('game') or {}
    print('BUILD MATCH: report build %s, HEAD %s' % (commit[:10], head[:10]))
    results = {}
    for tag, c in (('report', commit), ('head', None)):
        if c is None:
            exe = os.path.join(ROOT, 'build', 'pc', 'mhview')
            if not os.path.exists(exe):
                print('  HEAD: build/pc/mhview missing: run tools/build_pc.sh')
                results[tag] = None
                continue
            label = 'HEAD %s (build/pc/mhview)' % head[:10]
        else:
            exe = binary_for(c)
            if exe is None:
                results[tag] = None
                continue
            label = 'report build %s' % c[:10]
        got, where = replay(exe, rep, d, tag)
        if got is None:
            print('  %s: no report written (did not reach tick %s; see %s/run.log)' % (label, g.get('tick'), os.path.relpath(where, ROOT)))
            results[tag] = False
            continue
        diff = same_state(g, got)
        results[tag] = not diff
        print('  %s: %s  (%s)' % (label, 'MATCH: the same state at tick %s' % g.get('tick') if not diff else 'DIFFERS: ' + '; '.join(diff),
                                  os.path.relpath(where, ROOT)))
    if results.get('report'):
        print('  -> the recording replays exactly with the build that made it' + (
            '; HEAD reaches the same state with the same inputs (the bug should still be reachable this way)' if results.get('head')
            else '; HEAD does not reach the same state with these inputs (the game changed since: the bug may or may not still be there)'))
    return results


if __name__ == '__main__':
    sys.exit(main())
