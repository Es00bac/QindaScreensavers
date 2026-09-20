# Validation — Prism Circuit 1.2

Date: 2026-09-19. These results describe the updated local workspace, not the
older published 1.0/1.1 package. Historical logs elsewhere in `validation/`
are retained and should not be confused with this run.

## Environment

Linux x86-64, GCC 15.3, Qt 6.11.1, SDL2 2.32.8, Cairo 1.18.4 and Mesa 26.1.8.
Native captures used an AMD Radeon integrated Renoir GPU, OpenGL 4.6 core and
SDL's X11 backend. CMake/Ninja produced the build and test binaries.

## Automated checks

The five CTest checks pass: race/motion/mesh; combat/recovery/cameras;
level grounding and clearance; help; and invalid-option rejection.

The race/motion/mesh test reports **618,844 assertions**. The combat test
simulates ten minutes per course, exercises all seven items, all eight camera
subjects, spinouts, takedowns, rescues, jumping/landing, deterministic seeking,
camera hold time, finite transforms, driver size differences and steering grips.
The seed-41 run records 572 hits, 378 knockouts, 369 completed rescues,
238 trap hits, 97 blocks, and 56 jumps with 56 landings.

Scenery checks visit sixteen positions on each of five courses with three seeds.
The latest run validates **84,163 reserved solids**, rejects deliberately
intersecting placements, verifies facade parts remain inside their reserved
silhouettes, and independently finds four city crossings. Minimum centerline
vertical separation is **25.8127 m**. This is sampled regression coverage,
not a proof over every seed, camera position or possible modification.

The seeded stress test passes sixty course/seed combinations, ten minutes each:
**10 simulated hours**, **4,320,000 fixed ticks**, **27,360,000 checks**.
It records 17,432 overtakes, 13,721 boosts and 215 completed race resets.
Maximum non-reset displacement at 60 Hz is 0.90987 m; maximum speed is
37.72986 m/s. Tests check continuity, finite/bounded state and separation outside
explicit spinout/airborne/rejoin states.

Development checks caught two genuine visual problems: unstable bank estimates
and a discontinuity in nearest-road terrain grading. The bank now uses a wider,
smoothed derivative stencil; the landscape blends nearby route samples
continuously. Falling racers use that same terrain surface.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer builds passed the simulation,
combat/camera and scenery tests with leak detection enabled. LeakSanitizer
cannot inspect threads under this workspace sandbox's tracing restrictions,
so those tests were run outside the sandbox after approval. No desktop input
was injected. These are CPU tests, not a sanitizer guarantee for the GPU driver.

## Actual renderer captures

The new files are in `previews/refinement-1.2/`:

- `city-race.mp4`: 24 s, 720 frames, simulation time 12–36, automatic director.
- `tow-bot-recovery.mp4`: 10 s, 300 frames, time 17–27, full takedown/recovery.
- `bliss-race.mp4`: 12 s, 360 frames, time 12–24, Ducké chase view.
- PNGs show the city pack, jump, cockpit, full layouts, Bliss terminal,
  Tow-Bot and Patches's worn kart.

All videos are 1280×720, 30 fps H.264 with no audio; FFprobe confirmed dimensions,
durations and frame counts. Renderer/encoder processes exited successfully.
Frames sampled through the action and camera changes were visually inspected.
The videos are direct native rendering, not generated concept images or
montages of stills. Only the material/facade bitmap atlases use image generation.

Separate unpaced 120-frame benchmarks at 1280×720, seed 41, start time 16,
default 4× MSAA measured **40.966 fps city** and **89.261 fps Bliss**.
These short, view-dependent measurements apply only to this machine/run.
Offline video export throughput is not a native frame-rate measurement.
The 60 fps option is a cap, not a promise.

Fifteen regenerated OBJ/MTL sets have finite coordinates and valid positive
vertex/UV/normal triangle references. Their current counts and bounds are in
`assets/models/manifest.json`; they are static models, not complete animated levels.

## Reproduce

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/prism-stress
./build/prism-circuit --course city --seed 41 --race-log 600
./build/prism-circuit --course city --seed 41 --start 16 --size 1280x720 --benchmark 120 --mute
cmake -S . -B build/sanitizers -DCMAKE_BUILD_TYPE=Debug -DPRISM_SANITIZERS=ON
cmake --build build/sanitizers
ASAN_OPTIONS=detect_leaks=1 ./build/sanitizers/prism-tests
ASAN_OPTIONS=detect_leaks=1 ./build/sanitizers/prism-combat-tests
ASAN_OPTIONS=detect_leaks=1 ./build/sanitizers/prism-scenery-tests
```

## Not revalidated in this refinement

Physical multi-monitor hotplug, native Wayland presentation, desktop idle/DPMS
integration, secure locking, resize/hide/input lifecycle automation, mixed-DPI
hardware and audio-device behavior were not re-exercised for this update.
Their existing suite implementation remains in place. There is no playable
mode, general rigid-body solver, online service, telemetry or bundled idle daemon.

No package publication, remote push or system-wide installation was performed.
