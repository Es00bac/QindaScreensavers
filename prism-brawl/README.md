# Prism Brawl

## 1.2.0 · Seven wallpaper-inspired arenas

Four new environments join three rebuilt arenas, taking their colors, scenery
and lighting from the Pengu and Ducké wallpapers in Downloads and the installed
QindaQt collection. Bliss Circuit has rolling hills and reclaimed hardware;
Compile Club has warm workbenches, jade terminals, steam and a spinning desk fan;
Aurora Glacier has an ice lake, crystal peaks and moving aurora curtains;
Azure Fold has broad animated blue mineral ribbons and porcelain decks.

Prism Terminal now has a dimensional orbital gate and an obsidian eclipse sky.
Reactor Garden grows layered foliage around its moving platforms. Afterglow
Rooftop adds a deeper neon skyline, rain, passing traffic and rooftop machinery.
All seven rotate automatically between matches.

[Arena gallery](previews/Prism_Brawl_Stages.png) ·
[Arena film](previews/Prism_Brawl_Stages.mp4) ·
[References and stage details](docs/STAGES.md)

```sh
./build/prism-brawl --list-stages
./build/prism-brawl --windowed --stage compile
```

## Full-body animation update

The fighters now wind up, twist, kick, roll, flail and tumble as complete rigs.
Heavy hits drive directional airborne spins, open-handed panic, impact holds
and smoke trails; hard landings compress the body into a braced pose and kick
up dust. Light hits, blocked attacks, broken shields and near misses have
different reactions. Eyes, brows, mouths, ears, gills, tails and the scarf follow
the performance, with smoothly interpolated torso, head and limb targets.

Eight signature taunts include a hammer salute, sunglasses adjustment, fox bow,
raccoon laugh, rabbit dance, cat paw flick, panda chest drums and axolotl wave.
Fighters also beckon and slow-clap, react to opponents' challenges, celebrate
knockouts and wins, and slump after a loss. Taunts use safe gaps in combat and
can be interrupted by danger. Jab, heavy and aerial attacks have three pose
variations, and all eight specials have individual body choreography.

[Watch the animation reel](previews/Prism_Brawl_Animation.mp4) ·
[Pose examples](validation/animation/pose-review.jpg) ·
[Implementation and validation](docs/ANIMATION.md)

```sh
./build/prism-brawl --windowed
# Inspect a fighter's moves, taunts, launches and landings up close.
./build/prism-brawl --showcase 4 --animation-demo --mute
```


## 1.1.0 refinement

Each fighter has an individual special: Patch Quake, Twin Thrusters, Ember Dash, Cache Return, Lunar Rise, Gravity Hex, Meteor Stomp, and Tidal Mend. Their projectile motion, damage, lift, defense, healing, and animation differ. Fighters seek healing, overclock, and shield pickups, including supplies on moving platforms. Synthesized stereo impacts, blocks, pickups, jumps, ring-outs, and eight special timbres accompany the action.

`--fullscreen` and `--screensaver` cover every connected output by default. `--screen N` selects one; `--list-screens` shows names, geometry, and scale. Wayland uses LayerShellQt output binding, with independent framebuffers, mixed-scale support, portrait framing, and live output removal/reconnection. `--capture-dir DIR` saves one native PNG per output. SDL remains the preview and offline-capture backend. Build from the suite tree with the adjacent `common/` directory, Qt 6 Gui, LayerShellQt, SDL2, Cairo, and OpenGL development packages installed.

Sound effects are enabled at 20% volume. Use `--mute`, `--sound`, or `--volume 0..1`; offline captures remain silent. A missing audio device disables sound without stopping the saver.

## Neon Knockout · QindaQt

A real C++20 / OpenGL 3D platform-fighting screensaver for Linux. The eight
cyber-animal characters from Prism Circuit fight autonomous matches on original
floating arenas. This executable simulates the combat and renders the models.
It does not display the earlier generated concept picture or play a movie.

The 1.2.0 arenas were captured with the native OpenGL renderer on AMD Radeon
graphics under Linux/X11. See [the stage validation](docs/STAGES.md) for current
checks and [the original validation report](docs/TESTING.md) for the initial
release's test boundary.

**Visual screensaver, not a secure session locker.** Your desktop remains
responsible for idle activation, authentication, suspend and display power.

![Actual renderer capture](previews/Prism_Brawl_Preview.png)

[Watch the actual-renderer preview](previews/Prism_Brawl_Preview.mp4) ·
[Meet the fighters](previews/Prism_Brawl_Fighters.png) ·
[The seven arenas](previews/Prism_Brawl_Stages.png)

## Build and run

Requirements: Linux, a C++20 compiler, CMake 3.20+, Qt 6.4+ Gui, LayerShellQt,
SDL2 2.0.18+, Cairo, and a working desktop OpenGL 3.3 core implementation. Keep
the suite's adjacent `common/` directory. SDL2 must support the video
backend you use. Cairo draws the small interface labels and writes PNG captures;
the entire 3D scene is rendered by OpenGL.

Python, ffmpeg, a browser, a game engine, an account and external image packs
are **not runtime requirements**. Python and ffmpeg are optional preview-export
tools only.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
./build/prism-brawl --windowed
```

For normal distribution builds, install SDL2 and Cairo development packages.
CMake prefers pkg-config targets. In minimal Linux environments without those
headers, the included small ABI declaration headers can link the already
installed runtime libraries. The creation-environment build used that fallback.
They are declarations, not bundled implementations of SDL2, Cairo or OpenGL.

An optional dynamically linked Linux x86-64 test executable is included in
`bin/`. It is not a portable AppImage. Building locally is the preferred route.

## Launch options

```sh
# Fullscreen on every currently connected display.
./build/prism-brawl --all-screens

# Dimmer, lower-cost presentation without the damage cards.
./build/prism-brawl --fullscreen --eco --brightness 0.75 --no-hud

# All eight characters in the same fight.
./build/prism-brawl --fighters 8

# Two-fighter matches on the garden stage, with a reproducible seed.
./build/prism-brawl --fighters 2 --stage garden --seed 41

# A steadier camera and half-speed presentation.
./build/prism-brawl --reduced-motion --camera fixed
```

`--stage auto` is the default and advances the arena after each match. Other
choices are `prism`, `garden`, `rooftop`, `bliss`, `compile`, `aurora` and `azure`.
`--camera auto` follows the active
field smoothly; `fixed` keeps a wider stationary composition; `close` moves in.
`--screen N` selects one output when not using `--all-screens`.

Escape exits immediately. In fullscreen mode, keyboard, button, wheel, touch or
accumulated pointer movement dismisses the app after a one-second launch grace.
In the preview window ordinary mouse movement does not close it. SIGINT and
SIGTERM shut down cleanly. There are no player controls. Sound effects are enabled by default and support `--mute`.

There is **no telemetry collector and no network access**. `--private` is
accepted only for compatibility with the other QindaQt screensavers.

## The cast

| ID | Fighter | Visual identity and movement profile |
|---|---|---|
| 0 | CyberPengu | Original mint cyber-eye and copper scarf, articulated patch hammer, balanced weight. |
| 1 | Ducké | Sunglasses, amber hardware, forearm buckler, lighter and quicker. |
| 2 | Vix | Orange fox, pointed ears and pale tail tip, fastest base run speed. |
| 3 | Cache | Masked raccoon, striped tail and blue emitters, midweight. |
| 4 | Mochi | Pale rabbit, tall animated ears and lilac emitters, lightest. |
| 5 | Hex | Dark cat, magenta accents and curled tail, light and quick. |
| 6 | Patches | Red panda, ringed tail and green emitters, heaviest with stronger melee hits. |
| 7 | Axi | Pink axolotl, independently animated side gills, light and agile. |

The head geometry, species details, materials and palette are reused from the
actual Prism Circuit source. The bodies now have upright fighting rigs, legs,
boots, animated hands and combat equipment instead of kart seats.

The default is **four simultaneous fighters**. The first two matches show the
entire roster, then the selection is shuffled in subsequent pairs of matches.
`--fighters 2` and `--fighters 8` are also implemented. Match composition can be
reproduced with a seed.

All fighters share a compact core move vocabulary. Their appearance, speed,
weight, damage multiplier and projectile speed vary. This is not a claim of
eight fully independent, tournament-balanced commercial-game move lists.

## Actual combat simulation

* Autonomous targeting, pursuit, jump decisions, drop-throughs and defense.
* Jab, heavy strike, aerial attack, grab/throw, emitter projectile and dodge.
* Double jumps, limited recovery bursts and one-way platform collision.
* Damage percentages increase launch velocity; weight changes knockback.
* Shields lose energy while held and when hit, regenerate when released, and
  break into a brief stun when depleted. Grabs bypass them.
* A dodge has a limited invulnerability window. Respawns have temporary protection.
* Ring-outs consume one of three stocks; damage resets on respawn.
* Small crystal pickups repair damage, briefly strengthen attacks or recharge guard.

The outcome comes from the simulated fight, not a scripted winner. A match ends
when one fighter remains or after 90 seconds of active combat. Timeout ties are
resolved by remaining stocks, then credited knockouts, then lower damage, with
a stable slot-order fallback for an exact tie. A winner announcement and a dark
transition cover the next match's spawn and arena change.

Attack startup, active and recovery phases are separate. Each move can hit a
particular opponent only once per activation. Overlapping attacks are gathered
before resolution so trades can occur. Projectile collision is swept in the
travel direction. Falling characters use swept crossings against platform tops.
The actors remain on a two-dimensional combat plane; the art, lighting, camera
and environments are fully 3D. This is an arcade platform fighter, not ragdoll
physics, a full playable game or a reproduction of Melee's exact mechanics.

## Seven original arenas

| Stage | Distinct environment |
|---|---|
| **Prism Terminal** | Obsidian decks, a dimensional mint/copper orbital gate, spires and an eclipse sky. |
| **Reactor Garden** | Layered circuit-tree foliage, hydroponic channels, fireflies and sliding side platforms. |
| **Afterglow Rooftop** | Rain, layered neon city towers, rooftop machinery and passing distant traffic. |
| **Bliss Circuit** | Sunlit hills, flowers, reclaimed computers and a distant skyline; staggered upper decks. |
| **Compile Club** | Warm workshop lamps, server racks, steaming coffee and a desk fan; a vertical central lift. |
| **Aurora Glacier** | Fractured ice, crystalline mountains, an ice lake and aurora curtains; drifting side decks. |
| **Azure Fold** | Broad flowing blue mineral ribbons, floating porcelain stones and a gliding upper deck. |

All seven use a readable main platform and three one-way upper platforms. Their
layouts are authored, not copied from a Nintendo stage. Scenery variations are
seeded. There is no unbounded procedural platform network.

## Smooth presentation

The simulation uses a fixed **120 Hz timestep**, independent of presentation.
The renderer interpolates actor roots, animation channels, projectiles and
camera framing. Gaits follow traveled distance; hands, legs, body lean, head
motion, eyes, ears, gills, tails and scarf animation are evaluated continuously.
The hammer stays attached to its animated hand.

Attack poses ease through smoothed joint targets rather than switching whole-body
sprite frames. Impacts deliberately include a short 35–59 ms hit hold. Recovery,
launch trails, shield outlines and contact sparks are anchored to world-space
actors. Respawn appearance uses a visible materialization effect. Match resets
occur beneath a dark fade. A narrow/portrait output widens the vertical field of
view rather than stretching the characters or cropping the entire fighting plane.

The camera has no shake and there are no full-screen white explosion flashes.
Reduced-motion mode halves simulation playback and uses fixed wide framing;
it is not a medical photosensitivity guarantee.

## Rendering and power

The instanced OpenGL renderer includes depth testing, directional shadows,
metal/ceramic materials, prismatic floor shading, procedural star/nebula/planet
backgrounds, HDR emission, bloom, tone mapping and 4x MSAA by default. Guards use
cut-out holographic shells and contour rings rather than opaque spheres hiding
the fighters. The sky and all meshes are created locally.

`--eco` sets a 30 fps cap, 2x MSAA and lower bloom. `--no-msaa` disables MSAA;
`--fps N` accepts 10..240. These are caps, not hardware performance promises.
Normal presentation defaults to a 60 fps cap. The native loop pauses when all
windows are hidden/minimized, and caps catch-up after a stall at 100 ms. At very
low frame rates the simulation therefore slows rather than accumulating an
unbounded catch-up queue.

Stop the process while outputs are powered down. A normal application window
cannot infer every compositor-specific display-power state. Each connected output
gets its own GL context and perspective-correct view of the same match. Display
topology changes recreate only the affected fullscreen windows.

## Source, assets and installation

The source is the editable master. `src/battle.cpp` contains the simulation;
`src/animation.cpp` choreographs the poses and reactions; `src/models.cpp`
contains the character rigs; `src/stages.cpp` builds the arenas and
`src/director.cpp` composes the spectator view. The material and post-processing
code lives in `shaders/`.
CMake embeds the shaders into the executable and tracks shader edits.

Fifteen OBJ/MTL sets are provided: all eight fighters and seven arena platform
assemblies. They are static geometry exports, not runtime requirements. OBJ does
not retain the live hierarchy, animation, shader effects or whole backgrounds.
The C++ source retains all of those. No font files or third-party library binaries
are bundled.

```sh
cmake --install build --prefix "$HOME/.local"
```

Installation adds the executable, icon, launcher and documentation. It does not
change your idle/lock/startup configuration or edit the QindaQt repository. See
[the QindaQt integration guide](docs/QINDAQT_INTEGRATION.md).

[Rendering/export instructions](docs/RENDERING.md) ·
[Validation report](docs/TESTING.md) · [Technical references](docs/SOURCES.md)

## License and originality

Original source and generated models: GPL-3.0-or-later, including the reused
Prism Circuit rendering/character foundation. See `LICENSE`. This is an original
QindaQt-themed platform fighter. No Nintendo or other commercial game's code,
characters, stage layouts, models, logos, textures, music or sounds are included.
