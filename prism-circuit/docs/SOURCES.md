# Implementation references

Checked for this build on 2026-09-19. These references concern the public library
interfaces and packaging, not the original characters/course designs.

- SDL2 OpenGL context creation: https://wiki.libsdl.org/SDL2/SDL_GL_CreateContext
- SDL2 window creation: https://wiki.libsdl.org/SDL2/SDL_CreateWindow
- SDL2 logical versus drawable window size: https://wiki.libsdl.org/SDL2/SDL_GetWindowSize
- Gentoo SDL2 package and platform/OpenGL USE flags: https://packages.gentoo.org/packages/media-libs/libsdl2
- Gentoo Mesa package: https://packages.gentoo.org/packages/media-libs/mesa

The project uses its own C++ fixed-step, track-constrained arcade simulation.
No external game code, kart game assets or Nintendo/SuperTuxKart material were
used. Low-level mesh/GL infrastructure was adapted from the user's earlier
Starward Reimagined deliverable under the same GPL-3.0-or-later terms.
