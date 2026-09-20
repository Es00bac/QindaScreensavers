// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"

namespace sw {
const char *cameraName(Camera camera) {
    static const char *names[]{"AUTO DIRECTOR", "CHASE",      "FRONT",
                               "ORBIT",         "OVERVIEW",   "COCKPIT",
                               "TRACKSIDE",     "BATTLE CAM", "RECOVERY CAM"};
    return names[int(camera)];
}
void Race::direct() {
    auto &d = current_.director;
    double now = current_.time, age = now - d.startedAt;
    // A recovery stays in shot through hook-up and landing. Ordinary action
    // can only interrupt after a minimum hold, avoiding rapid-fire camera cuts.
    bool rescueShot = d.camera == Camera::Rescue && airborne(current_.cars[d.focus]);
    if (rescueShot && age < 7.5)
        return;
    const RaceEvent *story = nullptr;
    int priority = 0;
    if (age >= 3.0)
        for (const auto &e : current_.events) {
            if (e.serial <= d.lastEvent || now - e.time > 1.3)
                continue;
            int p = e.kind == EventKind::Knockout  ? 6
                    : e.kind == EventKind::Rescue  ? 5
                    : e.kind == EventKind::Jump    ? 4
                    : e.kind == EventKind::Launch  ? 3
                    : e.kind == EventKind::Spinout ? 2
                                                   : 0;
            if (p > priority) {
                priority = p;
                story = &e;
            }
        }
    if (now < d.until && !story)
        return;
    Camera camera;
    int focus = -1, secondary = -1;
    double hold = 5.4;
    if (story) {
        d.lastEvent = story->serial;
        focus = story->kind == EventKind::Launch ? story->actor : story->target;
        secondary = story->actor;
        camera = priority >= 5 ? Camera::Rescue : priority == 4 ? Camera::Front : Camera::Pack;
        hold = priority >= 5 ? 6.2 : 4.5;
    } else {
        float best = -1e6;
        for (int i = 0; i < RacerCount; ++i) {
            const auto &car = current_.cars[i];
            if (airborne(car))
                continue;
            float interest = float(std::min(45., now - d.lastSeen[i])) * .16f;
            if (i == d.focus)
                interest -= 6;
            if (car.item == Item::Seeker || car.item == Item::Pulse)
                interest += 3;
            if (car.rank == 1)
                interest += 1.2f;
            for (int j = 0; j < RacerCount; ++j)
                if (j != i && !airborne(current_.cars[j])) {
                    float gap =
                        float(std::abs(track.signedGap(car.distance, current_.cars[j].distance)));
                    if (gap < 28)
                        interest += (28 - gap) * .11f;
                }
            if (interest > best) {
                best = interest;
                focus = i;
            }
        }
        if (focus < 0)
            focus = current_.order[0];
        constexpr Camera sequence[]{Camera::Chase, Camera::Trackside, Camera::Cockpit, Camera::Pack,
                                    Camera::Front, Camera::Trackside, Camera::Orbit};
        camera = sequence[d.shot % 7];
        hold = camera == Camera::Cockpit ? 5 : 6;
        if (current_.cars[focus].mode == CarMode::Spinout && camera == Camera::Cockpit)
            camera = Camera::Front;
    }
    if (focus < 0)
        focus = current_.order[0];
    if (secondary < 0 || secondary == focus) {
        float best = 1e9;
        for (int j = 0; j < RacerCount; ++j)
            if (j != focus && !airborne(current_.cars[j])) {
                float gap = float(std::abs(
                    track.signedGap(current_.cars[j].distance, current_.cars[focus].distance)));
                if (gap < best) {
                    best = gap;
                    secondary = j;
                }
            }
    }
    if (secondary < 0)
        secondary = focus;
    if (camera == Camera::Trackside) {
        double interval = track.length / 32;
        double ahead = current_.cars[focus].distance + 45;
        d.stationDistance = std::ceil(ahead / interval) * interval;
        hold = std::clamp((d.stationDistance - current_.cars[focus].distance) /
                                  std::max(14.f, current_.cars[focus].speed) +
                              1.2,
                          3.6, 7.0);
    }
    d.camera = camera;
    d.focus = focus;
    d.secondary = secondary;
    d.startedAt = now;
    d.until = now + hold;
    d.lastSeen[focus] = now;
    ++d.shot;
    ++counters.cameraCuts;
}
void frameCamera(Frame &f, const RaceState &race, const Track &track, const SceneOptions &opt) {
    const auto &director = race.director;
    Camera camera = opt.camera == Camera::Director ? director.camera : opt.camera;
    int focus = opt.focus >= 0 ? opt.focus : opt.camera == Camera::Director ? director.focus : 0;
    const auto &c = race.cars[focus];
    auto pose = track.at(c.distance);
    V3 hero = carPosition(c, track), up = pose.up, forward = pose.forward, right = pose.right;
    int second = director.secondary;
    V3 companion = carPosition(race.cars[second], track);
    if (length(companion - hero) > 36)
        companion = hero;
    if (opt.reduced && camera != Camera::Overview)
        camera = Camera::Chase;
    if ((c.mode == CarMode::Falling || c.mode == CarMode::Rescuing) && camera != Camera::Overview)
        camera = Camera::Rescue;
    else if (camera == Camera::Rescue && opt.focus >= 0)
        camera = Camera::Front;
    if (c.mode == CarMode::Spinout && camera == Camera::Cockpit)
        camera = Camera::Chase;
    double age =
        opt.camera == Camera::Director ? std::max(0., race.time - director.startedAt) : race.time;
    f.focus = focus;
    f.pengu = hero;
    f.ducke = companion;
    f.cockpit = camera == Camera::Cockpit;
    f.lap = std::clamp(int(std::max(0., c.distance) / track.length) + 1, 1, 3);
    f.position = c.rank;
    f.speed = c.speed * 3.6f;
    f.boost = c.boost;
    f.damage = c.damage;
    f.takedowns = c.takedowns;
    f.shotLabel = cameraName(camera);
    f.countdown = race.roundTime < 4 ? std::max(1, 4 - int(race.roundTime)) : 0;
    f.target = hero + up * 1.2f;
    f.up = unit(mix(V3{0, 1, 0}, up, .4f));
    f.fov = .83f;
    if (camera == Camera::Chase) {
        auto behind = track.at(c.distance - (opt.reduced ? 17 : 9.8));
        f.eye =
            behind.position + behind.right * (c.lane * .45f) + behind.up * (opt.reduced ? 7 : 3.9f);
        f.target = hero + forward * 3 + up * 1.1f;
        f.fov = .85f + clamp(c.speed / 35) * .06f;
    } else if (camera == Camera::Cockpit) {
        auto anim = kartAnimation(c, true);
        M4 m = carMatrix(c, track) * kartChassis(race.time, focus, c.drift, anim);
        f.eye = point(m, {0, driverSeatY[focus] + driverSizes[focus].y * 1.1f, -.22f});
        auto ahead = track.at(c.distance + 26);
        f.target = ahead.position + ahead.right * c.lane * .65f + ahead.up * 1.0f;
        f.up = unit(direction(m, {0, 1, 0}));
        f.fov = 1.17f;
    } else if (camera == Camera::Front) {
        auto ahead = track.at(c.distance + 11.4);
        f.eye = ahead.position + ahead.right * 5.8f + ahead.up * 3.7f;
        if (c.mode == CarMode::Jumping)
            f.eye = hero + forward * 11.4f + right * 5.8f + up * 3.7f;
        f.target = hero + up * 1.18f;
        f.fov = .75f;
    } else if (camera == Camera::Trackside) {
        double spacing = track.length / 32;
        double station = opt.camera == Camera::Director
                             ? director.stationDistance
                             : std::floor((c.distance + 24) / spacing) * spacing;
        auto stand = track.at(station);
        int side = int(std::floor(station / spacing)) % 2 ? 1 : -1;
        // The camera body is fixed at an authored station; only its pan follows the pack.
        f.eye = stand.position + stand.right * (side * 11.f) + stand.up * 4.8f;
        f.target = mix(hero, companion, .18f) + up * 1.3f;
        f.up = {0, 1, 0};
        f.fov = .83f;
    } else if (camera == Camera::Pack) {
        V3 center = mix(hero, companion, .4f);
        float spread = length(companion - hero);
        f.eye = center + right * (10 + spread * .25f) - forward * 5 + up * (4.8f + spread * .10f);
        f.target = center + up * 1.2f;
        f.fov = .89f;
    } else if (camera == Camera::Rescue) {
        f.eye = hero + right * 10 + forward * 11 + V3{0, 7.6f, 0};
        f.target = hero + V3{0, 2.2f, 0};
        f.up = {0, 1, 0};
        f.fov = .84f;
    } else if (camera == Camera::Overview) {
        float angle = float(race.time) * .011f;
        f.eye = {std::sin(angle) * 360, 270, std::cos(angle) * 360};
        f.target = {0, 4, 0};
        f.up = {0, 1, 0};
        f.fov = 1.0f;
    } else {
        float angle = .7f + float(age) * .13f + opt.orbitYaw;
        f.eye = hero + right * (std::sin(angle) * 13) + forward * (std::cos(angle) * 13) +
                up * (5.6f + opt.orbitPitch * 9);
    }
    if (camera != Camera::Cockpit)
        f.eye = f.target + (f.eye - f.target) * opt.zoom;
    f.eventLabel.clear();
    for (auto it = race.events.rbegin(); it != race.events.rend(); ++it) {
        if (race.time - it->time > 2.4)
            break;
        if (it->kind == EventKind::Knockout) {
            f.eventLabel = std::string(racerNames[it->actor]) + "  >  " + racerNames[it->target] +
                           "  /  TAKEDOWN";
            break;
        }
        if (it->kind == EventKind::Rescue) {
            f.eventLabel = "TOW-BOT  /  RECOVERY IN PROGRESS";
            break;
        }
        if (it->kind == EventKind::Jump) {
            f.eventLabel = "SKYWAY GAP  /  AIRBORNE";
            break;
        }
    }
}

void clearCamera(Frame &f, const RaceState &race, const Track &track, const SceneOptions &opt) {
    if (f.cockpit || opt.camera == Camera::Overview)
        return;
    auto blocked = [&]() {
        if (f.eye.y < terrainElevation(track, f.eye.x, f.eye.z) + 1.5f)
            return true;
        V3 ray = f.eye - f.target;
        auto coordinate = [](V3 v, int axis) { return axis == 0 ? v.x : axis == 1 ? v.y : v.z; };
        for (const auto &b : f.scenerySolids) {
            float enter = .06f, leave = 1;
            for (int axis = 0; axis < 3; axis++) {
                float origin = coordinate(f.target, axis), d = coordinate(ray, axis),
                      center = coordinate(b.center, axis), half = coordinate(b.half, axis) + .3f;
                if (std::abs(d) < .00001f) {
                    if (std::abs(origin - center) > half) {
                        leave = -1;
                        break;
                    }
                } else {
                    float a = (center - half - origin) / d, z = (center + half - origin) / d;
                    if (a > z)
                        std::swap(a, z);
                    enter = std::max(enter, a);
                    leave = std::min(leave, z);
                }
            }
            if (enter < leave)
                return true;
        }
        return false;
    };
    if (!blocked())
        return;
    const auto &c = race.cars[f.focus];
    auto p = track.at(c.distance);
    if (airborne(c)) {
        f.eye = carPosition(c, track) + V3{0, 13, 0} + p.right * 4 + p.forward * 5;
        f.eye.y = std::max(f.eye.y, terrainElevation(track, f.eye.x, f.eye.z) + 4);
    } else {
        auto behind = track.at(c.distance - 9);
        f.eye = behind.position + behind.right * (c.lane * .65f) + behind.up * 3.5f;
        f.target = carPosition(c, track) + p.up * 1.3f;
        f.fov = .92f;
    }
    f.shotLabel += " / CLEAR VIEW";
}
} // namespace sw
