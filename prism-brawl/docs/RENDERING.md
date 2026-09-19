# Captures, preview export and editable art

## Capture the actual renderer

A working GL-capable display context is required, including for hidden exports.
A headless simulation log and OBJ exports do not need a display.

```sh
./build/prism-brawl --seed 41 --start 17 --size 3840x2160 \
  --snapshot fight.png
./build/prism-brawl --showcase 1 --seed 41 --no-hud \
  --size 1200x1200 --snapshot ducke.png
./build/prism-brawl --stage garden --start 108 --seed 41 \
  --snapshot garden.png
./build/prism-brawl --battle-log 3600 --seed 41
```

`--start` simulates forward to the requested time; it is not an isolated artist's
pose override. Snapshots and videos therefore show actual battle states.

## Preview video

The packaged video contains three eight-second excerpts from the executable,
starting at simulation seconds 6, 108 and 213 with seed 41. They are three
separate clips of continuous simulated animation, joined with straight cuts.
It is an offline 1280x720, 30 fps export in eco mode, not a measurement of native
frame rate, not the complete three matches, and not a video used by the runtime.

With Python 3 and ffmpeg installed for development:

```sh
python3 tools/render_preview.py
```

The script uses a temporary FIFO to avoid keeping a multi-gigabyte RGB dump.
Set `DISPLAY` or the appropriate SDL backend for the intended GL-capable session.
The renderer also directly supports:

```sh
./build/prism-brawl --seed 41 --start 6 --size 1280x720 --eco \
  --frames 240 --fps 30 --raw frames.rgb
ffmpeg -f rawvideo -pixel_format rgb24 -video_size 1280x720 -framerate 30 \
  -i frames.rgb -an -c:v libx264 -crf 18 -pix_fmt yuv420p preview.mp4
```

## Assets

```sh
./build/prism-brawl --export-models assets/models --seed 41
```

The eleven OBJ/MTL sets are static rest-pose fighters and arena platform
assemblies. All geometry is original or reused from this conversation's Prism
Circuit C++ project. Editable procedural meshes are in `src/meshes.cpp`; rig
assembly and animation are in `src/models.cpp`; stage/environment construction
is in `src/director.cpp`. Material and lighting sources are in `shaders/`.

OBJ does not carry shader-driven deformation, runtime animation curves, skin
hierarchies, pickup behavior or the complete procedural skyline. Use the C++
sources to retain those features. No model loader or model download is required
when the screensaver runs.
