# QindaQt integration

## Recommended boundary

Circuit Reef is the idle visual, not the idle authority or session locker. The standalone executable is complete and can be launched manually. Wiring it to QindaQt's actual idle and output lifecycle is a host integration task. This package does not guess QindaQt's private signal names or write to its repository.

Use an executable launch on visual-idle activation:

```text
circuit-reef --all-screens --palette lagoon
```

For a privacy-first default:

```text
circuit-reef --all-screens --private --eco
```

Resolve the executable using the host's PATH or an absolute configured path. Avoid shell interpolation; pass arguments as a string list. Keep a process handle, allow only one instance, and terminate it on user activity, screen locking, suspend preparation, or output-power-off. A display topology change exits the application deliberately; relaunch it if the session is still eligible for idle visuals.

A generic Qt host can use `QProcess::start(program, arguments)` and `terminate()`. Treat these as public Qt integration primitives, not as claims about existing QindaQt APIs. Do not call `waitForFinished()` on the GUI thread. If a short asynchronous shutdown timeout expires, the host may kill only its own child process.

Do not install a blanket autostart rule that immediately runs fullscreen on login. Configure your chosen idle threshold in the desktop's own policy layer.

## Native backend

The standalone application uses SDL2 for windows, events, output selection, and accelerated triangle submission. Cairo provides cached background/text generation and the reference/export backend. No Qt or GTK widget toolkit is involved in that executable.

Wayland placement, fullscreen routing, input delivery, and focus are controlled by the compositor. An ordinary fullscreen window cannot reliably observe every global input event. The host must stop the child when its own idle monitor reports activity. Input dismissal inside the application's windows is a convenience, not a global seat grab.

`SDL_VIDEO_ALLOW_SCREENSAVER=1` is set before initialization, and `SDL_EnableScreenSaver()` is called. The application intentionally does not create idle or power inhibitors. This permits the desktop's existing screen blanking policy; it does not replace or guarantee that policy.

## Optional in-process Qt Quick item

Enable `REEF_BUILD_QT`. Link the resulting `circuit-reef-qt` target into your host and call:

```cpp
#include "CircuitReefItem.hpp"

// Register before loading a QML scene which imports the module.
registerCircuitReefQmlType();
```

A minimal QML use is:

```qml
import QtQuick
import QindaQt.CircuitReef 1.0

CircuitReef {
    anchors.fill: parent
    active: true
    privateMetrics: true
    seed: 2026
    palette: "lagoon"
    brightness: 0.75
    reducedMotion: false
}
```

In the real host, bind `active` to the appropriate visual-idle state and output visibility. Do not leave the item active behind an opaque lock view or while the display is off. Setting it inactive stops the timer and the telemetry worker. `privateMetrics` defaults to true.

This optional adapter deliberately uses the Cairo reference rendering path and copies immutable `QImage` frames across the Qt Quick thread boundary. It is not a zero-copy native QSG geometry implementation. Its frame cap is 30 and rendering height cap is 720. The separate standalone application remains the accelerated option.

The Qt adapter and example have not been compiled here. Review and build them against QindaQt's Qt version before integrating. Neither compositor-private integration nor secure lockscreen embedding has been claimed as tested.

## Security boundary

Circuit Reef does not collect credentials, authenticate users, call PAM, terminate processes, modify kernel parameters, or own a secure session-lock surface. It must not be substituted for a lock screen. Secure lock surfaces have compositor-enforced guarantees that an ordinary fullscreen window does not provide.

If a future QindaQt lock implementation wants to use the scene as decoration, the lock host must retain all security and authentication responsibilities and render it only inside its own trusted surface. The Qt item itself is not a locker.
