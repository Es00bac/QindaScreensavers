# Prism Circuit


## 1.1.0 refinement

Eight rows of three collectible item boxes supply turbo cells, shields, ion pulses, and magnet drives. Drivers seek boxes, hold items briefly, and activate effects that change race physics. Shields absorb attacks, pulses slow nearby opponents, and magnets improve speed and collection reach. Item and boost sounds use one synthesized stereo mixer for the entire process. Botanical terraces and glacial landmarks distinguish the garden and aurora courses.

`--fullscreen` and `--screensaver` cover every connected output by default. `--screen N` selects one; `--list-screens` shows names, geometry, and scale. Wayland uses LayerShellQt output binding, with independent framebuffers, mixed-scale support, portrait framing, and live output removal/reconnection. `--capture-dir DIR` saves one native PNG per output. SDL remains the preview and offline-capture backend. Build from the suite tree with the adjacent `common/` directory, Qt 6 Gui, LayerShellQt, SDL2, Cairo, and OpenGL development packages installed.

Sound effects are enabled at 20% volume. Use `--mute`, `--sound`, or `--volume 0..1`; offline captures remain silent. A missing audio device disables sound without stopping the saver.

## Afterglow Grand Prix · QindaQt

An autonomous **C++20 / OpenGL 3D racing screensaver**. CyberPengu, Ducké and six
cyber-animal friends race on a suspended prismatic circuit above a neon city.
This is a running race simulation, not a movie player, image carousel or concept
mockup. The included preview is a capture of this renderer.

The art and course layouts are original. No Nintendo or SuperTuxKart code,
models, textures, characters, logos or audio are included.

**Visual screensaver, not a session locker.** QindaQt retains control of idle
activation, secure locking, suspend and display power. No desktop settings or
repository files are modified by this package.

[Watch the actual-renderer preview](previews/Prism_Circuit_Preview.mp4) · [Meet the racers](previews/Prism_Circuit_Racers.png) · [Course overview](previews/Prism_Circuit_Courses.png)

![Actual C++ race capture](previews/Prism_Circuit_Preview.png)

## Build and launch

Requirements: Linux, C++20, CMake 3.20+, SDL2 2.0.18+, Cairo, and a desktop
OpenGL 3.3 core implementation. Cairo handles small text decals, the broadcast
layer and PNG export. It does not render the 3D scene. Qt, a browser, Python and
an external game engine are not runtime dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
./build/prism-circuit --windowed
```

On Gentoo, with the graphics driver already working:

```sh
sudo emerge --ask dev-build/cmake media-libs/libsdl2 x11-libs/cairo media-libs/libglvnd
```

Enable SDL2's video/OpenGL support and the backend for your session, typically
`wayland` and/or `X`. Review existing package USE settings rather than replacing
your global USE configuration. Package/documentation references are in
`docs/SOURCES.md`.

CMake prefers the normal pkg-config development packages. For stripped-down
Linux test environments, the included `compat/` declarations can link installed
SDL2/Cairo runtime libraries. This is the route used by the creation-environment
build. These files are ABI declarations, not redistributed library binaries.
Install the normal development packages for a distro build.

An optional dynamically linked Linux x86-64 test executable is in `bin/`.
Building locally is preferred; the test executable is not an AppImage.

## Run

```sh
# Fullscreen on every currently connected display.
./build/prism-circuit --all-screens

# Lower-cost rendering, slightly dimmer, without the broadcast overlay.
./build/prism-circuit --all-screens --eco --brightness 0.8 --no-hud

# A reproducible course and race.
./build/prism-circuit --seed 41 --course prism

# The other two complete circuits.
./build/prism-circuit --course eight
./build/prism-circuit --course aurora

# A close racing camera or a quiet view of the whole circuit.
./build/prism-circuit --camera chase --driver 1
./build/prism-circuit --camera overview --reduced-motion
```

Escape exits immediately. In fullscreen, keyboard, button, touch, wheel or
accumulated pointer activity also dismisses the app after a one-second launch
grace period. Windowed mode is intended for preview, so ordinary mouse motion
does not close it. SIGINT and SIGTERM exit cleanly.

There are no player controls or telemetry collection; audio is synthesized locally. `--private`
is accepted for compatibility with the other QindaQt screensavers, but it has
nothing to disable here. The displayed speed, place and lap come from the race,
not your computer. The program makes no network requests.

`--screen N` chooses a display. `--all-screens` shows the same live race in a
separate context on each output, using each output's aspect ratio. Display
topology changes rebuild the affected fullscreen windows in place. Secure lock-surface integration remains owned by the desktop.

## The courses

| CLI name | Circuit | Character |
|---|---|---|
| `prism` | **Prism Knot** | A closed 3D torus-knot ribbon, with banked curves, three elevated crossings, prismatic paving and suspended dream-gates. About 1.39 km for seed 41. |
| `eight` | **Chromatic Eight** | A broad figure eight with a vertically separated crossover. About 1.03 km. |
| `aurora` | **Aurora Loop** | A flowing, less tangled orbital circuit with rolling elevation. About 0.84 km. |

These are three authored parametric layouts, not an unlimited random road
network. A seed varies driver pace, decisions, city details, and a small shape
variation in the knot. A course does not visibly rebuild underneath the karts.
Select the course at launch. The same course hosts repeated three-lap races.

The road is a real closed mesh with a structural underside, physical rails,
material seams, metallic prismatic shading and six lane-specific boost pads.
Track-bank frames are shared by the road, karts, grid markings and gate mounts.
The course never requires a jump or a fall-recovery teleport.

Neon city towers sit below the road. Floating orbital structures, a distant
ringed planet, aurora and alien-like suspended shards add depth without turning
the driving surface into a wall of props. Gate and atmospheric motion is slow;
there are no full-screen flashes or camera shake.

## Eight racers

| ID | Name | Character details |
|---|---|---|
| 0 | **CyberPengu** | Penguin, mint cyber-eye, copper scarf, mint kart. |
| 1 | **Ducké** | Duck, sunglasses, gold headset hardware, amber kart. |
| 2 | **Vix** | Fox, pointed ears, pale cheeks and tail tip, coral kart. |
| 3 | **Cache** | Raccoon, dark mask and striped tail, blue kart. |
| 4 | **Mochi** | Rabbit, animated illuminated ears, lilac kart. |
| 5 | **Hex** | Dark cat, magenta ear accents and curled tail, pink kart. |
| 6 | **Patches** | Red panda, round ears and ringed tail, lime kart. |
| 7 | **Axi** | Axolotl, animated side gills, rose kart. |

Their karts share a coherent chassis language: sculpted ceramic cowl, colored
side pods, exposed tire tread, five-spoke rims, short exhausts, headlights,
spoiler, frame struts, race-number decals and Q insignia. Drivers are seated,
with articulated arms aimed toward the steering wheel and continuous blinking,
head motion and species-specific appendage animation.

Use `--showcase 0` through `--showcase 7` to inspect any model on a turntable.

## Autonomous racing and motion

The simulation uses a **fixed 120 Hz timestep**. Presentation interpolates between
states, so it is not limited to a sprite sheet or tied to display refresh rate.
Wheel rotation follows traveled distance; front wheels steer; chassis lean,
drift yaw and small suspension motion are continuous. The road's tangent and
bank orient every kart. Boost trails and drift sparks originate at the vehicle.

Drivers score candidate passing lanes using nearby occupancy, avoid merging into
occupied space, brake for slower karts, receive a modest slipstream benefit and
aim for reachable boost pads. Pace differs by racer and race seed. Gentle
catch-up assistance keeps the field interesting. Winners are determined by
traveled race distance, not a scripted outcome.

Three laps are followed by a brief winner display and rolling slowdown. A dark
fade covers the return to the starting grid, then a new countdown begins with
fresh seeded driver pace. Races run indefinitely without menu interaction.

This is an arcade, track-constrained simulation. It is **not** a general rigid-body
vehicle engine, tire-friction model, full pathfinding game or playable kart game.
Approximate fender boxes resolve small contacts along the least-penetrating
axis. There are no weapons, damage systems, player inputs or falling mechanics.

The automatic director gently changes viewing angle and blends between the live
positions of racers. `front`, `chase`, `orbit` and `overview` cameras are also
available, along with `--driver N`. Reduced motion uses half-speed simulation
playback and a steadier, more distant racing camera. It does not make a medical
claim about photosensitivity safety.

## Rendering and power

The renderer uses instanced 3D meshes, depth testing, a following shadow map,
metal/ceramic/rubber materials, HDR emissive lighting, bloom, tone mapping, and
4x multisample antialiasing by default. `--eco` caps presentation at 30 fps,
uses 2x MSAA and lowers bloom. `--no-msaa` disables multisample antialiasing.
The default presentation cap is 60 fps; `--fps` accepts 10..240.

Those are caps, not performance promises. This renderer was tested on X11 with
Mesa llvmpipe software rendering. Hardware-GPU performance, native Wayland and
physical multi-monitor operation were not validated in this environment.

The loop pauses while all windows are hidden/minimized. After a stall it caps
the simulation advance to 100 ms rather than attempting an unlimited catch-up.
Below 10 fps this results in slower simulation playback. QindaQt should stop the
process while the display is powered off or the real lock/suspend policy takes
over; the standalone window cannot reliably infer every compositor power state.

## Source and assets

`src/models.cpp` and `src/meshes.cpp` are the authoritative model/rig sources.
`src/track.cpp` constructs the courses. `src/race.cpp` handles AI and physics;
`src/director.cpp` builds the environment and spectator camera. Editable GLSL
sources are in `shaders/` and are embedded by CMake into the executable.
Installed runs do not depend on a source-tree path or an online asset service.

The package includes ten OBJ/MTL asset sets: all eight racers with their karts,
the start arch and the knot road. They are reusable static geometry/material
exports. OBJ does not retain the runtime hierarchy, continuous animation,
procedural tread/road shaders or generated race-number texture. The source
retains those features. Preview PNGs are not transparent sprite sheets.

```sh
./build/prism-circuit --export-models assets/models --seed 41
./build/prism-circuit --showcase 7 --no-hud --snapshot axi.png --size 1200x1200
./build/prism-circuit --start 22 --driver 1 --camera front \
  --snapshot race.png --size 3840x2160 --seed 41
./build/prism-circuit --race-log 600 --seed 41
```

See `docs/RENDERING.md`, `docs/TESTING.md` and `docs/QINDAQT_INTEGRATION.md` for
export, validation and host-integration details.

## Install

```sh
cmake --install build --prefix "$HOME/.local"
```

This installs the executable, launcher, icon and documentation. It does not
register an idle service, replace your locker or change startup configuration.

## License

Original source and generated artwork: **GPL-3.0-or-later**. See `LICENSE`.
The low-level renderer is adapted from the earlier Starward Reimagined project
provided in this conversation. The race simulation, courses, kart artwork,
additional animals, material changes and camera director are new here.
No third-party font files or binary library bundles are redistributed.
