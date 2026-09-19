# Circuit Reef


## 1.1.0 refinement

The aquarium now includes two sharks, two octopi with eight articulated arms, two rays, a turtle, and two seahorses, alongside the koi, puffer, and jellyfish. Continuous paths take the inhabitants beyond the glass; jellyfish propel themselves through the water and rising bubbles have translucent rims and highlights. Fullscreen now uses output-bound LayerShellQt windows and the existing vector triangle stream on OpenGL.

`--fullscreen` and `--screensaver` cover every connected output by default. `--screen N` selects one; `--list-screens` shows names, geometry, and scale. Wayland uses LayerShellQt output binding, with independent framebuffers, mixed-scale support, portrait framing, and live output removal/reconnection. `--capture-dir DIR` saves one native PNG per output. SDL remains the preview and offline-capture backend. Build from the suite tree with the adjacent `common/` directory, Qt 6 Gui, LayerShellQt, SDL2, Cairo, and OpenGL development packages installed.

## Kind of Quiet, for QindaQt

A procedural cyberpunk aquarium, written in C++20. This is a different screensaver from Qinda Patrol: no platforming, enemies, scoring, or gameplay. Mechanical koi glide through a submerged circuit garden, jellyfish breathe and drift, and a little QQ-eyed maintenance puffer inspects the reef.

The complete standalone application, original artwork generators, reusable asset exports, tests, and an optional Qt 6 Quick embedding adapter are included. Nothing needs a web service, downloaded art pack, or proprietary engine.

**Visual screensaver, not a security lock.** The existing desktop/compositor remains responsible for session locking, idle activation, suspend, and output power. See `docs/QINDAQT_INTEGRATION.md`.

The included `previews/Circuit_Reef_preview.mp4` is a 12-second, 60 FPS reference-renderer export with labeled demonstration counters. It is an animation preview, not a native performance benchmark.

## Build and run

Requirements: a C++20 compiler, CMake 3.20+, SDL **2** 2.0.18 or newer, Cairo with PNG/SVG support, and their development headers. Ninja is optional. The standalone application does not require Qt, GTK, Python, or a browser. The optional embedding adapter requires Qt 6.4+ Core, Gui, Quick, and Qml.

On Gentoo, the package names are:

```sh
sudo emerge --ask dev-build/cmake dev-build/ninja x11-libs/cairo media-libs/libsdl2
```

For a native Wayland session, build SDL2 with its `wayland` and suitable accelerated-renderer support enabled. `X` enables an X11 fallback. Do not blindly replace your system-wide USE flags. The Gentoo package references are recorded in `docs/SOURCES.md`.

From the extracted `circuit-reef` directory:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
./build/circuit-reef --windowed
```

Start fullscreen on every display:

```sh
./build/circuit-reef --fullscreen
```

Start on every connected display:

```sh
./build/circuit-reef --all-screens
```

A separate seeded aquarium is created per output. There is no stretched image spanning several monitors. Fullscreen output windows are added and removed live when displays change.

For a quiet, private setup:

```sh
./build/circuit-reef --all-screens --private --eco --palette amethyst
```

Use SDL's `SDL_VIDEODRIVER=wayland` or `SDL_VIDEODRIVER=x11` environment selection when diagnosing a platform backend. These are optional overrides, not requirements for normal launching.

### Installation

```sh
cmake --install build --prefix "$HOME/.local"
```

This installs the executable, icon, desktop launcher, and documentation under the selected prefix. Ensure `$HOME/.local/bin` is on your session PATH. The desktop file offers ordinary preview, fullscreen, private, and eco launch actions. Installation does **not** edit your idle policy, lock configuration, startup files, or repository.

### Runtime-only build fallback

CMake first looks for normal `pkg-config` development packages. For stripped-down Linux environments that have the libraries but not headers, the repository also includes a small set of ABI declarations under `compat/`. CMake explicitly warns when this fallback is used. The standalone target was compiled and tested in this mode on Linux x86-64. Normal distro development packages are preferred. SDL2 runtime versions older than 2.0.18 are unsupported.

## What the reef does

The scene contains ten large articulated koi by default, three little schools of glass minnows, four jellyfish, a QQ puffer, layered coral and ribbon kelp, submerged circuit towers, drifting particles, and instrument buoys. The random seed selects lane geometry, phases, sizes, plants, and ruins. There are three palettes:

| Palette | Appearance |
| --- | --- |
| `lagoon` | Deep teal, mint circuitry, pearl and copper fish |
| `amethyst` | Violet water, ice-blue light, lavender jellyfish |
| `ember` | Warm dark water, pale jade accents, peach and copper light |

There is no timer that swaps the scene for another wallpaper. Fish follow continuous, seeded three-dimensional paths. Background elements remain in the same world while independent animation periods keep the composition changing.

### Motion, not sprite swapping

Fish are deformed meshes with tapered cross-sections, a traveling lateral body wave, flexible tail lobes, and independently lagging fins. They turn through depth. Their silhouettes narrow naturally when turning toward or away from the viewer instead of instantly mirroring horizontally. Pitch follows the slope of their path without rotating a fish upside down.

Fin contours are smoothly sampled splines. Jellyfish bells contract continuously, and their attached tentacles use delayed traveling waves. The puffer's fins flap, its eyes look around, and a smooth eyelid function produces short blinks. Plants stay attached at their roots.

The application advances an animation clock using measured elapsed time, not one fixed increment per rendered frame. Long stalls and resume-from-suspend gaps are capped rather than fast-forwarding the aquarium by minutes. Toggling reduced motion in the native preview changes the clock rate without jumping to a different pose.

This is stylized animation, not a fluid-dynamics or fish-behavior research simulator. Paths are analytically generated rather than obstacle-avoiding flocking AI. Creatures can cross in projected screen space at different depths. The atlas files are reusable exports, not the source of runtime animation.

## Real system information

Two drifting buoys alternate between CPU/RAM and network/uptime displays. A small CPU-linked change in reservoir illumination is intentionally restrained: load does not make the animals panic or speed up.

The native application samples approximately once per second on a worker thread:

| Reading | Source and meaning |
| --- | --- |
| CPU | Delta of aggregate `/proc/stat` counters, excluding duplicated guest fields; idle plus iowait counted as idle |
| Memory | `(MemTotal - MemAvailable)` and `MemTotal` from `/proc/meminfo`, displayed in GiB |
| Network | RX/TX byte-counter deltas for one interface from `/proc/net/dev`, displayed as KiB/s |
| Uptime | `/proc/uptime` |

Network selection uses the lowest-metric active IPv4 default route when available. Without one, it chooses a stable, lexicographically first non-loopback interface. On IPv6-only/VPN/bridge setups, supply `--interface NAME` to select the intended interface. It never adds every interface together, which could double-count traffic through virtual links.

The first CPU/network interval and missing, invalid, reset, or stale readings display `--`. Unavailable readings are not replaced with invented values. No hostnames, process names, file contents, browser data, or network payloads are collected.

`--private` means no telemetry worker, no procfs metric reads, no metric panels. `--no-metrics` only hides the panels and retains the tiny local CPU-reactive effect. `--demo` is an explicit synthetic-data mode; the panels identify it as DEMO. Private and demo modes are mutually exclusive.

Headless snapshots and video exports never read your actual system counters. They omit readings unless `--demo` is explicitly supplied.

## Rendering and performance

The default native backend builds antialiased triangle batches and sends them to SDL's accelerated renderer. Background artwork and small text textures are cached. Positions, fins, tentacles, and other moving geometry continue to change each frame. Color/alpha gradients are carried by vertices; glow meshes do not need an external shader pack.

`--raster` uses the Cairo reference renderer instead. It is also the backend for PNG/SVG and headless video export. Both backends use the same C++ scene and articulation functions, with slightly different tessellation and antialiasing. Raster rendering can be substantially slower at high resolutions.

| Option | Behavior |
| --- | --- |
| `--fps N` | Presentation cap, 10 to 240; default 60, not a guarantee of achieved FPS |
| `--eco` | Caps at 30 FPS, uses at most seven large koi, caps drawing coordinates/background height at 720 |
| `--quality balanced` | Default 900-pixel height budget, with output scaling |
| `--quality high` | 1440-pixel height budget |
| `--quality native` | Native output dimensions, bounded to 8192 pixels per side |
| `--brightness 0.65` | Overall image dimming |
| `--reduced-motion` | Runs the ecosystem at 40% speed |
| `--density 6` | Chooses 3 to 24 large koi |

SDL output scaling preserves the viewport aspect ratio. World positions adapt horizontally while the animals themselves keep their proportions. The interface is primarily designed for landscape displays but portrait and ultrawide rendering are included in the tests.

The native app was exercised on an X11 virtual display using Mesa software rendering, **not a physical GPU benchmark**. Do not interpret the 60 FPS preview or configured cap as proof of 60 FPS on every desktop. The logs include achieved native presentation rates. See `docs/TESTING.md`.

## Controls

In an ordinary preview window, Escape closes, F toggles fullscreen presentation, M toggles the metric labels, and R toggles reduced motion. A fullscreen screensaver exits on keyboard/button/wheel/touch input, or on pointer movement exceeding a small jitter threshold, after a one-second launch grace period. Escape always exits. There are no game controls.

The process honors SIGINT/SIGTERM. Minimized/hidden windows pause scene presentation. The host should stop it when outputs are powered off or the session locks, because not every compositor exposes those changes as ordinary application visibility events.

## Reuse the assets

`assets/` contains PNG and SVG exports of three koi, two jellyfish, the puffer, four plant/coral forms, a background, and an application icon. There are three 32-frame koi sheets and two 24-frame jellyfish sheets. `assets/manifest.json` describes cell sizes and pivots.

Rebuild them from their C++ masters:

```sh
./build/circuit-reef --seed 2026 --export-assets assets
```

The master drawings live in `src/renderer.cpp`; articulation and world generation live in `src/world.cpp`. No font files are bundled. System fonts provide the small labels. SVG exports may contain normal Cairo-generated glyph outlines.

## Render a preview or still

```sh
./build/circuit-reef --snapshot reef.png --size 3840x2160 --seed 2026 --start 4 --demo
./tools/render-preview.sh preview.mp4
```

The shell helper needs `ffmpeg` and a built executable at `build/circuit-reef`. Override its location with `REEF_BIN=/path/to/circuit-reef`. The exported video uses 1280x720, 60 FPS, 24 seconds, seed 2026, and visibly labeled demo telemetry. Exported jelly sheets cover one bell pulse; their independently animated tentacles are not forced into an artificial identical endpoint.

## Qt 6 / QML embedding

A complete optional `CircuitReefItem` adapter and small preview executable are under `qt/`. Build with:

```sh
cmake -S . -B build-qt -G Ninja -DCMAKE_BUILD_TYPE=Release -DREEF_BUILD_QT=ON
ninja -C build-qt $(portageq envvar MAKEOPTS)
./build-qt/qt/circuit-reef-qt-preview
```

The adapter uses `QQuickPaintedItem` with immutable copied frames from the Cairo reference backend. It targets 30 FPS and a 720-pixel render-height cap, not the standalone app's accelerated triangle path. Properties expose active state, privacy, seed, palette, brightness, and reduced motion. It defaults to inactive and private until the host enables it.

**The optional Qt adapter has not been compiled in the delivery environment**, which has no Qt 6 development packages. The standalone SDL application and both of its rendering paths were compiled and executed. See the integration guide for a host embedding example and the exact testing boundary.

## Source map

- `src/world.cpp`: seeded lanes, world contents, three-dimensional poses, body waves.
- `src/renderer.cpp`: original artwork, environment, characters, buoys, export pipeline.
- `src/geometry.cpp`: antialiased GPU triangle generation, path flattening, text cache.
- `src/metrics.cpp`: procfs parsing and sampling worker.
- `src/main.cpp`: native windows, command line, output handling, privacy, input dismissal.
- `qt/`: optional Qt Quick embedding library and preview.
- `tests/`: deterministic motion, telemetry, geometry, and render checks.
- `integration/`: desktop launcher; no automatic system configuration changes.

Original project code and generated artwork are provided under GPL-3.0-or-later. Dependencies retain their own licenses. The normal GPL text is in `LICENSE`.
