#!/usr/bin/env python3
"""mhg_dataparts.py [--files FILE_BASE...] PART... : register G parts whose functions use switch tables: compile the part fresh, find the tables'
G range (object symbols @N in .data/.rodata, located through the lui/addiu pair in the G bytes of the function), and add
`MODULE START END PART` + `MODULE:rodata START END PART` to config/mhg/c_files.txt (replacing any older lines of the part).
Run it after tools/mhg_parts.py: --files mhg/main/pl/f_pl treats every listed part of that file (_pN). A part with other data sections
(.sdata/.bss/strings) is dropped from the list. Use it INSTEAD of mhg_dropparts.py --sections, which drops parts whose rodata line exists."""
import os,re,subprocess,struct,sys,tempfile
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
B=os.path.expanduser('~/.local/opt/ps2-binutils-2.47/bin/mips64r5900el-ps2-elf-')
WIBO='tools/compilers/wibo'; MWCC='tools/compilers/mwcps2-3.0b52-030722/mwccps2.exe'
LIST='config/mhg/c_files.txt'
def vram(m):
    y=open('config/mhg/%s.yaml'%m).read()
    return int(re.search(r'vram:\s*(0x[0-9A-Fa-f]+|\d+)',y).group(1),0)
def compile_part(part,out):
    r=subprocess.run([WIBO,MWCC,'-c','-O4,p','-nostdinc','-stderr','-pragma','divbyzerocheck on','-DVERSION_MHG','-Iinclude','-Ibuild/mhg/raw','-Ibuild/raw','src/%s.c'%part,'-o',out],capture_output=True,text=True)
    return r.returncode==0
def checkrows(part):
    env=dict(os.environ,VERSION='mhg')
    out=subprocess.run(['python3','tools/check.py','src/%s.c'%part],capture_output=True,text=True,env=env).stdout
    rows=[]
    for l in out.splitlines():
        m=re.match(r'(OK|--|\?\?)\s+(\S+)\s+(\S+)\s+0x([0-9A-F]+)\s+(\d+) bytes',l)
        if m: rows.append((m[1],m[2],m[3],int(m[4],16),int(m[5])))
    return rows
def work(part):
    rows=checkrows(part)
    if not rows or any(r[0]!='OK' for r in rows): return None,'functions not OK'
    mod=rows[0][2]; fa={r[1]:r[3] for r in rows}
    with tempfile.TemporaryDirectory(dir='build') as t:
        obj=os.path.join(t,'p.o')
        if not compile_part(part,obj): return None,'compile'
        secs=subprocess.run([B+'size','-A',obj],capture_output=True,text=True).stdout
        bad=[m[1] for l in secs.splitlines() for m in [re.match(r'(\.\S+)\s+(\d+)\s+',l)] if m and int(m[2])>0 and m[1] not in('.text','.comment','.mwcats','.reginfo','.pdr','.mdebug','.gnu.attributes','.MIPS.abiflags','.sdata_unused','.data','.rodata')]
        if bad: return None,'sections '+','.join(bad)
        syms={}
        for l in subprocess.run([B+'objdump','-x',obj],capture_output=True,text=True).stdout.split('\n'):
            m=re.match(r'([0-9a-f]+) l\s+O (\.data|\.rodata)\s+([0-9a-f]+) (@\d+)',l)
            if m: syms[m[4]]=int(m[3],16)
        out=subprocess.run([B+'objdump','-dr',obj],capture_output=True,text=True).stdout
    a=min(r[3] for r in rows); b=max(r[3]+r[4] for r in rows)
    text=(mod,a,b)
    if not syms: return text,None
    base=vram(mod); img=open('build/mhg/split/%s.bin'%mod,'rb').read()
    gw=lambda x: struct.unpack_from('<I',img,x-base)[0]
    addr={}
    for blk in out.split('Disassembly of section')[1:]:
        fm=re.search(r'^[0-9a-f]+ <(\w+)>:',blk,re.M)
        if not fm or fm[1] not in fa: continue
        f0=fa[fm[1]]
        pairs=[(int(r[1],16),r[2],r[3]) for r in re.finditer(r'\s+([0-9a-f]+): R_MIPS_(HI16|LO16)\s+(@\d+)',blk) if r[3] in syms]
        for k,(off,tt,s) in enumerate(pairs):
            if tt!='HI16' or s in addr: continue
            lo=next((o for (o,t2,s2) in pairs[k+1:] if t2=='LO16' and s2==s),None)
            if lo is None: continue
            ghi=gw(f0+off)&0xFFFF; glo=gw(f0+lo)&0xFFFF
            if glo>=0x8000: glo-=0x10000
            addr[s]=(ghi<<16)+glo
    if len(addr)!=len(syms): return None,'table symbols not located'
    return text,(mod,min(addr.values()),max(addr[s]+syms[s] for s in addr))
def parts_of(bases):
    out=[]
    for l in open(LIST):
        p=l.split()
        if len(p)==4 and not l.startswith('#') and ':' not in p[0] and re.search(r'_p\d+$',p[3]):
            if any(re.fullmatch(re.escape(b)+r'_p\d+',p[3]) for b in bases): out.append(p[3])
    return out
if __name__=='__main__':
    lines=open(LIST).read().split('\n')
    new=[]
    args=sys.argv[1:]
    if args and args[0]=='--files':      # --files mhg/main/pl/f_pl ... : every listed part of those files
        args=parts_of(args[1:])
    for part in args:
        text,ro=work(part)
        if text is None:
            print('DROP',part,ro); lines=[l for l in lines if not(len(l.split())==4 and l.split()[3]==part)]; continue
        lines=[l for l in lines if not(len(l.split())==4 and l.split()[3]==part)]
        new.append('%s 0x%08X 0x%08X %s'%(text[0],text[1],text[2],part))
        if ro and isinstance(ro,tuple):
            new.append('%s:rodata 0x%08X 0x%08X %s'%(ro[0],ro[1],ro[2],part))
        print('ok',part,new[-2:] if ro else new[-1:])
    open(LIST,'w').write('\n'.join(lines).rstrip('\n')+'\n'+'\n'.join(new)+'\n')
