// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "scene.hpp"
#include "sound_events.hpp"
namespace sw {
constexpr int RacerCount=8;
constexpr double FixedStep=1.0/120.0;
struct TrackPose { V3 position,right,up,forward; float bank=0,curvature=0; M4 matrix(float lane=0,float height=0)const{return basis(position+right*lane+up*height,right,up,forward);} };
struct BoostPad { double distance; float lane; };
class Track {
 static constexpr int Samples=4096;
 std::array<double,Samples+1> lengths_{};
 std::array<TrackPose,Samples+1> poses_{};
 V3 curve(double u)const;
 TrackPose exact(double u)const;
public:
 std::uint64_t seed; int kind; float halfWidth=7.1f; double length=1; float variation=0;
 std::array<BoostPad,6> pads{};
 explicit Track(std::uint64_t s=41,int k=0);
 TrackPose at(double distance)const;
 double signedGap(double a,double b)const;
 const char* name()const;
};
enum class Item { None, Turbo, Shield, Pulse, Magnet };
const char* itemName(Item);
struct Car {
 double distance=0;
 float lane=0,laneSpeed=0,speed=0,targetLane=0,boost=0,cooldown=0,steer=0,drift=0;
 float pace=25,decision=0;
 int rank=1;
 Item item=Item::None;float itemAge=0,shield=0,magnet=0,pulse=0,stun=0;
};
struct RaceState {
 std::array<Car,RacerCount> cars{};
 std::array<float,24> crates{};std::vector<saver::SoundEvent> sounds;
 double time=0,roundTime=0,finishedAt=-1;
 unsigned round=0; int winner=-1; std::array<int,8> order{};
};
struct RaceCounters {std::uint64_t ticks=0,passes=0,boosts=0,rounds=0,contacts=0,pickups=0,itemsUsed=0,blocks=0;};
class Race {
 RaceState current_,previous_;
 double accumulator_=0,requestedTime_=0;std::uint64_t soundSerial_=0;
 void cue(saver::Cue,int car);
 void grid(unsigned round,double globalTime);
 void step();
public:
 Track track; RaceCounters counters;
 explicit Race(std::uint64_t seed=41,int course=0);
 void advance(double seconds);
 void seek(double time);
 RaceState sample()const;
 const RaceState& state()const{return current_;}
};
enum class Camera {Director,Chase,Front,Orbit,Overview};
struct SceneOptions {Camera camera=Camera::Director;int focus=-1;bool reduced=false;bool gallery=false;int galleryId=0;};
void buildTrackMeshes(Frame&,const Track&);
void compose(Frame&,const RaceState&,const Track&,const SceneOptions&);
}
