# Validation report

Validated on 19 September 2026 in the creation container. These are measured checks,
not claims about untested desktop configurations.

## Build and automated checks

- Linux x86-64, GCC 14.2, C++20, CMake/Ninja Release build completed.
- SDL2 and Cairo were linked through installed runtime libraries and the small
  Linux ABI declaration fallback. The normal development-header route is included,
  but the container did not have those development packages to test that route.
- All three CTest entries passed: scene/motion/mesh/metrics checks, help output and
  rejection of invalid numeric CLI input.
- The scene test executed 41,473,389 small assertions across geometry indices,
  finite/unit vertex normals, 3,360 timeline samples over four seeds, chapter/fade
  bounds, finite instance transforms, the instance budget, hero/muzzle continuity,
  CPU delta parsing and MemAvailable accounting. Peak sampled instance count: 1,687.
- AddressSanitizer and UndefinedBehaviorSanitizer CTest builds passed. Leak detection
  was enabled for these non-graphical tests.
- The model exporter generated all 14 OBJ/MTL pairs. A separate validation checked
  vertex/UV/normal counts, finite coordinates and normals, face-index bounds and
  matching material files.

These are regression and numeric-consistency tests. They do not establish that
all possible driver errors, visual defects or long-duration failures are absent.

## Packaged-copy check

A source archive was extracted into a different directory, configured from scratch,
built, tested with CTest, and installed into a separate local prefix. The installed
executable then successfully rendered a pursuit-scene PNG without relying on the
original source or build directory. All these steps passed. Only documentation and
validation logs were added after that check; the executable/source payload did not
change.

## Native graphics and input

The actual SDL/OpenGL executable rendered every chapter on an isolated Xvfb X11
server. Driver identification was:

```
OpenGL 4.5 (Core Profile) Mesa 25.0.7-2
llvmpipe (LLVM 19.1.7, 256 bits)
SDL backend: x11
```

The application requests a desktop OpenGL 3.3 core context; this driver supplied
a compatible newer context. Real shader compilation, framebuffer creation,
instanced drawing, shadow mapping, MSAA, bloom, compositing and PNG readback ran.
Final 1920x1080 chapter captures and a 3840x2160 capture were produced through that
same native GL renderer.

A separate private Xvfb display was used for input checks. Sending Escape to a
preview window exited with code 0. Sending a space key to a fullscreen window
past the launch grace period also exited with code 0. No input was injected into
the user's desktop or a host seat. The measured exits after injected input were
0.810 seconds and 0.376 seconds respectively in this loaded software-rendering
container.

A sanitizer-built native GL snapshot also passed, with UBSan halt-on-error enabled.
Leak detection was disabled for that graphical run because it includes external
Mesa/X11 driver allocations. This is not a leak-check pass for those drivers.

## Preview and performance boundary

The preview is seven five-second excerpts, encoded as a 35-second 1280x720,
30 fps H.264 video. All frames come from the executable's deterministic RGB export.
Cuts join different excerpts; this is not a recording of a whole voyage or its
natural chapter transitions. Native chapters use dark-space fades.

**The encoded 30 fps is not the speed achieved by llvmpipe.** Software export was
roughly 1 to 3 frames per second under the current container load. A hardware-
accelerated desktop GPU is the intended runtime. No particular 60 fps or 4K
performance claim is made. `--eco` reduces cost but does not make the app suitable
for every GPU or battery budget.

## Not tested

Native Wayland, a physical GPU, heterogeneous multi-monitor hardware, display
hotplug on a real compositor, QindaQt's actual idle hook, Qt embedding, secure
session locking, and a full-day unattended run were not tested. The standalone
is not a secure locker or a QindaQt plugin. Installation does not configure
idle activation or change the desktop's power/lock policy.

Representative build and native-check output is included under `validation/`.
