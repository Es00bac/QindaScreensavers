# Refinement 1.2 — combat and integrated levels

The design rule is that a level is a place the route travels through, not a
road floating in front of unrelated props. Prism Knot deliberately remains the
suspended, otherworldly exception.

## Level design

Bliss uses one continuously graded terrain mesh, fitted banked verges and
authored ravine bridges. Reclaimed CRTs and circuit boards sit in soil; large
terminal frames provide open drive-through apertures. Ferns, computer ruins
and distant infrastructure follow the wallpaper's nature-reclaiming-technology
theme. The wallpaper itself is not copied or redistributed.

The city route connects high skyways, lower industrial sections and enclosed
depot underpasses. Four true over/under crossings and a 34 m missing road segment
form its main spatial structure. Cable-stayed launch and landing pylons frame
the jump; road slabs, piers and foundations connect the highway to the district.
Buildings use five massing families and four generated facade materials:
residential balconies, factories, stepped glass offices, twin concrete wings
and utility towers. Street frontage, roof plant and ground traffic establish
scale. There are no freestanding, bottomless building boxes.

The garden and aurora routes now also receive graded terrain and fitted verges,
rather than sharing the city's floating skyline. Their materials, planted
structures and ice formations follow each level's theme.

## Clearance contract

`scenery.cpp` samples the complete banked road at approximately one metre.
The reserved corridor includes road width, side clearance, headroom, underside
and extra jump height. Building placement reserves podiums, balconies and roof
equipment together. A too-tall plot may become a safe low-rise block; otherwise
the plot stays empty.

Bridge piers and attached structures exclude only their own nearby road span
from the test. They must remain clear of all other levels. Tunnel bores are
deliberate empty space inside separately modeled walls and roofs, not roads
driven through a solid building. Terrain has a shared height function used by
both scene construction and falling karts.

`prism-scenery-tests` checks multiple viewpoints over all five courses and
three seeds, checks facade geometry stays inside its reserved silhouette, and
independently measures the city's four crossings. This is a regression test,
not a mathematical proof over every possible configuration.

## Combat and character motion

The fixed-step simulation now owns projectile and trap lifetimes, target
selection, damage, spinouts, knockouts, jumping and rescue phases. The renderer
shows those states instead of inventing unrelated crash effects. Tow-Bot
approaches, hooks, reels up, carries, waits for traffic and lowers its payload.
Short rejoin protection avoids immediate repeat takedowns.

Per-driver proportions and shared wheelbases are separate. Hand endpoints
derive from the same steering-wheel transform; taunts temporarily release a
hand. Suspension, hit reactions, airborne tumbles, face direction and appendages
remain analytic articulation, not a skeletal asset pipeline.

Spoke/tread contrast attenuates at speed to avoid temporal reversal. Wheel
distance is accumulated from forward motion, not position corrections.

## Cameras

The deterministic director scores recent events and nearby battles, rotates
coverage among all eight racers, holds shots, and stays with recovery sequences.
Trackside stations have fixed camera positions and pan toward the pack.
Cockpits share the animated chassis frame. Camera rays are checked against
scenery bounds, with a clear-view fallback when a building or terrain blocks
the desired angle.

## Source and deployment scope

Source version is 1.2.0. Generated atlas assets are embedded by Qt resources.
All edits are local to this workspace. No remote push, Portage install, session
configuration change, or suite source-lock publication is part of this update.
The shared QindaQt fullscreen/audio implementation remains in `../common/`.
