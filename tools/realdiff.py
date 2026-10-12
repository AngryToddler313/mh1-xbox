#!/usr/bin/env python3
"""realdiff.py FILE.c: per function, the number of differing lines that tools/check.py -v shows, not counting calls
to overlay addresses check.py cannot name (func_XXXXXX vs "?"). Prints only the differing functions. Standard library only."""
import sys,subprocess,re
out=subprocess.run(['python3','tools/check.py','-v',sys.argv[1]],capture_output=True,text=True).stdout.split('\n')
cur=None;n=0;res=[]
def fin():
    if cur: res.append((cur,n))
for l in out:
    m=re.match(r'^(OK|--)  (\S+)',l)
    if m:
        fin(); cur=m.group(2); n=0; continue
    if l.strip().startswith('>>') and not re.search(r'\(calls func_[0-9A-F]+, original calls \?\)',l):
        n+=1
fin()
for c,n in res:
    if n: print(n,c)
print('real-diff functions:',sum(1 for c,n in res if n),'of',len(res))
