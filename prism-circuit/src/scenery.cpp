// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"

namespace sw {
namespace {
void box(Frame &f, M4 m, V3 p, V3 size, Material mat) {
    add(f, Architecture, m * translate(p) * scale(size), mat);
}
void beam(Frame &f, M4 m, V3 a, V3 b, float r, Material mat) {
    add(f, Cylinder, m * segment(a, b, r), mat);
}
const Material steel{{.12f, .19f, .24f}, 0, .65f, .4f, 20, .60f};
const Material concrete{{.19f, .23f, .25f}, 0, .89f, .02f};
const Material dark{{.022f, .03f, .047f}, 0, .48f, .6f};
const Material cyan{{.13f, .86f, .83f}, 1.1f, .24f, .3f};
const Material amber{{.82f, .42f, .12f}, .9f, .4f, .2f};
float groundLevel(const Track &tr) {
    return tr.kind == 4 ? -62.f : tr.kind == 3 ? -42.f : -75.f;
}

struct CorridorSample {
    V3 center, half;
    double distance;
    TrackPose pose;
};
const std::vector<CorridorSample> &corridor(const Track &tr) {
    // One-metre samples reserve the banked deck, headroom and full jump arc.
    static thread_local std::uint64_t seed = 0;
    static thread_local int kind = -1;
    static thread_local std::vector<CorridorSample> samples;
    if (seed == tr.seed && kind == tr.kind)
        return samples;
    seed = tr.seed;
    kind = tr.kind;
    samples.clear();
    int n = int(std::ceil(tr.length));
    samples.reserve(n);
    for (int i = 0; i < n; i++) {
        double s = tr.length * i / n;
        auto p = tr.at(s);
        float high = 12;
        if (tr.jumpStart >= 0 &&
            std::abs(tr.signedGap(s, tr.jumpStart + tr.jumpLength * .5)) < tr.jumpLength * .5 + 8)
            high = 17;
        V3 half{}, center = p.position + p.up * ((high - 3) * .5f);
        for (int side : {-1, 1})
            for (float h : {-3.f, high}) {
                V3 d = p.position + p.right * (side * (tr.halfWidth + 2.5f)) + p.up * h - center;
                half.x = std::max(half.x, std::abs(d.x) + 1.2f);
                half.y = std::max(half.y, std::abs(d.y) + 1.2f);
                half.z = std::max(half.z, std::abs(d.z) + 1.2f);
            }
        samples.push_back({center, half, s, p});
    }
    return samples;
}

bool naturalCourse(const Track &tr) {
    return tr.kind >= 1 && tr.kind <= 3;
}
bool landBridge(const Track &tr, double distance) {
    float u = float(distance / tr.length);
    return tr.kind == 3   ? ((u > .18f && u < .245f) || (u > .64f && u < .70f))
           : tr.kind == 1 ? ((u > .93f || u < .075f) || (u > .47f && u < .54f))
                          : u > .23f && u < .30f;
}
float bridgeBlend(const Track &tr, double distance) {
    float u = float(distance / tr.length);
    auto span = [&](float a, float b) {
        return ease(a - .023f, a + .012f, u) * (1 - ease(b - .012f, b + .023f, u));
    };
    if (tr.kind == 3)
        return std::max(span(.18f, .245f), span(.64f, .70f));
    if (tr.kind == 1)
        return std::max({span(.47f, .54f), 1 - ease(.063f, .098f, u), ease(.907f, .942f, u)});
    return span(.23f, .30f);
}
float landscapeHeight(const Track &tr, float x, float z) {
    if (!naturalCourse(tr))
        return groundLevel(tr);
    float r = std::sqrt(x * x + z * z);
    float h =
        -42 + 16 * std::sin(x * .014f) * std::cos(z * .011f) + 7 * std::sin(z * .029f + x * .009f);
    h += ease(230, 530, r) * (52 + 30 * std::sin(x * .008f + z * .006f));
    float nearest = 100000, bed = 0, ceiling = 100000, bridge = 0, weight = 0;
    // Grade the terrain to the roadbed. Only authored ravines and upper-level
    // crossings are viaducts; the rest of the road is part of the landscape.
    for (const auto &p : corridor(tr)) {
        float dx = x - p.center.x, dz = z - p.center.z;
        if (std::abs(dx) > 85 || std::abs(dz) > 85)
            continue;
        float d = std::sqrt(dx * dx + dz * dz);
        if (d >= 85)
            continue;
        V3 right = p.pose.right;
        float lateral =
            (dx * right.x + dz * right.z) / std::max(.4f, right.x * right.x + right.z * right.z);
        V3 forward = p.pose.forward;
        float along = (dx * forward.x + dz * forward.z) /
                      std::max(.4f, forward.x * forward.x + forward.z * forward.z);
        float surface = p.pose.position.y + right.y * lateral + forward.y * along - 1.35f;
        nearest = std::min(nearest, d);
        float w = std::pow(1 - d * d / (85 * 85), 4);
        bed += surface * w;
        bridge += bridgeBlend(tr, p.distance) * w;
        weight += w;
        ceiling = std::min(ceiling, surface - 1.0f + std::max(0.f, d - 11) * 2.8f);
    }
    if (weight > 0) {
        bed /= weight;
        bridge /= weight;
        h = h + (bed - h) * (1 - ease(15, 85, nearest)) * (1 - bridge);
    }
    h = std::min(h, ceiling);
    return h;
}

struct Building {
    V3 base;
    float width, depth, height;
    int style;
    ScenerySolid bounds;
};
const std::vector<Building> &buildings(const Track &tr) {
    static thread_local std::uint64_t seed = 0;
    static thread_local int kind = -1;
    static thread_local std::vector<Building> result;
    if (seed == tr.seed && kind == tr.kind)
        return result;
    seed = tr.seed;
    kind = tr.kind;
    result.clear();
    Random rng(seed + 91873);
    int n = tr.kind == 4 ? 9 : 7;
    for (int x = -n; x <= n; x++)
        for (int z = -n; z <= n; z++) {
            float px = x * 52.f + 26, pz = z * 52.f + 26;
            if (tr.kind != 4 && px * px + pz * pz < 230 * 230)
                continue;
            if (tr.kind == 3 && pz > -250)
                continue;
            float width = rng.range(10, 16), depth = rng.range(10, 16);
            int style = int(rng.next() % 5);
            float height = rng.range(38, 110) + (style == 2 ? 65 : 0);
            if (tr.kind != 4)
                height *= tr.kind == 3 ? .55f : .63f;
            float soil = landscapeHeight(tr, px, pz);
            if (naturalCourse(tr))
                for (int sx : {-1, 1})
                    for (int sz : {-1, 1})
                        soil = std::min(soil, landscapeHeight(tr, px + sx * (width + 3),
                                                              pz + sz * (depth + 3)));
            V3 base{px, soil - 0.1f, pz};
            // The reserved silhouette includes podiums, balconies, signs and antennas.
            auto bound = [&](float h) {
                return ScenerySolid{base + V3{0, (h + 16) * .5f, 0},
                                    {width + 3.8f, (h + 16) * .5f, depth + 3.8f}};
            };
            if (!trackClearance(tr, bound(height))) {
                height = 18;
                style = 1;
            }
            if (!trackClearance(tr, bound(height)))
                continue;
            result.push_back({base, width, depth, height, style, bound(height)});
        }
    return result;
}

void building(Frame &f, const Building &b) {
    f.scenerySolids.push_back(b.bounds);
    V3 center = b.base + V3{0, b.height * .5f, 0};
    if (length(center - f.eye) > 830)
        return;
    M4 m = translate(b.base);
    float w = b.width, d = b.depth, h = b.height;
    Material stone{{.29f, .33f, .36f}, 0, .75f, .1f, 14.0f};
    Material brick{{.40f, .30f, .23f}, 0, .85f, .05f, 14.1f};
    Material glass{{.20f, .36f, .43f}, 0, .25f, .55f, 14.2f};
    Material service{{.18f, .23f, .26f}, 0, .8f, .25f, 14.3f};
    // Sidewalk/plinth, occupied street frontage, then a stepped podium.
    box(f, m, {0, .20f, 0}, {w + 3, .20f, d + 3}, concrete);
    box(f, m, {0, 4.2f, 0}, {w + 1.4f, 4, d + 1.4f}, b.style == 1 ? brick : stone);
    box(f, m, {0, 8.4f, 0}, {w + 1.7f, .22f, d + 1.7f}, concrete);
    if (b.style == 0) { // Residential balconies and a separate service core.
        box(f, m, {0, (h + 8.6f) * .5f, 0}, {w * .80f, (h - 8.6f) * .5f, d * .84f}, stone);
        box(f, m, {-w * .76f, h * .5f, 0}, {w * .21f, h * .5f, d * .9f}, service);
        for (float y = 13; y < h - 2; y += 8) {
            box(f, m, {w * .2f, y, d * .88f}, {w * .62f, .16f, 1.45f}, concrete);
            box(f, m, {w * .2f, y + .85f, d * .88f + 1.25f}, {w * .62f, .075f, .055f}, steel);
        }
    } else if (b.style == 1) { // Old brick factory, roof monitors and chimneys.
        box(f, m, {0, (h + 8) * .5f, 0}, {w, (h - 8) * .5f, d}, brick);
        for (int k = 0; k < 3; k++) {
            float x = (k - 1) * w * .64f;
            box(f, m, {x, h + 1.1f, 0}, {w * .30f, 1.1f, d * .93f}, service);
            box(f, m, {x, h + 2.23f, 0}, {w * .31f, .13f, d * .96f}, steel);
        }
        for (int k = 0; k < 2; k++)
            beam(f, m, {-w * .68f + k * 2.5f, h, -d * .6f},
                 {-w * .68f + k * 2.5f, h + 11 - k * 3, -d * .6f}, .75f, brick);
    } else if (b.style == 2) { // Asymmetric three-tier glass headquarters.
        box(f, m, {0, h * .32f + 4, 0}, {w * .94f, h * .32f - 4, d * .93f}, glass);
        box(f, m, {-w * .12f, h * .75f, 0}, {w * .76f, h * .11f, d * .77f}, glass);
        box(f, m, {-w * .24f, h * .93f, -d * .08f}, {w * .53f, h * .07f, d * .59f}, glass);
        for (int side : {-1, 1})
            box(f, m, {side * w * .95f, h * .32f, 0}, {.24f, h * .32f, d * .97f}, concrete);
        for (float y = 17; y < h * .64f; y += 16)
            box(f, m, {0, y, 0}, {w * .965f, .16f, d * .945f}, steel);
        box(f, m, {-w * .24f, h + .22f, -d * .08f}, {w * .55f, .22f, d * .61f}, cyan);
    } else if (b.style == 3) { // Twin concrete wings and recessed connecting floors.
        for (int side : {-1, 1}) {
            box(f, m, {side * w * .60f, (h + 8) * .5f, 0}, {w * .38f, (h - 8) * .5f, d * .86f},
                stone);
            box(f, m, {side * w * .60f, h + .4f, 0}, {w * .40f, .4f, d * .9f}, concrete);
        }
        box(f, m, {0, h * .43f, 0}, {w * .28f, h * .43f, d * .66f}, glass);
        for (float y = 14; y < h * .85f; y += 18)
            box(f, m, {0, y, d * .74f}, {w * .27f, 1.5f, 1.5f}, service);
    } else { // Utilities building with a narrower upper plant room.
        box(f, m, {0, h * .38f + 4, 0}, {w * .91f, h * .38f - 4, d * .90f}, service);
        box(f, m, {-w * .25f, h * .88f, 0}, {w * .52f, h * .12f, d * .68f}, stone);
        for (float y = 13; y < h * .76f; y += 12) {
            box(f, m, {0, y, 0}, {w * .96f, .28f, d * .95f}, concrete);
            box(f, m, {w * .92f, y + 2, 0}, {.13f, 1.35f, d * .7f}, glass);
        }
    }
    if (length(center - f.eye) > 380)
        return;
    box(f, m, {w * .24f, h + 1.9f, -d * .15f}, {2.4f, 1.8f, 2.8f}, service);
    for (int k = 0; k < 3; k++)
        box(f, m, {w * .24f + (k - 1) * 1.5f, h + 3.74f, -d * .15f}, {.5f, .07f, 2.3f}, dark);
    beam(f, m, {-w * .30f, h, -d * .4f}, {-w * .30f, h + 13, -d * .4f}, .14f, steel);
    add(f, Sphere, m * translate({-w * .30f, h + 13, -d * .4f}) * scale({.16f, .16f, .16f}), amber);
    for (int side : {-1, 1}) {
        box(f, m, {side * w * .52f, 2.1f, d + 1.43f}, {w * .24f, 1.7f, .04f}, glass);
        box(f, m, {side * w * .52f, 4.3f, d + 1.6f}, {w * .26f, .22f, .26f},
            side == 1 ? cyan : amber);
        box(f, m, {side * w * .93f, 1.1f, d + 2.0f}, {.13f, 1.1f, .13f}, steel);
    }
}

void piers(Frame &f, const Track &tr, int count, float ground) {
    for (int i = 0; i < count; i++) {
        double s = tr.length * (i + .25) / count;
        auto p = tr.at(s);
        if (tr.jumpStart >= 0 &&
            std::abs(tr.signedGap(s, tr.jumpStart + tr.jumpLength * .5)) < tr.jumpLength * .5 + 5)
            continue;
        if (length(p.position - f.eye) > 340)
            continue;
        for (int side : {-1, 1}) {
            V3 top = p.position + p.right * (side * 5.65f) - V3{0, 1, 0};
            float y = naturalCourse(tr) ? landscapeHeight(tr, top.x, top.z) : ground;
            V3 base{top.x, y, top.z};
            if (top.y <= y + 2)
                continue;
            ScenerySolid bound{(top + base) * .5f, {1.45f, (top.y - y) * .5f, 1.45f}, s};
            if (!trackClearance(tr, bound))
                continue;
            f.scenerySolids.push_back(bound);
            box(f, M4::identity(), bound.center, {.8f, bound.half.y, .9f}, concrete);
            box(f, M4::identity(), base + V3{0, .4f, 0}, {1.4f, .4f, 1.4f}, concrete);
            box(f, M4::identity(), top - V3{0, .35f, 0}, {1.35f, .35f, 1.3f}, steel);
        }
    }
}

void reclaimed(Frame &f, const Track &tr, double time) {
    add(f, Terrain, M4::identity(), {{.12f, .28f, .04f}, 0, .96f, 0, 21});
    if (tr.kind == 3)
        for (const auto &b : buildings(tr))
            building(f, b);
    piers(f, tr, 90, 0);
    Random rng(tr.seed + 81347);
    for (int i = 0; i < 90; i++) {
        auto p = tr.at(tr.length * i / 90);
        int side = i % 2 ? 1 : -1;
        V3 pos = p.position + p.right * (side * rng.range(25, 56));
        pos.y = landscapeHeight(tr, pos.x, pos.z);
        ScenerySolid bound{pos + V3{0, 3, 0}, {5, 3, 5}};
        if (!trackClearance(tr, bound))
            continue;
        f.scenerySolids.push_back(bound);
        if (length(pos - f.eye) > 180)
            continue;
        M4 m = translate(pos) * ry(rng.range(-pi, pi));
        Material shell{{.48f, .53f, .39f}, 0, .8f, .38f, 20, .85f};
        // Half-buried CRT terminals and circuit-board beds are anchored in soil.
        M4 screen = m * translate({0, 1.2f, 0}) * rz(.13f) * rx(-.12f);
        box(f, screen, {0, 0, 0}, {1.9f, 1.5f, 1.1f}, shell);
        box(f, screen, {0, .05f, 1.12f}, {1.6f, 1.17f, .05f}, dark);
        box(f, screen, {0, .05f, 1.19f}, {1.35f, .95f, .018f},
            Material{{.018f, .09f, .049f}, .07f, .17f, .25f});
        for (int k = 0; k < 4; k++)
            box(f, screen, {.80f + k * .18f, -1.30f, 1.14f}, {.035f, .09f, .02f}, steel);
        M4 board = m * translate({2.5f, .22f, 1}) * rx(-pi / 2);
        box(f, board, {0, 0, 0}, {1.25f, 1.5f, .045f}, Material{{.03f, .15f, .065f}, 0, .85f, .1f});
        for (int j = 0; j < 5; j++) {
            float x = (j - 2) * .43f;
            box(f, board, {x, 0, .12f}, {.13f, .22f, .08f}, dark);
            beam(f, board, {x, -1.3f, .06f}, {x, 1.3f, .06f}, .02f,
                 Material{{.43f, .33f, .10f}, 0, .6f, .5f});
            float sway = .09f * std::sin(float(time) * 1.3f + i + j);
            beam(f, m, {x - 2.4f, 0, 0}, {x - 2.4f + sway, 1.5f, 0}, .025f,
                 Material{{.18f, .28f, .035f}, 0, .85f, 0});
            add(f, Wing,
                m * translate({x - 2.4f, 1, 0}) * rz(.6f + sway) * scale({.14f, .19f, .14f}),
                Material{{.19f, .33f, .035f}, 0, .9f, 0});
        }
    }
}

void attachedBox(Frame &f, const Track &tr, double distance, M4 m, V3 center, V3 size,
                 Material mat) {
    V3 low{1e9f, 1e9f, 1e9f}, high{-1e9f, -1e9f, -1e9f};
    for (int x : {-1, 1})
        for (int y : {-1, 1})
            for (int z : {-1, 1}) {
                V3 p = point(m, center + V3{x * size.x, y * size.y, z * size.z});
                low = {std::min(low.x, p.x), std::min(low.y, p.y), std::min(low.z, p.z)};
                high = {std::max(high.x, p.x), std::max(high.y, p.y), std::max(high.z, p.z)};
            }
    ScenerySolid solid{(low + high) * .5f, (high - low) * .5f, distance};
    if (!trackClearance(tr, solid))
        return;
    f.scenerySolids.push_back(solid);
    box(f, m, center, size, mat);
}

void valleyLandmarks(Frame &f, const Track &tr, double time) {
    for (double u : {.315, .77}) {
        double s = tr.length * u;
        auto p = tr.at(s);
        M4 m = p.matrix();
        if (length(p.position - f.eye) > 260)
            continue;
        Material shell{{.42f, .47f, .34f}, 0, .86f, .2f, 20, .79f};
        // An abandoned terminal forms a drive-through landmark: an open screen
        // aperture, never an opaque box intersecting the driving surface.
        for (int side : {-1, 1}) {
            attachedBox(f, tr, s, m, {side * 11.5f, 4.9f, 0}, {2.7f, 5.5f, 3.2f}, shell);
            attachedBox(f, tr, s, m, {side * 9.0f, 4.6f, 3.23f}, {.19f, 4.5f, .2f}, dark);
            for (int j = 0; j < 4; j++)
                attachedBox(f, tr, s, m, {side * 12.2f, 2.2f + j * 1.1f, 3.25f},
                            {1.15f, .17f, .04f}, dark);
        }
        attachedBox(f, tr, s, m, {0, 10.25f, 0}, {14.2f, 1.15f, 3.2f}, shell);
        attachedBox(f, tr, s, m, {0, 9.12f, 3.25f}, {9, .18f, .20f}, dark);
        attachedBox(f, tr, s, m, {8.9f, 10.3f, 3.24f}, {.18f, .18f, .08f}, cyan);
        for (int j = 0; j < 9; j++) {
            float x = (j - 4) * 2.0f;
            beam(f, m, {x, 11.35f, -1}, {x + .2f, 8.5f - std::sin(float(j)) * 1.8f, 3.5f}, .045f,
                 Material{{.08f, .2f, .025f}, 0, .9f, 0});
            add(f, Wing,
                m * translate({x, 10.3f, 3.2f}) * rz(.4f + .05f * std::sin(time + j)) *
                    scale({.44f, .40f, .7f}),
                Material{{.16f, .33f, .04f}, 0, .9f, 0});
        }
    }
    // Ferns and small trees mark the actual verges, not a detached backdrop.
    Random rng(tr.seed + 3761);
    for (int i = 0; i < 150; i++) {
        auto p = tr.at(tr.length * i / 150);
        float lane = (i % 2 ? 1 : -1) * rng.range(14, 28);
        V3 pos = p.position + p.right * lane;
        pos.y = landscapeHeight(tr, pos.x, pos.z) - .3f;
        float h = rng.range(2.5f, 5.5f);
        ScenerySolid bound{pos + V3{0, h * .5f, 0}, {3.1f, h * .5f + 1, 3.1f}};
        if (!trackClearance(tr, bound))
            continue;
        f.scenerySolids.push_back(bound);
        if (length(pos - f.eye) > 155)
            continue;
        M4 m = translate(pos);
        beam(f, m, {0, 0, 0}, {0, h, 0}, .14f, Material{{.15f, .11f, .045f}, 0, .96f, 0});
        for (int j = 0; j < 5; j++) {
            M4 crown = m * translate({0, h * .70f, 0}) * ry(j * 2 * pi / 5) * rz(.25f);
            add(f, Wing, crown * scale({1.0f, .65f, 1.0f}),
                Material{{.11f, .27f, .025f}, 0, .92f, 0});
            add(f, Wing, crown * translate({0, h * .20f, 0}) * scale({.70f, .60f, .7f}),
                Material{{.21f, .36f, .055f}, 0, .92f, 0});
        }
    }
}

void naturalLevel(Frame &f, const Track &tr, double time) {
    if (tr.kind == 3) {
        reclaimed(f, tr, time);
        valleyLandmarks(f, tr, time);
        return;
    }
    add(f, Terrain, M4::identity(), {{.12f, .28f, .04f}, 0, .96f, 0, tr.kind == 2 ? 26.f : 21.f});
    piers(f, tr, 85, 0);
    Random rng(tr.seed + 29377);
    for (int i = 0; i < 90; i++) {
        auto p = tr.at(tr.length * i / 90);
        V3 pos = p.position + p.right * ((i % 2 ? 1 : -1) * rng.range(19, 43));
        pos.y = landscapeHeight(tr, pos.x, pos.z) - 1.5f;
        float h = rng.range(4, 11);
        ScenerySolid solid{pos + V3{0, h * .5f, 0}, {6, h, 6}};
        if (!trackClearance(tr, solid))
            continue;
        f.scenerySolids.push_back(solid);
        if (length(pos - f.eye) > 210)
            continue;
        M4 m = translate(pos);
        if (tr.kind == 2) {
            add(f, Primitive(Rock0 + i % 6),
                m * translate({0, h * .3f, 0}) * scale({5.5f, h * .6f, 5.5f}),
                Material{{.35f, .56f, .68f}, 0, .5f, .12f, 26});
            for (int j = 0; j < 3; j++)
                add(f, Monolith,
                    m * translate({float(j - 1) * 1.9f, h * .55f, 0}) * rz((j - 1) * .13f) *
                        scale({.8f, h * .19f, .9f}),
                    Material{{.10f, .38f, .52f}, .04f, .14f, .45f, 5});
        } else {
            box(f, m, {0, 1, 0}, {5, .8f, 5}, concrete);
            beam(f, m, {0, 1, 0}, {0, h, 0}, .25f, Material{{.11f, .19f, .09f}, 0, .9f, 0});
            for (int j = 0; j < 6; j++)
                add(f, Wing,
                    m * translate({0, h * .73f, 0}) * ry(j * pi / 3) * rz(.36f) *
                        scale({1.8f, 1.8f, 1.8f}),
                    Material{{.12f, .32f, .095f}, 0, .8f, .03f});
        }
    }
    // Garden conservatories / glacier braces frame the road at two crossings.
    for (double u : {.22, .71}) {
        double s = tr.length * u;
        auto p = tr.at(s);
        M4 m = p.matrix();
        if (length(p.position - f.eye) > 220)
            continue;
        Material finish = tr.kind == 2 ? Material{{.23f, .44f, .59f}, 0, .45f, .15f, 26} : steel;
        for (int side : {-1, 1})
            attachedBox(f, tr, s, m, {side * 10.5f, 5, 0}, {1.4f, 5.4f, 3}, finish);
        attachedBox(f, tr, s, m, {0, 10.4f, 0}, {11.9f, .65f, 3}, finish);
        for (int j = 0; j < 5; j++)
            beam(f, m, {-9, 9.7f, (j - 2) * 1.1f}, {9, 9.7f, (j - 2) * 1.1f}, .06f,
                 tr.kind == 2 ? cyan : amber);
    }
}

void city(Frame &f, const Track &tr, double time) {
    groundedSkyline(f, tr);
    for (int i = 0; i < 360; i++) {
        double s = tr.length * i / 360;
        auto p = tr.at(s);
        double fraction = s / tr.length;
        if (length(p.position - f.eye) > 230)
            continue;
        bool gap =
            tr.signedGap(s, tr.jumpStart) > -1 && tr.signedGap(s, tr.jumpStart) < tr.jumpLength + 1;
        if (gap)
            continue;
        M4 m = p.matrix();
        bool tunnel = (fraction > .53 && fraction < .62) || (fraction > .82 && fraction < .91);
        float halfSegment = float(tr.length / 720 + .2);
        attachedBox(f, tr, s, m, {0, -1.3f, 0}, {8.3f, .53f, halfSegment}, concrete);
        if (tunnel) {
            for (int side : {-1, 1}) {
                box(f, m, {side * 8.5f, 4.25f, 0}, {.45f, 4.4f, halfSegment}, steel);
                if (i % 2 == 0)
                    box(f, m, {side * 8.0f, 2.1f, 0}, {.035f, .10f, halfSegment * .8f}, cyan);
            }
            box(f, m, {0, 8.5f, 0}, {8.9f, .4f, halfSegment}, concrete);
            for (int side : {-1, 1})
                box(f, m, {side * 4.9f, 8.04f, 0}, {.13f, .06f, halfSegment * .85f}, amber);
            if (i % 3 == 0) {
                box(f, m, {0, 8, 0}, {8.4f, .18f, .24f}, steel);
                box(f, m, {0, 7.8f, 0}, {1.2f, .1f, .25f}, cyan);
            }
            // The underpass passes THROUGH a built depot, with an open bore. Its
            // outer walls and plant roofs join the expressway to the city fabric.
            for (int side : {-1, 1})
                attachedBox(f, tr, s, m, {side * 11.2f, 4.7f, 0}, {2.15f, 5.1f, halfSegment},
                            Material{{.27f, .25f, .24f}, 0, .82f, .1f, 14.1f});
            attachedBox(f, tr, s, m, {0, 10, 0}, {13.4f, 1.1f, halfSegment}, concrete);
            if (i % 4 == 0)
                attachedBox(f, tr, s, m, {4, 12, 0}, {3, .9f, 2},
                            Material{{.12f, .16f, .18f}, 0, .7f, .3f, 14.3f});
        } else if (i % 4 == 0) {
            for (int side : {-1, 1}) {
                beam(f, m, {side * 8.f, -.5f, 0}, {side * 8.f, 7.3f, 0}, .10f, steel);
                beam(f, m, {side * 8.f, 7.3f, 0}, {side * 5.8f, 7.3f, 0}, .10f, steel);
                box(f, m, {side * 5.8f, 7.25f, 0}, {.9f, .06f, .22f}, cyan);
            }
        }
    }
    for (double offset : {-4., tr.jumpLength + 4}) {
        auto p = tr.at(tr.jumpStart + offset);
        M4 m = p.matrix();
        for (int side : {-1, 1}) {
            box(f, m, {side * 9.2f, 7, 0}, {.60f, 7.6f, .7f}, steel);
            beam(f, m, {side * 9.2f, 14, 0}, {side * 7.8f, 0, -22}, .09f, cyan);
            beam(f, m, {side * 9.2f, 14, 0}, {side * 7.8f, 0, 22}, .09f, cyan);
            box(f, m, {side * 9.2f, 7, .75f}, {.13f, 6.1f, .025f}, amber);
        }
    }
    for (int i = 0; i < 6; i++) {
        M4 ramp = tr.at(tr.jumpStart - 3 - i * 2.2).matrix(0, .045f);
        for (int side : {-1, 1})
            beam(f, ramp, {side * 4.f, 0, -1}, {0, 0, 1}, .095f, amber);
    }
    // Ground traffic follows the same street grid as the occupied blocks.
    for (int lane = -6; lane <= 6; lane++)
        for (int k = 0; k < 4; k++) {
            float x = std::fmod(float(time) * (k % 2 ? 7 : -6) + k * 191.f + 10000, 720.f) - 360;
            M4 m = translate({x, -60.9f, lane * 52.f + (k % 2 ? 2.5f : -2.5f)}) *
                   ry(k % 2 ? pi / 2 : -pi / 2);
            if (length(point(m, {0, 0, 0}) - f.eye) > 330)
                continue;
            box(f, m, {0, 0, 0}, {.9f, .65f, 1.9f}, steel);
            box(f, m, {0, .8f, -.2f}, {.78f, .3f, .95f}, dark);
            for (int side : {-1, 1})
                box(f, m, {side * .6f, 0, 1.94f}, {.17f, .08f, .025f}, amber);
        }
}
} // namespace

bool trackClearance(const Track &tr, const ScenerySolid &box) {
    for (const auto &p : corridor(tr)) {
        if (box.attachedAt >= 0 && std::abs(tr.signedGap(p.distance, box.attachedAt)) < 24)
            continue;
        V3 d = p.center - box.center;
        if (std::abs(d.x) <= p.half.x + box.half.x && std::abs(d.z) <= p.half.z + box.half.z &&
            std::abs(d.y) <= p.half.y + box.half.y)
            return false;
    }
    return true;
}
float terrainElevation(const Track &tr, float x, float z) {
    return tr.kind == 0 ? -1000.f : landscapeHeight(tr, x, z);
}

void buildSceneryMeshes(Frame &f, const Track &tr) {
    Mesh terrain;
    terrain.name = "continuous-ground";
    int n = naturalCourse(tr) ? 340 : 1;
    float extent = naturalCourse(tr) ? 850 : 4000;
    for (int z = 0; z <= n; z++)
        for (int x = 0; x <= n; x++) {
            float px = (float(x) / n * 2 - 1) * extent, pz = (float(z) / n * 2 - 1) * extent;
            float h = landscapeHeight(tr, px, pz);
            V3 normal = unit({landscapeHeight(tr, px - 1, pz) - landscapeHeight(tr, px + 1, pz), 2,
                              landscapeHeight(tr, px, pz - 1) - landscapeHeight(tr, px, pz + 1)});
            terrain.v.push_back({{px, h, pz}, normal, {px * .02f, pz * .02f}});
        }
    for (int z = 0; z < n; z++)
        for (int x = 0; x < n; x++) {
            unsigned a = z * (n + 1) + x, b = a + n + 1;
            terrain.ix.insert(terrain.ix.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
    // A fitted road verge joins the banked edge to the graded terrain. It is
    // deliberately absent over ravines so those sections read as bridges.
    if (naturalCourse(tr))
        for (int side : {-1, 1}) {
            unsigned start = terrain.v.size();
            constexpr int segments = 1300, across = 5;
            for (int i = 0; i <= segments; i++) {
                auto p = tr.at(tr.length * i / segments);
                for (int j = 0; j <= across; j++) {
                    float u = float(j) / across;
                    V3 pos = p.position + p.right * (side * (tr.halfWidth + u * 10));
                    float soil = landscapeHeight(tr, pos.x, pos.z);
                    float fitted = pos.y - .18f + (soil - pos.y + .18f) * smooth(u);
                    float approach = 1 - smooth(bridgeBlend(tr, tr.length * i / segments) * 1.8f);
                    pos.y = soil + (fitted - soil) * approach;
                    terrain.v.push_back(
                        {pos, unit(mix(p.up, V3{0, 1, 0}, u)), {pos.x * .02f, pos.z * .02f}});
                }
            }
            for (int i = 0; i < segments; i++) {
                if (landBridge(tr, tr.length * (i + .5) / segments))
                    continue;
                for (int j = 0; j < across; j++) {
                    unsigned a = start + i * (across + 1) + j, b = a + across + 1;
                    if (side == 1)
                        terrain.ix.insert(terrain.ix.end(), {a, b, a + 1, a + 1, b, b + 1});
                    else
                        terrain.ix.insert(terrain.ix.end(), {a, a + 1, b, a + 1, b + 1, b});
                }
            }
        }
    // Fit lighting to the actual graded triangles, including steep verges;
    // using an upright normal there made cliffs look like stretched grass.
    std::vector<V3> normals(terrain.v.size());
    for (size_t i = 0; i < terrain.ix.size(); i += 3) {
        auto a = terrain.ix[i], b = terrain.ix[i + 1], c = terrain.ix[i + 2];
        V3 normal = cross(terrain.v[b].p - terrain.v[a].p, terrain.v[c].p - terrain.v[a].p);
        normals[a] = normals[a] + normal;
        normals[b] = normals[b] + normal;
        normals[c] = normals[c] + normal;
    }
    for (size_t i = 0; i < terrain.v.size(); ++i) terrain.v[i].n = unit(normals[i]);
    f.batches[Terrain].mesh = std::move(terrain);
}

void groundedSkyline(Frame &f, const Track &tr) {
    add(f, Terrain, M4::identity(), {{.06f, .08f, .11f}, 0, .7f, .1f, 22});
    for (const auto &b : buildings(tr))
        building(f, b);
    if (tr.kind != 0)
        piers(f, tr, tr.kind == 4 ? 90 : 60, groundLevel(tr));
}
void courseScenery(Frame &f, const Track &tr, double time) {
    if (tr.kind == 4)
        city(f, tr, time);
    else if (tr.kind > 0)
        naturalLevel(f, tr, time);
}
} // namespace sw
