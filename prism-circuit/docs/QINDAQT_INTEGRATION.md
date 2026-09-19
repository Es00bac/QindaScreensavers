# QindaQt integration

Prism Circuit is a standalone Linux OpenGL client. This package does not contain
a Qt Quick item, compositor plugin, idle daemon or session-lock implementation.
No live QindaQt checkout was modified for this task.

## Host-managed visual lifecycle

Build/install the program, then have the desktop's existing idle visual-launch
hook spawn:

```sh
prism-circuit --all-screens --eco
```

Use the existing mechanism for managing a child process, not a root service.
Retain its PID or process handle. Stop it on user activity, real lock takeover,
output power-off, suspend or session teardown. The application accepts SIGTERM
and SIGINT and closes its own windows and GL contexts. Its own fullscreen input
dismissal is an additional convenience, not a substitute for the host's idle
or security policies.

`--all-screens` presents the same race on all currently connected outputs, using
one window and context per output. It exits on a display-topology event. The
host should wait for the new output topology and relaunch only if the session
is still in the appropriate idle state. Multi-output rendering costs more than
one output; there is no shared cross-context GPU asset cache in this version.

For an ordinary preview in a configuration UI, run `prism-circuit --windowed`.
Do not treat its fullscreen window as a secure Wayland lock surface. Do not
replace the real session locker with an `Exec=prism-circuit` shell command.

## Wayland and X11

SDL2 must be built for the chosen platform and with desktop OpenGL support.
The environment variable `SDL_VIDEODRIVER=wayland` can explicitly select the
Wayland backend; `SDL_VIDEODRIVER=x11` selects X11 when diagnosing a setup.
These are diagnostic overrides, not mandatory launch arguments.

Native X11/Mesa was exercised here. Native Wayland and a physical multi-output
QindaQt session were not available for verification. Fullscreen behavior and
output-power lifecycle need a check in the real desktop before enabling idle
activation unattended.

## Privacy

The application never reads procfs telemetry, window titles, process names,
user files or network content. There is no metrics worker and no network code.
The HUD reports only race speed, order and laps. The accepted `--private` switch
is a no-op for compatibility with the other screensavers in this collection.

The app deliberately calls SDL_EnableScreenSaver and does not request an idle
inhibitor. The desktop remains responsible for its own locking and power policy.
