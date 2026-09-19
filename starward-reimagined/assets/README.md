# Model assets

The 14 OBJ/MTL pairs are static exports of the actual C++ scene models. They include
positions, UV coordinates, transformed smooth normals, object names and basic
material colors/emission. The units are the renderer's authored scene units.

`manifest.json` records filenames, geometry counts and bounds. Files are exported
at one illustrative pose. They are not an animation cache and are not loaded by
the runtime. To edit the running models, change `src/models.cpp` or the base mesh
construction in `src/meshes.cpp` and rebuild.

OBJ/MTL cannot carry this project's C++ hierarchy, analytic motion, shader-driven
sails/scarf, HDR bloom, procedural sky, stencil atlas or all material parameters.
The full look is reproduced by the included renderer. The models can be imported
into another editor as starting geometry, with materials adjusted there.

No downloaded art pack, external image resource or font file is required or
included. All generated geometry and artwork here uses GPL-3.0-or-later alongside
the source. Re-export with `starward --export-models assets/models`.
