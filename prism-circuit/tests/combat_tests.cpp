// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <iostream>
#include <set>
#include <stdexcept>
using namespace sw;
namespace {
void check(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
bool finite(V3 p) {
    return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
}
} // namespace
int main() {
    try {
        std::uint64_t hits = 0, knockouts = 0, rescues = 0, jumps = 0, landings = 0, trapHits = 0,
                      blocks = 0;
        std::set<int> cameras, subjects;
        std::array<bool, 8> usedItems{};
        for (int kind = 0; kind < CourseCount; ++kind) {
            Race race(41, kind);
            unsigned lastShot = 0;
            double lastCut = 0;
            Frame f;
            initialize(f);
            buildTrackMeshes(f, race.track);
            SceneOptions opt;
            for (int tick = 0; tick < 72000; ++tick) {
                auto old = race.state();
                race.advance(FixedStep);
                const auto &s = race.state();
                check(s.projectiles.size() <= 24 && s.traps.size() <= 40 && s.events.size() <= 96,
                      "unbounded combat entities");
                for (int i = 0; i < 8; ++i) {
                    const auto &c = s.cars[i];
                    usedItems[int(c.item)] = true;
                    check(finite(carPosition(c, race.track)), "non-finite car pose");
                    check(c.damage >= 0 && c.damage <= 100, "unbounded damage");
                    if (old.round == s.round) {
                        const auto &b = old.cars[i];
                        check(c.wheelDistance + 1e-9 >= b.wheelDistance, "wheel distance reversed");
                        if (length(carPosition(c, race.track) - carPosition(b, race.track)) >=
                            2.5f) {
                            std::cerr
                                << "course=" << kind << " time=" << s.time << " car=" << i
                                << " modes=" << int(b.mode) << "->" << int(c.mode)
                                << " lanes=" << b.lane << "->" << c.lane
                                << " distance=" << b.distance << "->" << c.distance << " jump="
                                << length(carPosition(c, race.track) - carPosition(b, race.track))
                                << '\n';
                            std::cerr
                                << "air old=" << b.airPosition.x << "," << b.airPosition.y << ","
                                << b.airPosition.z << " new=" << c.airPosition.x << ","
                                << c.airPosition.y << "," << c.airPosition.z << " old floor="
                                << terrainElevation(race.track, b.airPosition.x, b.airPosition.z)
                                << " new floor="
                                << terrainElevation(race.track, c.airPosition.x, c.airPosition.z)
                                << '\n';
                            check(false, "visible position snap");
                        }
                        if (c.mode == CarMode::Falling || c.mode == CarMode::Rescuing)
                            check(std::abs(c.distance - b.distance) < .5,
                                  "off-track racer gaining progress");
                        if (b.mode != CarMode::Jumping && c.mode == CarMode::Jumping)
                            ++jumps;
                        if (b.mode == CarMode::Jumping && c.mode == CarMode::Recovering)
                            ++landings;
                        if (c.mode == CarMode::Recovering && b.mode == CarMode::Rescuing) {
                            check(c.damage == 0 && c.invulnerable > 0, "unsafe recovery");
                            check(c.speed > 0, "rescue did not restore racer");
                        }
                    }
                    if (c.mode == CarMode::Falling)
                        check(c.modeTime < 1.67, "stuck falling");
                    if (c.mode == CarMode::Rescuing)
                        check(c.modeTime < 3.42, "stuck lowering");
                }
                cameras.insert(int(s.director.camera));
                subjects.insert(s.director.focus);
                if (s.director.shot != lastShot) {
                    if (lastShot && old.round == s.round)
                        check(s.time - lastCut >= 2.99, "director cuts too quickly");
                    lastShot = s.director.shot;
                    lastCut = s.time;
                }
                if (tick % 600 == 0) {
                    compose(f, race.sample(), race.track, opt);
                    check(finite(f.eye) && finite(f.target) && length(f.eye - f.target) > 1,
                          "invalid director camera");
                    check(std::abs(dot(unit(f.target - f.eye), f.up)) < .999,
                          "degenerate camera up");
                    for (const auto &b : f.batches)
                        for (const auto &ins : b.instances)
                            for (float a : ins.model.a)
                                check(std::isfinite(a), "invalid rendered transform");
                }
            }
            hits += race.counters.hits;
            knockouts += race.counters.knockouts;
            rescues += race.counters.rescues;
            trapHits += race.counters.trapsHit;
            blocks += race.counters.blocks;
            check(race.counters.rounds > 0, "races never complete");
            // The entire simulation, events and director reproduce after a backwards seek.
            race.seek(42.5);
            Race replay(41, kind);
            replay.advance(42.5);
            for (int i = 0; i < 8; ++i) {
                check(race.state().cars[i].distance == replay.state().cars[i].distance,
                      "seek not deterministic");
                check(race.state().cars[i].mode == replay.state().cars[i].mode,
                      "combat seek not deterministic");
            }
            check(race.state().director.focus == replay.state().director.focus &&
                      race.state().director.shot == replay.state().director.shot,
                  "director seek not deterministic");
            // Manual cameras are finite for all drivers, including every cockpit fit.
            for (int id = 0; id < 8; id++)
                for (Camera camera : {Camera::Cockpit, Camera::Chase, Camera::Trackside,
                                      Camera::Pack, Camera::Overview}) {
                    SceneOptions view;
                    view.focus = id;
                    view.camera = camera;
                    compose(f, replay.sample(), replay.track, view);
                    check(finite(f.eye) && finite(f.target) && length(f.eye - f.target) > 1,
                          "bad manual camera");
                }
        }
        check(hits > 50 && knockouts > 20 && rescues > 20 && trapHits > 10 && blocks > 10,
              "combat features not exercised");
        check(jumps > 10 && landings > 5, "city gap never jumped/landed");
        check(cameras.count(int(Camera::Cockpit)) && cameras.count(int(Camera::Trackside)) &&
                  cameras.count(int(Camera::Rescue)),
              "missing cinematic shots");
        check(subjects.size() == 8, "director does not cover all racers");
        for (int i = 1; i < 8; i++)
            check(usedItems[i], "item never collected");
        check(driverSizes[1].y < driverSizes[0].y && driverSizes[6].y > driverSizes[4].y,
              "driver proportions not differentiated");
        // Positive rotation about +X makes the tire's contact patch move backwards
        // against +Z travel. The previous repeated-spoke pattern, not this sign,
        // produced the reverse-spinning optical effect.
        check(point(rx(.05f), {0, -.48f, 0}).z < 0, "wheel rolls in wrong direction");
        for (float steer : {-.4f, 0.f, .4f})
            for (int side : {-1, 1}) {
                V3 center = point(steeringWheel(steer), {0, 0, 0});
                check(std::abs(length(steeringGrip(side, steer) - center) - .33f) < .0001f,
                      "hands leave steering rim");
            }
        std::cout << "PASS combat/cameras: hits=" << hits << " knockouts=" << knockouts
                  << " rescues=" << rescues << " trap_hits=" << trapHits << " blocks=" << blocks
                  << " jumps=" << jumps << " landings=" << landings
                  << " camera_modes=" << cameras.size() << " subjects=" << subjects.size() << '\n';
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
