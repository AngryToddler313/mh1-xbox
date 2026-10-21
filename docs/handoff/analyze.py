import re,collections
cov={}
for l in open('/root/mh1-xbox/config/mhg/c_files.txt'):
    p=l.split('#')[0].split()
    if len(p)==4 and ':' not in p[0]: cov.setdefault(p[0],[]).append((int(p[1],16),int(p[2],16)))
raw=set()
for l in open('/root/mh1-xbox/config/mhg/c_rawfuncs.txt'):
    p=l.split('#')[0].split()
    if len(p)>=4: raw.add(int(p[1],16))
rows=[];ok_unc=[];seen=set()
for l in open('/root/survey.txt'):
    m=re.match(r'(\S+) (OK|--)\s+(\S+)\s+(\S+)\s+0x([0-9A-F]+)\s+(\d+) bytes(?:\s+\((\d+)/(\d+) instructions differ\))?',l)
    if not m: continue
    f,st,n,mod,a,sz,d,t=m.groups(); a=int(a,16); sz=int(sz)
    if (mod,a) in seen: continue
    seen.add((mod,a))
    if any(lo<=a<hi for lo,hi in cov.get(mod,[])) and a not in raw: continue
    if st=='OK': ok_unc.append((f,n,sz,a in raw))
    else: rows.append((int(d),int(t),sz,f,n,mod,a))
print('uncovered OK',len(ok_unc),sum(x[2] for x in ok_unc), 'of which raw', sum(x[2] for x in ok_unc if x[3]))
c=collections.Counter()
for d,t,sz,*_ in rows:
    k='1-2' if d<=2 else '3-5' if d<=5 else '6-10' if d<=10 else '>10'
    c[k]+=sz
print('differ bytes',c)
rows.sort()
open('/root/near.txt','w').write(''.join('%d/%d %d %s %s %s 0x%X\n'%r for r in rows))
print(collections.Counter(x[0] for x in ok_unc if not x[3]).most_common(12))
