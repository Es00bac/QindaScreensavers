// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace sw;
int main() {
    try {
        std::uint64_t ticks = 0, passes = 0, boosts = 0, contacts = 0, rounds = 0, checks = 0;
        double maxStep = 0;
        float maxLane = 0, maxSpeed = 0;
        for (int kind = 0; kind < CourseCount; kind++)
            for (int seed = 1; seed <= 12; seed++) {
                Race r(seed * 103, kind);
                for (int frame = 0; frame < 36000; frame++) {
                    auto old = r.state();
                    r.advance(1. / 60.);
                    const auto &now = r.state();
                    for (int i = 0; i < 8; i++) {
                        const auto &c = now.cars[i];
                        maxLane = std::max(maxLane, std::abs(c.lane));
                        maxSpeed = std::max(maxSpeed, c.speed);
                        if (!std::isfinite(c.distance) ||
                            std::abs(c.lane) > r.track.halfWidth + .5f || c.speed < 0 ||
                            c.speed > 42 || c.damage < 0 || c.damage > 100)
                            throw std::runtime_error("State outside bounds");
                        if (old.round == now.round) {
                            double d =
                                length(carPosition(c, r.track) - carPosition(old.cars[i], r.track));
                            maxStep = std::max(maxStep, d);
                            if (d > 1.5) {
                                std::cerr
                                    << "course=" << kind << " seed=" << seed * 103
                                    << " frame=" << frame << " car=" << i << " movement=" << d
                                    << " old_lane=" << old.cars[i].lane << " new_lane=" << c.lane
                                    << " modes=" << int(old.cars[i].mode) << "->" << int(c.mode)
                                    << " distances=" << old.cars[i].distance << "->" << c.distance
                                    << " hops=" << old.cars[i].hop << "->" << c.hop
                                    << " banks=" << r.track.at(old.cars[i].distance).bank << "->"
                                    << r.track.at(c.distance).bank << "\n";
                                throw std::runtime_error("Discontinuous car position");
                            }
                        }
                        checks++;
                    }
                    if (frame % 6 == 0)
                        for (int i = 0; i < 8; i++)
                            for (int j = i + 1; j < 8; j++) {
                                auto a = now.cars[i], b = now.cars[j];
                                if (!airborne(a) && !airborne(b) && a.invulnerable <= 0 &&
                                    b.invulnerable <= 0 && a.mode != CarMode::Spinout &&
                                    b.mode != CarMode::Spinout &&
                                    std::abs(r.track.signedGap(a.distance, b.distance)) < 3.935 &&
                                    std::abs(a.lane - b.lane) < 2.77)
                                    throw std::runtime_error("Car body penetration");
                                checks++;
                            }
                }
                ticks += r.counters.ticks;
                passes += r.counters.passes;
                boosts += r.counters.boosts;
                contacts += r.counters.contacts;
                rounds += r.counters.rounds;
            }
        std::cout << std::setprecision(9)
                  << "{\"status\":\"passed\",\"course_seed_combinations\":" << CourseCount * 12
                  << ",\"simulated_hours\":" << CourseCount * 2 << ",\"fixed_ticks\":" << ticks
                  << ",\"checks\":" << checks << ",\"overtakes\":" << passes
                  << ",\"boosts\":" << boosts << ",\"completed_rounds\":" << rounds
                  << ",\"soft_contact_corrections\":" << contacts
                  << ",\"maximum_travel_per_60hz_sample_m\":" << maxStep
                  << ",\"maximum_abs_lane_m\":" << maxLane
                  << ",\"maximum_speed_m_per_s\":" << maxSpeed << "}\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
