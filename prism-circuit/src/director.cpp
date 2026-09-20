// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
namespace sw {
namespace {
void line(Frame &f, V3 a, V3 b, float radius, V3 color, float emission = 0) {
    add(f, Cylinder, segment(a, b, radius), {color, emission, .3f, .65f});
}
void panel(Frame &f, M4 m, V3 p, V3 s, V3 color, float emission = 0, float mode = 0) {
    add(f, Box, m * translate(p) * scale(s), {color, emission, .32f, .55f, mode});
}
V3 prism(float h) {
    return {.52f + .43f * std::cos(h * 2 * pi), .53f + .43f * std::cos((h - .333f) * 2 * pi),
            .54f + .43f * std::cos((h + .333f) * 2 * pi)};
}
void environment(Frame &f, const Track &track, double time) {
    const V3 dark{.022f, .038f, .060f}, steel{.15f, .21f, .29f};
    add(f, RoadSurface, M4::identity(), {{.1f, .2f, .3f}, .18f, .28f, .52f, 13});
    add(f, RoadShell, M4::identity(), {dark, 0, .38f, .68f});
    // Continuous edge lights come from the road shader. Physical rails and ribs add depth.
    int n = int(track.length / 3.4);
    for (int i = 0; i < n; i++) {
        double s = track.length * i / n;
        auto a = track.at(s), b = track.at(s + track.length / n);
        if (track.jumpStart >= 0 && track.signedGap(s, track.jumpStart) > -2 &&
            track.signedGap(s, track.jumpStart) < track.jumpLength + 1)
            continue;
        if (length(a.position - f.eye) > 175)
            continue;
        V3 hue = track.kind == 0   ? prism(float(s / track.length) + .09f)
                 : track.kind == 1 ? V3{.20f, .55f, .30f}
                 : track.kind == 2 ? V3{.24f, .68f, .83f}
                 : track.kind == 3 ? V3{.45f, .52f, .22f}
                                   : V3{.10f, .68f, .73f};
        for (int side : {-1, 1}) {
            V3 p = a.position + a.right * (side * (track.halfWidth + .08f)),
               q = b.position + b.right * (side * (track.halfWidth + .08f));
            line(f, p + a.up * .32f, q + b.up * .32f, .075f, hue, 1.55f);
            if (i % 3 == 0) {
                line(f, p - a.up * .40f, p + a.up * .37f, .085f, steel);
            }
        }
        if (i % 5 == 0) {
            auto m = a.matrix(0, -.90f);
            panel(f, m, {0, 0, 0}, {track.halfWidth + .25f, .11f, .24f}, steel);
            for (int side : {-1, 1})
                panel(f, m, {side * track.halfWidth, 0, 0}, {.27f, .22f, .33f}, dark);
        }
    }
    // Six lane-specific boost pads have a fixed position shared with the simulation.
    for (const auto &pad : track.pads) {
        auto p = track.at(pad.distance);
        if (length(p.position - f.eye) > 200)
            continue;
        auto m = p.matrix(pad.lane, .045f);
        panel(f, m, {0, 0, 0}, {1.52f, .025f, 2.30f}, {.02f, .09f, .14f});
        for (int j = 0; j < 3; j++) {
            float z = (j - 1) * 1.23f;
            line(f, point(m, {-1.1f, .045f, z - .45f}), point(m, {0, .045f, z + .32f}), .070f,
                 {.12f, .83f, 1.f}, 2.2f);
            line(f, point(m, {0, .045f, z + .32f}), point(m, {1.1f, .045f, z - .45f}), .070f,
                 {.12f, .83f, 1.f}, 2.2f);
        }
    }
    for (int sector = 0; sector < 4; sector++) {
        auto p = track.at(track.length * sector / 4);
        arch(f, p.matrix(), sector, time);
    }
    // Start / finish grid is actual geometry on the road, aligned to its bank.
    for (int row = 0; row < 2; row++)
        for (int col = 0; col < 16; col++) {
            auto m = track.at(row * .9).matrix((col - 7.5f) * .85f, .045f);
            panel(f, m, {0, 0, 0}, {.42f, .025f, .43f},
                  (col + row) % 2 ? V3{.72f, .82f, .88f} : V3{.01f, .018f, .035f});
        }
    if (track.kind >= 1) {
        courseScenery(f, track, time);
        return;
    }
    // Three dream-gates, suspended above a clear driving corridor. No opaque portal plane.
    for (int j = 0; j < 3; j++) {
        auto p = track.at(track.length * (.40 + j * .018));
        M4 m = p.matrix(0, 2.5f);
        float turn = .06f * std::sin(time * .12 + j);
        add(f, Torus,
            m * translate({0, 2.2f, 0}) * rz(turn) * rx(pi / 2) * scale({9.6f, .65f, 9.6f}),
            {{.20f, .16f, .34f}, 0, .27f, .8f});
        add(f, Torus,
            m * translate({0, 2.2f, .15f}) * rz(turn) * rx(pi / 2) * scale({9.35f, .16f, 9.35f}),
            {{.55f, .22f, 1.f}, 1.8f, .2f, .2f});
        for (int k = 0; k < 12; k++) {
            float a = k * 2 * pi / 12 + time * .07f * (j % 2 ? -1 : 1);
            M4 shard = m * translate({std::cos(a) * 10.45f, 2.2f + std::sin(a) * 10.45f, 0}) *
                       rz(a) * ry(.45f);
            add(f, Monolith, shard * scale({.38f, .48f, .28f}), {{.19f, .24f, .40f}, 0, .18f, .8f});
            add(f, Sphere, shard * translate({0, 0, .26f}) * scale({.07f, .30f, .07f}),
                {{.29f, .82f, 1.f}, 2, .3f, .2f});
        }
    }
    groundedSkyline(f, track);
    // Course-specific landmarks make each route feel like a place.
    for (int j = 0; j < 18; ++j) {
        double s = track.length * (j + .25) / 18;
        auto p = track.at(s);
        int side = j % 2 ? -1 : 1;
        auto m = p.matrix(side * (18.f + j % 3 * 3), -6);
        if (length(point(m, {0, 0, 0}) - f.eye) > 200)
            continue;
        if (track.kind == 1) { // Suspended botanical terraces and translucent leaf canopies.
            add(f, Cylinder, m * scale({5.f, 1.5f, 5.f}), {{.04f, .13f, .13f}, 0, .6f, .1f});
            line(f, point(m, {0, 0, 0}), point(m, {0, 12, 0}), .34f, {.10f, .21f, .20f});
            for (int k = 0; k < 5; ++k) {
                float a = k * 2 * pi / 5;
                auto leaf = m * translate({std::cos(a) * 3, 10.f + k * .65f, std::sin(a) * 3}) *
                            ry(a) * rz(.45f);
                add(f, Wing, leaf * scale({2.4f, 2.6f, 2.3f}),
                    {{.15f, .42f, .30f}, .18f, .22f, .15f});
                line(f, point(m, {0, 8, 0}),
                     point(m, {std::cos(a) * 6, 12.f + k * .65f, std::sin(a) * 6}), .045f,
                     {.25f, .85f, .60f}, .7f);
            }
        } else if (track.kind == 2) { // Glacial arches and crystalline ridges under the aurora.
            add(f, Primitive(Rock0 + j % 6), m * scale({6.f, 8.f + j % 3 * 3, 5.f}),
                {{.16f, .25f, .35f}, 0, .54f, .25f});
            for (int k = 0; k < 3; ++k)
                add(f, Monolith,
                    m * translate({float(k - 1) * 2, 8.f + k * 2, 0}) * rz((k - 1) * .19f) *
                        scale({1.1f, 3.f + k, 1.1f}),
                    {{.28f, .67f, .8f}, .15f, .12f, .48f, 5});
        }
    }
    // Distant orbital infrastructure, sparsely placed so the road keeps visual priority.
    for (int j = 0; j < 3; j++) {
        V3 pos{float(-185 + j * 190), float(-46 - j * 9), float(-240 + j * 52)};
        M4 m = translate(pos) * rx(.31f + j * .22f) * rz(.15f) * ry(float(time * .017));
        add(f, Torus, m * scale({29.f + j * 6, 1.5f, 29.f + j * 6}),
            {{.06f, .17f, .23f}, .08f, .34f, .75f});
        add(f, Torus, m * translate({0, .16f, 0}) * scale({28.5f + j * 6, .3f, 28.5f + j * 6}),
            {{.09f, .57f, .83f}, 1.1f, .3f, .2f});
    }
    // Floating iridescent architecture beside, not on, the racing line.
    for (int j = 0; j < 15; j++) {
        double s = track.length * (j + .55) / 15;
        auto p = track.at(s);
        int side = j % 2 ? -1 : 1;
        V3 pos = p.position + p.right * (side * 17.f) +
                 p.up * (5.f + 2.f * std::sin(float(time * .32 + j)));
        if (length(pos - f.eye) > 160)
            continue;
        M4 m = translate(pos) * ry(float(time * .10 + j)) * rz(.42f);
        add(f, Monolith, m * scale({.82f, 1.40f, .82f}), {{.12f, .16f, .26f}, 0, .24f, .8f, 5});
        add(f, Torus, m * rx(.65f) * scale({2.1f, .22f, 2.1f}), {prism(j / 15.f), 1.4f, .3f, .2f});
    }
}
} // namespace
void compose(Frame &f, const RaceState &race, const Track &track, const SceneOptions &opt) {
    for (auto &batch : f.batches)
        batch.instances.clear();
    f.scenerySolids.clear();
    f.time = race.time;
    f.motionTime = std::fmod(race.time, 8192.0);
    f.voyage = track.seed % 997;
    f.courseName = track.name();
    f.fade = 1;
    f.winner = race.winner;
    f.order = race.order;
    f.chapter = track.kind;
    if (opt.gallery) {
        int id = opt.galleryId;
        f.eye = {5.5f, 3.7f, 7.8f};
        f.target = {0, 1.14f, 0};
        f.up = {0, 1, 0};
        f.fov = .66f;
        f.pengu = {0, 1, 0};
        f.focus = id;
        f.position = id + 1;
        f.lap = 1;
        f.speed = 0;
        f.boost = 0;
        f.countdown = 0;
        add(f, Cylinder, translate({0, -.4f, 0}) * scale({3.25f, .40f, 3.25f}),
            {{.027f, .041f, .063f}, 0, .35f, .72f});
        add(f, Torus, translate({0, -.025f, 0}) * scale({3.19f, .25f, 3.19f}),
            {racerColors[id], 1.5f, .3f, .2f});
        f.courseName = "MODEL STUDIO";
        f.shotLabel = racerNames[id];
        f.map.clear();
        kart(f, ry(float(race.time) * .23f), id, race.time, 0, .15f, 0, 0);
        return;
    }
    frameCamera(f, race, track, opt);
    int focus = f.focus;
    // Fade a race reset through black. No visible grid teleport or new-course pop.
    if (race.finishedAt >= 0) {
        float age = float(race.time - race.finishedAt);
        f.fade = 1 - ease(8.6f, 10.8f, age);
    } else if (race.round > 0 && race.roundTime < 1.5)
        f.fade = ease(0, 1.3f, float(race.roundTime));
    environment(f, track, race.time);
    clearCamera(f, race, track, opt);
    for (const auto &trap : race.traps) {
        auto pose = track.at(trap.distance);
        M4 m = pose.matrix(trap.lane, .06f);
        if (length(point(m, {0, 0, 0}) - f.eye) > 160)
            continue;
        if (trap.item == Item::Oil) {
            add(f, Sphere, m * scale({1.7f, .023f, 2.35f}), {{.016f, .01f, .032f}, 0, .10f, .6f});
            for (int j = 0; j < 4; j++)
                add(f, Torus,
                    m * translate({(j - 1.5f) * .30f, .026f, 0}) *
                        scale({.45f + j * .18f, .07f, .75f + j * .22f}),
                    {{.18f, .10f, .25f}, .08f, .12f, .5f});
        } else {
            add(f, Cylinder, m * translate({0, .17f, 0}) * scale({.64f, .17f, .64f}),
                {{.11f, .07f, .04f}, 0, .4f, .7f});
            add(f, Torus, m * translate({0, .36f, 0}) * scale({.49f, .22f, .49f}),
                {{1, .26f, .06f}, 1.4f, .3f, .2f});
            for (int j = 0; j < 4; j++)
                add(f, Box,
                    m * ry(j * pi / 2) * translate({.52f, .22f, 0}) * scale({.25f, .05f, .10f}),
                    {{.6f, .43f, .14f}, 0, .3f, .7f});
        }
    }
    for (const auto &projectile : race.projectiles) {
        auto pose = track.at(projectile.distance);
        M4 m = pose.matrix(projectile.lane, 1.25f);
        V3 co = racerColors[projectile.owner];
        add(f, Sphere, m * scale({.34f, .30f, .66f}), {{.22f, .27f, .30f}, 0, .23f, .8f});
        add(f, Sphere, m * translate({0, 0, .55f}) * scale({.18f, .15f, .12f}), {co, 2, .2f, .1f});
        for (int side : {-1, 1})
            add(f, Wing, m * translate({side * .25f, 0, -.2f}) * scale({.14f, .18f, .19f}),
                {co, 0, .3f, .6f});
        for (int j = 0; j < 7; j++)
            add(f, Sphere,
                m * translate({0, 0, -.7f - j * .43f}) *
                    scale({.13f - j * .015f, .13f - j * .015f, .3f}),
                {co, (1 - j * .12f) * 2, .3f, 0});
    }
    for (const auto &event : race.events) {
        float age = float(race.time - event.time);
        if (age < 0 || age > 1.1f ||
            (event.kind != EventKind::Hit && event.kind != EventKind::Block &&
             event.kind != EventKind::Knockout))
            continue;
        Random rng(event.serial * 7919);
        V3 color = event.kind == EventKind::Block ? V3{.15f, .73f, 1} : V3{1, .45f, .09f};
        for (int j = 0; j < 16; j++) {
            V3 velocity{rng.range(-5, 5), rng.range(2, 6), rng.range(-5, 5)};
            V3 p = event.position + V3{0, .6f, 0} + velocity * age + V3{0, -4 * age * age, 0};
            float size = .11f * (1 - age / 1.1f) + .01f;
            add(f, Box, translate(p) * ry(age * 8 + j) * scale({size, size, size * 2.4f}),
                {color, 1.8f * (1 - age / 1.1f), .4f, .5f});
        }
        add(f, Torus,
            translate(event.position + V3{0, .6f, 0}) * scale({.6f + age * 4, .3f, .6f + age * 4}),
            {color, (1 - age / 1.1f) * 1.5f, .3f, .1f});
    }
    for (int box = 0; box < 24; ++box)
        if (race.crates[box] <= 0) {
            auto pose = track.at(track.length * (box / 3 + .35) / 8);
            auto m = pose.matrix((box % 3 - 1) * 4.1f, 1.25f + .2f * std::sin(race.time * 2 + box));
            V3 color = prism(box * .19f);
            if (length(point(m, {0, 0, 0}) - f.eye) > 180)
                continue;
            add(f, Box, m * ry(race.time * .8 + box) * scale({.54f, .54f, .54f}),
                {{.045f, .09f, .16f}, 0, .16f, .72f});
            for (int side : {-1, 1})
                add(f, Torus,
                    m * ry(race.time * .8 + box) * translate({0, side * .54f, 0}) *
                        scale({.48f, .15f, .48f}),
                    {color, 1.7f, .22f, .2f});
            add(f, Monolith, m * scale({.15f, .17f, .15f}), {color, 2.1f, .3f, .3f});
        }
    for (int i = 0; i < RacerCount; ++i) {
        const auto &car = race.cars[i];
        auto pose = track.at(car.distance);
        auto m = pose.matrix(car.lane, .8f);
        V3 co = racerColors[i];
        if (car.item != Item::None) {
            auto icon = m * translate({0, 3.7f, 0}) * ry(race.time * 2);
            V3 hue = int(car.item) == 1   ? V3{1, .6f, .13f}
                     : int(car.item) == 2 ? V3{.2f, .7f, 1}
                     : int(car.item) == 3 ? V3{.85f, .2f, 1}
                                          : V3{.2f, 1, .65f};
            add(f, Monolith, icon * scale({.27f, .25f, .27f}), {hue, 1.3f, .24f, .5f});
            add(f, Torus, icon * scale({.5f, .2f, .5f}), {hue, 1.5f, .2f, .2f});
        }
        if (car.shield > 0) {
            for (int k = 0; k < 3; ++k)
                add(f, Torus, m * translate({0, .6f, 0}) * rx(k * pi / 3) * scale({2.f, .20f, 2.f}),
                    {{.2f, .65f, 1}, .85f, .2f, .2f});
        }
        if (car.pulse > 0) {
            float radius = 2 + (1 - car.pulse / .85f) * 8;
            add(f, Torus, m * scale({radius, .24f, radius}), {co, car.pulse * 2, .2f, .2f});
        }
        if (car.magnet > 0) {
            for (int k = 0; k < 3; ++k)
                add(f, Torus,
                    m * translate({0, .2f, -1.8f - k * .8f}) * rx(pi / 2) *
                        scale({.65f + k * .22f, .12f, .65f + k * .22f}),
                    {{.23f, 1, .65f}, .85f, .2f, .2f});
        }
    }
    f.itemLabel = itemName(race.cars[focus].item);
    if (f.itemLabel.empty()) {
        const auto &c = race.cars[focus];
        f.itemLabel = c.shield > 0   ? "AEGIS ACTIVE"
                      : c.magnet > 0 ? "MAGNET DRIVE"
                      : c.boost > 0  ? "TURBO"
                                     : "";
    }

    for (int i = 0; i < RacerCount; i++) {
        const auto &c = race.cars[i];
        auto road = track.at(c.distance);
        M4 m = carMatrix(c, track);
        float wheel = float(std::fmod(c.wheelDistance / .48, 2 * pi));
        auto animation = kartAnimation(c, f.cockpit && i == f.focus);
        kart(f, m, i, race.time, wheel, c.steer, c.drift, c.boost, animation);
        if (c.mode == CarMode::Rescuing)
            rescueRobot(f, carPosition(c, track), road.up, road.forward, race.time, c.modeTime);
        if (c.invulnerable > 1 && c.mode == CarMode::Recovering) {
            add(f, Torus, road.matrix(c.lane, .35f) * scale({1.8f, .25f, 2.2f}),
                {{.13f, .85f, 1}, 1.3f, .25f, .2f});
        }
        if (c.damage > 35) {
            for (int j = 0; j < 7; j++) {
                float age = std::fmod(float(race.time) * 1.7f + j * .14f, 1.f),
                      size = .12f + age * .3f;
                V3 smoke = point(m, {.4f + age * .4f, .95f + age * 1.6f, -1.2f - age * 1.5f});
                add(f, Smoke, translate(smoke) * scale({size, size, size}),
                    {{.06f, .066f, .075f}, 0, .95f, 0, 25, (1 - age) * .32f});
            }
        }
        V3 pos = point(m, {0, 0, 0});
        f.mapX[i] = pos.x / 190;
        f.mapY[i] = pos.z / 190;
        // Gentle near-road drift sparks tied to wheel contact, never random screen flashes.
        if ((std::abs(c.drift) > .07f && c.speed > 15) || c.mode == CarMode::Spinout) {
            int side = c.drift > 0 ? -1 : 1;
            for (int j = 0; j < 4; j++) {
                float age = std::fmod(float(race.time) * 2.6f + j * .25f, 1.f);
                V3 spark =
                    point(m, {side * 1.22f, .08f + std::sin(age * pi) * .24f, -1.2f - age * 1.8f});
                add(f, Sphere, translate(spark) * scale({.035f, .035f, .09f}),
                    {{1, .50f, .12f}, (1 - age) * 2, .3f, 0});
            }
        }
    }
}
} // namespace sw
