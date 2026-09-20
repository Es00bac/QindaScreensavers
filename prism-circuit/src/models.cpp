// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
namespace sw {
namespace {
const Material carbon{{.021f, .031f, .048f}, 0, .30f, .65f};
const Material steel{{.21f, .29f, .36f}, 0, .24f, .84f};
const Material ceramic{{.81f, .86f, .87f}, 0, .24f, .18f};
const Material copper{{.76f, .32f, .12f}, 0, .27f, .75f};
const Material rubber{{.009f, .014f, .021f}, 0, .75f, .03f, 15};
const Material onyx{{.008f, .013f, .025f}, 0, .14f, .15f};
const Material cream{{.96f, .90f, .74f}, 0, .52f, .02f};
const Material iris{{.003f, .01f, .018f}, 0, .06f, .40f};
Material paint(V3 c) {
    return {c, 0, .26f, .42f};
}
Material glow(V3 c, float e = 2) {
    return {c, e, .23f, .2f};
}
void ell(Frame &f, M4 m, V3 p, V3 s, Material c) {
    add(f, Sphere, m * translate(p) * scale(s), c);
}
void plate(Frame &f, M4 m, V3 p, V3 s, Material c) {
    add(f, Box, m * translate(p) * scale(s), c);
}
void tube(Frame &f, M4 m, V3 a, V3 b, float r, Material c) {
    add(f, Cylinder, m * segment(a, b, r), c);
}
void ring(Frame &f, M4 m, V3 p, V3 s, Material c) {
    add(f, Torus, m * translate(p) * scale(s), c);
}
void badge(Frame &f, M4 m, float size, Material c) {
    ring(f, m * rx(pi / 2), {0, 0, 0}, {size, .35f, size}, c);
    tube(f, m, {size * .45f, -size * .55f, .02f}, {size * .95f, -size * 1.04f, .02f}, size * .11f,
         c);
}
} // namespace
M4 steeringWheel(float steer) {
    return translate({0, 1.31f, .57f}) * rx(1.06f) * ry(-steer * 2.6f);
}
V3 steeringGrip(int side, float steer) {
    return point(steeringWheel(steer), {side * .33f, 0, 0});
}
M4 kartChassis(double time, int id, float drift, const KartAnimation &a) {
    return translate(
               {0, float(.013f * std::sin(time * 6.5 + id)) - .12f * std::sin(a.landing * pi), 0}) *
           rz(-drift * .24f) * rx(a.hit * .07f * std::sin(float(time) * 25) + a.landing * .10f);
}
void animal(Frame &f, M4 m, int id, double time, float steering, const DriverAnimation &anim) {
    float t = float(std::fmod(time, 4096.0));
    const V3 accent = racerColors[id];
    Material coat = onyx;
    if (id == 1)
        coat = {{1.f, .62f, .12f}, 0, .46f, .02f};
    if (id == 2 || id == 6)
        coat = {{id == 2 ? .84f : .61f, .19f, .074f}, 0, .60f, .02f};
    if (id == 3)
        coat = {{.29f, .33f, .39f}, 0, .62f, .03f};
    if (id == 4)
        coat = {{.83f, .79f, .90f}, 0, .55f, .02f};
    if (id == 7)
        coat = {{1.f, .56f, .70f}, 0, .44f, .03f};
    const Material belly = id == 1 ? Material{{1.f, .82f, .35f}, 0, .5f, .01f} : cream;
    M4 body = rz(anim.lean) * rx(-anim.hit * .20f);
    if (!anim.cockpit) {
        ell(f, m * body, {0, .30f, 0}, {.52f, .59f, .42f}, coat);
        ell(f, m * body, {0, .22f, .34f}, {.40f, .40f, .17f}, belly);
    }
    // Tail silhouettes make the species readable from rear chase views.
    if (id == 2 || id == 3 || id == 6) {
        for (int k = 0; k < 6; k++) {
            float a = k / 5.f;
            V3 p{.25f + std::sin(a * 2.1f) * .44f, .13f + a * .36f, -.30f - a * 1.0f};
            Material color = coat;
            if (id == 3)
                color = k % 2 ? onyx : coat;
            if (id == 6)
                color = k % 2 ? cream : coat;
            if (id == 2 && k > 3)
                color = cream;
            ell(f, m * ry(.09f * std::sin(t * 1.5f)), p,
                {.22f * (1 - a * .35f), .23f * (1 - a * .3f), .22f}, color);
        }
    }
    if (id == 5) {
        for (int k = 0; k < 7; k++)
            ell(f, m,
                {.3f + float(k) * .045f, .10f + std::pow(k / 6.f, 2.f) * .7f, -.38f - k * .12f},
                {.09f, .12f, .12f}, coat);
    }
    // Racing suit and seated feet. Metallic buckles and piping remain non-emissive.
    for (int side : {-1, 1}) {
        plate(f, m, {side * .30f, .30f, .42f}, {.087f, .30f, .06f}, carbon);
        plate(f, m, {side * .30f, .57f, .41f}, {.099f, .063f, .061f}, copper);
        ell(f, m, {side * .25f, -.22f, .34f}, {.23f, .13f, .34f}, id < 2 ? copper : carbon);
        plate(f, m, {side * .25f, -.23f, .53f}, {.18f, .057f, .095f}, steel);
        V3 shoulder = point(body, {side * .48f, .50f, .01f});
        V3 hand = side < 0 ? anim.leftGrip : anim.rightGrip;
        if (side > 0 && anim.taunt > 0) {
            float gesture = std::sin(clamp(anim.taunt / 1.7f) * pi);
            hand = hand + V3{.24f * gesture, .65f * gesture, -.2f * gesture};
        }
        if (anim.airborne > .5f)
            hand = hand + V3{side * .26f, .45f + .16f * std::sin(t * 7 + side), -.18f};
        V3 upper = unit(hand - shoulder), out = unit(cross(upper, {0, 0, 1}));
        if (out.x * side < 0)
            out = -out;
        float half = length(hand - shoulder) * .5f;
        V3 elbow =
            (shoulder + hand) * .5f + out * std::sqrt(std::max(.012f, .50f * .50f - half * half));
        tube(f, m, shoulder, elbow, .12f, coat);
        ell(f, m, elbow, {.15f, .15f, .15f}, coat);
        tube(f, m, elbow, hand, .105f, carbon);
        ell(f, m, hand, {.14f, .12f, .15f}, carbon);
        // Separate knuckles, thumb and cuff read in the cockpit camera.
        for (int finger = 0; finger < 3; ++finger)
            ell(f, m, hand + V3{(finger - 1) * .065f, .071f, .055f}, {.037f, .053f, .069f}, steel);
        ell(f, m, hand + V3{-side * .105f, -.025f, .05f}, {.055f, .069f, .087f}, carbon);
        ell(f, m, elbow * .18f + hand * .82f, {.117f, .08f, .117f}, paint(accent * .5f));
    }
    plate(f, m, {0, .24f, .46f}, {.027f, .23f, .018f}, steel);
    badge(f, m * translate({-.25f, .43f, .492f}), .055f, glow(accent, .9f));
    float lean = .025f * std::sin(t * 2.1f + id);
    if (anim.cockpit)
        return;
    M4 head = m * body * translate({0, 1.08f + .012f * std::sin(t * 1.8f + id), 0}) *
              ry(steering * .45f + anim.look + anim.taunt * .18f) * rz(lean + anim.hit * .14f);
    ell(f, head, {0, 0, 0},
        id == 1   ? V3{.63f, .62f, .59f}
        : id == 6 ? V3{.72f, .66f, .62f}
                  : V3{.66f, .69f, .59f},
        coat);
    if (id == 0) {
        for (int side : {-1, 1})
            ell(f, head, {side * .225f, -.02f, .46f}, {.35f, .44f, .20f}, cream);
        ell(f, head, {0, -.27f, .48f}, {.42f, .24f, .18f}, cream);
    } else if (id == 2 || id == 6) {
        for (int side : {-1, 1}) {
            ell(f, head * rz(side * .25f), {side * .28f, -.16f, .45f}, {.35f, .25f, .21f}, cream);
            if (id == 6)
                ell(f, head, {side * .30f, .105f, .53f}, {.20f, .245f, .096f},
                    Material{{.32f, .08f, .034f}, 0, .6f, 0});
        }
    } else if (id == 3) {
        for (int side : {-1, 1})
            ell(f, head * rz(side * -.16f), {side * .26f, .015f, .486f}, {.33f, .245f, .15f}, onyx);
        ell(f, head, {0, -.24f, .52f}, {.26f, .20f, .15f}, cream);
    } else if (id == 4) {
        for (int side : {-1, 1})
            ell(f, head, {side * .19f, -.20f, .53f}, {.22f, .20f, .11f}, cream);
    } else if (id == 7) {
        for (int side : {-1, 1})
            ell(f, head, {side * .43f, -.13f, .49f}, {.16f, .10f, .05f},
                Material{{1.f, .26f, .49f}, 0, .5f, 0});
    }
    // Species-specific ears and gills are actual articulated geometry.
    for (int side : {-1, 1}) {
        if (id == 2 || id == 5) {
            M4 e = head * translate({side * .43f, .42f, -.05f}) *
                   rz(side * -.18f + std::sin(t * 1.8f + id) * .025f);
            add(f, Ear, e * scale({.72f, .51f, .86f}), coat);
            add(f, Ear, e * translate({0, .09f, .075f}) * scale({.43f, .36f, .62f}),
                id == 2 ? cream : paint({.26f, .12f, .28f}));
            tube(f, e, {-.16f, .27f, .18f}, {-.055f, .66f, .08f}, .017f, glow(accent, 1));
        } else if (id == 3 || id == 6) {
            ell(f, head, {side * .51f, .49f, -.035f}, {.23f, .27f, .13f}, cream);
            ell(f, head, {side * .51f, .49f, .071f}, {.145f, .185f, .04f}, coat);
        } else if (id == 4) {
            M4 e = head * translate({side * .28f, .46f, -.08f}) *
                   rz(-side * .17f + .045f * std::sin(t * 1.7f + side));
            ell(f, e, {0, .44f, 0}, {.22f, .74f, .17f}, coat);
            ell(f, e, {0, .48f, .13f}, {.115f, .49f, .047f}, paint({.70f, .31f, .53f}));
            tube(f, e, {0, .12f, .17f}, {0, .73f, .13f}, .017f, glow(accent, 1));
        } else if (id == 7) {
            for (int k = 0; k < 3; k++) {
                float a = (k - 1) * .48f + .05f * std::sin(t * 1.4f + k);
                V3 base{side * .54f, .10f, 0},
                    tip{side * (.89f + .08f * std::cos(a)), .12f + std::sin(a) * .68f, .08f};
                tube(f, head, base, tip, .065f, paint({.9f, .25f, .47f}));
                ell(f, head, tip, {.15f, .11f, .095f}, paint({1.f, .32f, .56f}));
                ell(f, head, tip + V3{side * .06f, 0, .07f}, {.04f, .037f, .025f},
                    glow(accent, 1.3f));
            }
        }
    }
    float phase = std::fmod(t + id * .68f, 5.8f),
          blink = phase < .21f ? std::pow(std::sin(phase * pi / .21f), 2.f) : 0;
    blink = std::max(blink, anim.hit * .7f);
    if (id != 1) {
        for (int side : {-1, 1}) {
            ell(f, head, {side * .25f, .045f, .665f}, {.16f, .199f, .073f}, iris);
            ell(f, head, {side * .25f - .043f, .112f, .731f}, {.035f, .046f, .018f}, cream);
            ell(f, head, {side * .25f + .024f, -.003f, .735f}, {.014f, .018f, .012f},
                glow(accent, .45f));
            if (blink > .002f)
                ell(f, head, {side * .25f, .236f - blink * .19f, .747f},
                    {.17f, .209f * blink, .022f}, id == 0 ? cream : coat);
            M4 brow = head * translate({side * .25f, .29f, .71f}) *
                      rz(side * (anim.hit > .1f ? -.22f : .16f + std::abs(anim.look) * .2f));
            plate(f, brow, {0, 0, 0}, {.18f, .035f, .035f}, coat);
        }
    } else {
        for (int side : {-1, 1}) {
            plate(f, head, {side * .265f, .085f, .553f}, {.275f, .225f, .087f}, copper);
            plate(f, head, {side * .265f, .09f, .631f}, {.232f, .178f, .04f},
                  Material{{.013f, .028f, .064f}, 0, .055f, .72f});
            plate(f, head * rz(-.18f), {side * .265f, .16f, .677f}, {.15f, .012f, .011f},
                  paint({.34f, .59f, .81f}));
        }
        plate(f, head, {0, .13f, .62f}, {.09f, .043f, .04f}, onyx);
    }
    if (id == 0) {
        M4 lens = head * translate({.255f, .065f, .753f}) * rx(pi / 2);
        ring(f, lens, {0, 0, 0}, {.245f, .45f, .245f}, steel);
        ring(f, lens, {0, -.036f, 0}, {.209f, .18f, .209f}, glow({.04f, .92f, .62f}, 1.8f));
        plate(f, head, {.52f, .09f, .65f}, {.12f, .055f, .04f}, carbon);
    }
    if (id < 2) {
        ell(f, head, {0, -.25f, .67f}, id == 0 ? V3{.21f, .13f, .24f} : V3{.34f, .125f, .29f},
            paint({1.f, .43f, .055f}));
        ell(f, head, {0, -.314f, .72f}, id == 0 ? V3{.19f, .033f, .18f} : V3{.32f, .027f, .24f},
            paint({.54f, .17f, .035f}));
        ell(f, head, {0, -.35f, .68f}, id == 0 ? V3{.16f, .055f, .19f} : V3{.27f, .052f, .23f},
            paint({1.f, .65f, .14f}));
    } else if (id == 7) {
        tube(f, head, {-.12f, -.26f, .581f}, {0, -.285f, .59f}, .014f, paint({.45f, .09f, .18f}));
        tube(f, head, {0, -.285f, .59f}, {.12f, -.26f, .581f}, .014f, paint({.45f, .09f, .18f}));
    } else {
        ell(f, head, {0, -.21f, .59f}, {.22f, .145f, id == 2 ? .24f : .13f},
            id == 5 ? coat : cream);
        ell(f, head, {0, -.17f, id == 2 ? .80f : .714f}, {.10f, .072f, .07f},
            id == 4 ? paint({.75f, .29f, .40f}) : iris);
        tube(f, head, {0, -.22f, .70f}, {0, -.29f, .675f}, .011f, onyx);
    }
    // Shared cyberpunk headset language, individual color-coded ear lights.
    for (int side : {-1, 1}) {
        ell(f, head, {side * .65f, -.03f, -.09f}, {.10f, .27f, .28f}, carbon);
        M4 cup = head * translate({side * .742f, -.025f, -.09f}) * rz(pi / 2);
        ring(f, cup, {0, 0, 0}, {.185f, .36f, .185f}, steel);
        ring(f, cup, {0, -side * .018f, 0}, {.14f, .15f, .14f}, glow(accent, .85f));
    }
    tube(f, head, {-.73f, .13f, -.13f}, {-.80f, .63f, -.15f}, .018f, steel);
    ell(f, head, {-.80f, .63f, -.15f}, {.037f, .045f, .037f}, glow(accent, 1.6f));
    if (id == 0) {
        for (int side : {-1, 1})
            add(f, Ribbon,
                m * translate({side * .15f, .68f, -.37f}) * ry(-pi / 2) * scale({.6f, .5f, .55f}),
                Material{{.77f, .21f, .055f}, 0, .64f, .03f, 11});
    }
}
void kart(Frame &f, M4 m, int id, double time, float wheel, float steer, float drift, float boost,
          const KartAnimation &anim) {
    const V3 accent = racerColors[id];
    constexpr float wear[]{.28f, .18f, .38f, .78f, .07f, .43f, .92f, .16f};
    float patina = clamp(wear[id] + anim.damage * .002f);
    Material enamel{
        accent * .72f + V3{.045f, .045f, .045f}, 0, .30f + patina * .26f, .42f, 20, patina};
    Material shell{id == 5 ? V3{.07f, .055f, .11f} : mix(V3{.76f, .81f, .80f}, accent, .22f),
                   0,
                   .30f,
                   .28f,
                   20,
                   patina};
    // Wheels stay on the track while the chassis leans on its suspension.
    M4 chassis = m * kartChassis(time, id, drift, anim);
    plate(f, chassis, {0, .57f, 0}, {1.04f, .19f, 1.48f}, carbon);
    plate(f, chassis, {0, .76f, .17f}, {.89f, .16f, 1.29f}, enamel);
    add(f, Hull, chassis * translate({0, .82f, .86f}) * ry(-pi / 2) * scale({.30f, .68f, .81f}),
        shell);
    // Layered nose cone and contrasting center stripe.
    add(f, Hull, chassis * translate({0, .953f, .76f}) * ry(-pi / 2) * scale({.245f, .42f, .47f}),
        enamel);
    add(f, Hull, chassis * translate({0, 1.005f, .81f}) * ry(-pi / 2) * scale({.225f, .41f, .09f}),
        carbon);
    plate(f, chassis, {0, .65f, 1.815f}, {.16f, .095f, .017f},
          Material{{.055f, .085f, .1f}, 0, .30f, .18f, 16.f + id * .1f});
    for (int side : {-1, 1}) {
        tube(f, chassis, {side * .86f, .67f, 1.67f}, {side * 1.10f, .58f, 1.47f}, .072f, steel);
        tube(f, chassis, {side * .86f, .67f, 1.67f}, {0, .69f, 1.78f}, .072f, carbon);
        plate(f, chassis, {side * .66f, .87f, 1.36f}, {.25f, .065f, .09f}, carbon);
        plate(f, chassis, {side * .66f, .881f, 1.44f}, {.19f, .033f, .02f},
              glow({.20f, .85f, .61f}, 1.3f));
        plate(f, chassis, {side * .985f, .74f, .36f}, {.078f, .12f, .79f}, steel);
        plate(f, chassis, {side * 1.019f, .75f, .45f}, {.028f, .05f, .68f}, glow(accent, 1.0f));
        // Side pod vents, fasteners and exposed frame rails.
        for (int k = 0; k < 4; k++)
            plate(f, chassis, {side * 1.059f, .82f, -.05f - k * .14f}, {.018f, .072f, .034f},
                  carbon);
        for (float z : {-.67f, .85f})
            ell(f, chassis, {side * 1.069f, .84f, z}, {.028f, .032f, .032f}, copper);
        tube(f, chassis, {side * .77f, .88f, -1.04f}, {side * .78f, 1.40f, -.88f}, .046f, steel);
        tube(f, chassis, {side * .78f, 1.40f, -.88f}, {side * .46f, 1.43f, -.82f}, .046f, steel);
        plate(f, chassis, {side * .68f, .64f, -1.56f}, {.18f, .069f, .054f},
              glow({1, .13f, .045f}, 1.7f));
    }
    // Seat, shoulder surround and steering wheel.
    plate(f, chassis, {0, 1.15f, -.41f}, {.53f, .42f, .17f}, carbon);
    plate(f, chassis, {0, 1.46f, -.43f}, {.40f, .16f, .17f}, enamel);
    plate(f, chassis, {0, .99f, -.10f}, {.49f, .095f, .44f}, carbon);
    tube(f, chassis, {0, 1.02f, .62f}, {0, 1.27f, .56f}, .034f, steel);
    M4 steering = chassis * steeringWheel(steer);
    ring(f, steering, {0, 0, 0}, {.33f, .6f, .33f}, carbon);
    tube(f, steering, {-.29f, 0, 0}, {.29f, 0, 0}, .027f, steel);
    ell(f, steering, {0, 0, 0}, {.089f, .045f, .089f}, enamel);
    tube(f, steering, {0, 0, 0}, {0, 0, -.29f}, .027f, steel);
    for (int side : {-1, 1})
        ell(f, steering, {side * .32f, 0, 0}, {.072f, .065f, .13f}, carbon);
    plate(f, steering, {0, .05f, .26f}, {.034f, .018f, .045f}, glow(accent, .6f));
    // Recessed instrument binnacle, tachometer, energy gauge and warning lamps.
    M4 dash = chassis * translate({0, 1.10f, .89f}) * rx(-.24f);
    plate(f, dash, {0, 0, 0}, {.54f, .10f, .12f}, carbon);
    plate(f, dash, {0, .04f, -.137f}, {.24f, .064f, .008f}, glow({.015f, .17f, .18f}, .6f));
    for (int k = 0; k < 10; k++)
        plate(f, dash, {(k - 4.5f) * .044f, .035f, -.15f}, {.015f, .037f, .008f},
              k < anim.speed / 3.4f ? glow(accent, 1.3f) : steel);
    for (int side : {-1, 1}) {
        M4 dial = dash * translate({side * .37f, .04f, -.145f}) * rx(pi / 2);
        ring(f, dial, {0, 0, 0}, {.093f, .3f, .093f}, steel);
        tube(f, dial, {0, 0, 0},
             {std::sin(anim.speed * .08f) * .071f, -.015f, std::cos(anim.speed * .08f) * .071f},
             .009f, glow(accent, 1.2f));
    }
    for (int k = 0; k < 3; k++)
        ell(f, dash, {.27f + k * .08f, -.07f, -.147f}, {.014f, .014f, .008f},
            glow(anim.damage > 65 ? V3{1, .15f, .04f} : accent, .8f));
    // Rear power unit, ceramic manifold, twin short boost exhausts.
    plate(f, chassis, {0, .89f, -1.19f}, {.64f, .26f, .34f}, steel);
    for (int k = 0; k < 5; k++)
        plate(f, chassis, {(k - 2) * .19f, 1.17f, -1.18f}, {.055f, .06f, .30f}, carbon);
    for (int side : {-1, 1}) {
        M4 e = chassis * translate({side * .55f, .69f, -1.43f}) * rx(pi / 2);
        add(f, Cylinder, e * scale({.18f, .30f, .18f}), steel);
        ring(f, e, {0, .31f, 0}, {.15f, .4f, .15f}, glow(accent, boost > 0 ? 3 : 1));
        ell(f, chassis, {side * .55f, .69f, -1.79f - boost * .15f},
            {.095f, .095f, .16f + boost * .32f},
            glow(boost > 0 ? V3{.33f, .77f, 1} : accent, boost > 0 ? 4 : 1.4f));
    }
    // Spoiler: a material wing with a luminous edge, not a floating text billboard.
    for (int side : {-1, 1})
        plate(f, chassis, {side * .62f, 1.18f, -1.38f}, {.055f, .37f, .08f}, carbon);
    plate(f, chassis * rx(-.1f), {0, 1.47f, -1.42f}, {1.09f, .069f, .25f}, enamel);
    plate(f, chassis, {0, 1.51f, -1.18f}, {.85f, .029f, .039f}, ceramic);
    // Individual silhouettes on a shared wheelbase: hardware, not just recolors.
    for (int side : {-1, 1}) {
        if (id == 0) {
            plate(f, chassis, {side * .67f, 1.02f, .25f}, {.25f, .10f, .56f}, enamel);
            tube(f, chassis, {side * .84f, .97f, .72f}, {side * .84f, .97f, -.6f}, .027f, copper);
        }
        if (id == 1) {
            ell(f, chassis, {side * .6f, .99f, 1.12f}, {.33f, .14f, .56f}, enamel);
            add(f, Wing,
                chassis * translate({side * .86f, 1.45f, -1.3f}) * ry(side * .4f) *
                    scale({.22f, .30f, .24f}),
                copper);
        }
        if (id == 2) {
            add(f, Ear,
                chassis * translate({side * .9f, 1.16f, -1.48f}) * rx(-.5f) *
                    scale({.55f, .57f, .72f}),
                enamel);
            tube(f, chassis, {side * .90f, .91f, 1.0f}, {side * .62f, 1.01f, .2f}, .055f, carbon);
        }
        if (id == 3) {
            plate(f, chassis * rz(side * .07f), {side * .72f, .99f, .48f}, {.29f, .08f, .5f},
                  Material{{.23f, .36f, .35f}, 0, .67f, .6f, 20, .96f});
            for (int k = 0; k < 3; k++)
                plate(f, chassis, {side * .8f, 1.09f, .16f + k * .20f}, {.21f, .028f, .05f},
                      copper);
        }
        if (id == 4) {
            ell(f, chassis, {side * .73f, .93f, .4f}, {.32f, .22f, .61f}, enamel);
            add(f, Ear,
                chassis * translate({side * .7f, 1.43f, -1.46f}) * scale({.30f, .50f, .45f}),
                ceramic);
        }
        if (id == 5) {
            for (int k = 0; k < 3; k++)
                ring(f, chassis * translate({side * .86f, 1.f, -.64f + k * .25f}) * rx(pi / 2),
                     {0, 0, 0}, {.15f, .25f, .15f}, glow(accent, .8f));
        }
        if (id == 6) {
            tube(f, chassis, {side * .78f, .77f, 1.8f}, {side * .97f, 1.15f, 1.42f}, .10f, steel);
            plate(f, chassis, {side * .75f, 1.23f, -1.13f}, {.24f, .17f, .33f},
                  Material{{.22f, .27f, .16f}, 0, .75f, .65f, 20, .95f});
        }
        if (id == 7) {
            for (int k = 0; k < 3; k++)
                add(f, Wing,
                    chassis *
                        translate({side * (.72f + k * .13f), .94f + k * .12f, -.3f - k * .3f}) *
                        rz(-side * .65f) * scale({.16f, .27f, .24f}),
                    enamel);
        }
    }
    // Tire rubber uses its own procedural tread material. The sidewall has modeled rims.
    for (int side : {-1, 1})
        for (int axle : {-1, 1}) {
            float z = axle > 0 ? 1.03f : -1.02f;
            M4 w = m * translate({side * 1.17f, .49f, z}) * ry(axle > 0 ? steer : 0) * rx(wheel);
            M4 cyl = w * rz(pi / 2);
            Material tire = rubber;
            tire.detail = clamp(anim.speed / 7);
            add(f, Cylinder, cyl * scale({.48f, .245f, .48f}), tire);
            for (float x : {-.235f, .235f}) {
                M4 sidewall = w * translate({x, 0, 0}) * rz(pi / 2);
                ring(f, sidewall, {0, 0, 0}, {.36f, .80f, .36f}, carbon);
                ring(f, sidewall, {0, 0, 0}, {.252f, .50f, .252f}, steel);
                ring(f, sidewall, {0, 0, 0}, {.293f, .28f, .293f}, glow(accent, .65f));
                add(f, Cylinder, sidewall * scale({.095f, .025f, .095f}), copper);
                // Analytic swept rim: sharp at low speed, averaged at racing speed.
                // Five frozen spokes at 30 Hz otherwise produce a strong wagon-wheel reversal.
                add(f, Cylinder, sidewall * scale({.249f, .012f, .249f}),
                    Material{{.65f, .72f, .75f}, 0, .26f, .8f, 24, anim.speed});
                if (anim.speed < 14)
                    ell(f, w, {x, .305f, 0}, {.026f, .039f, .020f}, copper);
            }
            // Suspension arms and spring collars attach to the wheel carrier.
            tube(f, m, {side * .71f, .59f, z}, {side * 1.02f, .49f, z}, .051f, steel);
        }
    badge(f, chassis * translate({-.38f, .665f, 1.82f}), .074f, steel);
    V3 size = driverSizes[id], seat{0, driverSeatY[id], -.26f};
    auto localGrip = [&](int side) {
        V3 p = steeringGrip(side, steer) - seat;
        return V3{p.x / size.x, p.y / size.y, p.z / size.z};
    };
    DriverAnimation driver{localGrip(-1), localGrip(1), -drift * .28f, anim.hit,
                           anim.taunt,    anim.look,    anim.airborne, anim.cockpit};
    animal(f, chassis * translate(seat) * scale(size), id, time, steer, driver);
    if (boost > .02f) {
        for (int side : {-1, 1})
            for (int j = 0; j < 5; j++) {
                float z = -2.0f - j * .55f;
                tube(f, m, {side * .85f, .22f, z}, {side * .85f, .22f, z - .32f},
                     .025f * (1 - j * .13f), glow(accent, 2.4f * clamp(boost)));
            }
    }
}
void rescueRobot(Frame &f, V3 kartPosition, V3 up, V3 forward, double time, float phase) {
    V3 right = unit(cross(up, forward));
    float height = 4.1f + .18f * std::sin(float(time) * 3);
    float approach = 1 - ease(0, .65f, phase);
    M4 m = basis(kartPosition + up * (height + approach * 3) + right * (approach * 6), right, up,
                 forward);
    Material orange = paint({.96f, .41f, .065f}), white = paint({.73f, .81f, .79f});
    ell(f, m, {0, .15f, 0}, {.62f, .63f, .5f}, orange);
    plate(f, m, {0, .35f, .43f}, {.46f, .22f, .065f}, carbon);
    for (int side : {-1, 1}) {
        ell(f, m, {side * .21f, .37f, .51f}, {.09f, .095f, .025f}, glow({.2f, .94f, .85f}, 1.5f));
        V3 shoulder{side * .56f, .1f, 0}, elbow{side * .89f, -.16f, .19f},
            grip{side * .37f, -.42f, .35f};
        tube(f, m, shoulder, elbow, .09f, white);
        ell(f, m, elbow, {.14f, .14f, .14f}, steel);
        tube(f, m, elbow, grip, .08f, orange);
        ell(f, m, grip, {.13f, .10f, .13f}, carbon);
        tube(f, m, {side * .4f, -.3f, -.12f}, {side * 1.03f, -.32f, -.15f}, .075f, steel);
        ring(f, m, {side * 1.05f, -.33f, -.15f}, {.50f, .65f, .50f}, orange);
        add(f, Cylinder, m * translate({side * 1.05f, -.37f, -.15f}) * scale({.39f, .036f, .39f}),
            Material{{.11f, .25f, .28f}, .1f, .2f, .8f});
        ring(f, m, {side * 1.05f, -.41f, -.15f}, {.32f, .18f, .32f}, glow({.19f, .8f, 1}, 1.5f));
    }
    tube(f, m, {0, .73f, 0}, {0, 1.04f, 0}, .025f, steel);
    ell(f, m, {0, 1.05f, 0}, {.09f, .06f, .09f}, glow({1, .60f, .1f}, 1.5f));
    M4 winch = m * translate({0, -.47f, .31f}) * rz(pi / 2);
    add(f, Cylinder, winch * scale({.17f, .31f, .17f}), steel);
    for (int k = 0; k < 5; k++)
        ring(f, winch, {0, (k - 2) * .08f, 0}, {.18f, .22f, .18f}, carbon);
    // A visible cable, yoke and two hooks stay attached to the kart's roll bar.
    V3 a = point(m, {0, -.54f, .3f}), b = kartPosition + up * 1.6f - forward * .8f;
    add(f, Cylinder, segment(a, b, .022f), steel);
    for (int side : {-1, 1}) {
        V3 hook = b + right * (side * .57f) - up * .18f;
        add(f, Cylinder, segment(b, hook, .030f), orange);
        add(f, Torus, basis(hook, right, forward, up) * scale({.11f, .40f, .11f}), orange);
    }
    (void)phase;
}
void arch(Frame &f, M4 m, int sector, double time) {
    V3 c = sector == 0   ? V3{.14f, .93f, .69f}
           : sector == 1 ? V3{1, .30f, .51f}
           : sector == 2 ? V3{.45f, .31f, 1}
                         : V3{.20f, .67f, 1};
    for (int side : {-1, 1}) {
        plate(f, m, {side * 7.65f, 2.15f, 0}, {.26f, 2.4f, .47f}, carbon);
        tube(f, m, {side * 7.60f, 4.2f, 0}, {side * 5.5f, 6.5f, 0}, .23f, steel);
        tube(f, m, {side * 7.62f, .25f, -.49f}, {side * 7.62f, 4.3f, -.49f}, .063f, glow(c, 2));
        for (int j = 0; j < 6; j++)
            plate(f, m, {side * 7.96f, .7f + j * .57f, -.07f}, {.067f, .14f, .32f}, paint(c * .5f));
    }
    plate(f, m, {0, 6.5f, 0}, {5.7f, .24f, .45f}, carbon);
    tube(f, m, {-5.6f, 6.70f, -.48f}, {5.6f, 6.70f, -.48f}, .064f, glow(c, 2.2f));
    badge(f, m * translate({0, 6.46f, .49f}), .35f, glow(c, 1.1f));
    for (int k = 0; k < 3; k++)
        ell(f, m, {(k - 1) * 1.5f, 5.85f, 0}, {.16f, .16f, .16f},
            glow(c, 1.5f + .18f * std::sin(time * .5 + k)));
}
} // namespace sw
