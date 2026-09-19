// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>
using namespace sw;
int main(){try{
 std::uint64_t ticks=0,passes=0,boosts=0,contacts=0,rounds=0,checks=0;
 double maxStep=0;float maxLane=0,maxSpeed=0;
 for(int kind=0;kind<3;kind++)for(int seed=1;seed<=12;seed++){
  Race r(seed*103,kind);
  for(int frame=0;frame<36000;frame++){
   auto old=r.state();r.advance(1./60.);const auto& now=r.state();
   for(int i=0;i<8;i++){
    const auto& c=now.cars[i];
    maxLane=std::max(maxLane,std::abs(c.lane));maxSpeed=std::max(maxSpeed,c.speed);
    if(!std::isfinite(c.distance)||std::abs(c.lane)>4.651||c.speed<0||c.speed>42)throw std::runtime_error("State outside bounds");
    if(old.round==now.round){double d=std::abs(c.distance-old.cars[i].distance);maxStep=std::max(maxStep,d);if(d>.75){std::cerr<<"course="<<kind<<" seed="<<seed*103<<" frame="<<frame<<" car="<<i<<" delta="<<c.distance-old.cars[i].distance<<" old_lane="<<old.cars[i].lane<<" new_lane="<<c.lane<<" old_s="<<old.cars[i].distance<<" new_s="<<c.distance<<"\n";for(int j=0;j<8;j++)std::cerr<<j<<" s="<<old.cars[j].distance<<"->"<<now.cars[j].distance<<" lane="<<old.cars[j].lane<<"->"<<now.cars[j].lane<<" speed="<<now.cars[j].speed<<"\n";throw std::runtime_error("Discontinuous car position");}}
    checks++;
   }
   if(frame%6==0)for(int i=0;i<8;i++)for(int j=i+1;j<8;j++){
    auto a=now.cars[i],b=now.cars[j];
    if(std::abs(r.track.signedGap(a.distance,b.distance))<3.935&&std::abs(a.lane-b.lane)<2.77)throw std::runtime_error("Car body penetration");
    checks++;
   }
  }
  ticks+=r.counters.ticks;passes+=r.counters.passes;boosts+=r.counters.boosts;contacts+=r.counters.contacts;rounds+=r.counters.rounds;
 }
 std::cout<<std::setprecision(9)<<"{\"status\":\"passed\",\"course_seed_combinations\":36,\"simulated_hours\":6,\"fixed_ticks\":"<<ticks<<",\"checks\":"<<checks<<",\"overtakes\":"<<passes<<",\"boosts\":"<<boosts<<",\"completed_rounds\":"<<rounds<<",\"soft_contact_corrections\":"<<contacts<<",\"maximum_travel_per_60hz_sample_m\":"<<maxStep<<",\"maximum_abs_lane_m\":"<<maxLane<<",\"maximum_speed_m_per_s\":"<<maxSpeed<<"}\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
