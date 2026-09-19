# Starward / Reimagined


## 2.1.0 refinement

The space scenes now add analytically occluded planetary rings, shadowed ring bands, auroral curtains, drifting particles, and shuttle traffic at the dock. Higher-resolution curved meshes improve close-up silhouettes. Portrait outputs preserve the scene framing, and the sky and geometry share the same camera projection.

`--fullscreen` and `--screensaver` cover every connected output by default. `--screen N` selects one; `--list-screens` shows names, geometry, and scale. Wayland uses LayerShellQt output binding, with independent framebuffers, mixed-scale support, portrait framing, and live output removal/reconnection. `--capture-dir DIR` saves one native PNG per output. SDL remains the preview and offline-capture backend. Build from the suite tree with the adjacent `common/` directory, Qt 6 Gui, LayerShellQt, SDL2, Cairo, and OpenGL development packages installed.

## CyberPengu and Ducké, beyond the shipping lanes

A C++20, real-time 3D screensaver for the QindaQt desktop. This is the replacement
for the earlier flat-vector Starward prototype, not a reskin of its sprite sheet
and not a player for the included preview movie.

The archive also includes an optional Linux x86-64 test executable in `bin/`;
see its README for the dynamic-library/ABI limits. Local compilation is preferred.

The application builds its own geometry, material/stencil atlas, procedural sky,
planet surfaces and character poses. It does not need an asset download, Python,
a game engine, an account, or a network service to run.

**The Linux executable was compiled and its OpenGL renderer was exercised on
X11/Mesa llvmpipe. Native Wayland and physical GPU performance were not tested.**
See `docs/TESTING.md` for the exact validation boundary.

**This is a visual screensaver, not a secure session locker.** Keep the desktop's
existing idle, locking, suspend and display-power mechanisms.

## Build

Requirements: Linux, a C++20 compiler, CMake 3.20+, SDL2, Cairo, a working desktop
OpenGL 3.3 core implementation, and ordinary platform threading support. Cairo
is used for the small paint/stencil atlas, captions and PNG export, not for the
3D scene. Qt is not a dependency of the standalone executable.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
./build/starward --windowed
```

On Gentoo, with a working graphics driver already configured:

```sh
sudo emerge --ask dev-build/cmake media-libs/libsdl2 x11-libs/cairo media-libs/libglvnd
```

SDL2 needs its `video` and `opengl` support and the backend appropriate to your
session, such as `wayland` or `X`. Review your existing package USE configuration
instead of replacing system-wide USE flags. The official package references are
in `docs/SOURCES.md`. No repository or desktop configuration was modified.

The build first uses pkg-config library targets when available. On a stripped-down
Linux environment it can also link installed runtime libraries using the small
ABI declarations in `compat/`. These are declarations, not bundled SDL, Cairo or
OpenGL implementations. The Linux test build used that runtime-library route.

## Run

```sh
# Preview first. Escape always exits.
./build/starward --windowed

# Fullscreen on every currently connected display.
./build/starward --all-screens

# A dimmer, lower-frame-rate presentation; telemetry remains disabled.
./build/starward --all-screens --private --eco --brightness 0.75

# Tour all seven chapters in 84 seconds instead of 266 seconds.
./build/starward --windowed --tour

# Inspect a particular composition without changing chapters.
./build/starward --chapter relic --start 20 --seed 41
```

`--fullscreen --screen 1` selects one output. `--all-screens` creates an independent
window/context for each output, preserving perspective rather than stretching one
bitmap across different aspect ratios. Display topology changes recreate the affected fullscreen windows without restarting the scene.

The default cap is 60 frames per second. `--eco` caps at 30, uses 2x rather than
4x multisample antialiasing, and reduces bloom. `--no-msaa` disables MSAA when
troubleshooting a driver or reducing cost. These are presentation targets, not
performance guarantees. This 3D revision is more GPU-demanding than the old
vector prototype. The intended runtime is a hardware-accelerated desktop GL driver.

Use `--reduced-motion` for slower camera, formation, character and shader-driven
movement. No mode uses camera shake, audio, score, player controls or game-over
screens. `--no-titles` hides the brief chapter captions. All options are documented
by `--help`.

Fullscreen input dismisses after a one-second startup grace period. Escape is
immediate. Small pointer motion is accumulated before dismissal to avoid a
single tiny launch-motion event. The loop pauses when every view is hidden or
minimized and limits a resume/stall step. SIGINT and SIGTERM exit cleanly.

## Seven connected chapters

| Chapter | The visual story |
| --- | --- |
| Ember Dock | Depart a rotating human ring habitat, with docking arms, illuminated bays, solar wings, antennae and moving tugs. |
| The Quiet Belt | Scan a rock, cut it with a mint laser, expose a capsule, then lift it onto the courier. |
| Freighter 08 | Ducké approaches a disabled human freighter. A repair beam and sparks restore its relay. |
| The Sleeping Archive | The capsule's signal awakens an alien ring assembly and its patterned aperture. |
| The Watchers | Sentinel craft send moving laser pulses toward the courier. Shield impact rings respond while Ducké transmits the signal. |
| First Contact | The weapons vanish. A broad-winged, luminous sail organism drifts into view, followed by smaller companions. |
| Homecoming | Return to the human station, deliver the capsule, and bring a small alien visitor along. |

Normal chapters last 38 seconds, for a 4 minute 26 second voyage. The authored
sequence repeats, with a new deterministic scenery seed for the asteroid field
and scattered alien fragments. `--seed` makes that sequence reproducible. This is
not a random plot generator: keeping the story order fixed is intentional.

Set changes occur while the scene fades into dark space. Human port machinery,
asteroid fields and alien structures do not pile up in every chapter.

## What changed visually

- **Modeled characters and vehicles:** a layered directional hull, exposed engine
  collars and vents, gun mounts, cheek armor, copper trim, body panels, fasteners,
  painted stencils, headset hardware, the cyber-eye, sunglasses and a flowing scarf.
- **Actual depth and lighting:** perspective, a depth buffer, directional shadow
  mapping, metal/ceramic/cloth/rock shading, GGX-style highlights, rim/fill light,
  HDR emission, bloom, tone mapping and multisample edge antialiasing.
- **A separate visual language for each place:** machined human equipment,
  cratered rock meshes, suspended alien structures, an animated aperture, and
  deforming organic sails and filament trails.

Characters are assembled hierarchically from parts and evaluated continuously at
animation time. They are not independent whole-body images that jump between
frames. Heads turn, the penguin blinks, the flight pose breathes, the scarf deforms,
ships bank, the duck follows delayed motion, and fins/filaments carry coherent waves.
Beams begin at the transformed gun mounts. Capsule pickup uses the ship's actual
attachment coordinates.

The animation is staged cinematic motion, not collision physics or a flight
simulator. Material shading is a compact real-time approximation, not a ray tracer
or a complete image-based-lighting/PBR engine.

## Privacy and system readings

Telemetry is **off by default**. `--telemetry` opts into the optional CPU/RAM flight
readout, shown only in human-space chapters. `--private` disables the collector.
`--demo` instead supplies clearly labeled synthetic values. Preview/snapshot export
never starts the real telemetry collector.

The local worker samples procfs at approximately one-second intervals. It does not
collect process names, usernames, window titles, network content or credentials,
and the application makes no network requests. Unavailable readings show `--`.
The readout is a small cockpit-style overlay; this revision does not map live text
onto the station's physical geometry. The parsing library also contains network
and uptime support inherited from the first prototype; those values are not shown.

## Included assets and source

The source is the authoritative, editable master for the art. Fourteen OBJ/MTL
model exports are included under `assets/models`: both pilots with their ships,
the station, freighter, alien archive, sentinel, capsule, sail organism and six
asteroid variants. The exports are reusable static geometry, not runtime inputs.
OBJ/MTL does not retain the runtime's hierarchy, analytic animation or GLSL effects.

```sh
# Re-export the geometry. No display or GL context needed.
./build/starward --export-models assets/models

# Capture the real renderer at 4K. Requires a GL-capable display context.
./build/starward --chapter harbor --start 10 --seed 41 \
  --size 3840x2160 --snapshot harbor.png --no-titles
```

Editable model construction is in `src/models.cpp` and `src/meshes.cpp`. The
material, sky, animation, antialiasing and bloom shaders are in `shaders/`.
CMake embeds the shader sources in the executable, so installed runs do not depend
on finding a source directory. Rebuilding after editing a shader regenerates the
embedded strings.

A raw RGB frame exporter is included for deterministic previews. See
`docs/RENDERING.md`. The packaged preview is a 35-second montage of seven
five-second excerpts, rendered at 1280x720 and 30 fps by this executable. It is
not a measurement of native frame rate and does not show a complete voyage.

## Install and QindaQt

```sh
cmake --install build --prefix "$HOME/.local"
```

This installs the executable, a desktop launcher and documentation. Add the
fullscreen command to QindaQt's existing idle visual-launch hook as described in
`docs/QINDAQT_INTEGRATION.md`. The package does not install an idle daemon, replace
your locker, change startup files or patch your QindaQt repository. No Qt Quick
adapter is claimed in this revision.

## License

Original source and generated artwork: GPL-3.0-or-later. See `LICENSE`.
No third-party font files or binary library bundles are redistributed.
