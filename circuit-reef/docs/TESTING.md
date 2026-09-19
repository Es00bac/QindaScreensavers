# Delivery validation

Validated September 19, 2026, in a Debian 13 Linux x86-64 container, using GCC 14.2, CMake, Cairo 1.18.4 and SDL2 2.32.4. The runtime-only ABI fallback was used because development headers were absent. Distro development packages remain the preferred build route.

## Compiled and executed

- Release `reef-core`, `circuit-reef`, and `reef-tests` targets.
- Native SDL window, accelerated triangle-submission path, on an X11 virtual display through Mesa software rendering.
- SDL dummy/software smoke test and headless Cairo reference rendering.
- Escape dismissal in private fullscreen and live-telemetry raster windows, using injected input only inside a private Xvfb display.
- AddressSanitizer plus UndefinedBehaviorSanitizer builds and the same four CTest cases.
- PNG/SVG masters and animation-sheet export.
- A 12-second, 960x540, 60 FPS H.264 preview rendered by the actual C++ reference renderer. All 720 frames decoded without errors. The movie uses explicitly labeled DEMO counters, not machine telemetry.
- A 3840x2160 reference screenshot and snapshots for all three palettes.

The final release CTest run passed 4/4 cases in 1.88 seconds. The sanitizer run passed 4/4 in 14.82 seconds. These elapsed times describe this environment, not a performance target.

## Scope of automated checks

The core test executable reports **31,446,134 checks passed**. Most checks are inexpensive finite-vertex assertions, not millions of independent scenarios. The more meaningful coverage is:

- 48 seeds, ten koi per seed, one-hour path sweeps sampled every five seconds, with nearby 1 ms samples for derivative checks. This is not 48 hours of continuously rendered or real-time simulation.
- Reproducibility, finite poses, aquarium bounds, bounded swimming speed, continuous turning, and bounded path acceleration.
- Periodic body deformation and finite mesh geometry.
- CPU parsing, duplicate-guest exclusion, counter resets, impossible deltas, memory availability, network columns, and default-route selection.
- Three palettes across landscape, 4:3, portrait, and ultrawide reference render sizes, including opaque output checks.
- 80 native geometry-frame samples, valid triangle batch ranges, bounded text caches, and finite vertex coordinates. The observed maximum was **370,275 vertices** for these samples at the default density. This is not a maximum for every supported density or scene.
- Rejection of invalid render sizes, NaN animation time, and malformed command-line values.

Sanitizers were run with `ASAN_OPTIONS=detect_leaks=0` and `UBSAN_OPTIONS=halt_on_error=1`. Address and undefined-behavior checks passed. **Leak detection was disabled**, so this report does not claim a LeakSanitizer pass.

## Native presentation observation

A final 1280x720 X11 window trial with the default triangle renderer presented 76 frames in 3.067 seconds, approximately **24.78 FPS**, using Mesa software rendering. This verifies that the window and drawing path run. It is not a physical-GPU benchmark. The default 60 FPS setting is a presentation cap, not a promised achieved frame rate. Offline export can be 60 FPS without rendering in real time.

Both native and reference paths share scene and articulation code. Their tessellation, gradients, and antialiasing are not pixel-identical. `previews/native-x11.png` is a captured native frame; the supplied video uses the reference backend.

## Not verified here

- The optional Qt 6 Quick adapter and its preview executable, because Qt 6 development packages were unavailable.
- Wayland presentation and QindaQt compositor-specific lifecycle integration.
- Physical multi-monitor behavior, monitor hotplug, suspend/resume, and output-power policies.
- Real hardware GPU performance, power draw, and long-running graphical-driver behavior.
- Secure session locking. This software is deliberately not a locker.

## Reproduce the tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure

cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DREEF_SANITIZERS=ON
ninja -C build-asan $(portageq envvar MAKEOPTS)
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build-asan --output-on-failure
```

Before integrating, build with your system's development headers, inspect a complete fish turn, test Escape and input dismissal, review one-minute telemetry behavior, and check that your host stops the scene when the session locks or outputs turn off. Do not replace an existing secure lock policy with this visual.
