# 2.1-32 harness test (see README.md). Run from ~/emu with the test-only orgtest.gba/.elf.
import sys; pass
sys.argv=['x','none']
import os as _o; exec(open(_o.path.join(_o.path.dirname(__file__), 'organizer_test.py')).read().split("import sys as _s")[0])
import os
if os.path.exists(os.path.expanduser('~/emu/pt/o.sav')): os.remove(os.path.expanduser('~/emu/pt/o.sav'))
_,_,_,before,names_b,wps_b=run('pt/o_type60m.ss',0,3,0,0,'pt/o_t60dump.ss')
cmds=["load pt/o.sav","loadstate pt/o_type60m.ss"]+["press DOWN 4","wait 10"]*5+["press A 4","wait 30"]+["press DOWN 4","wait 10"]*7+["press A 4","wait 60","mash A 10 40","wait 300","shot /tmp/claude-0/org/saved.png"]
E_.run(cmds)
print('sav size', os.path.getsize(os.path.expanduser('~/emu/pt/o.sav')))
E_.run(["load pt/o.sav","wait 300","mash START 6 40","mash A 2 60","wait 300","savestate pt/o_reload.ss","shot /tmp/claude-0/org/reload.png"])
E_.menu('pt/o_reload.ss','pt/o_reloadm.ss')
_,_,_,after,names_a,wps_a=run('pt/o_reloadm.ss',0,3,0,0,'pt/o_reload2.ss')
k=lambda r:(r['has'],r['pers'],r['species'],r['egg'],r['level'])
print('slots equal:', all(k(a)==k(b) for a,b in zip(before,after)), sum(r['has'] for r in after))
print('names equal:', names_a==names_b, names_a)
print('wallpapers equal:', wps_a==wps_b)
