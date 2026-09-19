# Full-body animation pass

The procedural rigs now coordinate feet, knees, elbows, hands, torso, head,
expression and secondary motion. The eight existing characters and their
combat rules remain the foundation. The movement is authored animation driven
by the live simulation, not a ragdoll or a video played by the screensaver.

## Motion and reactions

- Three jab poses: right cross, left cross and rising punch.
- Three heavy poses: overhead strike, sweeping backfist and rising blow.
  CyberPengu keeps his hammer-driven overhead form.
- Three aerial poses: right kick, left kick and a tucked somersault.
- Full rolls with tucked hands and knees; jumping, falling and jet recovery poses.
- Separate wind-up, fast extension and follow-through, aligned to attack windows.
- Launch strength controls whole-body spin, flailing, facial shock and smoke.
  The first impact pose freezes during hit-stop, then the body starts travelling.
- Hard contact with a platform produces a braced, compressed landing and dust.
- Guard recoil, broken-shield dizziness with orbiting stars, near-miss surprise,
  pickup smiles, challenge reactions, entrances, victory performances and defeat.
- A signature taunt for each species, plus beckoning and slow applause. Taunts
  are available in safe gaps and after knockouts, with a cooldown. Nearby
  opponents and incoming projectiles interrupt them; a hit cancels the action.

Hands retain their equipment throughout the animation. Legs connect the moving
hips to independently posed feet. Eye shape, brows, mouth opening, head tracking,
ears, gills, tails and scarf movement make reactions readable in closeups.

`src/animation.cpp` owns the choreography and pose blending. `src/models.cpp`
draws the joint targets. `src/battle.cpp` owns event triggers, gesture selection,
spin impulses and timing. `src/director.cpp` applies the world-space rotation
and draws effects at actual contact points. Animation does not consume combat
random numbers. Action choices remain seeded and deterministic at 120 Hz.

Root rotation is kept as an unwrapped angle and settles to an equivalent upright
angle. It never lerps from 2π back to zero. Root and joint transforms interpolate
between simulation ticks together, including during changes in render cadence.
Respawn resets the old tumble and reactions. Effects retain the existing bounded
pools. Reduced motion still runs the simulation at half speed with fixed framing.

## Inspect and reproduce

```sh
# Regular autonomous match, including contextual reactions and taunts.
./build/prism-brawl --seed 41 --fighters 8 --windowed

# A 38.4-second repeating animation inspection reel for any fighter, ID 0..7.
./build/prism-brawl --showcase 4 --animation-demo --mute

# The hard launch and landing portion of that reel.
./build/prism-brawl --showcase 4 --animation-demo --start 21.6 --mute

# Capture the combined battle / closeup preview, 1280x720 at 60 fps.
python3 tools/render_animation_preview.py
```

The preview's first and last clips are actual autonomous combat. The five
middle clips use the explicit animation inspection mode: hammer salute,
sunglasses taunt, hard launch/landing, dodge roll, and victory wave. It is a
20-second offline export, not a benchmark. The ordinary showcase without
`--animation-demo` remains an idle character inspection view.

## Validation — 2026-09-19

The release was built using the machine's unchanged Portage `MAKEOPTS` of
`-j16 -l16`. All four CTest checks passed. New checks exercise hit-stop,
opposite-direction launch rotation, torso/limb reactions, pose interpolation,
roll continuity, hard landings, broken-shield reactions, interruptible taunts,
distinct character gestures and finite transforms across every reel segment.
The release and AddressSanitizer/UndefinedBehaviorSanitizer builds each passed
1,172,848 assertions; most are repeated finite-transform and state checks.

The stress run covered 48 configurations (automatic and all fixed arenas,
2/4/8 fighters, four seeds), totaling **24 simulated hours**: 10,368,000 ticks,
73,138 hits, 9,524 knockouts, 12,957 taunts, 52,200 tumbling launches and 83,496
hard landings. All transient pools remained bounded; root rotations and joint
targets stayed finite. This is not a 24-hour GPU soak test.

Actual-renderer pose, battle, and portrait captures and the 60 fps reel were
rendered through SDL/X11 on AMD Radeon Graphics, OpenGL 4.6 / Mesa 26.1.8.
Visual review covered all eight signatures, launch/flail/landing poses,
equipment attachment and 4/8-fighter arena composition. Native Wayland lifecycle
and monitor hotplug behavior were not changed or revalidated in this pass.

Evidence is in `validation/animation/`: `ctest.log`, `unit-tests.log`,
`stress.json`, `asan-build.log`, `asan-tests.log`, `video-render.log`,
`pose-review.jpg`, and the individual renderer captures.
