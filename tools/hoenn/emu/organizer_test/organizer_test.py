# 2.1-32 harness test (see README.md). Run from ~/emu with the test-only orgtest.gba/.elf.
import sys, struct, random
import os; sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import pt
E_ = pt.Emu('orgtest.gba','orgtest.elf',cwd=os.path.expanduser('~/emu'))
T = E_.s('gHoennOrgTest')
ORDER=[1,11,12,13,14,16,2,4,5,3,15,7,6,8,17,18,9,19]
TNAME={1:'NORMAL',2:'FIGHTING',3:'FLYING',4:'POISON',5:'GROUND',6:'ROCK',7:'BUG',8:'GHOST',9:'STEEL',11:'FIRE',12:'WATER',13:'GRASS',14:'ELECTRIC',15:'PSYCHIC',16:'ICE',17:'DRAGON',18:'DARK',19:'FAIRY'}
WP=[3,5,10,0,14,6,1,9,2,11,15,0,4,7,8,7,14,12,13]
N=420
def group(r):
    if r['egg']: return 18
    return ORDER.index(r['type']) if r['type'] in ORDER else 17
def key(r, mode):
    k=(r['dex']<<16)|r['species']
    if mode==1: k|=group(r)<<40
    if mode==2 and not r['egg']: k|=(100-r['level'])<<32
    return k|((1 if r['egg'] else 0)<<48)
def recs(words):
    b=E_._bytes(words); out=[]
    for i in range(N):
        sp,dex,lv,egg,ty,has,pers=struct.unpack_from('<HHBBBBI',b,i*12)
        out.append(dict(species=sp,dex=dex,level=lv,egg=egg,type=ty,has=has,pers=pers))
    return out
def run(state, seed, mode, skip, fill, out_state):
    cmds=[f"loadstate {state}", f"w32 {T:x} {seed}", f"w8 {T+4:x} {mode}", f"w8 {T+5:x} {skip}", f"w8 {T+6:x} {fill}", f"w8 {T+7:x} 0"]
    cmds+=["press DOWN 4","wait 10"]*5+["press A 4","wait 30"]+["press DOWN 4","wait 10"]*6+["press A 4","wait 200", f"savestate {out_state}"]
    E_.run(cmds)
    w=E_.read_words(out_state, T, 4)
    done=(w[1]>>24)&0xff; result=w[2]&0xffff; pre,post=w[3] if False else None,None
    pp=E_.read_words(out_state, T+12, 2)
    pre=recs(E_.read_words(out_state, pp[0], N*3)); post=recs(E_.read_words(out_state, pp[1], N*3))
    sp=E_.read_words(out_state, E_.s('gPokemonStoragePtr'),1)[0]
    raw=E_._bytes(E_.read_words(out_state, sp+0x8344, (14*9+14+3)//4+1))
    names=[pt.decode(raw[i*9:i*9+9]) for i in range(14)]
    wps=list(raw[14*9:14*9+14])
    return done,result,pre,post,names,wps
def expected(pre, mode, skip):
    first=skip*30; size=N-first
    occ=[r for r in range(size) if pre[first+r]['has']]
    order=sorted(occ,key=lambda r:(key(pre[first+r],mode),r))
    newslot=list(range(len(order))); packed=False
    if mode==1:
        ns=[];pos=0;i=0;ok=True
        while i<len(order):
            g=group(pre[first+order[i]]);n=0
            while i+n<len(order) and group(pre[first+order[i+n]])==g:n+=1
            inb=pos%30
            if inb and n>30-inb: pos+=30-inb
            if pos+n>size: ok=False;break
            for _ in range(n): ns.append(pos);pos+=1;i+=1
        if ok: newslot=ns
        else: packed=True
    tgt=[None]*size
    for i,o in enumerate(order): tgt[newslot[i]]=o
    empt=[r for r in range(size) if not pre[first+r]['has']]
    j=0
    for s in range(size):
        if tgt[s] is None: tgt[s]=empt[j];j+=1
    return [pre[i] for i in range(first)]+[pre[first+t] for t in tgt], packed
def expnames(post, skip):
    starts={};res={}
    for b in range(skip,14):
        box=post[b*30:b*30+30]
        if not box[0]['has']: continue
        g=group(box[0]); shared=any(r['has'] and group(r)!=g for r in box)
        starts[g]=starts.get(g,0)+1; nth=starts[g]
        nm='EGGS' if g==18 else TNAME[ORDER[g]]
        suf=(' %d'%nth if nth>1 else '')+('+' if shared and nth>1 else '')
        if nth==1 and shared: suf=' +'
        res[b]=(nm[:8-len(suf)]+suf, WP[g])
    return res
def check(label, state, seed, mode, skip, fill, prev=None):
    done,result,pre,post,names,wps=run(state,seed,mode,skip,fill,f"pt/o_{label}.ss")
    exp,packed=expected(pre,mode,skip)
    errs=[]
    if not done: errs.append('not done')
    same=lambda a,b:(a['has'],a['pers'],a['species'],a['egg'],a['level'])==(b['has'],b['pers'],b['species'],b['egg'],b['level'])
    bad=[i for i in range(N) if not same(post[i],exp[i])]
    if bad: errs.append(f'{len(bad)} slots differ, first {bad[0]}')
    count=sum(r['has'] for r in pre[skip*30:])
    want = 2 if count==0 else (1 if packed else 0)
    if result!=want: errs.append(f'result {result} want {want}')
    if sorted((r['pers'],r['species']) for r in pre if r['has'])!=sorted((r['pers'],r['species']) for r in post if r['has']): errs.append('multiset differs')
    if mode==1 and count:
        en=expnames(post,skip)
        for b,(nm,wp) in en.items():
            if names[b]!=nm or wps[b]!=wp: errs.append(f'box {b}: {names[b]!r}/{wps[b]} want {nm!r}/{wp}')
    if prev is not None:
        for b in range(skip,14):
            if prev[b] and names[b]!='BOX%d'%(b+1): errs.append(f'box {b} not reset: {names[b]!r}')
    print(label, 'mode',mode,'skip',skip,'fill',fill,'count',count,'result',result,'packed' if packed else '', 'OK' if not errs else errs[:5])
    print('   names:',names)
    return names
import sys as _s
if len(_s.argv)>1 and _s.argv[1]=='renum':
    B='pt/o_base.ss'
    nm=check('type60',B,14,1,0,60)
    E_.menu('pt/o_type60.ss','pt/o_type60m.ss')
    check('renum','pt/o_type60m.ss',0,0,0,0,prev=[n!='BOX%d'%(i+1) for i,n in enumerate(nm)])
    _s.exit()
B='pt/o_base.ss'
check('num',B,11,0,0,80)
check('lvl',B,12,2,0,80)
check('type30',B,13,1,0,30)
nm=check('type60',B,14,1,0,60)
check('type95',B,15,1,0,95)
check('skip3',B,16,0,3,80)
check('skip13',B,17,2,13,90)
check('empty',B,18,0,0,0)
# second sort on the type-sorted boxes: names she wrote go back to BOX<n>
check('renum','pt/o_type60.ss',0,0,0,0,prev=[n!='BOX%d'%(i+1) for i,n in enumerate(nm)])
