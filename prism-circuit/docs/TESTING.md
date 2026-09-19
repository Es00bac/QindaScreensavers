# Validation report

Date: 2026-09-19. Source version: Prism Circuit 1.0.0.

## Environment and scope

Linux x86-64, GNU C++ 14.2, CMake, SDL2/Cairo runtime-library ABI declarations,
Xvfb virtual X11 display, Mesa 25.0.7 llvmpipe (LLVM 19.1.7, 256-bit).
The driver exposes an OpenGL 4.5 core context; the application requests 3.3 core.
Normal distro development headers were not installed in this creation environment.

The native app genuinely created windows, compiled its GLSL shaders, rendered
its models and responded to events. This was **software rendering**, not a test
of a physical graphics card, native Wayland, or physical multi-monitor operation.
Those deployment paths remain unverified. No user desktop input was injected;
native event tests used only the dedicated virtual X11 display and PID-matched
application windows.

## Unit and regression tests

The latest release run passed **454,797 assertions**. Coverage includes track
closure, negative-distance wrapping, track frame orthogonality, bounded banking,
fixed-step determinism under different presentation increments, finite state,
road bounds, approximate fender-box separation, boost-pad activation, overtakes,
race completion/reset, triangle indices, instance transforms and safe camera range.

The standard CTest suite passed 3/3: race/motion/mesh tests, help output, and
rejection of an invalid numeric option. Logs are in `validation/`.

A lane-convergence defect found during development could move a rear kart
backward abruptly. The final resolver uses the smaller penetration axis:
side-by-side contacts are resolved laterally; longitudinal corrections brake
the following car. A motion-displacement regression check is included in the
long simulation test. The failing development build is not the packaged build.

## Six-hour seeded stress run

The final build completed 36 course/seed combinations, each simulated for ten
minutes: three courses times twelve seeds, **six simulation hours total**.
The run processed 2,592,000 fixed ticks and 16,416,000 validation checks.

Recorded results:

| Measure | Result |
|---|---:|
| Pairwise race-order overtakes | 15,014 |
| Boost-pad activations | 8,144 |
| Completed race resets | 144 |
| Small contact corrections | 1,077 |
| Largest measured displacement at 60 Hz | 0.568908393 m |
| Maximum absolute lateral lane position | 4.09998941 m |
| Maximum speed observed | 34.1356125 m/s |

This test checks finite and bounded state, travel continuity between non-reset
samples, and approximate body separation. It is not a formal proof for every
possible seed. Contact corrections are expected arcade collision responses,
not missed-landing or off-track recoveries. Counters and checks are reproduced
by `prism-stress`; exact results are in `validation/stress.json`.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer passed the complete latest unit
suite with leak detection enabled. A separate sanitized native window smoke test
also exited successfully. Leak detection was disabled only for that GL/driver
smoke test because driver-owned allocations are outside the application test's
ownership boundary. See `asan-tests.log` and `asan-native.log`.

## Native lifecycle and input tests

All passed, with exit code 0:

- Windowed launch, resize from 640x360 to 900x600, hide, resume, and Escape.
- Hidden-window pause: 0.0 process CPU seconds over the measured two-second
  hidden interval, after allowing the last submitted frame to finish.
- Fullscreen launch and pointer-activity dismissal after the startup grace period.
- SIGTERM shutdown of the native window.

The app uses an elapsed-time cap after a stall. On a slow renderer it can play
more slowly than wall time rather than allowing unlimited simulation catch-up.
This is documented behavior, not a native performance guarantee.

## Visual and export checks

All three course overview frames and eight character turntables were captured
through the running OpenGL renderer and inspected. Full-HD and 3840x2160 race
captures were also produced. No concept-image generator was used for the preview.

The video is a continuous **24-second**, **1280x720**, **30 fps** H.264 export:
720 frames from simulation time 12 through the next 24 seconds. Renderer and
encoder both exited 0. This contains normal continuous racing and the automatic
camera's first racer handoff, not a full race or three-course montage.

Export took approximately 313 wall-clock seconds on the software renderer.
Therefore its encoded frame rate is **not** evidence of 30 fps native performance.
A separate 12-frame 1280x720 eco-mode llvmpipe benchmark reported 2.633 fps; do not
extrapolate that software result into a promise about a physical GPU.

Ten OBJ/MTL model sets passed finite-coordinate, triangle-index, UV/normal-index
and material-reference checks. They contain 518,554 exported vertices and
833,352 triangles in aggregate. Those totals are static exports, not a native
per-frame triangle count. Instance culling/composition varies at runtime.

## Reproduce

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
./build/prism-stress
./build/prism-circuit --race-log 600 --seed 41
./build/prism-circuit --windowed --size 1280x720 --benchmark 120 --eco
```

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DPRISM_SANITIZERS=ON
ninja -C build-asan $(portageq envvar MAKEOPTS)
ASAN_OPTIONS=detect_leaks=1 ./build-asan/prism-tests
```

## Package rebuild

The source archive is also extracted into a clean directory for a fresh build,
CTest run, installation, and a screenshot made with the installed executable.
The results of that check are recorded separately in
`validation/package-verification.json` and `validation/package-verification.log`.

## Explicitly unverified or out of scope

Native Wayland; a physical GPU and its frame pacing; multiple physical displays;
compositor-specific idle/DPMS hooks; Qt Quick embedding; secure session-lock
surfaces; touchscreen input on real hardware; and visual comfort for every user.
There is no playable mode, audio, general vehicle rigid-body simulation, network
functionality, telemetry collector or bundled idle daemon.
