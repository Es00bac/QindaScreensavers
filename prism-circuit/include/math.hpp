// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
namespace sw {
constexpr float pi=3.14159265358979323846f;
struct V2 { float x=0,y=0; };
struct V3 { float x=0,y=0,z=0; V3 operator+(V3 b)const{return{x+b.x,y+b.y,z+b.z};} V3 operator-(V3 b)const{return{x-b.x,y-b.y,z-b.z};} V3 operator*(float a)const{return{x*a,y*a,z*a};} V3 operator/(float a)const{return{x/a,y/a,z/a};} V3 operator-()const{return{-x,-y,-z};} };
inline V3 operator*(float a,V3 b){return b*a;}
struct V4 {float x=0,y=0,z=0,w=0;};
inline float dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline V3 cross(V3 a,V3 b){return{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float length(V3 a){return std::sqrt(dot(a,a));}
inline V3 unit(V3 a){float l=length(a);return l>1e-9f?a/l:V3{0,1,0};}
inline V3 mix(V3 a,V3 b,float t){return a*(1-t)+b*t;}
inline float clamp(float a,float l=0,float h=1){return std::clamp(a,l,h);}
inline float smooth(float a){a=clamp(a);return a*a*a*(a*(a*6-15)+10);}
inline float ease(float a,float b,float x){return smooth((x-a)/(b-a));}
struct M4 {std::array<float,16> a{}; static M4 identity(){M4 m;m.a={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};return m;} const float* data()const{return a.data();}};
inline M4 operator*(const M4& a,const M4& b){M4 r;for(int j=0;j<4;j++)for(int i=0;i<4;i++)for(int k=0;k<4;k++)r.a[j*4+i]+=a.a[k*4+i]*b.a[j*4+k];return r;}
inline V3 point(M4 m,V3 p){return{m.a[0]*p.x+m.a[4]*p.y+m.a[8]*p.z+m.a[12],m.a[1]*p.x+m.a[5]*p.y+m.a[9]*p.z+m.a[13],m.a[2]*p.x+m.a[6]*p.y+m.a[10]*p.z+m.a[14]};}
inline M4 translate(V3 t){M4 m=M4::identity();m.a[12]=t.x;m.a[13]=t.y;m.a[14]=t.z;return m;}
inline M4 scale(V3 s){M4 m=M4::identity();m.a[0]=s.x;m.a[5]=s.y;m.a[10]=s.z;return m;}
inline M4 rx(float t){M4 m=M4::identity();float c=std::cos(t),s=std::sin(t);m.a[5]=c;m.a[6]=s;m.a[9]=-s;m.a[10]=c;return m;}
inline M4 ry(float t){M4 m=M4::identity();float c=std::cos(t),s=std::sin(t);m.a[0]=c;m.a[2]=-s;m.a[8]=s;m.a[10]=c;return m;}
inline M4 rz(float t){M4 m=M4::identity();float c=std::cos(t),s=std::sin(t);m.a[0]=c;m.a[1]=s;m.a[4]=-s;m.a[5]=c;return m;}
inline M4 perspective(float fov,float aspect,float n,float f){M4 m;float c=1/std::tan(fov/2);m.a[0]=c/aspect;m.a[5]=c;m.a[10]=(f+n)/(n-f);m.a[11]=-1;m.a[14]=2*f*n/(n-f);return m;}
inline M4 ortho(float l,float r,float b,float t,float n,float f){auto m=M4::identity();m.a[0]=2/(r-l);m.a[5]=2/(t-b);m.a[10]=-2/(f-n);m.a[12]=-(r+l)/(r-l);m.a[13]=-(t+b)/(t-b);m.a[14]=-(f+n)/(f-n);return m;}
inline M4 lookAt(V3 eye,V3 target,V3 up={0,1,0}){V3 f=unit(target-eye),s=unit(cross(f,up)),u=cross(s,f);auto m=M4::identity();m.a={s.x,u.x,-f.x,0,s.y,u.y,-f.y,0,s.z,u.z,-f.z,0,-dot(s,eye),-dot(u,eye),dot(f,eye),1};return m;}
// Column 1 follows the segment. Useful for articulated beams, struts and antennae.
inline M4 segment(V3 a,V3 b,float radius){V3 d=b-a,y=unit(d),x=unit(cross(y,std::abs(y.y)<.95f?V3{0,1,0}:V3{1,0,0})),z=cross(x,y);M4 m=M4::identity();m.a={x.x*radius,x.y*radius,x.z*radius,0,y.x*length(d)*.5f,y.y*length(d)*.5f,y.z*length(d)*.5f,0,z.x*radius,z.y*radius,z.z*radius,0,(a.x+b.x)*.5f,(a.y+b.y)*.5f,(a.z+b.z)*.5f,1};return m;}
struct Random {std::uint64_t state;explicit Random(std::uint64_t s):state(s+0x9e3779b97f4a7c15ULL){} std::uint32_t next(){state^=state>>12;state^=state<<25;state^=state>>27;return (state*2685821657736338717ULL)>>32;} float f(){return float(next()>>8)/16777216.f;} float range(float a,float b){return a+(b-a)*f();}};
}
