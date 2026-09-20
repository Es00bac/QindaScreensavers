# Prism Circuit — Afterglow Grand Prix 1.2

An original C++20 / OpenGL autonomous combat-racing screensaver for QindaQt.
Eight cyber-animal racers compete, attack, crash, recover and race again.
This is a live simulation, not a movie player.

Version 1.2 adds targeted seeker drones, oil traps, mines, damage, spinouts,
takedowns and a winch-equipped rescue robot; a cinematic director; fitted
cockpits and steering hands; individual character proportions and kart hardware;
generated wear/facade textures; and two new, fully three-dimensional levels.

[Current captures and motion previews](previews/refinement-1.2/) ·
[Level design and changes](docs/REFINEMENT.md) · [Validation](docs/TESTING.md)

## Build and run

Requires Linux, a C++20 compiler, CMake 3.20+, Qt 6.4+ Core/Gui, LayerShellQt,
SDL2 2.0.18+, Cairo, and desktop OpenGL 3.3 core. Build inside the suite tree:
the adjacent `common/` directory provides the shared fullscreen and audio code.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/prism-circuit --windowed --course city --seed 41
./build/prism-circuit --windowed --course bliss --seed 41
```

`--fullscreen`, `--screensaver` and `--all-screens` cover connected outputs.
`--screen N` selects one; `--list-screens` lists them. Wayland presentation uses
LayerShellQt; SDL handles previews and offline captures. No browser or external
game engine is used. Materials and shaders are embedded in the executable.

This is a visual screensaver, **not a security locker**. QindaQt retains idle,
secure-lock, DPMS and suspend policy. No startup or desktop settings are changed.
Sound is synthesized locally, defaults to 20%, and can be disabled with `--mute`.

## Five levels

| Course | Integrated setting |
|---|---|
| `prism` | **Prism Knot**: the intentionally suspended Rainbow-Road-like course, with prismatic paving, dream gates, orbital structures and a ringed planet. |
| `eight` | **Chromatic Eight**: a planted terrace circuit with a separated crossover, green road materials and conservatory structures. |
| `aurora` | **Aurora Loop**: an icy route graded into a glacial landscape, with grounded ice ridges, frozen braces and an aurora sky. |
| `bliss` | **Qinda Bliss / Reclaimed Valley**: a service road embedded in rolling green terrain, reclaimed terminals, circuit-board beds, fern verges and drive-through terminal ruins. Two ravines become bridges. Inspired by the user's QindaBliss wallpaper. |
| `city` | **Neon Undercity / Skyway**: a connected, grounded district of factories, residential blocks and setback office towers; stacked expressways, depot underpasses, covered tunnels and a 34 m open jump. |

The city has four projected road crossings, with at least 25.8 m of centerline
vertical separation for seed 41. Buildings reserve their **complete silhouette**
against a sampled banked-road corridor, including roof plant and antennas.
Supports are checked against other road levels. The terrain, bridge structures,
landmarks and road share the same route data.

Select a course at launch (`random` is also supported). Three-lap races repeat
on that level with a fade-covered grid reset; levels do not swap beneath racers.
A seed fixes driver decisions, combat, scenery and the automatic shot sequence.

## Racing and recovery

Racers seek item boxes and boost pads, overtake, slipstream, select targets and
avoid known traps. Seven items affect the simulation: turbo, shield, pulse,
magnet, seeker, oil and mine. Hits damage and destabilize karts; severe hits cause
takedowns. Shields and rejoin protection prevent immediate repeat hits.

A knocked-out kart tumbles away, lands on terrain where present, and is retrieved
by Tow-Bot. Its winch lifts, carries and lowers the racer into a checked rejoin
lane. Progress stops during recovery. Jumping karts follow the city's actual
open road gap, then land with suspension compression.

This remains a track-constrained arcade simulation, not a general rigid-body
vehicle engine or a playable game. Driving, damage and camera decisions run at
120 Hz, independently of presentation. Wheel rotation follows accumulated
forward travel. High-speed spokes/tread converge toward their temporal average,
reducing the reverse-spinning wagon-wheel illusion without reversing rotation.

## Racers and cameras

CyberPengu, Ducké, Vix, Cache, Mochi, Hex, Patches and Axi have different body
proportions. Ducké is smaller; kart wheelbases remain consistent. Species-specific
pods, fins, coils, toolboxes, bash bars and patched panels personalize the karts.
Cache and Patches carry heavier grime and corrosion than the cleaner racers.

Hands use the steering rim's actual transformed grip points. Drivers lean,
flinch, look at rivals, release a hand for taunts, and flail during takedowns.
Cockpit cameras hide only the obstructing driver body/head, leaving fitted arms,
hands, steering wheel and instruments visible.

The auto director chooses battles, launches, jumps and rescues, alternates
subjects, and holds shots for at least three seconds. Fixed trackside cameras
pan as the pack passes; chase, front, cockpit, orbit, pack and overview views
are also available. Obstructed external views fall back to a clear racing view.

In windowed previews:

- **A**: automatic director; **C**: cycle cameras; **V**: cockpit.
- **1–8**: select racer; **[ / ]**: previous/next racer.
- Left-drag: orbit; wheel: zoom; **Escape**: exit.

Fullscreen dismisses on activity after its one-second startup grace period.

```sh
./build/prism-circuit --course city --camera cockpit --driver 0
./build/prism-circuit --course bliss --camera trackside --seed 41
./build/prism-circuit --showcase 6 --no-hud
./build/prism-circuit --all-screens --eco --brightness 0.8 --no-hud
```

## Rendering, assets and validation

Instanced geometry, HDR lighting, shadows, bloom, tone mapping and 4x MSAA are
used. `--eco` selects a 30 fps cap, 2x MSAA and reduced bloom. The normal 60 fps
setting is a **cap, not a performance guarantee**. See the current GPU measurements
and explicitly unverified deployment paths in [TESTING.md](docs/TESTING.md).

`src/models.cpp` and `src/meshes.cpp` are model masters; `track.cpp` and
`scenery.cpp` define the levels; `race.cpp`/`combat.cpp` define racing;
`cameras.cpp` directs shots. GLSL lives in `shaders/`.

Fifteen OBJ/MTL sets include eight racer/kart models, five road meshes, the start
arch and Tow-Bot. These are static exports, not animated rigs or complete levels.
The running app constructs its geometry directly; it does not load the OBJ files.

Generated material and facade atlases are in `assets/textures/`.
[Exact prompts and provenance](assets/textures/README.md) describe the built-in
image-generation workflow. No Nintendo or SuperTuxKart assets, characters, code,
logos, music or audio are bundled.

```sh
./build/prism-circuit --export-models assets/models --seed 41
./build/prism-circuit --course city --seed 41 --start 28.9 \
  --camera front --driver 1 --size 1920x1080 --snapshot jump.png
./build/prism-circuit --course city --seed 41 --race-log 600
./build/prism-stress
```

See [RENDERING.md](docs/RENDERING.md) for video export. Installation is optional:
`cmake --install build --prefix "$HOME/.local"`. It does not register an idle
service or alter the user's session policy. The separate suite source lock
still describes the prior published package; this workspace update does not
publish or push a new release.

Original source and generated artwork: **GPL-3.0-or-later**. See `LICENSE`.
No telemetry, network requests, bundled library binaries or font files.
