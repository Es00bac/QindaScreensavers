# QindaQt integration

## Scope

Prism Brawl is a standalone C++20 Linux visual screensaver, with no Qt runtime
dependency and no private compositor API dependency. The current package does
not implement a Qt Quick plugin, an idle daemon or an authentication surface.
It is intended to be launched by the desktop's existing visual-idle hook.

## Install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build $(portageq envvar MAKEOPTS)
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$HOME/.local"
```

Ensure the session can find `$HOME/.local/bin`, or use the absolute installed
path in the launcher configuration. Test an ordinary window first.

## Host lifecycle contract

On entry into the desktop's visual-idle state, launch:

```sh
"$HOME/.local/bin/prism-brawl" --all-screens --eco
```

On activity, display power-off, session locking, suspend or session teardown,
send SIGTERM to the child PID owned by the idle controller, then reap the child.
Do not kill unrelated processes by name. Allow a short normal shutdown grace
before escalation. The standalone app also dismisses itself on window input,
but the host remains authoritative for activity outside that window.

Maintain at most one visual instance per session. `--all-screens` creates its
own output windows; do not also start one copy for each output. Display topology
events cause a clean exit. The host can relaunch after topology settles if the
session is still in the visual-idle state.

Keep the real lock timer, authentication, output-power and suspend policies
unchanged. This program requests no idle-inhibitor. Stop it on output power-off
so it does not waste GPU work rendering to a dark display.

## Platform boundaries

On X11 the standard SDL2 desktop-fullscreen window was exercised under Xvfb.
This is not an XScreenSaver `-root` or `-window-id` embedding hack; those flags are
not implemented. Launch it as a standalone visual process.

A Wayland-capable SDL2 build can be selected through its normal platform backend,
for example while diagnosing a session:

```sh
SDL_VIDEODRIVER=wayland "$HOME/.local/bin/prism-brawl" --fullscreen
```

Native Wayland was not tested in the creation environment. This is an ordinary
Wayland application surface, not an ext-session-lock surface or compositor
layer-shell surface. Compositor-specific fullscreen, focus and output assignment
policy must be verified in your QindaQt session.

No desktop configuration or repository files were modified by producing this
package. The supplied `.desktop` launcher only gives preview/fullscreen actions;
installing it does not silently enable automatic startup.
