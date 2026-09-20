// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <iostream>
#include <stdexcept>
using namespace sw;
namespace {
void check(bool ok, const char *what) {
    if (!ok)
        throw std::runtime_error(what);
}
} // namespace
int main() {
    try {
        std::uint64_t solids = 0;
        int crossings = 0;
        float minimum = 1000;
        for (int kind = 0; kind < CourseCount; kind++)
            for (std::uint64_t seed : {13, 41, 777}) {
                Track tr(seed, kind);
                Frame f;
                initialize(f);
                buildTrackMeshes(f, tr);
                for (int camera = 0; camera < 16; camera++) {
                    auto p = tr.at(tr.length * camera / 16);
                    f.eye = p.position + p.up * 5;
                    for (auto &b : f.batches)
                        b.instances.clear();
                    f.scenerySolids.clear();
                    if (kind == 0)
                        groundedSkyline(f, tr);
                    else
                        courseScenery(f, tr, 12);
                    check(!f.batches[Terrain].instances.empty(), "missing continuous ground");
                    for (const auto &b : f.scenerySolids) {
                        check(trackClearance(tr, b),
                              "building, roof, prop or pier intersects racing corridor");
                        ++solids;
                    }
                    // Full building pieces, including their roof machinery, must be inside
                    // the reserved boxes used by the placement check, not untested extras.
                    for (const auto &ins : f.batches[Architecture].instances) {
                        if (ins.surface.z < 13.5f || ins.surface.z > 14.5f)
                            continue;
                        V3 lo{1e9f, 1e9f, 1e9f}, hi{-1e9f, -1e9f, -1e9f};
                        for (int x : {-1, 1})
                            for (int y : {-1, 1})
                                for (int z : {-1, 1}) {
                                    V3 v = point(ins.model, {float(x), float(y), float(z)});
                                    lo = {std::min(lo.x, v.x), std::min(lo.y, v.y),
                                          std::min(lo.z, v.z)};
                                    hi = {std::max(hi.x, v.x), std::max(hi.y, v.y),
                                          std::max(hi.z, v.z)};
                                }
                        bool contained = false;
                        for (const auto &b : f.scenerySolids) {
                            V3 a = b.center - b.half - V3{.02f, .02f, .02f},
                               z = b.center + b.half + V3{.02f, .02f, .02f};
                            if (lo.x >= a.x && lo.y >= a.y && lo.z >= a.z && hi.x <= z.x &&
                                hi.y <= z.y && hi.z <= z.z) {
                                contained = true;
                                break;
                            }
                        }
                        check(contained, "facade or rooftop escaped its reserved silhouette");
                    }
                    check(!trackClearance(tr, {p.position, {1, 1, 1}}),
                          "clearance check accepts a building on the road");
                }
            }
        // Independently locate all projected XZ crossings and verify real vertical
        // separation, not merely visually overlapping roads at the same height.
        Track city(41, 4);
        constexpr int n = 1600;
        std::array<V3, n + 1> path;
        for (int i = 0; i <= n; i++)
            path[i] = city.at(city.length * i / n).position;
        auto cross2 = [](V3 a, V3 b) { return a.x * b.z - a.z * b.x; };
        for (int i = 0; i < n; i++)
            for (int j = i + 12; j < n; j++) {
                if (i < 12 && j > n - 12)
                    continue;
                V3 a = path[i], r = path[i + 1] - a, b = path[j], s = path[j + 1] - b;
                float denominator = cross2(r, s);
                if (std::abs(denominator) < 1e-6f)
                    continue;
                float u = cross2(b - a, s) / denominator, v = cross2(b - a, r) / denominator;
                if (u > 0 && u < 1 && v > 0 && v < 1) {
                    float gap = std::abs(a.y + r.y * u - b.y - s.y * v);
                    minimum = std::min(minimum, gap);
                    ++crossings;
                }
            }
        check(crossings >= 4 && minimum > 23, "city crossings lack height clearance");
        check(city.jumpStart > 0 && city.jumpLength >= 30, "missing authored city jump");
        std::cout << "PASS scenery: " << solids
                  << " reserved solids verified across 15 course/seed combinations; city crossings="
                  << crossings << " minimum separation=" << minimum << " m\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
