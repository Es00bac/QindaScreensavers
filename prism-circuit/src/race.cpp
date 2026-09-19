// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <numeric>
namespace sw {
const char* itemName(Item item){static const char* names[]{"", "TURBO CELL", "AEGIS SHIELD", "ION PULSE", "MAGNET DRIVE"};return names[int(item)];}
void Race::cue(saver::Cue kind,int car){if(current_.sounds.size()<96)current_.sounds.push_back({++soundSerial_,current_.time,kind,car,clamp(current_.cars[car].lane/6,-1,1)});}
Race::Race(std::uint64_t seed,int kind):track(seed,kind){grid(0,0);}
void Race::grid(unsigned round,double globalTime){
 current_={};current_.round=round;current_.time=globalTime;
 Random rng(track.seed+round*8191);
 for(int i=0;i<RacerCount;i++){
  auto& c=current_.cars[i];c.distance=-double(i/2)*7.2-9;
  c.lane=(i%2?2.6f:-2.6f);c.targetLane=c.lane;
  c.pace=25.0f+rng.range(-.9f,.9f);c.decision=rng.range(0,.65);c.rank=i+1;current_.order[i]=i;
 }
 previous_=current_;
}
void Race::step(){
 previous_=current_;
 constexpr float dt=float(FixedStep);
 current_.time+=FixedStep;current_.roundTime+=FixedStep;counters.ticks++;
 std::erase_if(current_.sounds,[&](const saver::SoundEvent& e){return current_.time-e.time>.35;});
 for(auto& crate:current_.crates)crate=std::max(0.f,crate-dt);
 if(current_.finishedAt>=0&&current_.time-current_.finishedAt>=11){
  unsigned next=current_.round+1;double t=current_.time;grid(next,t);counters.rounds++;return;
 }
 const auto before=current_.cars;
 double lead=-1e20;for(const auto& c:before)lead=std::max(lead,c.distance);
 const bool go=current_.roundTime>4;
 for(int i=0;i<RacerCount;i++){
  auto& c=current_.cars[i];const auto& old=before[i];
  c.boost=std::max(0.f,c.boost-dt);c.cooldown=std::max(0.f,c.cooldown-dt);c.decision-=dt;
  for(float* timer:{&c.shield,&c.magnet,&c.pulse,&c.stun})*timer=std::max(0.f,*timer-dt);
  if(c.item!=Item::None&&go){c.itemAge+=dt;if(c.itemAge>1.1f+i*.11f){
   if(c.item==Item::Turbo)c.boost=3;
   if(c.item==Item::Shield)c.shield=7;
   if(c.item==Item::Magnet)c.magnet=7;
   if(c.item==Item::Pulse){c.pulse=.85f;for(int j=0;j<RacerCount;++j)if(j!=i){double gap=track.signedGap(before[j].distance,old.distance);if(gap>0&&gap<34){auto& target=current_.cars[j];if(target.shield>0){target.shield=std::max(0.f,target.shield-2);++counters.blocks;cue(saver::Cue::Shield,j);}else target.stun=1.3f;}}}
   cue(c.item==Item::Turbo?saver::Cue::Boost:saver::Cue::Special,i);c.item=Item::None;c.itemAge=0;++counters.itemsUsed;
  }}
  auto pose=track.at(old.distance+6);
  float target=old.pace/(1+std::abs(pose.curvature)*4.1f);
  target+=clamp(float(lead-old.distance)*.025f,0,2.1f);
  if(c.decision<=0&&go){
   // Score actual occupancy of candidate lanes before choosing a pass or pad.
   float ideal=clamp(-pose.curvature*90.f,-2.f,2.f);
   double padAhead=1e10;float padLane=0;
   for(const auto& pad:track.pads){double d=track.signedGap(pad.distance,old.distance);if(d>2&&d<padAhead){padAhead=d;padLane=pad.lane;}}
   float best=-1e9,chosen=c.targetLane;
   for(float lane:{-4.1f,0.f,4.1f}){
    float score=4.0f-std::abs(lane-ideal)*.16f-std::abs(lane-old.lane)*.14f;
    if(std::abs(lane-c.targetLane)<.5f)score+=.9f;
    if(padAhead<39&&std::abs(lane-padLane)<1.0)score+=2.2f;
    if(c.item==Item::None)for(int box=0;box<24;++box)if(current_.crates[box]<=0){double d=track.signedGap(track.length*(box/3+.35)/8,old.distance);float boxLane=(box%3-1)*4.1f;if(d>0&&d<45&&std::abs(lane-boxLane)<1)score+=2.8f;}
    for(int j=0;j<RacerCount;j++)if(i!=j){
     double ds=track.signedGap(before[j].distance,old.distance);
     bool crossing=before[j].lane>=std::min(old.lane,lane)-2.85f&&before[j].lane<=std::max(old.lane,lane)+2.85f;
     if(ds>-6&&ds<7&&crossing)score-=10;
     if(ds>0&&ds<21&&std::abs(before[j].lane-lane)<2.8)score-=float(21-ds)*.55f;
    }
    score+=.2f*std::sin(float(current_.time*.24+i*3+lane));
    if(score>best){best=score;chosen=lane;}
   }
   c.targetLane=chosen;c.decision=.68f+.045f*i;
  }
  float laneAccel=(c.targetLane-old.lane)*8.0f-old.laneSpeed*5.8f;
  c.laneSpeed=clamp(old.laneSpeed+laneAccel*dt,-2.6f,2.6f);
  c.lane=old.lane+c.laneSpeed*dt;
  // A merger yields instead of cutting through the neighboring kart.
  for(int j=0;j<RacerCount;j++)if(i!=j){
   double gap=track.signedGap(before[j].distance,old.distance);
   if(std::abs(gap)<4.9&&std::abs(c.lane-before[j].lane)<2.8f&&std::abs(old.lane-before[j].lane)>=2.8f){c.lane=old.lane;c.laneSpeed=0;}
  }
  c.lane=clamp(c.lane,-4.65f,4.65f);
  if(c.boost>0)target+=6.0f*clamp(c.boost/.22f);
  if(c.stun>0)target*=.62f;
  if(c.magnet>0)target+=2.8f;
  for(int j=0;j<RacerCount;j++)if(i!=j){
   double gap=track.signedGap(before[j].distance,old.distance);
   float dl=std::abs(before[j].lane-c.lane);
   if(gap>0&&gap<22&&dl<2.9f){
    if(gap>7.5&&gap<17)target+=1.0f; // Slipstream.
    float safe=before[j].speed+float(gap-5.1)*1.8f;
    target=std::min(target,std::max(0.f,safe));
   }
  }
  if(!go)target=0;
  if(current_.winner>=0)target=std::min(target,14.f);
  c.speed=std::max(0.f,old.speed+clamp(target-old.speed,-16*dt,7*dt));
  c.distance=old.distance+c.speed*dt;
  c.steer=old.steer+(clamp(pose.curvature*13-c.laneSpeed*.11f,-.40f,.40f)-old.steer)*dt*7;
  float desiredDrift=clamp(pose.curvature*c.speed*.30f,-.26f,.26f);
  c.drift=old.drift+(desiredDrift-old.drift)*dt*4;
  if(go&&c.cooldown<=0){
   for(const auto& pad:track.pads){
    double ahead=track.signedGap(pad.distance,old.distance),after=track.signedGap(pad.distance,c.distance);
    if(ahead>=0&&after<=0&&std::abs(c.lane-pad.lane)<1.6f){c.boost=1.8f;c.cooldown=2.5f;counters.boosts++;cue(saver::Cue::Boost,i);break;}
   }
  }
  if(go&&c.item==Item::None)for(int box=0;box<24;++box)if(current_.crates[box]<=0){
   double distance=track.length*(box/3+.35)/8,ahead=track.signedGap(distance,old.distance),after=track.signedGap(distance,c.distance);float lane=(box%3-1)*4.1f;
   if(ahead>=0&&after<=0&&std::abs(c.lane-lane)<(c.magnet>0?3.6f:1.65f)){
    Random reward(track.seed+std::uint64_t(box)*7919+std::uint64_t(i)*313+std::uint64_t(std::max(0.,c.distance)/track.length)*17171+current_.round*99991);
    c.item=Item(1+reward.next()%4);c.itemAge=0;current_.crates[box]=5;++counters.pickups;cue(saver::Cue::Pickup,i);break;
   }
  }
 }
 // Resolve approximate fender boxes along the smallest penetration axis.
 // Side-by-side karts receive a small lateral nudge, never a four-metre
 // backward correction. Longitudinal contacts only affect the following kart.
 for(int pass=0;pass<4;pass++)for(int i=0;i<RacerCount;i++)for(int j=i+1;j<RacerCount;j++){
  auto& a=current_.cars[i];auto& b=current_.cars[j];
  double d=track.signedGap(a.distance,b.distance);float dl=a.lane-b.lane;
  if(std::abs(d)<3.95&&std::abs(dl)<2.80f){
   float lat=2.80f-std::abs(dl);double longitudinal=3.95-std::abs(d);
   if(lat<longitudinal){
    float side=dl<0?-1.f:1.f;
    a.lane+=side*(lat*.5f+.00001f);b.lane-=side*(lat*.5f+.00001f);
    if(a.lane>4.65f){float extra=a.lane-4.65f;a.lane-=extra;b.lane-=extra;}
    if(a.lane<-4.65f){float extra=-4.65f-a.lane;a.lane+=extra;b.lane+=extra;}
    if(b.lane>4.65f){float extra=b.lane-4.65f;b.lane-=extra;a.lane-=extra;}
    if(b.lane<-4.65f){float extra=-4.65f-b.lane;b.lane+=extra;a.lane+=extra;}
    a.laneSpeed=0;b.laneSpeed=0;
   }else{
    Car& rear=d<0?a:b;const Car& front=d<0?b:a;
    rear.distance-=longitudinal;rear.speed=std::min(rear.speed,front.speed);rear.boost=0;
   }
   counters.contacts++;
  }
 }
 std::iota(current_.order.begin(),current_.order.end(),0);
 std::stable_sort(current_.order.begin(),current_.order.end(),[&](int a,int b){return current_.cars[a].distance>current_.cars[b].distance;});
 for(int rank=0;rank<RacerCount;rank++)current_.cars[current_.order[rank]].rank=rank+1;
 if(go)for(int a=0;a<RacerCount;a++)for(int b=a+1;b<RacerCount;b++){
  if(before[a].distance<before[b].distance&&current_.cars[a].distance>=current_.cars[b].distance)counters.passes++;
  if(before[a].distance>before[b].distance&&current_.cars[a].distance<=current_.cars[b].distance)counters.passes++;
 }
 if(current_.winner<0&&current_.cars[current_.order[0]].distance>=track.length*3){current_.winner=current_.order[0];current_.finishedAt=current_.time;}
}
void Race::advance(double seconds){
 if(!std::isfinite(seconds)||seconds<0)throw std::invalid_argument("Invalid simulation interval");
 requestedTime_+=seconds;accumulator_+=seconds;
 while(accumulator_+1e-12>=FixedStep){step();accumulator_-=FixedStep;}
 if(accumulator_<0)accumulator_=0;
}
void Race::seek(double t){
 if(!std::isfinite(t)||t<0||t>1e7)throw std::invalid_argument("Invalid seek time");
 if(t<requestedTime_-1e-9){counters={};accumulator_=0;requestedTime_=0;grid(0,0);}
 advance(std::max(0.,t-requestedTime_));
}
RaceState Race::sample()const{
 float a=float(accumulator_/FixedStep);RaceState r=current_;
 if(current_.round!=previous_.round)return r;
 r.time=previous_.time+(current_.time-previous_.time)*a;
 r.roundTime=previous_.roundTime+(current_.roundTime-previous_.roundTime)*a;
 for(int i=0;i<RacerCount;i++){
  auto& c=r.cars[i];const auto& b=previous_.cars[i];const auto& n=current_.cars[i];
  c.distance=b.distance+(n.distance-b.distance)*a;
  c.lane=b.lane+(n.lane-b.lane)*a;c.laneSpeed=b.laneSpeed+(n.laneSpeed-b.laneSpeed)*a;
  c.speed=b.speed+(n.speed-b.speed)*a;c.steer=b.steer+(n.steer-b.steer)*a;c.drift=b.drift+(n.drift-b.drift)*a;
  c.boost=b.boost+(n.boost-b.boost)*a;
 }
 return r;
}
}
