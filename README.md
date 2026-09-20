# Qinda screensaver suite

Five native, autonomous screensavers for QindaQt and other Wayland/X11 desktops.
All fullscreen launchers cover every connected monitor by default; `--screen N`
selects one. Output windows support live disconnect/reconnect and mixed display
scales. The 3D scenes preserve framing on portrait monitors.

| Saver | Version | Start on all monitors |
|---|---|---|
| Qinda Patrol | 0.2.0 | `./qinda-patrol/build/qinda-patrol --screensaver --no-metrics` |
| Circuit Reef | 1.1.0 | `./circuit-reef/build/circuit-reef --fullscreen --private` |
| Prism Circuit | 1.2.0 | `./prism-circuit/build/prism-circuit --fullscreen` |
| Prism Brawl | 1.2.1 | `./prism-brawl/build/prism-brawl --fullscreen` |
| Starward | 2.1.0 | `./starward-reimagined/build/starward --fullscreen` |

Race and Brawl have synthesized stereo effects, enabled at 20% volume. Use
`--mute` or `--volume 0..1`. One mixer serves all monitors. The other three are
silent. Escape closes a saver; ordinary input dismisses fullscreen after its
launch grace period.

Patrol has four shuffled guardians, phased arena battles, service objectives,
three powerups, and distinct machinery and architecture across its districts.
Reef adds sharks, octopi, rays, a turtle, seahorses, free-swimming jellyfish, and
bubbles. Race adds targeted attacks, traps, damage, takedowns, Tow-Bot recovery,
cinematic cameras, personalized racers, and integrated Bliss/city levels. Its
[current previews](prism-circuit/previews/refinement-1.2/README.md) and
[validation](prism-circuit/docs/TESTING.md) describe the local 1.2 update; the
published source lock has not been changed. Brawl has
eight mechanically distinct character specials, active item seeking, and
expressive full-body animations and seven wallpaper-inspired arenas. Starward
adds ring systems, auroras, traffic, and depth.

The editable artwork is C++ geometry and procedural rigs. The three 3D projects
and Reef share native display code in `common/`; keep this directory alongside
the projects. Existing OBJ exports are reference assets, not runtime models.

Build from this directory with `python tools/build-suite.py`. It reads
`portageq envvar MAKEOPTS` and passes both configured job and load limits to
Ninja unchanged. Prism Brawl 1.2.1 is installed on the desktop (`qinda`) and
laptop (`qinda-top`) through Portage, with shuffled arenas and a different
starting stage each launch. Its
[startup and desktop delivery checks](prism-brawl/validation/startup/README.md)
and the [five-saver binary delivery to the laptop](validation/laptop-binaries/README.md)
are recorded. System installation is managed through Portage and the user's
overlay, with its build configuration unchanged.

[Validation and preview frames](validation/REFINEMENT.md) describe the checks.
This work covers the visual scenes, behavior, and standalone multi-monitor
validation. QindaQt lock-screen/screensaver integration is being handled
separately by Opus. No desktop integration or lock settings were changed here.
