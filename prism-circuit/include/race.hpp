// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "scene.hpp"
#include "sound_events.hpp"
namespace sw {
constexpr int RacerCount = 8;
constexpr double FixedStep = 1.0 / 120.0;
struct TrackPose {
    V3 position, right, up, forward;
    float bank = 0, curvature = 0;
    M4 matrix(float lane = 0, float height = 0) const {
        return basis(position + right * lane + up * height, right, up, forward);
    }
};
struct BoostPad {
    double distance;
    float lane;
};
class Track {
    static constexpr int Samples = 4096;
    std::array<double, Samples + 1> lengths_{};
    std::array<TrackPose, Samples + 1> poses_{};
    V3 curve(double u) const;
    TrackPose exact(double u) const;

  public:
    std::uint64_t seed;
    int kind;
    float halfWidth = 7.1f;
    double length = 1;
    float variation = 0;
    std::array<BoostPad, 6> pads{};
    double jumpStart = -1, jumpLength = 34;
    explicit Track(std::uint64_t s = 41, int k = 0);
    TrackPose at(double distance) const;
    double signedGap(double a, double b) const;
    const char *name() const;
};
constexpr int CourseCount = 5;
enum class Item { None, Turbo, Shield, Pulse, Magnet, Seeker, Oil, Mine };
const char *itemName(Item);
enum class CarMode { Racing, Spinout, Jumping, Falling, Rescuing, Recovering };
enum class Camera { Director, Chase, Front, Orbit, Overview, Cockpit, Trackside, Pack, Rescue };
const char *cameraName(Camera);
enum class EventKind { Launch, Hit, Block, Spinout, Knockout, Rescue, Pass, Trap, Jump };
struct RaceEvent {
    std::uint64_t serial = 0;
    double time = 0;
    EventKind kind = EventKind::Hit;
    int actor = 0, target = -1;
    Item item = Item::None;
    V3 position;
};
struct Projectile {
    std::uint64_t serial = 0;
    int owner = 0, target = -1;
    double distance = 0;
    float lane = 0, age = 0, speed = 48;
};
struct Trap {
    int owner = 0;
    Item item = Item::Oil;
    double distance = 0;
    float lane = 0, age = 0;
};
struct DirectorState {
    Camera camera = Camera::Front;
    int focus = 0, secondary = 1;
    unsigned shot = 0;
    double startedAt = 0, until = 4.2, stationDistance = 0;
    std::uint64_t lastEvent = 0;
    std::array<double, RacerCount> lastSeen{};
};
struct Car {
    double distance = 0;
    double wheelDistance = 0;
    float lane = 0, laneSpeed = 0, speed = 0, targetLane = 0, boost = 0, cooldown = 0, steer = 0,
          drift = 0;
    float pace = 25, decision = 0;
    int rank = 1;
    Item item = Item::None;
    float itemAge = 0, shield = 0, magnet = 0, pulse = 0, stun = 0;
    CarMode mode = CarMode::Racing;
    float modeTime = 0, damage = 0, hit = 0, invulnerable = 0, taunt = 0, attackPose = 0,
          contactCooldown = 0;
    float spin = 0, spinRate = 0, impulse = 0, hop = 0, landing = 0, look = 0;
    int rival = -1, lastAttacker = -1, takedowns = 0;
    V3 airPosition{}, airVelocity{}, rescueFrom{};
    float returnLane = 0;
    double jumpDistance = 0;
};
struct RaceState {
    std::array<Car, RacerCount> cars{};
    std::array<float, 24> crates{};
    std::vector<saver::SoundEvent> sounds;
    double time = 0, roundTime = 0, finishedAt = -1;
    unsigned round = 0;
    int winner = -1;
    std::array<int, 8> order{};
    std::vector<Projectile> projectiles;
    std::vector<Trap> traps;
    std::vector<RaceEvent> events;
    DirectorState director;
};
struct RaceCounters {
    std::uint64_t ticks = 0, passes = 0, boosts = 0, rounds = 0, contacts = 0, pickups = 0,
                  itemsUsed = 0, blocks = 0, hits = 0, spinouts = 0, knockouts = 0, rescues = 0,
                  shots = 0, trapsHit = 0, cameraCuts = 0;
};
class Race {
    RaceState current_, previous_;
    double accumulator_ = 0, requestedTime_ = 0;
    std::uint64_t soundSerial_ = 0;
    void cue(saver::Cue, int car);
    std::uint64_t eventSerial_ = 0;
    void event(EventKind, int actor, int target = -1, Item item = Item::None);
    void hit(int target, int owner, float damage, float force, Item item);
    void fall(int target, int owner);
    void weapons();
    bool motion(int car);
    void direct();
    void grid(unsigned round, double globalTime);
    void step();

  public:
    Track track;
    RaceCounters counters;
    explicit Race(std::uint64_t seed = 41, int course = 0);
    void advance(double seconds);
    void seek(double time);
    RaceState sample() const;
    const RaceState &state() const { return current_; }
};
struct SceneOptions {
    Camera camera = Camera::Director;
    int focus = -1;
    bool reduced = false;
    bool gallery = false;
    int galleryId = 0;
    float orbitYaw = 0, orbitPitch = 0, zoom = 1;
};
bool airborne(const Car &);
V3 carPosition(const Car &, const Track &);
M4 carMatrix(const Car &, const Track &);
KartAnimation kartAnimation(const Car &, bool cockpit = false);
void frameCamera(Frame &, const RaceState &, const Track &, const SceneOptions &);
void clearCamera(Frame &, const RaceState &, const Track &, const SceneOptions &);
void courseScenery(Frame &, const Track &, double time);
void groundedSkyline(Frame &, const Track &);
void buildSceneryMeshes(Frame &, const Track &);
bool trackClearance(const Track &, const ScenerySolid &);
float terrainElevation(const Track &, float x, float z);
void buildTrackMeshes(Frame &, const Track &);
void compose(Frame &, const RaceState &, const Track &, const SceneOptions &);
} // namespace sw
