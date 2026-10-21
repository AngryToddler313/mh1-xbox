#!/bin/bash
# merge the helper branches into the main branch, verify all 7 G modules, push
cd /root/mh1-xbox || exit 1
[ -z "$(git status --porcelain)" ] || { echo "main tree dirty"; exit 1; }
before=$(git rev-parse HEAD)
for w in A B C; do
  git merge -q --no-edit -m "Merge helper branch g-work-$w

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01YMLQXcmriUqugoXujYbNpG" g-work-$w >/tmp/merge_$w.log 2>&1 || { git merge --abort; echo "conflict $w"; tail -3 /tmp/merge_$w.log; }
done
n=$(VERSION=mhg timeout 5400 tools/rebuild.sh 2>&1 | grep -c " OK ")
if [ "$n" != 7 ]; then echo "MERGED BUILD BROKEN ($n/7), resetting"; git reset -q --hard $before; exit 1; fi
VERSION=mhg python3 tools/progress.py | tail -2 | head -1
git push -q fork claude/mhg-decomp-tjvfyr 2>&1 | grep -v remote
echo merged $(git rev-list --count $before..HEAD) commits
