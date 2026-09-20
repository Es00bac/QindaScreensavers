#!/usr/bin/env python3
import os, pathlib, subprocess
root=pathlib.Path(__file__).resolve().parents[1]
env=os.environ.copy();env['WAYLAND_DISPLAY']=os.environ['SAVER_TEST_WAYLAND'];env['QT_QPA_PLATFORM']='wayland';env['SDL_AUDIODRIVER']='dummy'
for project,exe,args in [('qinda-patrol','qinda-patrol',['--screensaver','--duration','2.5','--verbose','--no-metrics']),('circuit-reef','circuit-reef',['--fullscreen','--quit-after','2.5','--start','20','--private']),('prism-circuit','prism-circuit',['--fullscreen','--quit-after','2.5','--start','20','--mute']),('prism-brawl','prism-brawl',['--fullscreen','--quit-after','2.5','--start','18','--mute']),('starward-reimagined','starward',['--fullscreen','--quit-after','2.5','--start','100'])]:
    if project!='qinda-patrol':args+=['--capture-dir',str(root/'validation'/'native'/project)]
    with (root/'validation'/f'{project}-native.log').open('w') as log:
        result=subprocess.run([str(root/project/'build'/exe),*args],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=25)
    print(project,result.returncode,flush=True)
    if result.returncode:raise SystemExit(result.returncode)
