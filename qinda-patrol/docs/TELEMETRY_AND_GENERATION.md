# Telemetry, generation and invariants

## Read-only telemetry

The collector reads `/proc/stat`, `/proc/meminfo`, `/proc/uptime`,
`/proc/loadavg`, `/proc/net/route` and `/proc/net/dev`. It uses file streams in
one worker thread and a mutex-protected snapshot. It does not enumerate process
directories, inspect command lines, read usernames, collect browsing history,
execute shell utilities, send network requests or make system changes.

CPU is the difference of aggregate CPU counters between samples. The first
8 fields are counted; guest and guest_nice are not added again. Idle plus iowait
are treated as not-busy. Decreasing counters or a zero interval invalidate that
sample and establish a fresh baseline. The first CPU/network readings are
unavailable until two samples exist. CPU utilization is not the same as load
average; the signs label each separately.

Memory used is `(MemTotal - MemAvailable) / MemTotal`. MemFree is deliberately
not substituted for MemAvailable. Missing, malformed or contradictory data is
shown as unavailable instead of a made-up percentage. Values labeled GiB use
1024^3 bytes; /proc meminfo kB is interpreted as 1024-byte units.

Network monitoring chooses the lowest-metric up IPv4 default-route interface,
or the exact interface supplied with `--interface`. Only that interface's RX/TX
counters are used; bridge/tunnel/physical counters are not indiscriminately
summed. An IPv6-only setup without an IPv4 default route needs `--interface`.
This is per-interface traffic, not an internet speed test. An interface change
or reset invalidates the first rate interval. There is no GPU, temperature,
per-process memory or per-cgroup accounting in this version. Containerized runs
may see host-level or namespaced metrics according to the container's proc mount.

The live sampler timestamps snapshots. Signs suppress stale data after four
seconds. Demo mode is opt-in in the desktop executable and the default only in
the headless art exporter; it is visibly labeled. Hidden mode does not create a
collector. Included screenshots and video are demo-mode renders, not readings
from Jarrod's computer.

## Seeded terrain

SplitMix64 provides explicit integer randomness. Terrain randomness and visual
effect randomness have separate streams so a particle count cannot alter the
next chunk. The renderer itself does not mutate the simulation. A given seed
and sequence of fixed simulation steps reproduce the route and behavior; exact
pixel hashes across every math library/CPU are not promised.

Roofs are 6–18 tiles long, with 1–3-tile gaps and height changes of up to two
32-pixel tiles. Heights are clamped to 320–448 logical pixels. The first roof is a
fixed safe staging area. About thirty percent of roofs are open catwalks; the rest
are buildings. Each roof also draws a decoration, structure and wall-pattern index
from the terrain stream, which the renderer maps to props, billboards, railings,
service cables and steam. Metric signs go to the next roof that is at least ten
tiles long and at least two roofs after the previous sign. This is an endless
stream, not a pre-recorded movie and not a shuffled fixed screenshot.

## Districts and transitions

Districts are measured in absolute world distance: every 4096 pixels a new
district begins, cycling the four palettes. Roof generation never lets a roof
straddle a district line. A roof that would cross the line is cut to end one tile
before it, and the first roof of the next district, which therefore starts within
six tiles of the line, is the gate roof: at least eight tiles long, always a
building, never a sign, and drawn with the gateway arch. `districtAt(x)` gives,
for any absolute x, the district and a smoothstep blend factor toward its
neighbor across 768 pixels on either side of the line. The renderer uses it three
ways: the sky is crossfaded per pixel for the screen center; every skyline
building takes its colors and rooftop silhouette from the district at its own
world position, so a line appears as a left-to-right gradient on the horizon well
before it is reached; and the street layer, transit line, weather and overlay
accent use the blended palette. Skylines are procedural, seeded per 480-pixel
chunk and cached, so they neither repeat every screen nor jump at a coordinate
rebase, which is why absolute coordinates are used throughout.

The penguin launches 26 pixels before a roof's right edge and lands 36 pixels
inside the next roof. With a known start and destination, `planJump()` selects a
bounded duration, `clamp(dx/185 + |dy|/260, 0.58, 1.0)` seconds, and solves `vy = (dy - gravity * duration^2 / 2) / duration`.
The world evaluates the corresponding ballistic arc at 120 Hz and snaps only
the final landing to its analytically known endpoint. This is a deliberately
constrained platformer route, not a general rigid-body engine, navmesh or arbitrary
procedural cave generator. Roof props are scenery, not collidable obstacles.

The safety recovery counter is exposed in World for tests. It should remain zero
under the shipped generator; the test suite verifies this across seeds and a
coordinate rebase. Impossible custom generator changes should be rejected by
planJump rather than silently treated as reachable gameplay.

## Resource limits

Only the current area, look-ahead and a short rear margin remain resident.
Actors behind the camera and defeated enemies are removed. Projectiles are
capped at 48 and particles at 180. Sprite and sky caches are fixed per renderer; skyline
chunks are limited to a twelve-entry cache. At a camera coordinate above 65536, all relevant x coordinates shift
back by 32768; a separate distance counter preserves traversal bookkeeping.

Simulation catch-up is capped; telemetry does not increase actor or effect
counts. CPU load affects the apparent speed of an already drawn fan, not the
number of calculations spawned. The screensaver itself contributes some CPU
usage, which will naturally be included in the displayed machine-wide value.

## Art editing

Change the layered primitive drawing functions in `src/assets.cpp` to adjust
characters or add states. Change `palette()` for district color families. The
exported atlas files are convenient interchange assets; editing a PNG alone does
not modify the running program because it currently uses the C++ masters.
Rebuild and run `qinda-patrol-render --export-assets assets/exported` after an edit.

The scene adds its own tiny stars, weather, sparks, beams, fan blades, steam,
water streaks, signs and metric bars. Those effects are fully implemented in
`src/renderer.cpp`; missing external effects files are not required assets.
