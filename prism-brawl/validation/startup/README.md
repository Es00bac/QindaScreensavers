# Randomized startup and stage rotation — 1.2.1

Installed on the desktop (`qinda`) through Portage, replacing 1.2.0.
Source: `0d5b59c4d7230e1eebce358431d405684a7c75e4`. Overlay: `527f80edd228f025df91e26b72500485dc77b4bc`.
Installed executable SHA-256: `9e60f086e2859078ff51eeb58ea608a6e087611169e0590c5ff2e0fd928fe3c5`.
The desktop's configured MAKEOPTS remains `-j24 -l24`.

Normal automatic launches randomly choose an opener different from the previous
launch. During a session, all seven stages play in shuffled bags with no adjacent
repeat, including at bag boundaries. Explicit stages stay pinned and explicit
seeds reproduce their opening and match order. Offline tools do not affect history.

Validation passed:

* All 13 release and ASan/UBSan CTest checks, including 1,958,312 unit assertions.
* 64 seeds over twelve shuffled bags each: unique coverage and no adjacent repeats.
* Thirty separate native launches locally, and sixteen from the installed desktop
  executable: no repeated opener; all seven arenas observed on both machines.
* Repeated explicit seeds match, and fixed stages/tools preserve stored history.
* Ninety-six stress cases: 48 simulated hours and 20,736,000 simulation ticks.
* Desktop launcher validation and exact installed source-commit verification.

Startup checks use disposable XDG state directories and the native offscreen GL
backend, leaving the user's live screensaver history untouched. The desktop GPU
is AMD Radeon RX 5700 XT with Mesa 26.1.8. Detailed logs and JSON reports are in
this directory. The full seven-arena art validation is retained in `../stages/`
and the previous 1.2.0 deployment evidence in `../deployment/`.
