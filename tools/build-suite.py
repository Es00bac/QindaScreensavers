#!/usr/bin/env python3
"""Build with the machine's actual Portage job and load limits, unchanged."""
import pathlib, shlex, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
limits=shlex.split(subprocess.check_output(['portageq','envvar','MAKEOPTS'],text=True).strip())
projects=sys.argv[1:] or ['qinda-patrol','circuit-reef','prism-circuit','prism-brawl','starward-reimagined']
for name in projects:
    source=root/name
    if not (source/'CMakeLists.txt').is_file(): raise SystemExit(f'Unknown project: {name}')
    options=[]
    if name=='qinda-patrol': options=['-DPATROL_BUILD_QML_MODULE=ON']
    if name=='circuit-reef': options=['-DREEF_BUILD_QT=ON','-DREEF_BUILD_QML_MODULE=ON']
    log=root/'validation'/f'{name}-build.log';log.parent.mkdir(exist_ok=True)
    with log.open('w') as out:
        subprocess.run(['cmake','-S',str(source),'-B',str(source/'build'),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release',*options],stdout=out,stderr=subprocess.STDOUT,check=True)
        subprocess.run(['ninja','-C',str(source/'build'),*limits],stdout=out,stderr=subprocess.STDOUT,check=True)
    print(f'Built {name} using {" ".join(limits)}',flush=True)
