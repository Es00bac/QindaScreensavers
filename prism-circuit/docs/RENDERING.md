# Renderer and exports — 1.2

Native previews and offline captures share the C++ scene and OpenGL renderer.
A display/context is needed for PNG/video, but not model export or simulation tests.

```sh
./build/prism-circuit --course city --seed 41 --start 28.9 --driver 1 --camera front --size 1920x1080 --snapshot jump.png
./build/prism-circuit --course bliss --seed 41 --start 18 --camera chase --driver 1 --snapshot bliss.png
```

`--frames N --raw FILE` exports tightly packed, top-down RGB24 frames at
`--fps`. Simulation advances by exact frame intervals; export is not paced
to wall time. The source is live geometry, not movie playback.

```sh
set -o pipefail
./build/prism-circuit --course city --seed 41 --start 12 --size 1280x720 --fps 30 --frames 720 --raw /dev/stdout --mute | ffmpeg -f rawvideo -pixel_format rgb24 -video_size 1280x720 -framerate 30 -i pipe:0 -an -c:v libx264 -crf 20 -pix_fmt yuv420p -movflags +faststart city.mp4
```

[Current previews](../previews/refinement-1.2/README.md) include a 24-second city
sequence, a 10-second recovery and a 12-second Bliss drive. Older previews are
retained for comparison and do not represent the current build.

For throughput, use `--benchmark 120`; `PRISM_PROFILE=1` reports stage timing.
`--eco` reduces MSAA, bloom and the presentation cap. Actual GPU measurements
and limits are in [TESTING.md](TESTING.md).

Geometry, materials and animation masters live in C++. Atlas PNGs and GLSL
are embedded by CMake/Qt resources; installed runs need no source-tree assets.
Changing these requires rebuilding. Facades use physical-scale wall coordinates
rather than stretching/rotating the cube's generic UVs. Wear uses a separate
per-kart scalar. Fast wheels converge toward averaged spoke/tread shading.

Smoke uses a sorted transparent pass with depth writes disabled. Opaque objects,
including all track/building solids, remain depth-tested. Falling motion,
projectiles and the robot's cable follow simulation/world-space positions.

OBJ exports preserve static geometry, UVs, normals and approximate materials.
They do not preserve rigs, continuous animations, generated atlas sampling,
text decals, shader effects, particles, terrain or complete level assembly.
Use the C++/GLSL sources for exact runtime behavior.
