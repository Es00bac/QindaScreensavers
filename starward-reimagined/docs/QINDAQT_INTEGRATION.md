# QindaQt integration

## Standalone visual process

Use the installed executable from your desktop's existing idle visual-launch hook:

```sh
$HOME/.local/bin/starward --all-screens --private --eco
```

The default preview launcher is `starward --windowed`. The included desktop file
provides preview, fullscreen and private/eco actions. Installing this package only
adds that launcher, the executable and documentation. It does not edit the host
session configuration.

The host should start at most one instance, stop it when the user returns, stop
it when outputs are powered down, and relaunch it when the display layout changes.
SIGTERM is supported and results in a clean shutdown. Avoid keeping it rendering
behind a powered-off or securely locked display.

## Locking and input

This application is not a security boundary. Ordinary SDL fullscreen windows are
not Wayland session-lock surfaces. Keep your existing authenticated lock screen
and input/power policy in control; do not replace a locker command with this app.

The app dismisses itself on input delivered to its fullscreen window after a
one-second startup grace period. It cannot promise to receive global input on every
compositor. Have the host idle/resume service stop it as well. Escape always exits.

## Wayland and X11

SDL chooses the video backend. For diagnosis only:

```sh
SDL_VIDEODRIVER=wayland starward --windowed
SDL_VIDEODRIVER=x11 starward --windowed
```

These require the relevant backend to be present in your SDL build. A working
hardware OpenGL 3.3 core context is expected. The creation environment exercised
X11 through llvmpipe; native Wayland was not tested.

Each display gets its own window and GL resources. This is intentionally simple,
but it duplicates render targets and geometry buffers. Very large multi-monitor
setups should use `--eco`, or launch only a selected display. Output hotplug causes
a clean exit rather than in-place renderer recreation.

## Embedding into the shell

The shipped artifact is the complete standalone C++ program, not a compiled
QindaQt plugin. There is no untested QML wrapper included under a claim of native
integration. `starward-scene` is independent of SDL and contains the director,
geometry, model construction and telemetry parsing. `Renderer` owns a GL rendering
pipeline and assumes a current desktop OpenGL context on its calling thread.

A future Qt host must create and make current that context, match context lifetime
to renderer lifetime, pass drawable pixel dimensions, route input/idle events, and
reset or explicitly preserve any GL state shared with Qt. Qt Quick may use a
non-OpenGL graphics backend, so this renderer cannot simply be called in an
arbitrary QML item's paint method. No such adapter was implemented or tested here.

## Permissions and privacy

No root privileges are used by the runtime. No inhibitors are requested, no power
settings are changed, and the app tells SDL to permit existing screen-saving
policy. No metric collector is started unless `--telemetry` is explicitly used.
