// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "math.hpp"
#include <vector>
#include <string>
namespace sw {
struct Vertex { V3 p,n; V2 uv; };
struct Mesh { std::string name; std::vector<Vertex> v; std::vector<std::uint32_t> ix; };
struct Material { V3 color; float emission=0,rough=.45f,metal=.1f,kind=0; };
struct Instance { M4 model; V4 color; V4 surface; };
struct Batch { Mesh mesh; std::vector<Instance> instances; };
enum Primitive {Sphere,Box,Torus,Cylinder,Hull,Wing,Rock0,Rock1,Rock2,Rock3,Rock4,Rock5,Monolith,Manta,Sideplate,Ribbon,RoadSurface,RoadShell,Ear,Terrain,Fold,Leaf,Crystal,Count};
inline V3 mineralFold(float u,float v){
 float z=v*2-1;
 float y=.24f*std::sin(u*2*pi)+.90f*std::exp(-std::pow((u-.80f)*4.4f,2.f))+.55f*(1-z*z)-.23f*z;
 return {(u-.5f)*2,y,z};
}
struct Frame {
 std::array<Batch,Count> batches;
 V3 eye,target,up{0,1,0},pengu,ducke; M4 pship,dship;
 int chapter=0; float local=0,fade=1,fov=.68f;
 double time=0,motionTime=0; std::uint64_t voyage=0;
 int focus=0,winner=-1,countdown=0,active=4,round=1,secondsLeft=90;
 std::string courseName="PRISM TERMINAL";
 std::array<int,8> roster{},stocks{},kos{};
 std::array<std::string,8> specials{};
 std::array<float,8> damage{},shield{};
 bool showcase=false;
};
inline M4 basis(V3 position,V3 right,V3 up,V3 forward){M4 m=M4::identity();m.a={right.x,right.y,right.z,0,up.x,up.y,up.z,0,forward.x,forward.y,forward.z,0,position.x,position.y,position.z,1};return m;}
inline V3 direction(M4 m,V3 p){return point(m,p)-point(m,{0,0,0});}
void initialize(Frame&);
void add(Frame&,Primitive,M4,Material);
void exportModels(const std::string&,std::uint64_t seed=41,int stage=0);
inline constexpr const char* racerNames[]={"CyberPengu","Ducké","Vix","Cache","Mochi","Hex","Patches","Axi"};
inline constexpr const char* racerSlugs[]={"cyberpengu","ducke","vix-fox","cache-raccoon","mochi-rabbit","hex-cat","patches-red-panda","axi-axolotl"};
inline constexpr V3 racerColors[]={{.10f,.84f,.66f},{1.f,.58f,.12f},{1.f,.22f,.39f},{.13f,.61f,1.f},{.61f,.34f,1.f},{.95f,.19f,.72f},{.51f,.88f,.20f},{.94f,.50f,.72f}};
}
