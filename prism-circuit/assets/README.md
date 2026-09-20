# Reusable assets — 1.2

`models/` contains fifteen static OBJ/MTL sets: eight racers with their karts,
all five road meshes, the start arch and Tow-Bot. The generated
[model manifest](models/manifest.json) records current part/vertex/triangle counts
and bounds. The root manifest identifies source masters and textures.

The running app constructs geometry directly from C++; it does not load OBJ
exports. OBJ preserves positions, UVs, normals and approximate material colors,
but not runtime rigs, steering, wheel filtering, combat state, generated texture
sampling, instancing, particles or GLSL materials. Road exports do not include
their complete scenery environments. Use the source for exact live behavior.

The two generated material atlases are embedded in the binary; their saved
paths, exact prompts and provenance are in [textures/README.md](textures/README.md).
The original application icon is `studio.qinda.PrismCircuit.svg`.

Coordinate system: right-handed, Y-up, kart forward +Z, game metres. Model names
use ASCII slugs; the duck's display name is Ducké.

Original source and generated artwork use GPL-3.0-or-later. No Nintendo,
SuperTuxKart or other third-party game assets/code are included.
