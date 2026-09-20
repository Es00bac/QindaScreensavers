// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "scene.hpp"
#include "sound_events.hpp"
#include <array>
#include <vector>
namespace sw {
constexpr double FixedStep=1.0/120.0;
constexpr int MaxFighters=8;
enum class Action {Idle,Run,Jump,Jab,Heavy,Aerial,Special,Shield,Dodge,Hurt,Recovery,Grab,Celebrate};
struct Move {float duration,start,end,damage,launch,reach,radius,lift;};
Move move(Action,int id=0);
const char* specialName(int id);
struct Platform {float x,y,half,depth; bool solid=false;};
std::array<Platform,4> platforms(int stage,double time);
const char* stageName(int);
struct Anim {
 float gait=0,walk=0,punch=0,heavy=0,aerial=0,shield=0,duck=0,recoil=0,cast=0,recover=0,grab=0,air=0,land=0,yaw=0;
};
struct Fighter {
 int id=0,stocks=3,kos=0,lastHit=-1,floor=0,jumps=2,face=1,target=-1;
 float x=0,y=0,vx=0,vy=0,damage=0,shield=1,actionTime=0;
 float decision=0,cooldown=0,specialCooldown=0,stun=0,invulnerable=0,respawn=0,drop=0,hitstop=0,overclock=0;
 bool projectileMade=false,recoveryUsed=false,seeking=false;float goalX=0,goalY=0;
 std::uint16_t hits=0;
 Action action=Action::Idle; Anim anim;
};
struct Projectile {V3 p,v;float age=0,life=2.5f,radius=.27f;int owner=0,id=0;float damage=8,launch=5.5f,lift=.43f;std::uint16_t hits=0;bool turned=false;};
struct Effect {V3 p,v;float age=0,life=.5f;int kind=0,id=0;};
struct Pickup {float x=0,y=0,age=0;int kind=0,platform=0;};
struct BattleState {
 std::array<Fighter,MaxFighters> fighters{};
 std::vector<saver::SoundEvent> sounds;
 std::vector<Projectile> shots;std::vector<Effect> effects;std::vector<Pickup> pickups;
 double time=0,roundTime=0,finishedAt=-1;unsigned round=0;int stage=0,active=4,winner=-1;
 V3 cameraTarget{0,3.1f,0};float cameraDistance=35;
};
struct Counters {std::uint64_t ticks=0,hits=0,blocks=0,kos=0,jumps=0,recoveries=0,specials=0,grabs=0,rounds=0,pickups=0;};
class Battle {
 BattleState current_,previous_;Random rng_;std::uint64_t seed_;int stageChoice_,count_;
 double accumulator_=0;float nextPickup_=10;std::uint64_t soundSerial_=0;
 void cue(saver::Cue,int slot);void special(int slot);
 void reset(unsigned round,double globalTime);
 void step();void think(int slot);void begin(int slot,Action action);
 void hit(int from,int to,float damage,float launch,float dx,float lift,bool bypass=false);
 void effect(V3 pos,int kind,int id,V3 velocity={});
 friend struct BattleTest;
public:
 Counters counters;
 explicit Battle(std::uint64_t seed=41,int stage=-1,int count=4);
 void advance(double seconds);void seek(double time);
 BattleState sample()const;
 const BattleState& state()const{return current_;}
};
enum class Camera {Director,Fixed,Close};
struct SceneOptions {Camera camera=Camera::Director;bool reduced=false,gallery=false;int galleryId=0;};
void fighterModel(Frame&,M4,int id,double time,const Anim&);
void stageModel(Frame&,int stage,double time,std::uint64_t seed,bool environment=true);
void compose(Frame&,const BattleState&,const SceneOptions&);
}
