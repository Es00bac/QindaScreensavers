# Rendering and artwork implementation

## Pipeline

1. Evaluate the deterministic director and hierarchically composed model parts.
2. Upload one bounded instance list per base mesh. Instance attributes carry the
   transform, albedo/emission, roughness, metallic weight and surface category.
3. Draw a 2048-square directional shadow map.
4. Draw the procedural star/nebula background and ray/sphere planet surface.
5. Draw depth-tested 3D geometry into an HDR color target. Normal transformation
   respects nonuniform model scale. The sail and scarf meshes deform in the vertex
   shader using the same reduced-motion clock as the CPU scene.
6. Resolve 4x MSAA, or 2x in eco mode, into the single-sample HDR/depth textures.
7. Extract and blur bright emission in one-third-size ping-pong targets.
8. Apply compact edge-aware filtering, bloom, a filmic tone curve, display gamma,
   restrained vignette, optional captions/readout and scene fade.

This is a forward raster renderer. There is no ray-traced lighting, screen-space
reflection, shadowed area lighting, SSAO or full material importer. The sky's planet
intersection is analytic, not a ray-traced world. No static concept art is used in
place of the 3D scene.

## Motion contract

Animation is evaluated at continuous elapsed time. The scene is not stepped through
a finite animation atlas. Preview export uses exact frame times. Native rendering
uses a monotonic clock, pauses while every window is hidden/minimized, and limits
a long resume/stall step to avoid a sudden large jump.

Head movement, body breathing, banking and delayed formation follow continuous
functions. Blinks use a smooth raised-sine lid closure. Story handoffs use quintic
easing. The shader-driven scarf, sails and filaments are coherent spatial waves,
not independent random offsets. Motion remains staged rather than physics-based.

After a long GPU stall the safety clamp slows progression instead of jumping ahead.
That avoids a large discontinuity but is another reason to use a hardware driver.
A software renderer is useful for deterministic validation, not smooth daily use.

## Assets

`src/meshes.cpp` owns base geometry, including rounded plates, a section-profiled
fuselage, swept fins, directional side armor, cratered rocks, alien shards and sails.
`src/models.cpp` assembles those into the complete vehicles and environments.
`src/renderer.cpp` paints the editable stencil atlas in memory with Cairo and uploads
it as a mipmapped texture. It uses the host's sans-serif font. No font files ship.

The OBJ exports contain static vertex positions, faces and simple material values.
They are not imported at runtime and do not encode the transform hierarchy, lights,
shader material effects, font stencil atlas or continuous deformation. The C++ and
GLSL sources retain all of those features.

## Frame export

A GL-capable X11 or Wayland context is required, even when exporting to a hidden
window. A virtual X11 display with Mesa software GL can be used for CI. The app
never reads real system metrics during export.

```sh
# This writes RGB24 frames, top row first. An existing parent directory is required.
./build/starward --chapter asteroids --start 16 --seed 41 \
  --size 1280x720 --fps 30 --frames 150 --raw asteroid.rgb --no-titles

ffmpeg -f rawvideo -pixel_format rgb24 -video_size 1280x720 -framerate 30 \
  -i asteroid.rgb -c:v libx264 -crf 18 -pix_fmt yuv420p asteroid.mp4
```

Raw output can be a named FIFO to avoid storing large intermediate files. The
packaged montage concatenates seven such five-second exports without changing the
rendered scene pixels. A montage cut is not a demonstration of the normal voyage's
dark-space transition; run `--tour` to inspect those transitions directly.

## Profiling

```sh
./build/starward --windowed --size 1280x720 --benchmark 120 --no-titles
STARWARD_PROFILE=1 ./build/starward --windowed --benchmark 3 --no-titles
```

The optional environment switch inserts GL completion points between passes and
prints per-pass timings. That synchronization changes performance and is for
bottleneck inspection only. Normal runs do not insert these per-pass completion
points. Do not treat the offline video's encoded frame rate as a native benchmark.
