#pragma once
#include <array>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace patrol {
constexpr int ViewW = 960, ViewH = 540, Tile = 32;
constexpr double FixedDt = 1.0 / 120.0, Gravity = 980.0;
// Districts are measured in absolute world distance rather than roof counts, so
// every parallax layer can resolve the district for any x it draws and blend
// toward the neighbor across DistrictBlend pixels on either side of the line.
constexpr double DistrictLength = 4096, DistrictBlend = 768;
constexpr double RunSpeed = 152, FightSpeed = 94;
struct Rng {
    std::uint64_t state;
    std::uint64_t next();
    int range(int lo, int hi);
    double unit();
};
struct Vec { double x=0, y=0; };
enum class Biome : int { Rooftops, Memory, Cooling, Network };
enum class EnemyKind : int { Bug, Leak, Zombie, Drone };
enum class RoofStyle : int { Building, Catwalk };
enum class Facility : int { Generator, Greenhouse, Pump, Uplink };
enum class Power : int { None, Rapid, Shield, Arc };
const char* bossName(int kind);
const char* facilityName(Facility kind);
const char* powerName(Power kind);
struct Platform {
    std::uint64_t id=0;
    double x=0, y=416, width=384;
    Biome biome=Biome::Rooftops;
    RoofStyle style=RoofStyle::Building;
    std::int64_t district=0;
    int sign=-1, decoration=0, structure=0, pattern=0;
    bool coffee=false, gate=false, cable=false, arena=false, repaired=false;
    Facility facility=Facility::Generator;
    bool service=false;
};
struct Enemy {
    std::uint64_t id=0, platform=0;
    Vec pos;
    double origin=0, phase=0, hit=0;
    EnemyKind kind=EnemyKind::Bug;
    int hp=2;
    bool boss=false;
    int bossKind=0, maxHp=2, stage=0;
    double attackClock=0;
};
struct Bolt { Vec pos, velocity; double life=1.5; bool duck=false; };
struct Pickup { Vec pos; Power kind=Power::Rapid; double age=0; };
struct Hazard { Vec pos, velocity; double life=0, radius=8; int kind=0; };
struct Particle { Vec pos, velocity; double life=0, initialLife=0; int color=0; };
struct Hero {
    Vec pos{120,416};
    Vec jumpStart, jumpEnd;
    double jumpAge=0, jumpDuration=0, jumpVy=0;
    bool airborne=false;
    double fire=0, coffee=0, coffeeCooldown=18, flash=0;
    std::uint64_t platform=0;
    Power power=Power::None; double powerTime=0, serviceTime=0, dodge=0, stagger=0;
};
struct JumpPlan { double duration=0, vy=0, apex=0; bool valid=false; };
JumpPlan planJump(Vec from, Vec to);
const char* biomeName(Biome biome);
const char* enemyName(EnemyKind kind);
// Which district owns an absolute x, and how far it has blended into the next.
struct DistrictMix { Biome from=Biome::Rooftops, to=Biome::Rooftops; double t=0; };
Biome districtBiome(std::int64_t district);
DistrictMix districtAt(double absoluteX);
class World {
public:
    explicit World(std::uint64_t seed=551767);
    void step(double dt=FixedDt);
    void reset(std::uint64_t seed);
    const Platform* platform(std::uint64_t id) const;
    Hero hero;
    Vec duck{66,340};
    double time=0, camera=0, duckFire=0, coffeeBeat=0;
    std::uint64_t cleared=0, distanceBase=0, rescues=0, bossesDefeated=0, repairs=0, collected=0, dodges=0, shieldBlocks=0;
    std::vector<Pickup> pickups; std::vector<Hazard> hazards;
    double victory=0; int lastBoss=-1;
    std::deque<Platform> platforms;
    std::vector<Enemy> enemies;
    std::vector<Bolt> bolts;
    std::vector<Particle> particles;
    Biome currentBiome() const;
    std::uint64_t seed() const { return seed_; }
    // Rebase-proof coordinates for anything that must stay continuous for hours.
    double absoluteCamera() const { return camera+double(distanceBase); }
    double absoluteHero() const { return hero.pos.x+double(distanceBase); }
    void burst(Vec at, int color, int count=12);
private:
    std::uint64_t seed_=0, nextPlatform_=0, nextEnemy_=0, nextSign_=0;
    int sinceSign_=2;
    Rng terrain_{0}, effects_{0}, bosses_{0};
    std::array<int,4> bossOrder_{0,1,2,3}; int bossIndex_=4;
    std::uint64_t nextArena_=7;
    int takeBoss();
    void appendPlatform();
    void maintain();
    void fire(Vec from, const Enemy& enemy, bool byDuck);
};
}
