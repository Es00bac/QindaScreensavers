# Optional Linux test executable — 1.2

`prism-circuit-linux-x86_64` is the current stripped, dynamically linked build
from this workspace. It includes the material/facade atlases and shaders.
It is not an AppImage or a portable dependency bundle; building locally is preferred.

This executable imports **GLIBC_2.43**, **GLIBCXX_3.4.32** and **Qt_6.11** symbols.
The host must provide compatible libc/libstdc++, Qt Gui/Core, LayerShellQt,
SDL2, Cairo, OpenGL and transitive libraries. `linked-libraries.txt` records
this machine's dynamic links, not files distributed with the project.

```sh
./bin/prism-circuit-linux-x86_64 --course city --seed 41 --windowed
./bin/prism-circuit-linux-x86_64 --course bliss --seed 41 --windowed
```

A working desktop OpenGL 3.3 core context is required. On older or different
distributions, compile the supplied source instead of replacing system libraries.
No system installation, idle registration or session-policy change is performed.
