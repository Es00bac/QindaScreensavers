#!/usr/bin/env python3
"""Exercise only the isolated labwc test session; never the desktop outputs."""
import os,pathlib,subprocess,signal,time
root=pathlib.Path(__file__).resolve().parents[1];config=root/'validation/outputs.conf'
env=os.environ.copy();env.update(WAYLAND_DISPLAY=os.environ['SAVER_TEST_WAYLAND'],QT_QPA_PLATFORM='wayland',SDL_AUDIODRIVER='dummy')
pids=subprocess.check_output(['pgrep','-f','^kanshi -c '+str(config)+'$'],text=True).split()
if len(pids)!=1:raise SystemExit('Expected exactly one isolated test kanshi')
pid=int(pids[0])
def layout(enabled):
 config.write_text('profile test {\n output HEADLESS-1 enable mode --custom 1440x900@60Hz position 0,0 scale 1\n output HEADLESS-2 '+('enable mode --custom 1440x2560@60Hz position 1440,0 scale 2' if enabled else 'disable')+'\n}\n')
 os.kill(pid,signal.SIGHUP)
for project,exe in [('qinda-patrol','qinda-patrol'),('circuit-reef','circuit-reef'),('prism-circuit','prism-circuit'),('prism-brawl','prism-brawl'),('starward-reimagined','starward')]:
 layout(True);time.sleep(.3)
 args=['--screensaver','--duration','5.5','--verbose','--no-metrics'] if project=='qinda-patrol' else ['--fullscreen','--quit-after','5.5']
 if project in ['prism-circuit','prism-brawl']:args+=['--mute']
 logpath=root/'validation'/f'{project}-hotplug.log'
 with logpath.open('w') as out:
  process=subprocess.Popen([str(root/project/'build'/exe),*args],env=env,stdout=out,stderr=subprocess.STDOUT)
  try:
   time.sleep(1.5);layout(False);time.sleep(1.0)
   if process.poll() is not None:raise RuntimeError(project+' exited on output removal')
   layout(True);time.sleep(1)
   if process.poll() is not None:raise RuntimeError(project+' exited on output addition')
   code=process.wait(timeout=20)
   if code:raise RuntimeError(project+' returned '+str(code))
  finally:
   if process.poll() is None:process.terminate();process.wait(timeout=5)
 text=logpath.read_text()
 if project!='qinda-patrol' and (text.count('OUTPUT_ADDED')!=3 or text.count('OUTPUT_REMOVED')!=1):raise RuntimeError(project+' did not reconstruct an output surface: '+text)
 if project=='qinda-patrol' and 'window 2 created' not in text:raise RuntimeError('Patrol did not recreate output')
 print(project+' PASS: portrait + mixed scale + remove + reconnect',flush=True)
layout(True)
