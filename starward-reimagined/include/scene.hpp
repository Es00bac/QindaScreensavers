// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "math.hpp"
#include <vector>
#include <string>
namespace sw {
struct Vertex {V3 p,n;V2 uv;};
struct Mesh {std::string name;std::vector<Vertex> v;std::vector<std::uint32_t> ix;};
struct Material {V3 color;float emission=0,rough=.45f,metal=.1f,kind=0;};
struct Instance {M4 model;V4 color;V4 surface;};
struct Batch {Mesh mesh;std::vector<Instance> instances;};
enum Primitive {Sphere,Box,Torus,Cylinder,Hull,Wing,Rock0,Rock1,Rock2,Rock3,Rock4,Rock5,Monolith,Manta,Sideplate,Ribbon,Count};
struct Frame {std::array<Batch,Count> batches;V3 eye,target;int chapter=0;float local=0,fade=1;double time=0,motionTime=0;std::uint64_t voyage=0;V3 pengu,ducke;M4 pship,dship;};
struct Director {int fixedChapter=-1;double chapterSeconds=38;bool reduced=false;std::uint64_t seed=20260919;};
inline constexpr const char* names[]={"harbor","asteroids","rescue","relic","pursuit","contact","home"};
inline constexpr const char* titles[]={"THE LAST HUMAN OUTPOST","A SIGNAL IN THE STONE","ONE GOOD TURN","THE SLEEPING ARCHIVE","PERMISSION TO PASS","SOMETHING ANSWERS","A LITTLE FURTHER FROM ALONE"};
inline constexpr const char* subtitles[]={"01 / EMBER DOCK   :   DEPARTURE","02 / THE QUIET BELT   :   RECOVERY","03 / FREIGHTER 08   :   REPAIR","04 / UNCHARTED SPACE   :   ALIGNMENT","05 / THE WATCHERS   :   PURSUIT","06 / FIRST CONTACT   :   LISTEN","07 / EMBER DOCK   :   HOMECOMING"};
void initialize(Frame&);
void compose(Frame&,double time,const Director&);
void add(Frame&,Primitive,M4,Material);
void ship(Frame&,M4,bool duck,double t,float power=1);
void asteroid(Frame&,M4,int type);
void station(Frame&,M4,double t,bool home);
void gate(Frame&,M4,double t,float awake);
void freighter(Frame&,M4,double t,float repair);
void sentinel(Frame&,M4,double t);
void creature(Frame&,M4,double t);
void capsule(Frame&,M4,double t);
void beam(Frame&,V3 a,V3 b,Material,float r=.023f);
void exportModels(const std::string& directory);
}
