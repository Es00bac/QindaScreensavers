# QindaQt integration

## Verified boundary

The inspected QindaQt repository snapshot is
`eed38fdb850544ec7277bdc1eb4bb19d365f3b7d`. Its README describes a Qt 6.11+
desktop and a native compositor integration pinned to KWin 6.6.6. This package
does not link that ABI and does not modify the repository. Its Qt surface is a
normal raster `QWindow`; it therefore has a smaller dependency boundary.

No published QindaQt screensaver plugin ABI was established in the files checked.
Do not interpret the following host integration recipe as an already existing
settings action, D-Bus service or lock-screen protocol in that repository.

## Integration choices

### Standalone idle decoration

Launch the installed `qinda-patrol --screensaver` from your idle controller. Let
the existing session manager decide when to lock, suspend and turn off outputs.
Stop the saver when returning to the desktop or transitioning into a trusted
locker. The program also dismisses itself when it receives input. It is not a
substitute for compositor-wide idle detection or wake handling under Wayland.

A windowed preview command for a future Settings page is:

```sh
qinda-patrol --preview --seed 551767 --demo
```

Use QProcess with an executable and argument list, not a shell command string
built from user-provided values. Store an explicit preview process handle and
terminate it when the preview UI closes. Do not launch multiple previews on each
settings redraw. The program does not need a privileged helper.

### Native Qt Quick background

Enable `PATROL_BUILD_QUICK=ON`, add this directory to the host build and link
`qinda-patrol-quick`. That target exposes a small C++ painted item rather than a
private QindaQt dependency.

```cmake
# In the host project, after vendoring this source directory:
set(PATROL_BUILD_DESKTOP OFF CACHE BOOL "" FORCE)
set(PATROL_BUILD_QUICK ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/qinda-patrol)
target_link_libraries(your_shell_target PRIVATE qinda-patrol-quick)
```

Before creating the host's QQmlEngine:

```cpp
#include "patrol_item.hpp"
patrol::registerQindaPatrolQmlTypes();
```

Then use `examples/PatrolBackground.qml`, or:

```qml
import QtQuick
import Qinda.Patrol 1.0

PatrolScene {
    anchors.fill: parent
    running: visible // replace with the host's actual idle + output-power state
    metricsEnabled: false
    demo: false
}
```

The imported type is `PatrolScene`; it starts paused and private by default.
`running` controls simulation and collection. `metricsEnabled` starts/stops its
local sampler. `demo` renders labeled synthetic data and never samples the OS.
The host must set `running: false` when the background is not needed, all outputs
are off, or the session is sleeping. Item visibility alone cannot establish
compositor output-power state.

The adapter renders on its owning GUI thread and passes a copied, immutable
QImage under a mutex to `paint()`. This avoids racing World/Renderer mutations
against a threaded Qt Quick render loop. The standard `QWindow` path can share
one MetricSampler across monitors; the current Quick adapter owns one collector
per item. Before a multi-output lock-screen deployment, inject a shared provider
or leave metric display disabled. This is a performance refinement, not a claim
that the current adapter secretly shares state.

### Trusted lock-screen background

Embed only inside surfaces owned and secured by the established locker. The
locker, not PatrolScene, must handle authentication, secure input, output hotplug,
lock acquisition, crash behavior and unlock. The background must not receive or
store passwords, call unlock, or open ordinary windows over a locked session.
Keep usage metrics disabled by default because they can reveal when the machine
is working. They do not include process names, window titles, addresses or files.

A failed or hung animation should produce a static/black background while the
locker remains locked. Integration must not make successful rendering a
prerequisite for entering or maintaining the locked state.

## Suggested settings model (new host work, not an existing API)

A future settings page can expose: enabled, preview, animation rate (20/30/60),
show metrics, show small title, current output/all outputs, and a seed override.
Keep security and screen-off timings in the existing desktop power/lock settings.
Use the normal session environment; do not force QT_QPA_PLATFORM or replace the
user's Qt platform theme. The standalone preview has ordinary window flags;
it does not install global shortcuts or compete with QindaQt's Meta+Shift-drag.

## Required live-session checks

Build both Qt targets against your real Qt 6.11 stack. Check normal preview,
fullscreen focus, Escape/mouse/touch dismissal, 100/150/200% scaling, 4K and
ultrawide letterboxing, multiple monitors, hot-unplug/replug, actual output
power-off, resume, and repeated launches. Verify the normal lock and suspend
policy still works. A headless renderer test is not evidence for any of these
compositor behaviors. No such real-session qualification is claimed here.
