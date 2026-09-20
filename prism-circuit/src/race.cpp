// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <numeric>
namespace sw {
const char *itemName(Item item) {
    static const char *names[]{"",          "TURBO CELL",   "AEGIS SHIELD",
                               "ION PULSE", "MAGNET DRIVE", "SEEKER DRONE",
                               "OIL SLICK", "ARC MINE"};
    return names[int(item)];
}
void Race::cue(saver::Cue kind, int car) {
    if (current_.sounds.size() < 96)
        current_.sounds.push_back(
            {++soundSerial_, current_.time, kind, car, clamp(current_.cars[car].lane / 6, -1, 1)});
}
Race::Race(std::uint64_t seed, int kind) : track(seed, kind) {
    grid(0, 0);
}
void Race::grid(unsigned round, double globalTime) {
    current_ = {};
    current_.round = round;
    current_.time = globalTime;
    current_.director.startedAt = globalTime;
    current_.director.until = globalTime + 4.2;
    Random rng(track.seed + round * 8191);
    for (int i = 0; i < RacerCount; i++) {
        auto &c = current_.cars[i];
        c.distance = -double(i / 2) * 7.2 - 9;
        c.lane = (i % 2 ? 2.6f : -2.6f);
        c.targetLane = c.lane;
        c.pace = 25.0f + rng.range(-.9f, .9f);
        c.decision = rng.range(0, .65);
        c.rank = i + 1;
        current_.order[i] = i;
    }
    previous_ = current_;
}
void Race::step() {
    previous_ = current_;
    constexpr float dt = float(FixedStep);
    current_.time += FixedStep;
    current_.roundTime += FixedStep;
    counters.ticks++;
    std::erase_if(current_.sounds,
                  [&](const saver::SoundEvent &e) { return current_.time - e.time > .35; });
    std::erase_if(current_.events, [&](const RaceEvent &e) { return current_.time - e.time > 9; });
    for (auto &crate : current_.crates)
        crate = std::max(0.f, crate - dt);
    if (current_.finishedAt >= 0 && current_.time - current_.finishedAt >= 11) {
        unsigned next = current_.round + 1;
        double t = current_.time;
        grid(next, t);
        counters.rounds++;
        return;
    }
    const auto before = current_.cars;
    double lead = -1e20;
    for (const auto &c : before)
        lead = std::max(lead, c.distance);
    const bool go = current_.roundTime > 4;
    for (int i = 0; i < RacerCount; i++) {
        auto &c = current_.cars[i];
        const auto &old = before[i];
        c.boost = std::max(0.f, c.boost - dt);
        c.cooldown = std::max(0.f, c.cooldown - dt);
        c.decision -= dt;
        for (float *timer : {&c.shield, &c.magnet, &c.pulse, &c.stun, &c.invulnerable, &c.taunt,
                             &c.attackPose, &c.contactCooldown})
            *timer = std::max(0.f, *timer - dt);
        c.hit = std::max(0.f, c.hit - dt * 2);
        c.landing = std::max(0.f, c.landing - dt * 2.8f);
        if (motion(i))
            continue;
        int ahead = -1, behind = -1;
        double aheadGap = 120, behindGap = 55;
        for (int j = 0; j < RacerCount; ++j)
            if (j != i && !airborne(before[j])) {
                double gap = track.signedGap(before[j].distance, old.distance);
                if (gap > 0 && gap < aheadGap) {
                    ahead = j;
                    aheadGap = gap;
                }
                if (gap < 0 && -gap < behindGap) {
                    behind = j;
                    behindGap = -gap;
                }
            }
        c.rival = ahead;
        c.look += (clamp(ahead >= 0 ? (before[ahead].lane - old.lane) * .12f : 0.f, -.55f, .55f) -
                   c.look) *
                  dt * 3;
        if (c.item != Item::None && go && c.mode != CarMode::Spinout) {
            c.itemAge += dt;
            bool opportunity = c.item == Item::Seeker  ? ahead >= 0 && aheadGap < 95
                               : c.item == Item::Pulse ? ahead >= 0 && aheadGap < 30
                               : (c.item == Item::Oil || c.item == Item::Mine)
                                   ? behind >= 0 && behindGap < 40
                                   : true;
            if (c.itemAge > .75f + i * .065f && (opportunity || c.itemAge > 7)) {
                Item used = c.item;
                if (c.item == Item::Turbo)
                    c.boost = 3;
                if (c.item == Item::Shield)
                    c.shield = 7;
                if (c.item == Item::Magnet)
                    c.magnet = 7;
                if (c.item == Item::Pulse) {
                    c.pulse = .85f;
                    for (int j = 0; j < RacerCount; ++j)
                        if (j != i) {
                            double gap = track.signedGap(before[j].distance, old.distance);
                            if (gap > -8 && gap < 32 && !airborne(before[j]))
                                hit(j, i, 36, (before[j].lane >= old.lane ? 1.f : -1.f) * 9,
                                    Item::Pulse);
                        }
                }
                if (c.item == Item::Seeker && ahead >= 0 && current_.projectiles.size() < 24) {
                    current_.projectiles.push_back(
                        {eventSerial_ + 1, i, ahead, c.distance + 2.1, c.lane, 0, 48});
                    ++counters.shots;
                    event(EventKind::Launch, i, ahead, used);
                }
                if ((c.item == Item::Oil || c.item == Item::Mine) && current_.traps.size() < 40) {
                    current_.traps.push_back({i, c.item, c.distance - 3.1, c.lane, 0});
                    event(EventKind::Trap, i, behind, used);
                }
                c.attackPose = 1;
                cue(c.item == Item::Turbo ? saver::Cue::Boost : saver::Cue::Special, i);
                c.item = Item::None;
                c.itemAge = 0;
                ++counters.itemsUsed;
            }
        }
        auto pose = track.at(old.distance + 6);
        float target = old.pace / (1 + std::abs(pose.curvature) * 4.1f);
        target += clamp(float(lead - old.distance) * .025f, 0, 2.1f);
        if (c.decision <= 0 && go && c.mode != CarMode::Spinout) {
            // Score actual occupancy of candidate lanes before choosing a pass or pad.
            float ideal = clamp(-pose.curvature * 90.f, -2.f, 2.f);
            double padAhead = 1e10;
            float padLane = 0;
            for (const auto &pad : track.pads) {
                double d = track.signedGap(pad.distance, old.distance);
                if (d > 2 && d < padAhead) {
                    padAhead = d;
                    padLane = pad.lane;
                }
            }
            float best = -1e9, chosen = c.targetLane;
            for (float lane : {-4.1f, 0.f, 4.1f}) {
                float score =
                    4.0f - std::abs(lane - ideal) * .16f - std::abs(lane - old.lane) * .14f;
                if (std::abs(lane - c.targetLane) < .5f)
                    score += .9f;
                if (padAhead < 39 && std::abs(lane - padLane) < 1.0)
                    score += 2.2f;
                if (c.item == Item::None)
                    for (int box = 0; box < 24; ++box)
                        if (current_.crates[box] <= 0) {
                            double d =
                                track.signedGap(track.length * (box / 3 + .35) / 8, old.distance);
                            float boxLane = (box % 3 - 1) * 4.1f;
                            if (d > 0 && d < 45 && std::abs(lane - boxLane) < 1)
                                score += 2.8f;
                        }
                // Line up a shot, bait a pursuer into a trap, and recognize hazards ahead.
                if (c.item == Item::Seeker && ahead >= 0 && std::abs(lane - before[ahead].lane) < 2)
                    score += 2.7f;
                if ((c.item == Item::Oil || c.item == Item::Mine) && behind >= 0 &&
                    std::abs(lane - before[behind].lane) < 2)
                    score += 3;
                for (const auto &trap : current_.traps) {
                    double gap = track.signedGap(trap.distance, old.distance);
                    if (gap > 3 && gap < 26 && std::abs(lane - trap.lane) < 2.3f)
                        score -= 8;
                }
                for (int j = 0; j < RacerCount; j++)
                    if (i != j && !airborne(before[j])) {
                        double ds = track.signedGap(before[j].distance, old.distance);
                        bool crossing = before[j].lane >= std::min(old.lane, lane) - 2.85f &&
                                        before[j].lane <= std::max(old.lane, lane) + 2.85f;
                        if (ds > -6 && ds < 7 && crossing)
                            score -= 10;
                        if (ds > 0 && ds < 21 && std::abs(before[j].lane - lane) < 2.8)
                            score -= float(21 - ds) * .55f;
                    }
                score += .2f * std::sin(float(current_.time * .24 + i * 3 + lane));
                if (score > best) {
                    best = score;
                    chosen = lane;
                }
            }
            c.targetLane = chosen;
            c.decision = .68f + .045f * i;
        }
        if (c.mode == CarMode::Spinout)
            c.targetLane = c.lane;
        float laneAccel = (c.targetLane - old.lane) * 8.0f - old.laneSpeed * 5.8f;
        c.laneSpeed = clamp(old.laneSpeed + laneAccel * dt, -2.6f, 2.6f);
        c.lane = old.lane + (c.laneSpeed + c.impulse) * dt;
        c.impulse *= std::exp(-2.1f * dt);
        // A merger yields instead of cutting through the neighboring kart.
        for (int j = 0; j < RacerCount; j++)
            if (i != j && !airborne(before[j]) && c.mode != CarMode::Spinout) {
                double gap = track.signedGap(before[j].distance, old.distance);
                if (std::abs(gap) < 4.9 && std::abs(c.lane - before[j].lane) < 2.8f &&
                    std::abs(old.lane - before[j].lane) >= 2.8f) {
                    c.lane = old.lane;
                    c.laneSpeed = 0;
                }
            }
        if (std::abs(c.lane) > track.halfWidth + .3f) {
            fall(i, c.lastAttacker);
            continue;
        }
        if (c.mode != CarMode::Spinout && std::abs(c.lane) > 4.65f) {
            float boundary = clamp(c.lane, -4.65f, 4.65f);
            c.lane = old.lane + clamp(boundary - old.lane, -2.6f * dt, 2.6f * dt);
        }
        if (c.boost > 0)
            target += 6.0f * clamp(c.boost / .22f);
        if (c.stun > 0)
            target *= .32f;
        if (c.magnet > 0)
            target += 2.8f;
        for (int j = 0; j < RacerCount; j++)
            if (i != j && !airborne(before[j]) && before[j].invulnerable <= 0) {
                double gap = track.signedGap(before[j].distance, old.distance);
                float dl = std::abs(before[j].lane - c.lane);
                if (gap > 0 && gap < 22 && dl < 2.9f) {
                    if (gap > 7.5 && gap < 17)
                        target += 1.0f; // Slipstream.
                    float safe = before[j].speed + float(gap - 5.1) * 1.8f;
                    target = std::min(target, std::max(0.f, safe));
                }
            }
        if (!go)
            target = 0;
        if (current_.winner >= 0)
            target = std::min(target, 14.f);
        c.speed = std::max(0.f, old.speed + clamp(target - old.speed, -16 * dt, 7 * dt));
        c.distance = old.distance + c.speed * dt;
        c.wheelDistance = old.wheelDistance + c.speed * dt;
        c.steer =
            old.steer +
            (clamp(pose.curvature * 13 - c.laneSpeed * .11f, -.40f, .40f) - old.steer) * dt * 7;
        float desiredDrift = clamp(pose.curvature * c.speed * .30f, -.26f, .26f);
        c.drift = old.drift + (desiredDrift - old.drift) * dt * 4;
        if (go && c.cooldown <= 0) {
            for (const auto &pad : track.pads) {
                double ahead = track.signedGap(pad.distance, old.distance),
                       after = track.signedGap(pad.distance, c.distance);
                if (ahead >= 0 && after <= 0 && std::abs(c.lane - pad.lane) < 1.6f) {
                    c.boost = 1.8f;
                    c.cooldown = 2.5f;
                    counters.boosts++;
                    cue(saver::Cue::Boost, i);
                    break;
                }
            }
        }
        if (go && c.item == Item::None)
            for (int box = 0; box < 24; ++box)
                if (current_.crates[box] <= 0) {
                    double distance = track.length * (box / 3 + .35) / 8,
                           ahead = track.signedGap(distance, old.distance),
                           after = track.signedGap(distance, c.distance);
                    float lane = (box % 3 - 1) * 4.1f;
                    if (ahead >= 0 && after <= 0 &&
                        std::abs(c.lane - lane) < (c.magnet > 0 ? 3.6f : 1.65f)) {
                        Random reward(
                            track.seed + std::uint64_t(box) * 7919 + std::uint64_t(i) * 313 +
                            std::uint64_t(std::max(0., c.distance) / track.length) * 17171 +
                            current_.round * 99991);
                        // Trailing drivers get more offensive tools; leaders can defend and lay
                        // traps.
                        static const Item attackBag[]{Item::Seeker, Item::Pulse,  Item::Mine,
                                                      Item::Turbo,  Item::Seeker, Item::Shield,
                                                      Item::Oil,    Item::Magnet};
                        static const Item leadBag[]{Item::Oil,    Item::Mine,  Item::Shield,
                                                    Item::Pulse,  Item::Turbo, Item::Seeker,
                                                    Item::Shield, Item::Magnet};
                        const auto rewardIndex = reward.next() % 8;
                        c.item = c.rank > 3 ? attackBag[rewardIndex] : leadBag[rewardIndex];
                        c.itemAge = 0;
                        current_.crates[box] = 4;
                        ++counters.pickups;
                        cue(saver::Cue::Pickup, i);
                        break;
                    }
                }
        if (track.jumpStart >= 0 && go && track.signedGap(track.jumpStart, old.distance) >= 0 &&
            track.signedGap(track.jumpStart, c.distance) < 0) {
            c.mode = CarMode::Jumping;
            c.modeTime = 0;
            c.jumpDistance = c.distance;
            c.speed = std::max(29.f, c.speed);
            auto p = track.at(c.distance);
            c.airPosition = p.position + p.right * c.lane;
            event(EventKind::Jump, i, i);
            cue(saver::Cue::Jump, i);
        }
    }
    weapons();
    // Keep rejoining karts visibly protected until their fenders have a clear
    // space. Expiring a grace timer inside traffic must not shove a racer back.
    for (int i = 0; i < RacerCount; i++) {
        auto &c = current_.cars[i];
        if (c.invulnerable <= 0 && before[i].invulnerable > 0 && !airborne(c)) {
            for (int j = 0; j < RacerCount; j++)
                if (i != j && !airborne(current_.cars[j])) {
                    const auto &other = current_.cars[j];
                    if (std::abs(track.signedGap(c.distance, other.distance)) < 4.2 &&
                        std::abs(c.lane - other.lane) < 2.9f) {
                        c.invulnerable = dt * 2;
                        break;
                    }
                }
        }
    }
    // Resolve approximate fender boxes along the smallest penetration axis.
    // Side-by-side karts receive a small lateral nudge, never a four-metre
    // backward correction. Longitudinal contacts only affect the following kart.
    for (int pass = 0; pass < 12; pass++)
        for (int i = 0; i < RacerCount; i++)
            for (int j = i + 1; j < RacerCount; j++) {
                auto &a = current_.cars[i];
                auto &b = current_.cars[j];
                if (airborne(a) || airborne(b) || a.invulnerable > 0 || b.invulnerable > 0 ||
                    a.mode == CarMode::Spinout || b.mode == CarMode::Spinout)
                    continue;
                double d = track.signedGap(a.distance, b.distance);
                float dl = a.lane - b.lane;
                if (std::abs(d) < 3.95 && std::abs(dl) < 2.80f) {
                    float lat = 2.80f - std::abs(dl);
                    double longitudinal = 3.95 - std::abs(d);
                    if (lat < longitudinal) {
                        float side = dl < 0 ? -1.f : 1.f;
                        a.lane += side * (lat * .5f + .00001f);
                        b.lane -= side * (lat * .5f + .00001f);
                        if (a.lane > 4.65f) {
                            float extra = a.lane - 4.65f;
                            a.lane -= extra;
                            b.lane -= extra;
                        }
                        if (a.lane < -4.65f) {
                            float extra = -4.65f - a.lane;
                            a.lane += extra;
                            b.lane += extra;
                        }
                        if (b.lane > 4.65f) {
                            float extra = b.lane - 4.65f;
                            b.lane -= extra;
                            a.lane -= extra;
                        }
                        if (b.lane < -4.65f) {
                            float extra = -4.65f - b.lane;
                            b.lane += extra;
                            a.lane += extra;
                        }
                        a.laneSpeed = 0;
                        b.laneSpeed = 0;
                    } else {
                        Car &rear = d < 0 ? a : b;
                        const Car &front = d < 0 ? b : a;
                        rear.distance -= longitudinal;
                        rear.speed = std::min(rear.speed, front.speed);
                        rear.boost = 0;
                    }
                    counters.contacts++;
                    if (a.contactCooldown <= 0 && b.contactCooldown <= 0 && go) {
                        a.hit = .5f;
                        b.hit = .5f;
                        a.contactCooldown = 1;
                        b.contactCooldown = 1;
                        cue(saver::Cue::Hit, i);
                    }
                }
            }
    std::iota(current_.order.begin(), current_.order.end(), 0);
    std::stable_sort(current_.order.begin(), current_.order.end(), [&](int a, int b) {
        return current_.cars[a].distance > current_.cars[b].distance;
    });
    for (int rank = 0; rank < RacerCount; rank++)
        current_.cars[current_.order[rank]].rank = rank + 1;
    if (go)
        for (int a = 0; a < RacerCount; a++)
            for (int b = a + 1; b < RacerCount; b++) {
                if (before[a].distance < before[b].distance &&
                    current_.cars[a].distance >= current_.cars[b].distance)
                    counters.passes++;
                if (before[a].distance > before[b].distance &&
                    current_.cars[a].distance <= current_.cars[b].distance)
                    counters.passes++;
            }
    if (current_.winner < 0 && current_.cars[current_.order[0]].distance >= track.length * 3) {
        current_.winner = current_.order[0];
        current_.finishedAt = current_.time;
    }
    direct();
}
void Race::advance(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::invalid_argument("Invalid simulation interval");
    requestedTime_ += seconds;
    accumulator_ += seconds;
    while (accumulator_ + 1e-12 >= FixedStep) {
        step();
        accumulator_ -= FixedStep;
    }
    if (accumulator_ < 0)
        accumulator_ = 0;
}
void Race::seek(double t) {
    if (!std::isfinite(t) || t < 0 || t > 1e7)
        throw std::invalid_argument("Invalid seek time");
    if (t < requestedTime_ - 1e-9) {
        counters = {};
        soundSerial_ = 0;
        eventSerial_ = 0;
        accumulator_ = 0;
        requestedTime_ = 0;
        grid(0, 0);
    }
    advance(std::max(0., t - requestedTime_));
}
RaceState Race::sample() const {
    float a = float(accumulator_ / FixedStep);
    RaceState r = current_;
    if (current_.round != previous_.round)
        return r;
    r.time = previous_.time + (current_.time - previous_.time) * a;
    r.roundTime = previous_.roundTime + (current_.roundTime - previous_.roundTime) * a;
    for (int i = 0; i < RacerCount; i++) {
        auto &c = r.cars[i];
        const auto &b = previous_.cars[i];
        const auto &n = current_.cars[i];
        if (b.mode != n.mode)
            continue;
        c.distance = b.distance + (n.distance - b.distance) * a;
        c.lane = b.lane + (n.lane - b.lane) * a;
        c.laneSpeed = b.laneSpeed + (n.laneSpeed - b.laneSpeed) * a;
        c.speed = b.speed + (n.speed - b.speed) * a;
        c.steer = b.steer + (n.steer - b.steer) * a;
        c.drift = b.drift + (n.drift - b.drift) * a;
        c.boost = b.boost + (n.boost - b.boost) * a;
        c.wheelDistance = b.wheelDistance + (n.wheelDistance - b.wheelDistance) * a;
        c.airPosition = mix(b.airPosition, n.airPosition, a);
        c.spin = b.spin + (n.spin - b.spin) * a;
        c.modeTime = b.modeTime + (n.modeTime - b.modeTime) * a;
        c.hop = b.hop + (n.hop - b.hop) * a;
    }
    return r;
}
} // namespace sw
