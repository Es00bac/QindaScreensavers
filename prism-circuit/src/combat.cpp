// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"

namespace sw {
KartAnimation kartAnimation(const Car &c, bool cockpit) {
    return {c.speed,
            c.damage,
            c.hit,
            c.taunt,
            c.attackPose,
            c.look,
            c.mode == CarMode::Falling    ? 1.f
            : c.mode == CarMode::Rescuing ? .7f
                                          : 0.f,
            c.landing,
            cockpit};
}
bool airborne(const Car &c) {
    return c.mode == CarMode::Jumping || c.mode == CarMode::Falling || c.mode == CarMode::Rescuing;
}
V3 carPosition(const Car &c, const Track &track) {
    if (airborne(c))
        return c.airPosition;
    auto p = track.at(c.distance);
    return p.position + p.right * c.lane + p.up * c.hop;
}
M4 carMatrix(const Car &c, const Track &track) {
    auto p = track.at(c.distance);
    M4 base = basis(carPosition(c, track) + p.up * .014f, p.right, p.up, p.forward);
    if (c.mode == CarMode::Falling)
        return base * ry(c.spin + c.modeTime * 2.2f) * rz(c.modeTime * 2.5f) *
               rx(-c.modeTime * 1.7f);
    if (c.mode == CarMode::Rescuing) {
        float settle = 1 - ease(0, .85f, c.modeTime);
        return base * ry((c.spin + 1.65f * 2.2f) * settle) * rz(1.65f * 2.5f * settle) *
               rx(-1.65f * 1.7f * settle);
    }
    if (c.mode == CarMode::Jumping) {
        float u = clamp(float((c.distance - c.jumpDistance) / track.jumpLength));
        return base * ry(-c.drift) * rx((u - .5f) * .48f);
    }
    return base * ry(-c.drift + c.spin) * rz(c.hit * .12f * std::sin(c.modeTime * 23));
}
void Race::event(EventKind kind, int actor, int target, Item item) {
    if (current_.events.size() >= 96)
        current_.events.erase(current_.events.begin());
    current_.events.push_back({++eventSerial_, current_.time, kind, actor, target, item,
                               carPosition(current_.cars[target >= 0 ? target : actor], track)});
}
void Race::fall(int target, int owner) {
    auto &c = current_.cars[target];
    if (c.mode == CarMode::Falling || c.mode == CarMode::Rescuing)
        return;
    c.airPosition = carPosition(c, track);
    auto p = track.at(c.distance);
    float side = c.impulse < 0 ? -1.f : c.impulse > 0 ? 1.f : (target % 2 ? 1.f : -1.f);
    c.airVelocity = p.forward * (c.speed * .36f) + p.right * (side * 11.f) + p.up * 7.5f;
    c.mode = CarMode::Falling;
    c.modeTime = 0;
    c.speed = 0;
    c.boost = 0;
    c.shield = 0;
    c.item = Item::None;
    c.damage = 100;
    c.lastAttacker = owner;
    c.laneSpeed = 0;
    if (owner >= 0 && owner != target) {
        ++current_.cars[owner].takedowns;
        current_.cars[owner].taunt = 2.2f;
    }
    ++counters.knockouts;
    event(EventKind::Knockout, owner >= 0 ? owner : target, target);
    cue(saver::Cue::Knockout, target);
}
void Race::hit(int target, int owner, float damage, float force, Item item) {
    auto &c = current_.cars[target];
    if (c.mode == CarMode::Falling || c.mode == CarMode::Rescuing || c.invulnerable > 0)
        return;
    if (c.shield > 0) {
        c.shield = std::max(0.f, c.shield - 2.8f);
        ++counters.blocks;
        event(EventKind::Block, owner, target, item);
        cue(saver::Cue::Shield, target);
        return;
    }
    c.damage = std::min(100.f, c.damage + damage);
    c.hit = 1;
    c.lastAttacker = owner;
    c.invulnerable = .65f;
    c.speed *= .48f;
    c.boost = 0;
    c.impulse = force;
    c.spinRate = force < 0 ? -7.8f : 7.8f;
    c.stun = 1.6f;
    c.attackPose = 0;
    current_.cars[owner].taunt = 1.7f;
    ++counters.hits;
    event(EventKind::Hit, owner, target, item);
    cue(saver::Cue::Hit, target);
    if (c.damage >= 100 || c.mode == CarMode::Jumping) {
        fall(target, owner);
        return;
    }
    c.mode = CarMode::Spinout;
    c.modeTime = 0;
    ++counters.spinouts;
    event(EventKind::Spinout, owner, target, item);
}
bool Race::motion(int id) {
    auto &c = current_.cars[id];
    constexpr float dt = float(FixedStep);
    c.modeTime += dt;
    if (c.mode == CarMode::Jumping) {
        c.distance += c.speed * dt;
        c.wheelDistance += c.speed * dt;
        float u = clamp(float((c.distance - c.jumpDistance) / track.jumpLength));
        auto p = track.at(c.distance);
        c.hop = 4 * 8.5f * u * (1 - u);
        c.airPosition = p.position + p.right * c.lane + p.up * c.hop;
        if (u >= 1) {
            c.mode = CarMode::Recovering;
            c.modeTime = 0;
            c.invulnerable = 1.4f;
            c.landing = 1;
            c.hop = 0;
            cue(saver::Cue::Hit, id);
        }
        return true;
    }
    if (c.mode == CarMode::Falling) {
        c.airPosition = c.airPosition + c.airVelocity * dt + V3{0, -9.f * dt * dt, 0};
        c.airVelocity.y -= 18 * dt;
        float floor = terrainElevation(track, c.airPosition.x, c.airPosition.z) + .65f;
        if (c.airPosition.y < floor) {
            c.airPosition.y = floor;
            c.airVelocity.y = std::max(0.f, -c.airVelocity.y * .18f);
            c.airVelocity.x *= .90f;
            c.airVelocity.z *= .90f;
        }
        if (c.modeTime >= 1.65f) {
            c.mode = CarMode::Rescuing;
            c.modeTime = 0;
            c.rescueFrom = c.airPosition;
            float best = -1e6f;
            for (float lane : {-4.1f, 0.f, 4.1f}) {
                float score = -std::abs(lane - c.lane) * .05f;
                for (int j = 0; j < RacerCount; ++j)
                    if (j != id && !airborne(current_.cars[j])) {
                        double gap =
                            std::abs(track.signedGap(current_.cars[j].distance, c.distance));
                        if (gap < 25 && std::abs(current_.cars[j].lane - lane) < 2.9f)
                            score -= float(25 - gap);
                    }
                if (score > best) {
                    best = score;
                    c.returnLane = lane;
                }
            }
            event(EventKind::Rescue, id, id);
            cue(saver::Cue::Special, id);
        }
        return true;
    }
    if (c.mode == CarMode::Rescuing) {
        auto p = track.at(c.distance);
        V3 target = p.position + p.right * c.returnLane;
        // Reel vertically out of the abyss, carry over the road, then lower the kart.
        V3 lifted = c.rescueFrom + V3{0, 13, 0}, hover = target + p.up * 5.8f;
        if (c.modeTime < .95f)
            c.airPosition = mix(c.rescueFrom, lifted, ease(0, .95f, c.modeTime));
        else if (c.modeTime < 2.2f)
            c.airPosition = mix(lifted, hover, ease(.95f, 2.2f, c.modeTime));
        else {
            bool clear = true;
            for (int j = 0; j < RacerCount; ++j)
                if (j != id && !airborne(current_.cars[j])) {
                    const auto &other = current_.cars[j];
                    double gap = track.signedGap(other.distance, c.distance);
                    if (gap > -other.speed * 1.25 - 5 && gap < 6 &&
                        std::abs(other.lane - c.returnLane) < 2.9f)
                        clear = false;
                }
            if (!clear && c.modeTime < 2.2f + dt * 1.5f)
                c.modeTime = 2.2f;
            c.airPosition = mix(hover, target, ease(2.2f, 3.4f, c.modeTime));
        }
        if (c.modeTime >= 3.4f) {
            c.mode = CarMode::Recovering;
            c.modeTime = 0;
            c.lane = c.returnLane;
            c.targetLane = c.lane;
            c.speed = 9;
            c.damage = 0;
            c.spin = 0;
            c.impulse = 0;
            c.hop = 0;
            c.landing = 1;
            c.invulnerable = 3;
            ++counters.rescues;
            cue(saver::Cue::Shield, id);
        }
        return true;
    }
    if (c.mode == CarMode::Recovering && c.modeTime >= 2.5f)
        c.mode = CarMode::Racing;
    if (c.mode == CarMode::Spinout) {
        c.spin += c.spinRate * dt;
        c.spinRate *= std::exp(-.55f * dt);
        c.hop = .24f * std::abs(std::sin(c.modeTime * 8)) * std::exp(-c.modeTime * 1.8f);
        if (c.modeTime > 1.65f) {
            if (std::abs(c.lane) > track.halfWidth - 1.0f) {
                fall(id, c.lastAttacker);
                return true;
            }
            c.mode = CarMode::Recovering;
            c.modeTime = 0;
            c.invulnerable = 1;
            c.hop = 0;
            c.targetLane = clamp(c.lane, -4.1f, 4.1f);
        }
    } else {
        // Settle to the nearest full turn; never interpolate a wrap through 360 degrees.
        c.spin += (std::round(c.spin / (2 * pi)) * 2 * pi - c.spin) * std::min(1.f, dt * 8);
    }
    return false;
}
void Race::weapons() {
    constexpr float dt = float(FixedStep);
    for (auto &p : current_.projectiles) {
        p.age += dt;
        if (p.target < 0 || current_.cars[p.target].mode == CarMode::Falling ||
            current_.cars[p.target].mode == CarMode::Rescuing) {
            p.age = 8;
            continue;
        }
        const auto &victim = current_.cars[p.target];
        double before = track.signedGap(victim.distance, p.distance);
        p.speed = std::min(68.f, p.speed + 12 * dt);
        p.distance += p.speed * dt;
        p.lane += clamp(victim.lane - p.lane, -6.5f * dt, 6.5f * dt);
        double after = track.signedGap(victim.distance, p.distance);
        if ((std::abs(after) < 2.4 || (before > 0 && after < 0)) &&
            std::abs(victim.lane - p.lane) < 2.5f) {
            float side = victim.lane > current_.cars[p.owner].lane ? 1.f : -1.f;
            hit(p.target, p.owner, 48, side * 9, Item::Seeker);
            p.age = 8;
        } else if (after < -5)
            p.age = 8;
    }
    std::erase_if(current_.projectiles, [](const Projectile &p) { return p.age >= 7; });
    for (auto &trap : current_.traps) {
        trap.age += dt;
        if (trap.age < .65f)
            continue;
        for (int id = 0; id < RacerCount; ++id)
            if (!airborne(current_.cars[id])) {
                const auto &c = current_.cars[id];
                if ((id == trap.owner && trap.age < 2) || c.invulnerable > 0)
                    continue;
                float radius = trap.item == Item::Oil ? 1.8f : 1.35f;
                if (std::abs(track.signedGap(c.distance, trap.distance)) < 2.5f &&
                    std::abs(c.lane - trap.lane) < radius) {
                    hit(id, trap.owner, trap.item == Item::Oil ? 29 : 60,
                        (c.lane >= 0 ? 1.f : -1.f) * (trap.item == Item::Oil ? 11.f : 14.f),
                        trap.item);
                    trap.age = 30;
                    ++counters.trapsHit;
                    break;
                }
            }
    }
    std::erase_if(current_.traps, [](const Trap &t) { return t.age >= 24; });
}
} // namespace sw
