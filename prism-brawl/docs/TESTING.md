# Validation report

The newer full-body animation pass has its own [validation record](ANIMATION.md#validation--2026-09-19),
including release/sanitizer tests, 24 simulated hours, and AMD hardware renderer
captures. The original package validation below is retained as historical evidence.

## Tested build

Date: 2026-09-19. Linux x86-64, C++20 release build. The SDL2/Cairo development
headers were absent in this environment, so CMake used the included Linux ABI
declaration fallback against the installed system libraries. No library binaries
are redistributed. The normal pkg-config development-header route remains the
recommended distribution build route, but was not exercised here.

Graphical tests used a dedicated Xvfb X11 display and Mesa llvmpipe software
rendering. The driver reported OpenGL 4.5 core / Mesa 25.0.7 / LLVM 19.1.7. The
application requests desktop OpenGL 3.3 core. No synthetic input was sent to a
person's desktop.

## Automated simulation and model checks

The release unit executable completed **462,037 assertions**. The three CTest
entries (combat/motion/mesh checks, help output, invalid numeric-option rejection)
passed. The assertion count includes repeated bounded-state and mesh-index
checks; it does not mean hundreds of thousands of independently designed tests.

Coverage includes damage-scaled knockback, shield absorption, grab bypass,
dodge invulnerability, swept one-way landing, ring-out stock consumption,
knockout attribution, protected respawn, deterministic results across two
advance cadences, active combat, bounded effect/projectile/pickup pools, finite
positions and poses, stock/damage/guard bounds, and valid mesh indices.

The stress executable completed **48 configurations totaling 24 simulated
hours**, across automatic/fixed stage selections, 2/4/8 fighters and four seeds.
Final counters: **10,368,000 simulation ticks, 80,688 hits, 9,574 knockouts and
916 completed rounds**. It checks finite positions, bounds and continuing combat.
This is simulated time, not a 24-hour wall-clock GPU soak test.

```sh
ctest --test-dir build --output-on-failure
./build/brawl-tests
./build/brawl-stress
```

Logs: `validation/ctest.log`, `unit-tests.log`, `stress.json`.

## Sanitizers

An independent Debug build enabled AddressSanitizer and UndefinedBehaviorSanitizer.
The same 462,037-assertion unit run passed with leak checking enabled. A native
windowed render run also exited without sanitizer diagnostics. Leak checking was
disabled for that native run because it includes system graphics/font libraries;
it was not disabled for the core simulation/model unit run.

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DPRISM_SANITIZERS=ON
ninja -C build-asan $(portageq envvar MAKEOPTS)
ASAN_OPTIONS=detect_leaks=1 ./build-asan/brawl-tests
ASAN_OPTIONS=detect_leaks=0 ./build-asan/prism-brawl \
  --windowed --size 800x450 --seed 41 --start 45 --quit-after 2 --no-msaa
```

Logs: `validation/asan-tests.log`, `asan-native.log`.

## Native lifecycle checks

On the dedicated X11 display, the final executable passed window creation,
resize to 960x540, hide/pause, remap/resume, Escape dismissal, fullscreen-mode
pointer-activity dismissal after the launch grace period, and SIGTERM shutdown.
The pointer test translated coordinates into the actual SDL window. Bare Xvfb
has no normal desktop window manager, so that test validates input handling in
fullscreen mode, not QindaQt's final fullscreen/focus policy.

All tested exits returned status 0. Logs include `native-smoke.json`,
`native-window-events.log`, `native-fullscreen-dismiss.log` and
`native-sigterm.log`. `PRISM_BRAWL_TRACE=1` enables lifecycle trace messages.

## Visual exports

All eight character showcases, all three stages and the eight-fighter option
were rendered with the actual executable. A native 3840x2160 capture was also
rendered. Eleven OBJ/MTL export sets were checked for finite positions, valid
indices and existing material references. These are static exports; continuous
poses, GLSL effects and the full backgrounds remain in the source.

The preview contains **720 frames, 1280x720, 30 fps, 24 seconds**, H.264 with no
audio. It joins three eight-second combat excerpts, starting at simulation times
6, 108 and 213 seconds with seed 41. It is an offline actual-renderer export,
not a complete match and not a measurement of native frame rate. Character and
arena overview PNGs are labeled collages of native captures, not generated
concept images. See `validation/preview-ffprobe.json` and `video-render.log`.

## Package/install check

The source archive was extracted into a separate directory. A clean release
configuration, build, all three CTest entries and installation to an isolated
prefix passed. The installed executable rendered a 960x540 PNG while launched
from `/tmp`, outside both source trees. This verifies that the shader embedding
and installation do not depend on the original working directory. All archive
member hashes checked successfully. The compiled input files were compared with
the packaged source byte for byte. Results are in `validation/package-check.log`
and `validation/package-verification.json`. No user desktop configuration or
system-wide installation was modified by these checks.

## Not verified or not implemented

* Physical-GPU performance, native Wayland, physical multi-monitor behavior,
  compositor-specific focus/fullscreen policy, and the normal development-header
  compilation route were not verified here. Frame caps are not speed guarantees.
* QindaQt idle activation, output power, suspend, and secure locking are host
  responsibilities. This package is not an idle daemon, Qt Quick plugin,
  layer-shell surface, XScreenSaver root-window hack, or secure locker.
* Combat is an arcade two-dimensional simulation with 3D rendering, not full
  rigid-body/ragdoll physics or a playable replica of a commercial fighter.
* A 120 Hz simulation and interpolation do not guarantee that every driver or
  output will present perfectly paced frames. Check the native preview on the
  actual desktop before enabling the visual-idle hook.
