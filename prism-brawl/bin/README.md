# Optional Linux x86-64 test executable

`prism-brawl-linux-x86_64` is the compiled release executable used for the native
captures and tests. It is dynamically linked against the creation environment's
system libraries. This is not an AppImage, a statically linked distribution, or
a promise of compatibility with every Linux installation.

Building the C++ source locally is preferred. The executable needs compatible
SDL2, Cairo, desktop OpenGL, glibc and libstdc++ runtimes plus a graphical session.
The recorded linkage is in `../validation/binary-linkage.txt`. No third-party
library binaries or font files are included.

From the project root, after reviewing the source:

```sh
chmod +x bin/prism-brawl-linux-x86_64
./bin/prism-brawl-linux-x86_64 --windowed
```

Installing through CMake builds and installs the ordinary `prism-brawl` name.
Do not run the program as root. Escape exits. Automatic idle activation still
belongs to your desktop's existing lifecycle controller.
