// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
namespace reef {
constexpr double pi=3.14159265358979323846, tau=2*pi;
inline double clamp(double x,double a=0,double b=1){return std::clamp(x,a,b);}
inline double mix(double a,double b,double t){return a+(b-a)*t;}
inline double smooth(double t){t=clamp(t);return t*t*(3-2*t);}
struct Vec2 {double x{},y{}; Vec2 operator+(Vec2 b)const{return{x+b.x,y+b.y};} Vec2 operator-(Vec2 b)const{return{x-b.x,y-b.y};} Vec2 operator*(double k)const{return{x*k,y*k};}};
struct Vec3 {double x{},y{},z{}; Vec3 operator+(Vec3 b)const{return{x+b.x,y+b.y,z+b.z};} Vec3 operator-(Vec3 b)const{return{x-b.x,y-b.y,z-b.z};} Vec3 operator*(double k)const{return{x*k,y*k,z*k};}};
inline double length(Vec3 v){return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);}
inline Vec3 normal(Vec3 v){return v*(1/std::max(1e-12,length(v)));}
inline Vec3 cross(Vec3 a,Vec3 b){return{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
struct Color {double r{},g{},b{},a{1}; Color alpha(double k)const{return{r,g,b,a*k};} Color light(double k)const{return{clamp(r*k),clamp(g*k),clamp(b*k),a};}};
inline Color rgb(unsigned x,double a=1){return{double((x>>16)&255)/255,double((x>>8)&255)/255,double(x&255)/255,a};}
inline Color blend(Color a,Color b,double k){return{mix(a.r,b.r,k),mix(a.g,b.g,k),mix(a.b,b.b,k),mix(a.a,b.a,k)};}
struct Palette {Color deep,water,glow,pink,gold,ink; std::string name;};
Palette palette(const std::string& name);
struct Rng {std::uint64_t state; explicit Rng(std::uint64_t s):state(s){} std::uint64_t next(){auto z=(state+=0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);} double unit(){return double(next()>>11)*0x1.0p-53;} double between(double a,double b){return mix(a,b,unit());}};
struct Fish {double cx,cy,rx,ry,rz,omega,phase,size,tailPhase; int style;};
struct FishPose {Vec2 p;double depth,yaw,scale,phase,pitch;};
struct Jelly {double cx,cy,size,phase,speed;int style;};
enum class Species { Shark, Octopus, Ray, Turtle, Seahorse };
struct Creature { Species species; double phase, speed, size, depth; };
struct Plant {double x,y,height,width,phase;int type;};
class World {
public:
 explicit World(std::uint64_t seed=2026,int density=10);
 FishPose fishPose(std::size_t i,double time)const;
 FishPose creaturePose(std::size_t i,double time)const;
 Vec2 jellyPose(std::size_t i,double time)const;
 std::vector<Creature> creatures;
 Vec3 fishLocal(double u,double angle,double time)const;
 std::vector<Fish> fish; std::vector<Jelly> jellies; std::vector<Plant> plants;
 std::uint64_t seed;
};
}
