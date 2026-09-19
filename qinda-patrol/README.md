# Qinda Patrol


## 0.2.0 refinement

Four guardians now use shuffled, nonrepeating cycles and dedicated sealed reactor halls. Each encounter alternates a visible warning, a dodgeable attack, and an exposed core; low health accelerates the cycle. Restoring generators, greenhouse irrigation, coolant pumps, and antenna links changes the machinery and yields rapid fire, shields, or arc blasters. Cranes, cargo, vines, glass canopies, coolant pipes, and frost distinguish the four districts. Autonomous traversal remains bounded and rebase-safe.


**Kind of cute. Very much on patrol.**

An autonomous, side-scrolling, cyberpunk platformer **screensaver**, written in
C++20, with a Qt 6 desktop launcher and an optional C++ Qt Quick embedding
adapter. There are no gameplay controls, no player deaths, and no machine
learning service. The penguin and duck are the show.

The visual designs preserve the supplied QindaPunk reference: a chunky penguin
with a mint cyber-eye, headset, orange boots, dark armor, orange-lined cape and
coffee mug; a yellow duck with sunglasses, mechanical armor and blue thrusters.
These are new, compact pixel-art adaptations, not extracted frames of the
original painted wallpaper.

![Actual C++ render, with labeled demo metrics](docs/previews/patrol-40s.png)

![Crossing a district line: the gateway arch, with the sky and skyline already blending](docs/previews/patrol-gate-29s.png)

[Watch the 32-second renderer preview](docs/previews/patrol-preview.mp4) ·
[Inspect the complete sprite/tile atlas overview](docs/previews/assets-overview.png)

The video is a montage of four eight-second excerpts, one per district line,
each showing the approach, the gateway and the blend into the next district.
Its statistics are visibly labeled demo values; the desktop launcher defaults
to real read-only metrics.

## What is implemented

- **Autonomous traversal and combat.** The penguin runs at a brisk pace, slows
  only to fight, shoots, and follows explicitly solved ballistic jumps across
  one- to three-tile gaps and up to two-tile climbs; the flying duck follows,
  bobs and chooses targets independently. Bugs, memory-leak slimes, zombie
  robots and runaway drones take hits and dissolve into sparks; each district
  favors its own species, and a roof carries zero, one or two of them. Periodic
  larger elite enemies and short coffee breaks change the rhythm. The heroes
  cannot lose or get stuck in a game-over state. This is choreography, not a
  playable game.
- **An indefinitely streamed tiled world.** Four districts cycle: Neon Rooftops,
  Memory Gardens, Cooling District and Packet Docks. Each district is 4096 world
  pixels long and opens with a lit gateway arch on its first roof. The sky, both
  skyline layers, the street below and the weather blend gradually across the
  768 pixels on either side of a district line instead of switching at a roof
  edge, and the skylines are procedural and anchored to world distance, so the
  next district's colors and rooftop silhouettes show on the horizon before the
  penguin arrives. Roofs are buildings or open catwalks over a visible street
  level; lengths (6–18 tiles), heights (five levels), gaps, billboards,
  railings, service cables, vents and district-specific props and wall tiles
  vary with a seed. Each roof is assembled from 32-pixel tiles. Old chunks and
  effects are discarded; coordinates are rebased during long runs. Jumps are
  planned from the actual roof geometry.
- **Real signs inside the scene.** CPU busy percentage, RAM usage, network
  receive/transmit rates and uptime appear on bolted street displays. CPU signs
  also show load average; RAM signs show used/total GiB. Fan animation speed
  responds to CPU usage. Statistics are sampled once per second by a worker.
  Enemies and patch counts are always fictional: nothing is scanned for malware,
  killed, fixed, freed or changed on the real machine.
- **A complete editable art set.** 32 penguin frames, 16 duck frames, 32 enemy
  frames, 36 tiles, 13 props, 4 skies plus 8 sample skyline strips, an icon, and
  procedural visual effects. C++ art functions are the masters. Exported PNG atlases and `atlas.json`
  are included. Runtime builds its own cached sprites and needs no downloads,
  font files, Python, external textures, audio engine or game engine.

## Build and run

Requires a C++20 compiler, CMake 3.20+, a build tool, threads, and **Qt 6.4 or
newer, Core and Gui**. **LayerShellQt** (KDE's `layer-shell-qt`) is optional
and detected automatically: with it, Wayland screensaver windows become
wlr-layer-shell overlay surfaces bound to each output; without it, or with
`--no-layer-shell`, Qt's ordinary fullscreen request is used. The project does
not depend on private Qt/KWin APIs or require Widgets, SDL, OpenGL, Qt
Multimedia or a JavaScript runtime. QindaQt's
inspected repository targets Qt 6.11; the independent screensaver uses a smaller
public-API subset. Native Wayland presentation also needs your Qt Wayland
platform plugin to be installed, as it normally is in a working QindaQt session.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure

# Safe first run: normal window, real local metrics, Escape to quit.
./build/qinda-patrol --preview

# Fullscreen screensaver on every connected monitor, each with its own world and a
# single shared statistics collector; keyboard, mouse movement/click, wheel or touch dismisses.
./build/qinda-patrol --screensaver

# Quiet/private and lighter on the CPU.
./build/qinda-patrol --screensaver --no-metrics --eco

# One monitor only. --list-screens prints the indices, names and geometry.
./build/qinda-patrol --list-screens
./build/qinda-patrol --screensaver --screen 1
```

On Wayland, `--screensaver` creates one wlr-layer-shell surface per output in the
overlay layer, anchored to all four edges of that output. This is why docks and
panels stay beneath the scene and why each monitor reliably gets its own window:
QindaQt, like some other compositors, ignores the output named in an xdg-shell
fullscreen request and maps every such window on the active output. Outputs that
appear or disappear while running get a window added or removed. `--verbose`
logs which shell is in use and where each window lands.

`--screensaver` has a 1.2-second input grace period so its own launch action does
not immediately dismiss it. Mouse movement must exceed 4 logical pixels after
an initial baseline event. Escape exits immediately. No input is used to steer
the characters.

Run `./build/qinda-patrol --help` for `--fps`, `--seed`, `--screen`, `--interface`,
`--demo`, `--no-overlay`, `--no-layer-shell`, `--verbose` and `--duration`.
`--all-screens` is still accepted; it is now the default for `--screensaver`. `--demo` is explicitly labeled in both
signs and the frame; it never silently substitutes fictional data for live data.
`--no-metrics` replaces statistics with in-world art and does not start a sampler.

Install without root:

```sh
./tools/install-user.sh
~/.local/bin/qinda-patrol --preview
```

The script installs binaries, a desktop entry and an SVG icon. It does **not**
modify QindaQt, enable autostart, change power policy, replace a locker or attach
itself to an idle daemon. Add the binary directory to your PATH when necessary.

## Screensaver versus lock screen

**This application does not secure or lock a session.** A fullscreen window or
an overlay layer surface is not an authentication boundary. It never disables your current locker, inhibits
sleep, grabs the desktop, simulates activity, or changes DPMS.

For ordinary idle use, configure the desktop's idle manager to start
`qinda-patrol --screensaver` and let its existing lock/suspend policy remain in
charge. No existing QindaQt idle-command setting or screensaver ABI is assumed by
this package. The specific settings-panel hook still needs to be connected in
the desktop project.

For a **locked-screen background**, embed the optional scene into the trusted
locker's own surface; keep authentication, input handling, output ownership and
session locking in the locker. Do not place this fullscreen window above a lock
screen. See [QindaQt integration](docs/QINDAQT_INTEGRATION.md).

## Rendering and resource design

A fixed 960 x 540 logical scene is rasterized in C++ and scaled with nearest
neighbor sampling. 1920 x 1080 is exactly 2x; 3840 x 2160 is exactly 4x. Other
aspect ratios are letterboxed rather than stretching the characters. This is
intentional pixel art, not a high-resolution illustration with arbitrary blur.

Simulation is 120 fixed steps per second; presentation defaults to approximately
30 frames per second, or at most 20 with `--eco`. Changing the presentation rate
does not change movement speed. Catch-up is capped at 24 simulation steps so an
overloaded machine slows the animation instead of accumulating unbounded work.
Skylines are generated on demand in 480-pixel chunks and kept in a twelve-entry
cache; the sky crossfade is a per-pixel integer blend that only runs while a
district line is within blend distance of the screen center.
Qt windows stop ticking when they report that they are not exposed. The shared
sampler pauses when all windows report unexposed. Compositor-specific occlusion
and display-off behavior still require real-session testing.

Render cost and memory are bounded by visible terrain, a look-ahead region,
48 projectiles and 180 particles. Telemetry does not spawn more enemies or add
more particles; a busy CPU cannot create a positive-feedback workload spiral.
There is no audio, camera shake or deliberate strobing. The tiny title drifts and
can be hidden. Animation is not a guarantee against OLED retention; retain normal
screen-off timers for long idle periods.

## Headless rendering and asset export

The **same C++ simulation, renderer and sprite masters** can run without Qt or a
display server. This is how the included previews and asset atlases were made.

```sh
cmake -S . -B build/headless -G Ninja \
  -DPATROL_BUILD_DESKTOP=OFF -DCMAKE_BUILD_TYPE=Release
ninja -C build/headless $(portageq envvar MAKEOPTS)
ctest --test-dir build/headless --output-on-failure

./build/headless/qinda-patrol-render --time 40 --output scene.png
./build/headless/qinda-patrol-render --export-assets assets/exported
./build/headless/qinda-patrol-render --frames frames --time 0 --seconds 12 --fps 20
./build/headless/qinda-patrol-render --benchmark
```

Headless previews use labeled synthetic telemetry by default. `--live` captures
a current still from the machine running the renderer, after establishing a
counter baseline; it is not supported for offline animation exports.

PNG export is implemented in C++ with stored DEFLATE blocks. The images are valid
but deliberately unoptimized; compress them losslessly with an image optimizer
for distribution. The bundled copies have been recompressed without changing
pixels. No image-decoding dependency is needed at runtime because C++ builds the
assets directly.

## Source map

| File | Responsibility |
|---|---|
| `src/world.*` | Seeded terrain, districts, route planning, actors, combat, bounded effects and rebasing |
| `src/metrics.*` | Read-only Linux collection, counter baselines, parsing and sampling thread |
| `src/canvas.*` | Small rasterizer, bitmap glyphs, alpha compositing and PNG export |
| `src/assets.*` | Editable C++ character frames, props, tiles and atlas export |
| `src/renderer.*` | Scene composition, world-anchored parallax, district blending, gateways, signs, weather and visual effects |
| `src/patrol_window.*`, `src/main.cpp` | Qt window lifecycle, input dismissal, per-output layer-shell placement, monitors, CLI |
| `src/patrol_item.*` | Optional Qt Quick painted-item adapter; immutable frame handoff |
| `src/render_main.cpp` | Display-independent still, animation and asset generation |
| `tests/test_main.cpp` | Parser, traversal, determinism, bounds, rebasing and raster checks |

## Validation status

The C++ core, Linux sampler, rasterizer, asset generator and headless renderer
were compiled and exercised. The test matrix includes 100 seeded five-minute
world simulations, a 300-second audit of district lines, gate roofs and catwalks,
and a separate 40-minute rebase run, plus parser and image checks. See [the validation report](docs/VALIDATION.md) for exact results and
limitations.

**The Qt 6 window executable and optional Qt Quick adapter could not be compiled
in the original delivery environment.** Both have since been built with GCC 15.3.0
against Qt 6.11.1, and the window executable has been run fullscreen on a QindaQt
Wayland session and dismissed by input; see the addendum in the validation
report. The included CI workflow provides a build/offscreen smoke-test route; it
has not been run on a remote service. Actual QindaQt/KWin idle, lock,
monitor-hotplug and resume qualification remains to be done on your session.

This is an initial source implementation, not an installed QindaQt module or a
claim that the existing desktop's settings have already been wired up.

## License and reference

The original code, generated pixel-art assets and documentation in this package
are offered under the included MIT license. No wallpaper raster, third-party
sprite pack or font file is bundled. Character styling follows the user-supplied
QindaPunk penguin/duck reference; the original reference image is not relicensed
by this package.

Technical references and the inspected QindaQt commit are recorded in
[docs/SOURCES.md](docs/SOURCES.md).
