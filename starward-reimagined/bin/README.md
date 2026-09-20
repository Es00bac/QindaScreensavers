# Optional Linux test executable

`starward-linux-x86_64` is the Release build produced alongside this source.
Run it with `./bin/starward-linux-x86_64 --windowed` from the extracted project.
It dynamically links the system SDL2, Cairo, libGL, libstdc++ and standard Linux
libraries. It is not a self-contained AppImage or a portable ABI guarantee.
Building the source on your own Linux installation is the recommended path.
No dependencies or fonts are bundled.
