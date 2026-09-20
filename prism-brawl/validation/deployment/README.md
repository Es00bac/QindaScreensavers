# Desktop delivery — Prism Brawl 1.2.0

Installed on `qinda` through Portage on September 19, 2026, replacing
`x11-misc/prism-brawl-1.1.0` with `x11-misc/prism-brawl-1.2.0::qindaqt`.
The exact one-package upgrade plan and successful merge are saved here.

* Runtime source: `0ced5ea7f5f4b85db60358f829c6c4d7dcc9293e` in `/home/cabewse/git/screensaver-suite.git`.
* Final overlay recipe: `8c6321e57adf01dee12cb7dbbd1bcacc77181c03`.
* Installed executable: `/usr/bin/prism-brawl`.
* Installed executable SHA-256: `6a310e2acdd71765ce5551189e820702baf2bd547d06b6b3d125a57baaa862a3`.
* Desktop MAKEOPTS: `-j24 -l24`, inherited unchanged.

The installed help reports 1.2.0; all seven stage names are registered; the
existing desktop launcher validates and resolves to the new executable.
Wayland display discovery reports DP-1 and HDMI-A-1, with their respective
scales of 1 and 2. All seven hidden X11 render captures succeeded on the
desktop's AMD Radeon RX 5700 XT / Mesa 26.1.8. Frames and machine-readable
results are in `desktop-frames/`.

A 900-second, eight-fighter headless run of the installed executable completed
108,000 ticks and ten rounds, with 1,383 hits, 187 taunts, 993 tumbles and 2,030
reactions. This is simulation validation, not a real-time rendering benchmark.

The initial fetch attempt exposed a missing `EGIT_BRANCH` in the new recipe.
The published correction selects `prism-brawl-1.2.0` while retaining the same
immutable commit. The subsequent package build and installation succeeded.
The old attempt log is retained for the audit trail.
