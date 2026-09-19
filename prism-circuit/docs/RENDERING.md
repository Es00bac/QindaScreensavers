# Renderer and deterministic exports

The native window and preview exports use the same C++ scene and OpenGL draw
path. A display/context is needed for images and video, but not for model export,
headless race logs, unit tests or stress tests.

## Captures

```sh
./build/prism-circuit --seed 41 --start 22 --camera front --driver 1 \
  --size 3840x2160 --snapshot race.png

./build/prism-circuit --seed 41 --camera overview --course eight \
  --start 14 --no-hud --size 1920x1080 --snapshot eight.png
```

The framebuffer size is bounded at 8192 per dimension and checked against the
GL texture limit. Normal windows use the drawable pixel size, not just logical
window dimensions, for high-DPI rendering.

## Video

`--frames N --raw FILE` writes tightly packed, top-down RGB24 frames at the
selected `--fps`. Simulation advances by an exact frame interval between
exports. It is not wall-clock paced, so it is appropriate for an offline preview
but not a benchmark of native frame rate.

```sh
./build/prism-circuit --seed 41 --start 12 --size 1280x720 \
  --fps 30 --frames 720 --raw /dev/stdout \
  | ffmpeg -f rawvideo -pixel_format rgb24 -video_size 1280x720 \
      -framerate 30 -i pipe:0 -an -c:v libx264 -crf 19 \
      -pix_fmt yuv420p -movflags +faststart preview.mp4
```

The included preview is a 24-second continuous excerpt, simulation time 12 to
36 seconds, at 1280x720 and 30 fps with seed 41. It shows the ordinary automatic
camera, including its first subject handoff. It is not a montage of stills,
video playback inside the app, or a hardware-performance claim.

`--benchmark N` draws unpaced and reports measured throughput for that run. The
creation environment used Mesa's llvmpipe software renderer, not a discrete GPU.
For performance diagnosis, `PRISM_PROFILE=1` reports render-stage wall times.

## Asset edits

Karts and animals are assembled from modeled parts with local transforms.
Geometry and material masters are in C++. Runtime motion is analytic or derived
from interpolated simulation state. There is no baked whole-body sprite cycle.
CMake embeds the shaders into the executable and tracks shader-file changes.
Editing a shader requires rebuilding the executable, not installing a loose file.

OBJ/MTL exports preserve positions, UVs, normals and approximate static material
colors/emission. They do not export skeletal animation, a rig, material graphs,
procedural text decals or a complete PBR exchange format. Edit the source to
change the live rig. The export code uses inverse-transpose normal transforms
for non-uniformly scaled component meshes.
