#!/usr/bin/env python3
"""Developer tool: record the executable's RGB24 frames with ffmpeg on a GL display.
Run from the project root. Python and ffmpeg are not runtime dependencies.
"""
import os, pathlib, subprocess, tempfile, argparse
p=argparse.ArgumentParser()
p.add_argument('--binary', default='build/prism-brawl')
p.add_argument('--output', default='previews/Prism_Brawl_Preview.mp4')
p.add_argument('--seconds',type=int,default=8)
p.add_argument('--width',type=int,default=1280)
p.add_argument('--height',type=int,default=720)
a=p.parse_args()
if not(1<=a.seconds<=120 and 320<=a.width<=8192 and 180<=a.height<=8192):p.error('invalid dimensions or duration')
binary=pathlib.Path(a.binary).resolve();out=pathlib.Path(a.output).resolve();out.parent.mkdir(parents=True,exist_ok=True)
env=os.environ.copy()
with tempfile.TemporaryDirectory(prefix='prism-brawl-export-') as td:
    td=pathlib.Path(td);clips=[]
    for i,start in enumerate([6,108,213]):
        fifo=td/f'frames{i}.rgb';os.mkfifo(fifo);clip=td/f'clip{i}.mp4';clips.append(clip)
        enc=subprocess.Popen(['ffmpeg','-hide_banner','-loglevel','error','-y','-f','rawvideo','-pixel_format','rgb24','-video_size',f'{a.width}x{a.height}','-framerate','30','-i',str(fifo),'-an','-c:v','libx264','-preset','fast','-crf','18','-pix_fmt','yuv420p','-movflags','+faststart',str(clip)],env=env)
        try:
            subprocess.run([str(binary),'--seed','41','--start',str(start),'--size',f'{a.width}x{a.height}','--eco','--fps','30','--frames',str(a.seconds*30),'--raw',str(fifo)],env=env,check=True)
            if enc.wait(timeout=60):raise RuntimeError('ffmpeg encoding failed')
        except BaseException:
            enc.terminate();enc.wait(timeout=10);raise
        print(f'Captured excerpt {i+1}/3 at simulation t={start}',flush=True)
    listing=td/'concat.txt';listing.write_text(''.join(f"file '{c.as_posix()}'\n" for c in clips))
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-f','concat','-safe','0','-i',str(listing),'-c','copy','-movflags','+faststart',str(out)],check=True)
print(out)
