# Validation report — 2026-09-18

## Scope

This report distinguishes executed code from provided-but-unqualified Qt source.
The delivered stills, video, sprite atlases and tiles were produced by the actual
C++ simulation / raster renderer included in this project. They are not concept
mockups of a different implementation. All distributed scene previews explicitly
use **DEMO** metrics; none contains readings from Jarrod's computer.

## Executed successfully

Environment: Linux container, Debian 13, GCC 14.2.0, CMake and Ninja. Build mode:
C++20 Release, `PATROL_BUILD_DESKTOP=OFF`, tests enabled. Qt is not needed by these
targets.

| Check | Result and scope |
|---|---|
| Core + asset generator + headless renderer build | Passed with GCC 14.2.0 |
| CTest, Release | Passed |
| Seeded traversal | 100 seeds, five simulated minutes each; progress, finite coordinates, successful combat, bounds and zero rescue teleports |
| Long-running world | Separate 40-minute simulated run; coordinate rebase exercised; zero rescue teleports |
| Determinism | Two worlds with the same seed checked over 12,000 fixed steps |
| Reachability | All shipped gap / adjacent-height combinations checked against the jump planner |
| Resource bounds | Retained terrain/enemies, projectiles and particles checked during simulation |
| Metric parsing | Aggregate CPU deltas, guest exclusion, decreasing counters, malformed numbers, MemAvailable semantics, per-interface RX/TX extraction and route choice |
| Raster | Clipping, PNG export, final-frame opaque-alpha checks |
| Actual local telemetry | Single live still sampled from the container's own `/proc`; not from the user's system |
| Sanitizers | Debug build with AddressSanitizer + UndefinedBehaviorSanitizer; entire test executable passed with leak detection enabled |
| Art export | PNG sprite/tile atlases, per-biome sky/skyline layers and JSON manifest exported by C++ |
| Preview animation | Four eight-second excerpts, 24 fps, rendered by C++ and encoded as a 32-second H.264 MP4 with FFmpeg |

The test executable reported:

```text
PASS: 554735 checks; 100 seeds x 5 minutes + 40-minute rebase simulation; metric parsers and renderer.
```

This is a count of individual assertions, including per-pixel alpha checks—not
554,735 independent scenarios. The seeded and rebase tests together cover nine
hours of **simulated** world time, not nine hours of desktop wall-clock operation.

## Performance observation, not a desktop guarantee

A Release benchmark measured approximately **4.3–4.6 ms per 960 × 540 frame**, each
measurement also including four fixed simulation steps, averaged over 200 frames
in this container. This excludes Qt presentation, compositor work, high-DPI
window scaling, asset construction, PNG encoding and the live telemetry worker.
It is not a benchmark of the user's hardware. Actual CPU use should be measured
on the target desktop, especially with multiple outputs.

The art uses a bounded set of cached sprites and background layers. The runtime
is not claimed to be cheaper than a blank or powered-off display. Normal screen-
off and sleep policy should remain enabled.

## Not executed here

**Qt 6 development packages were unavailable in this delivery environment. The
Qt window executable and optional Qt Quick adapter were not compiled or run.**
The complete wrapper sources are supplied, but they are not qualified binaries.

The following therefore remain target-session acceptance work:

- Building `qinda-patrol` and `qinda-patrol-quick` against the installed Qt stack.
- QindaQt / KWin Wayland window placement, focus, input dismissal and lifecycle.
- Multi-output creation, hot-unplug / replug, mixed scaling and output power-off.
- Resume, repeated preview launches and interaction with the actual idle manager.
- Embedding in a trusted locker without altering its security or authentication.
- Profiling the live Qt window's CPU and memory use on the target hardware.

The included CI workflow describes a Qt build and offscreen smoke test. It was
**not executed remotely** during this delivery. Offscreen CI, even after it runs,
will not qualify native Wayland or lock-screen behavior.

No files were committed to QindaQt, no desktop settings were changed, and no
screensaver/idle API was invented or assumed to exist in the inspected repository.

## Reproduce the executed checks

```sh
cmake -S . -B build/headless -G Ninja \
  -DPATROL_BUILD_DESKTOP=OFF -DCMAKE_BUILD_TYPE=Release
ninja -C build/headless $(portageq envvar MAKEOPTS)
ctest --test-dir build/headless --output-on-failure
./build/headless/qinda-patrol-render --benchmark

cmake -S . -B build/sanitized -G Ninja \
  -DPATROL_BUILD_DESKTOP=OFF -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
ninja -C build/sanitized $(portageq envvar MAKEOPTS)
ASAN_OPTIONS=detect_leaks=1 ./build/sanitized/patrol-tests
```

## Preview provenance

Seed: `551767`. Stills: simulation times 2, 29, 40, 70 and 100 seconds; the
29-second still is the first district line. Video: excerpts starting at 24, 52,
80 and 108 seconds, eight seconds each, concatenated in district order so that
each excerpt approaches and crosses one district line. These are a montage, not
a claim of a single continuous 32-second run across all four districts. There
are no gameplay controls or manually staged poses in these captures.

The renderer exports RGBA PNG files with uncompressed DEFLATE blocks; distributed
copies were losslessly recompressed. Decoded pixels were compared before and
after recompression. The atlas overview is a contact sheet composed from the
exported atlases by a small Pillow script; it is not hand-edited.

## Addendum — 2026-09-18, desktop build and world revision

Environment: Gentoo Linux, GCC {gcc}, CMake {cm}, Ninja, Qt {qt}, running a
QindaQt Wayland session.

Before any change, the unmodified source built only after normalizing file
timestamps: every file in the archive carried a modification time several hours
in the future, so Ninja re-ran CMake 100 times and stopped with `manifest
'build.ninja' still dirty`. Touching the tree fixed it; no source was changed
for that.

The world and renderer were then revised: faster traversal (152 px/s running,
94 px/s while fighting, shorter jump durations), 1–3-tile gaps and up to two-tile
height steps, distance-based districts with gate roofs, per-building district
blending on world-anchored procedural skylines, per-pixel sky crossfades, open
catwalks, a street layer, billboards, railings, cables, vents, five new props,
district-specific wall tiles and district-weighted enemy species. The test
matrix gained the wider gap/height reachability grid, a 300-second generator
audit (no roof straddles a district line; gate roofs are wide, unsigned
buildings on their line; every roof's biome matches its district; gates and
catwalks occur) and a render inside a district blend.

| Check | Result |
|---|---|
| Core, headless renderer, Qt 6 window executable, Qt Quick adapter | All built; `-Wall -Wextra -Wpedantic` clean |
| CTest, Release | Passed: `{test}` |
| Headless benchmark | `{bench}` |
| Qt window, `--preview --duration 6` | Opened and exited 0 on the Wayland session |
| Qt window, `--screensaver` | Ran fullscreen with live metrics; dismissed by input; exit 0 |
| Two outputs (eDP-1 1920x1080 at 0,0 and HDMI-A-1 1920x1080 at 1920,0) | See the multi-output section below |
| Previews and atlases | Regenerated from the revised renderer; PNGs recompressed with pixels verified unchanged |

### Multi-output placement on QindaQt

With two outputs connected, the original `--all-screens` path put both windows
on the laptop panel and left the HDMI output showing the desktop; the dock was
also drawn over the scene. `--screen 1` alone landed on the laptop panel too.
A `WAYLAND_DEBUG=1` trace showed why: `qindaqt-wm` maps every xdg-shell
fullscreen window on the active output regardless of the output passed in the
request, and an ordinary toplevel has no say over panel stacking.

The screensaver now uses wlr-layer-shell through LayerShellQt 6.6.6 on Wayland:
one overlay-layer surface per output, anchored to all four edges, exclusive
zone -1, exclusive keyboard interactivity, bound to its output with
`LayerShellQt::Window::setScreen`. (The deprecated `ScreenFromQWindow` mode
reads `QWindow::screen()`, which Qt Wayland reports as the first screen until
the compositor's `enter` event arrives, so it bound both surfaces to eDP-1; the
explicit form is required.) The trace then showed
`get_layer_surface(..., wl_output#23, ...)` and `get_layer_surface(...,
wl_output#30, ...)` with matching `enter` events, `--verbose` reported window 1
exposed at 1920,0 on HDMI-A-1, and a full-desktop capture taken during the run
showed both outputs covered with no dock visible. `--screensaver` now covers
every output by default; `--screen N` restricts it to one.

Still not exercised here: output hot-plug while running, idle-manager
integration, suspend/resume and any locker embedding.
