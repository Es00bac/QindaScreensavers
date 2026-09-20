// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "math.hpp"
#include <string>
#include <vector>
namespace sw {
struct Vertex {
    V3 p, n;
    V2 uv;
};
struct Mesh {
    std::string name;
    std::vector<Vertex> v;
    std::vector<std::uint32_t> ix;
};
struct Material {
    V3 color;
    float emission = 0, rough = .45f, metal = .1f, kind = 0, detail = 0;
};
struct Instance {
    M4 model;
    V4 color;
    V4 surface;
};
struct Batch {
    Mesh mesh;
    std::vector<Instance> instances;
};
enum Primitive {
    Sphere,
    Box,
    Torus,
    Cylinder,
    Hull,
    Wing,
    Rock0,
    Rock1,
    Rock2,
    Rock3,
    Rock4,
    Rock5,
    Monolith,
    Manta,
    Sideplate,
    Ribbon,
    RoadSurface,
    RoadShell,
    Ear,
    Terrain,
    Architecture,
    Smoke,
    Count
};
struct ScenerySolid {
    V3 center, half;
    double attachedAt = -1;
};
struct Frame {
    std::array<Batch, Count> batches;
    V3 eye, target, up{0, 1, 0}, pengu, ducke;
    M4 pship, dship;
    int chapter = 0;
    float local = 0, fade = 1, fov = .88f;
    double time = 0, motionTime = 0;
    std::uint64_t voyage = 0;
    int focus = 0, lap = 1, position = 1, winner = -1, countdown = 0;
    float speed = 0, boost = 0;
    std::string itemLabel;
    std::string shotLabel, eventLabel;
    float damage = 0;
    int takedowns = 0;
    bool cockpit = false;
    std::string courseName = "PRISM KNOT";
    std::array<int, 8> order{};
    std::array<float, 8> mapX{}, mapY{};
    std::vector<V2> map;
    std::vector<ScenerySolid> scenerySolids;
};
inline M4 basis(V3 position, V3 right, V3 up, V3 forward) {
    M4 m = M4::identity();
    m.a = {right.x,   right.y,   right.z,   0, up.x,       up.y,       up.z,       0,
           forward.x, forward.y, forward.z, 0, position.x, position.y, position.z, 1};
    return m;
}
inline V3 direction(M4 m, V3 p) {
    return point(m, p) - point(m, {0, 0, 0});
}
void initialize(Frame &);
void add(Frame &, Primitive, M4, Material);
struct KartAnimation {
    float speed = 0, damage = 0, hit = 0, taunt = 0, attack = 0, look = 0, airborne = 0,
          landing = 0;
    bool cockpit = false;
};
struct DriverAnimation {
    V3 leftGrip{}, rightGrip{};
    float lean = 0, hit = 0, taunt = 0, look = 0, airborne = 0;
    bool cockpit = false;
};
inline constexpr V3 driverSizes[] = {{.80f, .85f, .82f}, {.62f, .65f, .68f}, {.75f, .82f, .78f},
                                     {.82f, .76f, .83f}, {.65f, .77f, .65f}, {.68f, .76f, .69f},
                                     {.86f, .88f, .86f}, {.68f, .67f, .72f}};
inline constexpr float driverSeatY[] = {.94f, 1.02f, .95f, .97f, 1.00f, .98f, .93f, 1.03f};
M4 steeringWheel(float steer);
V3 steeringGrip(int side, float steer);
M4 kartChassis(double time, int id, float drift, const KartAnimation &);
void kart(Frame &, M4, int id, double time, float wheel, float steer, float drift, float boost,
          const KartAnimation &animation = {});
void animal(Frame &, M4, int id, double time, float steering, const DriverAnimation &);
void rescueRobot(Frame &, V3 kartPosition, V3 up, V3 forward, double time, float phase);
void arch(Frame &, M4, int sector, double time);
void exportModels(const std::string &, std::uint64_t seed = 41, int course = 0);
inline constexpr const char *racerNames[] = {"CyberPengu", "Ducké", "Vix",     "Cache",
                                             "Mochi",      "Hex",   "Patches", "Axi"};
inline constexpr const char *racerSlugs[] = {"cyberpengu",        "ducke",        "vix-fox",
                                             "cache-raccoon",     "mochi-rabbit", "hex-cat",
                                             "patches-red-panda", "axi-axolotl"};
inline constexpr V3 racerColors[] = {{.10f, .84f, .66f}, {1.f, .58f, .12f}, {1.f, .22f, .39f},
                                     {.13f, .61f, 1.f},  {.61f, .34f, 1.f}, {.95f, .19f, .72f},
                                     {.51f, .88f, .20f}, {.94f, .50f, .72f}};
} // namespace sw
