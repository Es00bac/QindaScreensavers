# Technical references

Reviewed for this implementation on 2026-09-18. These are references, not runtime
network dependencies.

1. QindaQt README and repository tree, commit
   `eed38fdb850544ec7277bdc1eb4bb19d365f3b7d`:
   https://github.com/Es00bac/QindaQt/blob/eed38fdb850544ec7277bdc1eb4bb19d365f3b7d/README.md
   Used for the Qt/KWin dependency boundary and ordinary application-window
   integration context. The repository was read, not modified. No screensaver
   ABI or current idle-command UI was established by this limited inspection.
2. Qt, Raster Window Example:
   https://doc.qt.io/qt-6/qtgui-rasterwindow-example.html
   Public QWindow/QBackingStore/QPainter presentation APIs and exposure lifecycle.
3. Linux kernel, The /proc Filesystem:
   https://docs.kernel.org/filesystems/proc.html
   Aggregate CPU counters, memory fields, network counters and proc semantics.
4. Wayland session-lock protocol, protocol text rendered by Wayland Explorer:
   https://wayland.app/protocols/ext-session-lock-v1
   Used to distinguish a trusted locker from a normal fullscreen client.

Visual reference: the user's earlier `qinda-punk.png`, retrieved from the user's
Library during this conversation. Its mint-eyed penguin, orange-trimmed dark
armor/cape and sunglasses/thruster duck informed the new pixel-art frames.
The reference raster is not bundled, and no claim of pixel-exact extraction is
made. All delivered screenshots are outputs of this package's own C++ renderer.
