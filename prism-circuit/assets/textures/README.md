# Generated material assets

Both assets were generated for this project with the **built-in image-generation tool**, not the API/CLI fallback. Final delivered images are 1254 × 1254 PNGs.
The original generated files were retained; project copies below are embedded
through Qt resources. The code samples each quadrant with an inset to avoid
atlas-edge bleeding and converts the image colors to linear light.

- `material-atlas.png`: worn gray paint, corroded steel, reclaimed moss/circuitry and wet asphalt.
- `facade-atlas.png`: concrete commercial, old brick industrial, glass curtain wall and service-panel facades.

Generated artwork is distributed under the project's GPL-3.0-or-later terms.
No proprietary game textures were used. The QindaBliss wallpaper was inspected
locally as visual inspiration for level design, not embedded as a background.

## Final material prompt

Use case: stylized-concept. Asset type: production game material texture atlas for an original cyberpunk kart racing game, not a concept painting. Generate a single square 2048x2048 texture atlas divided EXACTLY into FOUR equal edge-to-edge 1024x1024 quadrants, no margins or dividers. Top left: worn light gray painted metal, fine scratches, rubbed silver edges, a few small paint chips, neutral desaturated color so the shader can tint it per kart. Top right: heavily weathered dark steel with scattered copper-orange rust, grease streaks, tiny rivet scars, scratched paint remnants. Bottom left: lush moss, tiny green grass blades and small pale stones reclaiming broken circuitry, natural ground texture, restrained detail at multiple scales. Bottom right: dark charcoal wet asphalt, cracks, tire scuffs, subtle aggregate and patched tarmac, no road markings. Every quadrant is an orthographic flat albedo surface scan, evenly diffuse lit, no cast shadows, no perspective, no objects, no text, no logos, no UI, no borders. Tileable within each quadrant with subdued matching edge colors. Detailed physically convincing surface variation, coherent stylized realistic game aesthetic. No glowing colors baked into the materials.

## Final facade prompt

Use case: stylized-concept. Asset type: detailed production building-facade material atlas for a grounded, inhabited cyberpunk city in a 3D racing game. Generate one square texture atlas in four equal edge-to-edge quadrants, no padding or labels. Each quadrant is a straight-on orthographic seamless facade surface, flat diffuse lighting, no perspective or complete freestanding building. Top left: weathered gray concrete commercial facade with narrow structural ribs, deep rectangular window bays, several floors, a restrained mix of dim cyan glass and a few warm lit offices, drainage streaks. Top right: older industrial red-brown brick, steel-framed factory windows, copper oxidation, black vents, conduit, aging paint. Bottom left: dark blue-gray modern glass curtain wall, narrow steel mullions, subtle reflected neighboring architecture, varied unlit and softly warm lit rooms, understated and believable. Bottom right: charcoal metal service facade with panel seams, louvers, exhaust grilles, access hatches, rainfall grime, bolts and cable conduits. Strong material detail and practical scale, inhabited architecture, muted realistic cyberpunk palette. Avoid fluorescent pink barcode-like windows, giant neon stripes, sci-fi monoliths, floating structures, logos, signs, text, borders, sky, ground, people. This is a reusable facade surface atlas, not a scene illustration.
