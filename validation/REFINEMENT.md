# Screensaver visual refinements and validation

Validated locally on 2026-09-19. The five refined screensavers remain source-tree
builds; none was installed. QindaQt lock-screen/screensaver integration belongs
to the separate Opus task and was not changed or validated here.

## What changed

| Scene | Refinements | Preview |
|---|---|---|
| Qinda Patrol 0.2.0 | Generators, greenhouses, coolant pumps, and uplinks become service objectives that reward rapid fire, shields, or arc shots. Four shuffled bosses occupy sealed reactor arenas, with warnings, attack phases, exposed cores, and victory rewards. District machinery, vegetation, glass, pipes, cranes, and frost add environmental variety. | [Boss encounter](patrol-boss-fight.png), [district machinery](patrol-service.png) |
| Circuit Reef 1.1.0 | Sharks, eight-armed octopi, rays, a turtle, and seahorses join the reef. Fish and other creatures travel offscreen; jellyfish swim through the scene; rising bubbles have translucent rims and highlights. | [Aquarium](physical/circuit-reef/DP-1.png), [octopi](reef-octopi.png) |
| Prism Circuit 1.1.0 | Twenty-four replenishing item boxes provide turbo, shield, pulse, and magnet effects. Drivers seek and use items. New garden and glacial landmarks, smoother models, pickup effects, and synthesized stereo SFX enrich the race. | [Race](race-new.png) |
| Prism Brawl 1.1.0 | Eight distinct specials add shockwaves, dual shots, dashes, a returning boomerang, an uppercut, a gravity orb, a stomp, and healing bubbles. Fighters pursue healing, overclock, and shield items. Improved poses and curved meshes accompany attack, impact, pickup, and character-specific SFX. | [Combat](physical/prism-brawl/DP-1.png), [model](brawl-model.png) |
| Starward 2.1.0 | Planetary rings with occlusion and shadows, auroral curtains, drifting particles, dock traffic, smoother curved models, and consistent camera projection improve the space scenes. | [Space](physical/starward-reimagined/DP-1.png) |

Art remains editable procedural geometry and animation in the source tree.
Blender was not required. Race and Brawl sound defaults to 20% volume; `--mute`
and `--volume 0..1` control it. A missing audio device does not stop a scene.

## Checks and results

All five Release builds completed. The build helper queried the actual
`portageq envvar MAKEOPTS` value and used its `-j24 -l24` flags unchanged. No
Portage configuration or build limits were changed. See [build log](build-suite.log).

| Project | CTest result | Evidence |
|---|---|---|
| Patrol | 1/1 passed | [Log](qinda-patrol-tests.log) |
| Reef | 4/4 passed | [Log](circuit-reef-tests.log) |
| Race | 3/3 passed | [Log](prism-circuit-tests.log) |
| Brawl | 4/4 passed, including synthesized sound | [Log](prism-brawl-tests.log) |
| Starward | 3/3 passed | [Log](starward-reimagined-tests.log) |

The tests cover world/simulation invariants, boss ordering and encounters,
powerup behavior, sea-creature motion, character specials, model geometry,
sound synthesis, command-line validation, and applicable rendering smoke checks.

Race passed 36 course/seed combinations covering six simulated hours and
2,592,000 fixed simulation ticks. Brawl passed 48 cases covering 24 simulated
hours and 10,368,000 ticks. These are simulation stress tests, not 30 hours of
wall-clock graphical playback. See [race results](race-stress.json) and
[Brawl results](brawl-stress.json).

All five ran successfully on both physical displays, DP-1 and HDMI-A-1. The
native capture path produced separate 1920×1080 and 3840×2160 images for the four
shared-host scenes. Patrol's verbose log confirmed two exposed output windows.
See [physical run results](physical-suite.log), [Patrol output log](qinda-patrol-physical.log),
and the `physical/` preview directories. These short runs validate presentation
and clean shutdown; they are not sustained frame-rate benchmarks.

An isolated two-output Wayland compositor also exercised a landscape output
and a portrait output at 200% scale, then removed and reconnected the second
output while each saver kept running. All five passed. See
[mixed-scale results](native-mixed-scale.log), [hotplug results](hotplug-suite.log),
[Brawl portrait preview](native/prism-brawl/HEADLESS-2.png), and
[Starward portrait preview](native/starward-reimagined/HEADLESS-2.png).

The shared native presentation code is in `../common/`; keep it adjacent to the
projects when building. Patrol retains its existing native Qt output handling.
Fullscreen defaults to all monitors, and `--screen N` selects a single output.

## System changes and scope

No refined saver was installed. The earlier dependency check was an
`emerge --pretend` dry run only. The unused ebuild drafts added to the system
overlay were removed, leaving the preexisting Patrol and Reef ebuilds intact.
Local packaging drafts are unused artifacts of that abandoned packaging work.

The multi-monitor test dependency `gui-apps/kanshi-1.9.0` and its dependency
`dev-libs/libscfg-0.2.0` were installed through Portage. No unmanaged packages
were installed. The isolated labwc and kanshi test processes were stopped after
validation. The user's desktop session and the separate QindaQt build were not
stopped or modified.
