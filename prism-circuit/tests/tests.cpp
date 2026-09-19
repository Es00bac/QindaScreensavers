#include "race.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace sw;
static int checks=0;void require(bool b,const char* m){++checks;if(!b)throw std::runtime_error(m);}
int main(){try{
 for(int kind=0;kind<3;kind++){
  Track tr(41,kind);require(tr.length>600&&tr.length<2200,"course length");
  for(int i=0;i<2000;i++){
   auto p=tr.at(tr.length*i/2000);require(std::abs(length(p.right)-1)<1e-4,"unit right");require(std::abs(dot(p.up,p.forward))<1e-4,"orthogonal track frame");
   require(std::isfinite(p.position.y)&&std::abs(p.bank)<.48,"finite bank");
  }
  require(length(tr.at(-.01).position-tr.at(tr.length-.01).position)<.001,"negative distance wraps");
  require(length(tr.at(0).position-tr.at(tr.length).position)<.001,"closed course");
  Race a(41,kind),b(41,kind);a.advance(20);for(int i=0;i<1200;i++)b.advance(1./60);
  for(int i=0;i<8;i++){require(std::abs(a.state().cars[i].distance-b.state().cars[i].distance)<1e-6,"frame rate independence");}
  Race r(13,kind);
  for(int tick=0;tick<36000;tick++){
   r.advance(FixedStep);auto state=r.state();
   if(tick%30==0)for(int i=0;i<8;i++){
    const auto& c=state.cars[i];require(std::isfinite(c.distance)&&std::isfinite(c.speed),"finite racer");require(std::abs(c.lane)<=4.651f,"inside track");require(c.speed>=0&&c.speed<42,"bounded speed");
    for(int j=i+1;j<8;j++){
     const auto& d=state.cars[j];require(std::abs(r.track.signedGap(c.distance,d.distance))>=3.94||std::abs(c.lane-d.lane)>=2.77,"no body penetration");
    }
   }
  }
  require(r.counters.boosts>0,"boost pads used");require(r.counters.passes>8,"overtakes exist");require(r.counters.rounds>0,"race restarts");
  std::cout<<tr.name()<<": "<<tr.length<<" m, passes="<<r.counters.passes<<", boosts="<<r.counters.boosts<<", rounds="<<r.counters.rounds<<", soft contacts="<<r.counters.contacts<<'\n';
 }
 Frame frame;initialize(frame);Track tr;buildTrackMeshes(frame,tr);
 for(const auto& b:frame.batches){require(!b.mesh.v.empty(),"mesh exists");require(b.mesh.ix.size()%3==0,"triangle indices");for(auto ix:b.mesh.ix)require(ix<b.mesh.v.size(),"index bounds");}
 Race r;r.advance(12);SceneOptions opt;compose(frame,r.sample(),r.track,opt);
 for(const auto& b:frame.batches)for(auto ins:b.instances)for(auto x:ins.model.a)require(std::isfinite(x),"finite model transforms");
 require(length(frame.eye-frame.target)>5,"safe camera framing");
 {Race r(42);r.advance(180);require(r.counters.pickups>20&&r.counters.itemsUsed>20,"item boxes are collected and activated");require(r.counters.blocks>0,"shields absorb ion pulses");
  for(const auto& c:r.state().cars){require(c.shield>=0&&c.magnet>=0&&c.stun>=0,"powerup timers bounded below");require(c.lane>=-4.651f&&c.lane<=4.651f,"item AI keeps karts on the track");}
 }
 std::cout<<"PASS: "<<checks<<" assertions\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<" after "<<checks<<" checks\n";return 1;}}
