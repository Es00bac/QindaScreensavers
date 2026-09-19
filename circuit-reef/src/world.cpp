// SPDX-License-Identifier: GPL-3.0-or-later
#include "reef/types.hpp"
#include <stdexcept>
namespace reef {
Palette palette(const std::string& n){
 if(n=="lagoon")return{rgb(0x020c19),rgb(0x073b48),rgb(0x5bf5d6),rgb(0xd7a0fc),rgb(0xffb36b),rgb(0x16343e),n};
 if(n=="amethyst")return{rgb(0x080819),rgb(0x23264a),rgb(0x83d9ff),rgb(0xf9a6ef),rgb(0xffcb84),rgb(0x292744),n};
 if(n=="ember")return{rgb(0x100b17),rgb(0x333044),rgb(0x8ce8d0),rgb(0xffb9a6),rgb(0xfccb8b),rgb(0x32353f),n};
 throw std::invalid_argument("palette must be lagoon, amethyst, or ember");
}
World::World(std::uint64_t s,int density):seed(s){
 Rng r(s); density=std::clamp(density,3,24);
 for(int i=0;i<density;i++){
  // Spatially layered lanes, not sprites reflecting instantly at an edge.
  const double row=double(i%4)/3;
  fish.push_back({800+r.between(-45,45),225+row*320+r.between(-35,35),r.between(1030,1100),r.between(22,65),r.between(210,380),r.between(.024,.041),r.between(0,tau),r.between(.72,1.24),r.between(0,tau),i%3});
 }
 // One hero fish plus smaller, distant inhabitants.
 if(!fish.empty())fish[0]={835,411,405,65,285,.053,2.5,1.65,.8,0};
 for(int i=0;i<4;i++)jellies.push_back({330.0+i*330.0,160.0+(i%2)*180.0,r.between(.7,1.28),r.between(0,tau),r.between(.045,.075),i%2});
 for(int i=0;i<9;i++)creatures.push_back({i<2?Species::Shark:i<4?Species::Octopus:i<6?Species::Ray:i==6?Species::Turtle:Species::Seahorse,r.between(0,tau),r.between(.014,.024),r.between(.65,1.05),r.between(-200,230)});
 for(int i=0;i<54;i++){
  double x=r.between(-60,1660); double edge=std::abs(x-800)/800;
  plants.push_back({x,r.between(816,938),r.between(45,150)+edge*75,r.between(20,60),r.between(0,tau),i%4});
 }
}
FishPose World::fishPose(std::size_t i,double t)const{
 const auto& f=fish.at(i); const double a=t*f.omega+f.phase;
 const double z=f.rz*std::cos(a);
 const double x=f.cx+f.rx*std::sin(a);
 const double y=f.cy+f.ry*std::sin(2*a+.8)+12*std::sin(t*.13+f.phase);
 const double dx=f.rx*f.omega*std::cos(a), dz=-f.rz*f.omega*std::sin(a);
 const double dy=2*f.ry*f.omega*std::cos(2*a+.8)+1.56*std::cos(t*.13+f.phase);
 return{{x,y-z*.10},z,std::atan2(dz,dx),f.size*(.82+.18*(z/f.rz)),t*(2.0+f.omega*10)+f.tailPhase,clamp(dy/std::hypot(dx,dz),-.22,.22)};
}
Vec2 World::jellyPose(std::size_t i,double t)const {
 const auto& j=jellies.at(i);double a=t*j.speed*.46+j.phase;
 // Pulsed propulsion rides a slow current; complete turns happen beyond the glass.
 return {800+1060*std::sin(a),220+i*61+102*std::sin(a*.83+j.phase)-7*std::sin(t*1.65+j.phase)};
}
FishPose World::creaturePose(std::size_t i,double t)const {
 const auto& c=creatures.at(i);double a=t*c.speed+c.phase;
 double z=c.depth+115*std::cos(a),dx=1110*c.speed*std::cos(a),dz=-290*c.speed*std::sin(a);
 double y=c.species==Species::Octopus?590:c.species==Species::Shark?240:c.species==Species::Ray?460:c.species==Species::Turtle?360:610;
 return {{800+1110*std::sin(a),y+74*std::sin(a*1.4+c.phase)},z,std::atan2(dz,dx),c.size*(.90+z*.0003),t*(c.species==Species::Shark?1.7:1.1)+c.phase,.05*std::sin(a*1.4)};
}
Vec3 World::fishLocal(double u,double a,double phase)const{
 // Tapered ellipsoid with a traveling lateral wave; head displacement is tiny.
 const double radius=std::pow(std::max(0.0,std::sin(pi*u)),.68)*(1-.48*u);
 const double bend=std::sin(phase-u*5.8)*std::pow(u,2.2)*17;
 return{65-145*u,33*radius*std::cos(a),19*radius*std::sin(a)+bend};
}
}
