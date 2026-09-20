# Optional Linux test executable

`prism-circuit-linux-x86_64` is a stripped, dynamically linked ELF executable
built from the included source on Linux x86-64. It is provided for convenience,
not as a portable AppImage or distro package. Building from source is preferred.

The highest imported symbol versions in this build are `GLIBC_2.38` and
`GLIBCXX_3.4.32`. The host must supply compatible libc/libstdc++, SDL2, Cairo,
OpenGL and transitive libraries. `linked-libraries.txt` records the creation
machine's dynamic links, not files included in this package.

```sh
chmod +x bin/prism-circuit-linux-x86_64
./bin/prism-circuit-linux-x86_64 --windowed
```

The binary still needs a working desktop OpenGL 3.3 core context and SDL video
backend. On an older distro, an incompatible CPU architecture or musl-based
system, compile the supplied source rather than replacing system libraries.
No third-party library binaries or font files are bundled.
