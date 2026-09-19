# Art and motion guide

## Visual direction

This is a quiet counterpart to Qinda Patrol. The shared visual language is dark cyberpunk space, soft cyan/mint light, copper accents, visible machinery, and friendly QQ eyes. The environment is an aquarium, not a sidescrolling game. Plants grow from submerged circuit ruins; information appears on buoys instead of a large dashboard.

Three koi colorways share one articulated mesh: pearl/copper, deep teal, and pearl/amethyst. Jellyfish use mint or lilac light. A small service puffer carries the QQ eye motif. All drawings are original C++ geometry; there is no dependency on the older penguin/duck wallpaper files.

## Coordinate system

The nominal artboard is 1600x900. Runtime layout scales horizontal positions to the viewport's aspect ratio while preserving each creature's shape. Fish move in an orthographically projected 3D lane: x is horizontal travel, y is vertical travel, and z determines depth, scale, facing, and draw order.

The yaw follows the lane derivative. Rendering uses sine/cosine of yaw, so the mathematical wrap at plus/minus pi does not introduce an orientation discontinuity. Body vertices, fin controls, visible eyes, and etched circuits share the same transform.

Fish body deformation is a traveling lateral wave whose amplitude grows toward the tail. The head does not bob sideways with the same amplitude as the tail. Fin curves are sampled closed Catmull-Rom splines from dynamically deformed 3D control points. A seeded mix of periods avoids one conspicuous short loop across the whole tank.

Jellyfish tentacles are attached to their bell and driven by traveling waves with a phase lag along their length. The puffer's eyelids close and reopen with a narrow smooth pulse. Kelp roots remain planted while distal control points move with the current.

## Runtime versus atlas

The runtime does not step through the exported sprites. It evaluates geometry at the current animation time. The sheets are for reuse, inspection, or a future alternate renderer.

| Export | Layout |
| --- | --- |
| `koi-N-swim-32.png` | 8 columns x 4 rows, 320x192 cells, pivot at 198,94 |
| `jelly-N-pulse-24.png` | 8 columns x 3 rows, 192x256 cells, pivot at 96,65 |
| `koi-N.png/.svg` | Three still colorways |
| `jelly-N.png/.svg` | Two jellyfish stills |
| `qq-puffer.png/.svg` | QQ maintenance puffer |
| `coral-N.png/.svg` | Four plant/coral forms |
| `aquascape-lagoon.png/.svg` | Background reference |
| `icon.png/.svg` | Application icon |

SVG and PNG exports are produced by Cairo from the same C++ artwork. The application does not load those files during normal rendering. Editing a PNG alone therefore does not change the live scene; edit the C++ master or implement an alternate texture layer.

## Motion review checklist

Watch a complete fish turn from side-on to facing the camera and back. The body should foreshorten rather than flip. Examine the tail root: it stays attached while the tip flexes. Check the puffer's blinks for a smooth transition. Toggle R in the native preview and verify a speed change rather than a pose jump. Move the preview between aspect ratios and verify that fish do not stretch.

A few things are deliberately not simulated: obstacle-aware fish AI, fluid vortices, collision response, animal biology, and physically accurate refraction. The scene is a stylized procedural animation. Projected overlaps can occur because creatures occupy different depths.

## Performance knobs

Try the default triangle renderer first. `--raster` is a compatibility fallback and reference renderer, not a performance enhancement. Use `--eco` or fewer koi for lower-power machines. `--quality native` increases background/coordinate resolution and may increase graphics work.

Dim colors, moving text, and a fading startup label are aesthetic measures, not a guarantee against OLED image retention. Normal display-power-off policies should remain enabled.
