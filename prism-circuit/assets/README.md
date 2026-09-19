# Reusable assets

`models/` contains ten static OBJ/MTL sets: eight original cyber-animal racers
with their karts, the start/sector arch, and the Prism Knot road mesh. `manifest.json`
records model bounds, triangle/vertex counts, coordinates, source masters and hashes.

The original SVG application icon is `studio.qinda.PrismCircuit.svg`.

The running app constructs the models itself. It does not load these OBJ files,
preview PNGs or a movie. Editable model definitions and rigging are in
`src/models.cpp` and `src/meshes.cpp`; road construction is in `src/track.cpp`.

OBJ/MTL is a static interchange format here. It does not preserve the articulated
hierarchy, wheel/steering animation, material-mode shader code, bloom, road/tire
procedural textures or generated number decal texture. Approximate diffuse,
specular and emission material values are exported. Use the source for exact
runtime behavior. A model viewer can appear different from the screenshots.

Model names use ASCII slugs for compatibility. The duck's display name is **Ducké**.
No Mario Kart, Nintendo, Tux Kart or SuperTuxKart assets or game code are included.
All original source and exported artwork in this package use GPL-3.0-or-later.
