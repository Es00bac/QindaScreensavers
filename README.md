# Qinda screensaver suite

Five native, autonomous screensavers for QindaQt and other Wayland/X11 desktops.
All fullscreen launchers cover every connected monitor by default; `--screen N`
selects one. Output windows support live disconnect/reconnect and mixed display
scales. The 3D scenes preserve framing on portrait monitors.

| Saver | Version | Start on all monitors |
|---|---|---|
| Qinda Patrol | 0.2.0 | `qinda-patrol --screensaver --no-metrics` |
| Circuit Reef | 1.1.0 | `circuit-reef --fullscreen --private` |
| Prism Circuit | 1.1.0 | `prism-circuit --fullscreen` |
| Prism Brawl | 1.2.1 | `prism-brawl --fullscreen` |
| Starward | 2.1.0 | `starward --fullscreen` |

Race and Brawl have synthesized stereo effects, enabled at 20% volume. Use
`--mute` or `--volume 0..1`. One mixer serves all monitors. The other three are
silent. Escape closes a saver; ordinary input dismisses fullscreen after its
launch grace period.

Patrol has four shuffled guardians, phased arena battles, service objectives,
three powerups, and distinct machinery and architecture across its districts.
Reef adds sharks, octopi, rays, a turtle, seahorses, free-swimming jellyfish, and
bubbles. Race has collectible turbo, shield, pulse, and magnet items. Brawl has
eight mechanically distinct character specials, active item seeking, and
articulated animations. Starward adds ring systems, auroras, traffic, and depth.

The editable artwork is C++ geometry and procedural rigs. The three 3D projects
and Reef share native display code in `common/`; keep this directory alongside
the projects. Existing OBJ exports are reference assets, not runtime models.

Build from this directory with `python tools/build-suite.py`. It reads
`portageq envvar MAKEOPTS` and passes both configured job and load limits to
Ninja unchanged. Portage builds inherit the system configuration directly.
Gentoo installation is supplied by immutable source-snapshot ebuilds in
`packaging/ebuilds/`; all five binaries and desktop launchers are package-owned.
Patrol and Reef also install their existing QML modules.

[Validation and preview frames](validation/REFINEMENT.md) describe the checks.
These are visual screensavers; the desktop continues to own locking and idle
activation. Installation does not change the selected idle or lock-screen mode.
