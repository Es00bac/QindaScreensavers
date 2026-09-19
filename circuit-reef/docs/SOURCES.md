# Implementation references

The artwork and project code are original. These upstream references informed the platform and telemetry integration. They are not external runtime services.

- SDL2 `SDL_RenderGeometry`, including per-vertex color and alpha, available since SDL 2.0.18: https://wiki.libsdl.org/SDL2/SDL_RenderGeometry
- SDL2 vertex structure: https://wiki.libsdl.org/SDL2/SDL_Vertex
- SDL2 renderer creation and software fallback: https://wiki.libsdl.org/SDL2/SDL_CreateRenderer
- SDL2 streaming textures: https://wiki.libsdl.org/SDL2/SDL_CreateTexture
- SDL2 screensaver-allow hint: https://wiki.libsdl.org/SDL2/SDL_HINT_VIDEO_ALLOW_SCREENSAVER
- SDL2 screen-blanking allowance: https://wiki.libsdl.org/SDL2/SDL_EnableScreenSaver
- Linux procfs documentation, including CPU counters and MemAvailable: https://docs.kernel.org/filesystems/proc.html
- Cairo SVG surfaces: https://www.cairographics.org/manual/cairo-SVG-Surfaces.html
- Qt Quick painted items and their rendering/thread/performance considerations: https://doc.qt.io/qt-6/qquickpainteditem.html
- Wayland secure session-lock protocol: https://wayland.app/protocols/ext-session-lock-v1
- Gentoo SDL2 package and USE flags: https://packages.gentoo.org/packages/media-libs/libsdl2
- Gentoo Cairo package: https://packages.gentoo.org/packages/x11-libs/cairo
- Gentoo CMake package: https://packages.gentoo.org/packages/dev-build/cmake
- Gentoo Ninja package: https://packages.gentoo.org/packages/dev-build/ninja

References checked during construction of this package. The application uses public APIs and does not link to a specific QindaQt private ABI.
