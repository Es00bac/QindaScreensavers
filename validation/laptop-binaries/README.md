# Desktop binary packages installed on qinda-top

On 2026-09-19, installed the desktop's five published screensaver packages on
the laptop through Portage, using binary packages only. The installed Brawl
1.2.1 on `qinda` was packaged with `quickpkg` first; the other four packages
were already available from the desktop binhost. Package signature verification
remained enabled. No local compilation or desktop session restart was needed.

| Portage package | Installed executable | Result |
|---|---|---|
| `x11-misc/qinda-patrol-0.2.0::qindaqt` | `/usr/bin/qinda-patrol` | Reinstalled |
| `x11-misc/circuit-reef-1.1.0::qindaqt` | `/usr/bin/circuit-reef` | Reinstalled |
| `x11-misc/prism-circuit-1.1.0::qindaqt` | `/usr/bin/prism-circuit` | Reinstalled |
| `x11-misc/prism-brawl-1.2.1::qindaqt` | `/usr/bin/prism-brawl` | Upgraded from 1.1.0 |
| `x11-misc/starward-reimagined-2.1.0::qindaqt` | `/usr/bin/starward` | Reinstalled |

The package versions above come from the installed Portage database. Some
older applications' help banners still show their previous source versions.
Prism Circuit's separate, ongoing source development was not included.

The install used `emerge --oneshot --usepkgonly=y --getbinpkg=y` with the five
exact package atoms above. See [the plan](binary-plan.log),
[the successful merge](emerge.log), [overlay sync](overlay-sync.log), and
[desktop Brawl packaging](desktop-quickpkg.log).

All five installed executable SHA-256 hashes match the desktop's installed
copies. [Installed checks](installed-checks.json) also record unchanged laptop
`MAKEOPTS=-j16 -l16` and unchanged lock-screen, power, and QindaQt preference
files compared with [the pre-install hashes](settings-before.json).

Two unmanaged older executable copies in `/usr/local/bin` shadowed the
packaged Patrol and Reef. They were backed up, preserving metadata, to
`/var/backups/qinda-screensavers/20260919T223807Z/`, then replaced atomically
with symlinks to their corresponding `/usr/bin` executables. The idle
controller's existing PATH now resolves to the packaged versions. Paths and
original hashes are recorded in [overrides.json](overrides.json).

Validation:

- All five applications successfully enumerated the laptop's native Wayland
  output. All five desktop entries passed `desktop-file-validate`.
- Patrol completed an offscreen preview; Reef produced a headless render;
  Circuit, Brawl, and Starward rendered snapshots through the laptop's AMD
  OpenGL driver. All exited successfully. See [smoke-tests.json](smoke-tests.json)
  and the four PNG captures in this directory.
- Four normal, unseeded Brawl launches used isolated temporary state and chose
  `bliss`, `compile`, `prism`, then `bliss`: each start differed from its
  predecessor. See [startup-stages.json](startup-stages.json).
- Patrol and Reef lock-screen QML modules are installed with all linked
  dependencies present. See [integration-checks.json](integration-checks.json).

The desktop and laptop already had the same QindaQt desktop package,
`gui-wm/qindaqt-desktop-0.1.0_pre20260919-r8`. Its current lock-screen plugin
renders Patrol and Reef; the other three are standalone idle screensavers.
Installing the desktop's binary packages does not add new lock-screen renderers.
The laptop remains configured for Patrol after one minute of idle time;
automatic locking, locking on resume, and idle display power-off remain disabled
as they were before this delivery.
