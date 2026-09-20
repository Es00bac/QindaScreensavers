// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <algorithm>
namespace sw {
V3 Track::curve(double u) const {
    double a = u * 2.0 * 3.14159265358979323846;
    if (kind == 1) { // A broad figure eight. Crossings are separated vertically.
        return {float(151 * std::sin(a)), float(27 * std::cos(a) + 5 * std::sin(3 * a)),
                float(92 * std::sin(2 * a))};
    }
    if (kind == 2) {
        return {float(132 * std::cos(a) + 21 * std::cos(3 * a)),
                float(17 * std::sin(2 * a) + 5 * std::sin(3 * a)),
                float(115 * std::sin(a) + 13 * std::sin(3 * a))};
    }
    if (kind == 3) { // Bliss: rolling meadow switchbacks surrounding a reclaimed valley.
        return {float(163 * std::cos(a) + 32 * std::cos(3 * a)),
                float(12 + 20 * std::sin(2 * a) + 7 * std::cos(3 * a)),
                float(130 * std::sin(a) + 26 * std::sin(2 * a))};
    }
    if (kind == 4) {
        // Authored elevated expressway, descending underpass, and rooftop circuit.
        static constexpr V3 nodes[]{
            {-145, 12, -110}, {-20, 12, -130},  {125, 22, -95},  {150, 48, 10},  {85, 58, 95},
            {-30, 58, 120},   {-110, 48, 45},   {0, 42, 0},      {130, 28, -20}, {120, -12, 95},
            {0, -16, 145},    {-135, -8, 115},  {-150, 12, 0},   {-45, 16, -50}, {55, 10, 20},
            {40, -20, -115},  {-50, -24, -150}, {-150, -5, -155}};
        double wrapped = u - std::floor(u), phase = wrapped * 18;
        int i = int(phase);
        float t = float(phase - i);
        auto p0 = nodes[(i + 17) % 18], p1 = nodes[i % 18], p2 = nodes[(i + 1) % 18],
             p3 = nodes[(i + 2) % 18];
        return (p1 * 2 + (p2 - p0) * t + (p0 * 2 - p1 * 5 + p2 * 4 - p3) * (t * t) +
                (-p0 + p1 * 3 - p2 * 3 + p3) * (t * t * t)) *
               .5f;
    }
    double r = 48 * (2 + (.88 + variation * .04) * std::cos(3 * a));
    return {float(r * std::cos(2 * a)), float(30 * std::sin(3 * a)), float(r * std::sin(2 * a))};
}
TrackPose Track::exact(double u) const {
    // A wider derivative stencil avoids float cancellation in the bank estimate.
    // The former sub-centimetre differences made the road frame jitter visibly.
    constexpr double e = .00045;
    V3 p = curve(u), d = curve(u + e) - curve(u - e), forward = unit(d);
    V3 right = unit(cross({0, 1, 0}, forward)), up = unit(cross(forward, right));
    V3 ta = unit(curve(u) - curve(u - e * 2)), tb = unit(curve(u + e * 2) - curve(u));
    float curvature = dot((tb - ta) / std::max(.001f, sw::length(d)), right);
    float bank = clamp(-curvature * 16.f, -.46f, .46f);
    V3 br = right * std::cos(bank) + up * std::sin(bank),
       bu = up * std::cos(bank) - right * std::sin(bank);
    return {p, br, bu, forward, bank, curvature};
}
Track::Track(std::uint64_t s, int k) : seed(s), kind(k) {
    if (kind < 0 || kind >= CourseCount)
        throw std::invalid_argument("Unknown course");
    Random r(seed);
    variation = r.range(-1, 1);
    for (int i = 0; i <= Samples; i++) {
        poses_[i] = exact(double(i) / Samples);
        if (i)
            lengths_[i] = lengths_[i - 1] + sw::length(poses_[i].position - poses_[i - 1].position);
    }
    length = lengths_.back();
    std::array<float, Samples> banks{};
    for (int i = 0; i < Samples; i++) {
        float total = 0, weight = 0;
        for (int j = -12; j <= 12; j++) {
            float w = float(13 - std::abs(j));
            total += poses_[(i + j + Samples) % Samples].bank * w;
            weight += w;
        }
        banks[i] = total / weight;
    }
    for (int i = 0; i <= Samples; i++) {
        auto &p = poses_[i];
        p.bank = banks[i % Samples];
        V3 right = unit(cross(V3{0, 1, 0}, p.forward)), up = unit(cross(p.forward, right));
        p.right = right * std::cos(p.bank) + up * std::sin(p.bank);
        p.up = up * std::cos(p.bank) - right * std::sin(p.bank);
    }
    if (kind == 4)
        jumpStart = lengths_[int(Samples * 4.45 / 18)];
    for (int i = 0; i < 6; i++)
        pads[i] = {length * (.085 + i * .153), float((i % 3) - 1) * 4.1f};
}
TrackPose Track::at(double distance) const {
    double s = std::fmod(distance, length);
    if (s < 0)
        s += length;
    auto it = std::upper_bound(lengths_.begin(), lengths_.end(), s);
    int i = std::clamp(int(it - lengths_.begin()) - 1, 0, Samples - 1);
    float a = float((s - lengths_[i]) / (lengths_[i + 1] - lengths_[i]));
    const auto &p = poses_[i];
    const auto &q = poses_[i + 1];
    V3 f = unit(mix(p.forward, q.forward, a));
    V3 right = unit(mix(p.right, q.right, a));
    V3 up = unit(cross(f, right));
    right = unit(cross(up, f));
    V3 pos = mix(p.position, q.position, a);
    if (jumpStart >= 0) {
        double gap = signedGap(s, jumpStart);
        float height = 0, slope = 0;
        if (gap >= -12 && gap < 0) {
            float u = float((gap + 12) / 12);
            height = 2.5f * smooth(u);
            slope = 2.5f * 30 * u * u * (u - 1) * (u - 1) / 12;
        } else if (gap >= 0 && gap < jumpLength) {
            height = 2.5f * (1 - float(gap / jumpLength));
        }
        pos = pos + up * height;
        if (slope > 0) {
            f = unit(f + up * slope);
            up = unit(cross(f, right));
        }
    }
    return {pos,
            right,
            up,
            f,
            p.bank + (q.bank - p.bank) * a,
            p.curvature + (q.curvature - p.curvature) * a};
}
double Track::signedGap(double a, double b) const {
    double d = std::fmod(a - b + length * .5, length);
    if (d < 0)
        d += length;
    return d - length * .5;
}
const char *Track::name() const {
    static const char *names[]{"PRISM KNOT", "CHROMATIC EIGHT", "AURORA LOOP",
                               "QINDA BLISS / RECLAIMED VALLEY", "NEON UNDERCITY / SKYWAY"};
    return names[kind];
}
void buildTrackMeshes(Frame &f, const Track &tr) {
    Mesh top;
    top.name = "prismatic-road";
    Mesh shell;
    shell.name = "carbon-road-chassis";
    constexpr int N = 1300, W = 12;
    for (int i = 0; i <= N; i++) {
        double s = tr.length * i / N;
        auto p = tr.at(s);
        for (int j = 0; j <= W; j++) {
            float v = float(j) / W, x = (v * 2 - 1) * tr.halfWidth;
            top.v.push_back({p.position + p.right * x, p.up, {float(s), v}});
        }
        // Full structural underside and both sides, not a paper-thin floating plane.
        for (auto v : std::array<V2, 4>{{{-tr.halfWidth, 0},
                                         {-tr.halfWidth, -.75f},
                                         {tr.halfWidth, -.75f},
                                         {tr.halfWidth, 0}}}) {
            V3 n = v.y < 0 ? -p.up : (v.x < 0 ? -p.right : p.right);
            shell.v.push_back({p.position + p.right * v.x + p.up * v.y, n, {float(s), v.x}});
        }
    }
    for (int i = 0; i < N; i++) {
        double s = tr.length * (i + .5) / N;
        if (tr.jumpStart >= 0 && tr.signedGap(s, tr.jumpStart) > 0 &&
            tr.signedGap(s, tr.jumpStart) < tr.jumpLength)
            continue;
        for (int j = 0; j < W; j++) {
            unsigned a = i * (W + 1) + j, b = a + W + 1;
            top.ix.insert(top.ix.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
        for (int j = 0; j < 3; j++) {
            unsigned a = i * 4 + j, b = a + 4;
            shell.ix.insert(shell.ix.end(), {a, a + 1, b, a + 1, b + 1, b});
        }
    }
    f.batches[RoadSurface].mesh = std::move(top);
    f.batches[RoadShell].mesh = std::move(shell);
    buildSceneryMeshes(f, tr);
    f.map.clear();
    for (int i = 0; i <= 160; i++) {
        auto p = tr.at(tr.length * i / 160).position;
        f.map.push_back({p.x / 190.f, p.z / 190.f});
    }
}
} // namespace sw
