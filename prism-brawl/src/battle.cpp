// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
#include <numeric>
#include <limits>
namespace sw {
namespace {
float approach(float x,float target,float speed){return x+clamp(target-x,-speed,speed);}
float reach(float a,float b,float rate,float dt){return a+(b-a)*(1-std::exp(-rate*dt));}
bool offensive(Action a){return a==Action::Jab||a==Action::Heavy||a==Action::Aerial||a==Action::Grab;}
bool busy(Action a){return offensive(a)||a==Action::Special||a==Action::Dodge||a==Action::Recovery||a==Action::Taunt||a==Action::Land;}
float mass(int id){constexpr float m[]={1.05f,.90f,.93f,1.04f,.83f,.89f,1.12f,.87f};return m[id];}
float speed(int id){constexpr float s[]={6.2f,6.9f,7.5f,6.4f,7.2f,7.1f,5.8f,6.7f};return s[id];}
}
const char* specialName(int id){static const char* names[]{"PATCH QUAKE","TWIN THRUSTERS","EMBER DASH","CACHE RETURN","LUNAR RISE","GRAVITY HEX","METEOR STOMP","TIDAL MEND"};return names[id%8];}
void Battle::cue(saver::Cue kind,int slot){const auto& f=current_.fighters[slot];if(current_.sounds.size()<96)current_.sounds.push_back({++soundSerial_,current_.time,kind,f.id,clamp(f.x/15,-1,1)});}
void Battle::special(int i){
 auto& s=current_;auto& f=s.fighters[i];f.projectileMade=true;++counters.specials;cue(saver::Cue::Special,i);effect({f.x,f.y+1.2f,0},5,f.id);
 auto shot=[&](float vx,float vy,float y,float radius,float damage,float launch,float lift,float life){if(s.shots.size()<64)s.shots.push_back({{f.x+f.face*1.0f,f.y+y,0},{vx,vy,0},0,life,radius,i,f.id,damage,launch,lift});};
 switch(f.id){
 case 0:shot(f.face*10,0,.45f,.62f,16,8,.6f,1.05f);break;
 case 1:shot(f.face*15,1.3f,1.35f,.24f,7,5,.5f,1.6f);shot(f.face*13,-.6f,1.05f,.24f,7,5,.5f,1.6f);break;
 case 2:f.vx=f.face*19;f.invulnerable=.19f;shot(f.face*19,0,1.1f,.65f,13,7,.4f,.32f);break;
 case 3:shot(f.face*12,0,1.3f,.40f,11,6,.55f,1.5f);break;
 case 4:f.vy=11;f.floor=-1;shot(f.face*6,10,1,.72f,15,7,1.3f,.65f);break;
 case 5:shot(f.face*5,0,1.6f,.52f,10,5,.8f,2.1f);break;
 case 6:f.vy=8;f.floor=-1;shot(-10,0,.45f,.72f,17,9,.9f,.95f);shot(10,0,.45f,.72f,17,9,.9f,.95f);break;
 case 7:f.damage=std::max(0.f,f.damage-12);f.shield=std::min(1.f,f.shield+.25f);shot(f.face*8,.8f,1.2f,.58f,6,4,.75f,1.7f);break;
 }
}
Move move(Action a,int id){
 Move m{.4f,.10f,.22f,7,4.7f,1.15f,.84f,.40f};
 if(a==Action::Heavy)m={.84f,.26f,.40f,18,7.5f,1.65f,1.0f,.59f};
 if(a==Action::Aerial)m={.62f,.13f,.34f,10,6.0f,1.10f,1.24f,.48f};
 if(a==Action::Special)m={.72f,.23f,.27f,8,5.6f,1.9f,.6f,.38f};
 if(a==Action::Grab)m={.67f,.16f,.23f,9,7.6f,.72f,.88f,.70f};
 if(a==Action::Dodge)m={.42f,0,.34f,0,0,0,0,0};
 if(a==Action::Recovery)m={.62f,0,.45f,0,0,0,0,0};
 if(a==Action::Taunt)m={1.65f,0,0,0,0,0,0,0};
 if(a==Action::Land)m={.29f,0,0,0,0,0,0,0};
 m.damage*=id==6?1.13f:id==4?.88f:1;return m;
}
std::array<Platform,4> platforms(int stage,double time){
 float t=std::fmod(time,4096.);
 if(stage==1)return {{{0,0,11.8f,3.7f,true},{-6+.8f*std::sin(t*.38f),3.3f,2.6f,1.65f,false},{6+.8f*std::sin(t*.38f+pi),3.3f,2.6f,1.65f,false},{0,6.4f,2.6f,1.7f,false}}};
 if(stage==2)return {{{0,0,11.8f,3.7f,true},{-7,2.8f,2.4f,1.55f,false},{7,2.8f,2.4f,1.55f,false},{0,5.8f,2.9f,1.7f,false}}};
 if(stage==3)return {{{0,0,11.8f,3.8f,true},{-6.8f,2.45f,2.45f,1.7f,false},{6.3f,4.25f,2.45f,1.7f,false},{-.6f,6.9f,2.2f,1.65f,false}}};
 if(stage==4)return {{{0,0,11.8f,3.8f,true},{-6.8f,3.2f,2.65f,1.7f,false},{6.8f,3.2f,2.65f,1.7f,false},{0,5.9f+.65f*std::sin(t*.43f),2.5f,1.7f,false}}};
 if(stage==5)return {{{0,0,11.8f,3.9f,true},{-6.2f+.75f*std::sin(t*.28f),3.f,2.65f,1.7f,false},{6.2f-.75f*std::sin(t*.28f),4.1f,2.65f,1.7f,false},{0,6.8f,2.35f,1.7f,false}}};
 if(stage==6)return {{{0,0,11.8f,3.7f,true},{-6.7f,4.f,2.3f,1.6f,false},{6.7f,2.8f,2.8f,1.7f,false},{1.1f*std::sin(t*.26f),6.6f,2.6f,1.7f,false}}};
 return {{{0,0,11.8f,3.7f,true},{-5.7f,3.5f,2.7f,1.6f,false},{5.7f,3.5f,2.7f,1.6f,false},{0,6.9f,2.5f,1.7f,false}}};
}
const char* stageName(int id){return Stages.at(id).name;}
Battle::Battle(std::uint64_t seed,int stage,int count,int firstStage):rng_(seed),stageRng_(seed^0xb47a11e5ULL),seed_(seed),stageChoice_(stage),count_(count),firstStage_(firstStage){
 if(count!=2&&count!=4&&count!=8)throw std::runtime_error("Fighter count must be 2, 4 or 8");
 if(stage< -1||stage>=StageCount)throw std::runtime_error("Stage out of range");
 if(firstStage< -1||firstStage>=StageCount)throw std::runtime_error("Starting stage out of range");
 reset(0,0);previous_=current_;
}
void Battle::reset(unsigned round,double globalTime){
 int nextStage=stageChoice_;
 if(stageChoice_<0){
  if(round%StageCount==0){
   std::iota(stageOrder_.begin(),stageOrder_.end(),0);
   for(int i=StageCount-1;i>0;--i)std::swap(stageOrder_[i],stageOrder_[stageRng_.next()%(i+1)]);
   if(round==0&&firstStage_>=0){
    auto first=std::find(stageOrder_.begin(),stageOrder_.end(),firstStage_);
    std::iter_swap(stageOrder_.begin(),first);
   }else if(round>0&&stageOrder_[0]==current_.stage){
    // A new shuffled bag must not immediately repeat the previous arena.
    std::swap(stageOrder_[0],stageOrder_[1+stageRng_.next()%(StageCount-1)]);
   }
  }
  nextStage=stageOrder_[round%StageCount];
 }
 current_=BattleState{};current_.round=round;current_.time=globalTime;current_.active=count_;
 current_.stage=nextStage;rng_=Random(seed_+round*191099);
 std::array<int,8> order{0,1,2,3,4,5,6,7};
 // The first two rounds show the entire familiar cast; subsequent pairs shuffle it.
 if(round>=2){Random r(seed_+(round/2)*17171);for(int i=7;i>0;--i)std::swap(order[i],order[r.next()%(i+1)]);}
 for(int i=0;i<count_;i++){
  auto& f=current_.fighters[i];f.id=order[(i+(round*count_)%8)%8];
  f.x=(i-(count_-1)*.5f)*std::min(4.0f,18.f/count_);f.face=f.x<0?1:-1;f.anim.yaw=f.face*.93f;
  f.decision=rng_.range(.03,.3);f.invulnerable=.3f;
  f.action=Action::Taunt;f.actionTime=-i*.07f;f.gesture=0;f.tauntCooldown=4+i*.37f;++counters.taunts;
 }
 nextPickup_=11;previous_=current_;++counters.rounds;
}
void Battle::effect(V3 pos,int kind,int id,V3 velocity){
 if(current_.effects.size()>=192)current_.effects.erase(current_.effects.begin());
 current_.effects.push_back({pos,velocity,0,kind==2?1.05f:kind==4?.85f:kind==8?.62f:kind==7?.40f:.48f,kind,id});
}
void Battle::react(int i,Reaction reaction,float duration){
 auto& f=current_.fighters[i];f.reaction=reaction;f.reactionTime=f.reactionDuration=duration;++counters.reactions;
}
void Battle::begin(int i,Action a){
 if(a==Action::Jump)cue(saver::Cue::Jump,i);
 auto& f=current_.fighters[i];f.action=a;f.actionTime=0;f.hits=0;f.projectileMade=false;
 f.spinStart=f.anim.tumble;
 if(offensive(a))f.variant=int((f.moveSerial++ + unsigned(f.id))%3);
 if(a==Action::Taunt){
  f.gesture=int(f.tauntSerial++%3);f.tauntCooldown=5.0f+rng_.range(0,3.5f);f.tauntPending=false;++counters.taunts;
  if(f.target>=0&&current_.fighters[f.target].reactionTime<=0)react(f.target,Reaction::Challenge,.85f);
 }
 if(a==Action::Celebrate){f.gesture=0;react(i,Reaction::Happy,5.5f);}
}
void Battle::hit(int from,int to,float damage,float launch,float dx,float lift,bool bypass){
 auto& f=current_.fighters[to];if(f.stocks<=0||f.respawn>0||f.invulnerable>0)return;
 if(f.action==Action::Dodge&&f.actionTime>.03f&&f.actionTime<.31f){if(f.reactionTime<.1f)react(to,Reaction::NearMiss,.4f);return;}
 if(f.action==Action::Shield&&f.shield>.02f&&!bypass){
  f.shield=std::max(0.f,f.shield-damage*.018f);f.vx+=dx*1.5f;++counters.blocks;cue(saver::Cue::Shield,to);
  effect({f.x,f.y+1.2f,.2f},1,f.id);
  react(to,Reaction::Block,.28f);
  if(f.shield<=.02f){f.stun=1.2f;f.launchPower=f.spinVelocity=0;begin(to,Action::Hurt);react(to,Reaction::Dizzy,1.2f);effect({f.x,f.y+1.3f,0},3,f.id);}return;
 }
 f.damage=std::min(350.f,f.damage+damage);f.lastHit=from;
 float power=(launch+f.damage*.155f)/mass(f.id);power=std::min(power,37.f);
 f.vx=dx*power;f.vy=power*lift+2.7f;f.floor=-1;f.drop=0;
 f.launchPower=clamp((power-6)/23);f.spinVelocity=-dx*(5+power*.29f);f.hitDirection=dx>0?1:-1;
 f.stun=std::min(.9f,.11f+power*.018f);f.hitstop=.035f+.024f*f.launchPower;f.invulnerable=.105f;begin(to,Action::Hurt);
 react(to,f.launchPower>.4f?Reaction::Launched:Reaction::Hit,f.stun+.35f);
 if(f.launchPower>.18f)++counters.tumbles;
 f.anim.impact=1;f.trailClock=0;
 // Commit the first whole-body flinch before the impact hold freezes it.
 animateFighter(f,.025f,current_.roundTime);
 if(from>=0&&from<current_.active){auto& attacker=current_.fighters[from];
  if(std::abs(attacker.x-f.x)<3.4f)attacker.hitstop=std::max(attacker.hitstop,.018f+.018f*f.launchPower);
 }
 effect({f.x,f.y+1.2f,0},f.launchPower>.4f?6:0,f.id,{dx*2,1,0});++counters.hits;cue(saver::Cue::Hit,to);
}
void Battle::think(int i){
 auto& f=current_.fighters[i];const auto& s=current_;
 if(f.stocks<=0||f.respawn>0||f.stun>0||f.hitstop>0||(busy(f.action)&&f.action!=Action::Taunt))return;
 const auto ps=platforms(s.stage,s.roundTime);
 int target=-1;float best=1e9f;
 for(int j=0;j<s.active;j++){const auto& o=s.fighters[j];if(i==j||o.stocks<=0||o.respawn>0)continue;
  float cost=std::abs(o.x-f.x)+1.1f*std::abs(o.y-f.y);if(o.invulnerable>0)cost+=6;
  if(cost<best){best=cost;target=j;}}
 f.target=target;f.seeking=false;
 for(const auto& item:s.pickups){float d=std::abs(item.x-f.x)+1.4f*std::abs(item.y-f.y-1);bool useful=item.kind!=0||f.damage>20;
  if(useful&&d<9&&(d<best+2||f.damage>65)){f.seeking=true;f.goalX=item.x;f.goalY=item.y-1;break;}}
 // Recover before choosing combat actions. Air jumps and the jet burst are finite.
 if((std::abs(f.x)>11.2f&&f.floor<0&&f.vy<1.5f)||f.y<-.8f){
  f.face=f.x<0?1:-1;
  if(f.jumps>0&&f.vy<1.0f){f.vy=12.8f;--f.jumps;++counters.jumps;begin(i,Action::Jump);}
  else if(!f.recoveryUsed&&f.y<2.5f){f.vy=15.5f;f.vx=f.face*7.8f;f.recoveryUsed=true;begin(i,Action::Recovery);++counters.recoveries;effect({f.x,f.y,0},4,f.id);}
  return;
 }
 if(target<0)return;
 const auto& o=s.fighters[target];float dx=(f.seeking?f.goalX:o.x)-f.x,dy=(f.seeking?f.goalY:o.y)-f.y;
 if(std::abs(dx)>.18f)f.face=dx>0?1:-1;
 if(f.action==Action::Shield){if(f.actionTime>.26f&&(rng_.f()<.65f||f.shield<.25f))begin(i,Action::Idle);else return;}
 bool incoming=false;for(const auto& shot:s.shots)if(shot.owner!=i&&std::abs(shot.p.x-f.x)<4&&std::abs(shot.p.y-f.y-1.2f)<1.3f&&(shot.p.x-f.x)*shot.v.x<0)incoming=true;
 bool threat=offensive(o.action)&&std::abs(dx)<3.2f&&std::abs(dy)<2;
 if(f.action==Action::Taunt){
  if(incoming||threat||std::abs(dx)<3.8f||f.floor<0){begin(i,Action::Idle);react(i,Reaction::NearMiss,.35f);}
  else return;
 }
 if((threat||incoming)&&rng_.f()<.47f){
  if(f.floor>=0&&f.shield>.3f&&rng_.f()<.70f)begin(i,Action::Shield);
  else {begin(i,Action::Dodge);f.vx=-f.face*7.8f;}
  return;
 }
 if(f.floor>0&&dy< -1.2f&&std::abs(dx)<5.0f){f.drop=.3f;f.floor=-1;f.y-=.04f;f.vy=-1.2f;begin(i,Action::Jump);return;}
 if((dy>1.5f&&std::abs(dx)<8)||(f.floor<0&&f.vy<0&&dy>1.0f&&std::abs(dx)<4)){
  if(f.jumps>0){f.vy=12.8f;--f.jumps;f.floor=-1;begin(i,Action::Jump);++counters.jumps;return;}}
 if(f.seeking&&std::abs(dx)>1)return;
 // Flourishes happen in safe breathing space or after a KO, never offstage.
 if(f.floor>=0&&std::abs(f.x)<10&&!incoming&&!threat&&!f.seeking&&std::abs(dx)>5.0f&&
    ((f.tauntPending&&f.tauntCooldown<4)||f.tauntCooldown<=0)&&rng_.f()<.32f){begin(i,Action::Taunt);return;}
 if(f.cooldown>0)return;
 if(f.specialCooldown<=0&&std::abs(dx)<10&&std::abs(dy)<2.5f&&rng_.f()<.26f){begin(i,Action::Special);f.specialCooldown=3.0f+rng_.range(0,1.6f);return;}
 if(std::abs(dx)<2.55f&&std::abs(dy)<1.75f){
  if(o.action==Action::Shield&&std::abs(dx)<1.8f){begin(i,Action::Grab);++counters.grabs;}
  else if(f.floor<0)begin(i,Action::Aerial);
  else begin(i,rng_.f()<.38f?Action::Heavy:Action::Jab);
 }else if(f.specialCooldown<=0&&std::abs(dx)<12&&std::abs(dx)>3&&std::abs(dy)<1.6f&&rng_.f()<.48f){begin(i,Action::Special);f.specialCooldown=2.4f+rng_.range(0,1.5f);}
 else if(f.floor>=0&&std::abs(dx)>3&&rng_.f()<.14f){f.vy=12.4f;f.floor=-1;--f.jumps;begin(i,Action::Jump);++counters.jumps;}
 (void)ps;
}
void Battle::step(){
 constexpr float dt=float(FixedStep);previous_=current_;auto& s=current_;s.time+=FixedStep;s.roundTime+=FixedStep;++counters.ticks;
 std::erase_if(s.sounds,[&](const saver::SoundEvent& e){return s.time-e.time>.35;});
 for(auto& e:s.effects){e.age+=dt;e.p=e.p+e.v*dt;}
 std::erase_if(s.effects,[](const Effect& e){return e.age>=e.life;});
 if(s.finishedAt>=0&&s.roundTime-s.finishedAt>5.5){reset(s.round+1,s.time);return;}
 bool fighting=s.roundTime>=2.6&&s.finishedAt<0;
 auto ps=platforms(s.stage,s.roundTime),oldps=platforms(s.stage,s.roundTime-FixedStep);
 for(int i=0;i<s.active;i++){
  auto& f=s.fighters[i];if(f.stocks<=0)continue;
  if(f.respawn>0){f.respawn=std::max(0.f,f.respawn-dt);if(f.respawn==0){f.x=(i-(s.active-1)*.5f)*2.4f;f.y=10;f.vx=0;f.vy=-1;f.damage=0;f.invulnerable=2.8f;f.floor=-1;f.jumps=2;f.shield=1;f.recoveryUsed=false;f.launchPower=f.spinVelocity=0;f.reaction=Reaction::None;f.reactionTime=0;f.tauntPending=false;f.tauntCooldown=3;begin(i,Action::Jump);f.anim={};f.anim.yaw=f.face*.93f;effect({f.x,10,0},4,f.id);}continue;}
  for(float* timer:{&f.cooldown,&f.specialCooldown,&f.stun,&f.invulnerable,&f.drop,&f.overclock,&f.tauntCooldown,&f.reactionTime})*timer=std::max(0.f,*timer-dt);
  if(f.reactionTime<=0)f.reaction=Reaction::None;
  if(f.hitstop>0){f.hitstop=std::max(0.f,f.hitstop-dt);continue;}
  f.actionTime+=dt;f.anim.land=reach(f.anim.land,0,14,dt);
  if(busy(f.action)&&f.actionTime>move(f.action,f.id).duration){f.cooldown=.11f+rng_.range(0,.10f);begin(i,Action::Idle);}
  if(f.action==Action::Hurt&&f.stun<=0)begin(i,Action::Idle);
  if(f.floor>=0&&f.action!=Action::Hurt&&f.action!=Action::Land)f.launchPower=reach(f.launchPower,0,6,dt);
  if(!fighting){f.vx=approach(f.vx,0,dt*16);
   if(s.finishedAt>=0){Action result=s.winner==i?Action::Celebrate:Action::Defeat;if(f.action!=result)begin(i,result);}
  }
  else{
   f.decision-=dt;if(f.decision<=0){think(i);f.decision=rng_.range(.10,.25);}
   if(f.stun<=0&&f.action!=Action::Dodge&&f.action!=Action::Recovery){
    float goal=0;
    if(!busy(f.action)&&f.action!=Action::Shield){
     if((std::abs(f.x)>10.8f&&f.floor<0)||f.y<0)goal=(f.x<0?1.f:-1.f)*speed(f.id);
     else if(f.seeking){float dx=f.goalX-f.x;goal=std::abs(dx)>.5f?(dx>0?1:-1)*speed(f.id):0;}
     else if(f.target>=0){const auto& o=s.fighters[f.target];float dx=o.x-f.x;goal=std::abs(dx)>1.6f?(dx>0?1:-1)*speed(f.id):0;}
    }
    if(f.id==2&&f.action==Action::Special&&f.actionTime>.23f&&f.actionTime<.50f)goal=f.face*19;
    f.vx=approach(f.vx,goal,dt*(f.floor>=0?30.f:16.f));
   }
  }
  if(f.floor>=0){f.x+=ps[f.floor].x-oldps[f.floor].x;f.y+=ps[f.floor].y-oldps[f.floor].y;}
  float oldY=f.y,oldX=f.x;
  f.x+=f.vx*dt;
  if(f.floor>=0&&(std::abs(f.x-ps[f.floor].x)>ps[f.floor].half+.15f||f.vy>.1f))f.floor=-1;
  if(f.floor<0){f.vy=std::max(-25.f,f.vy-20.5f*dt);f.y+=f.vy*dt;
   if(f.vy<=0){int landed=-1;float high=-100;
    for(int p=0;p<4;p++){const auto& platform=ps[p];if(!platform.solid&&f.drop>0)continue;
     if(oldY>=platform.y-.03f&&f.y<=platform.y&&std::abs(f.x-platform.x)<platform.half+.2f&&platform.y>high){landed=p;high=platform.y;}}
    if(landed>=0){
     float impact=std::abs(f.vy);f.y=high;f.anim.land=clamp(impact/23);f.vy=0;f.floor=landed;f.jumps=2;f.recoveryUsed=false;
     if(impact>10){effect({f.x,high+.04f,0},7,f.id);++counters.hardLandings;
      if(fighting&&(!busy(f.action)||f.action==Action::Recovery)){begin(i,Action::Land);f.vx*=.65f;}
     }
    }
   }
  }else{f.y=ps[f.floor].y;f.vy=0;}
  if(f.action==Action::Shield)f.shield=std::max(0.f,f.shield-dt*.17f);else f.shield=std::min(1.f,f.shield+dt*.11f);
  if(f.action==Action::Special&&f.actionTime>=.23f&&!f.projectileMade&&s.shots.size()<64){
   special(i);
  }
  if((std::abs(f.x)>24||f.y< -11||f.y>23)&&fighting){
   effect({clamp(f.x,-21,21),clamp(f.y,-7,19),0},2,f.id);
   --f.stocks;++counters.kos;cue(saver::Cue::Knockout,i);if(f.lastHit>=0&&f.lastHit<s.active&&f.lastHit!=i){auto& victor=s.fighters[f.lastHit];++victor.kos;victor.tauntPending=true;victor.tauntCooldown=std::min(victor.tauntCooldown,1.2f);if(victor.stun<=0)react(f.lastHit,Reaction::Happy,1.2f);}
   f.respawn=1.2f;f.lastHit=-1;f.stun=f.hitstop=0;f.vx=f.vy=0;continue;
  }
  // Continuous animation channels are smoothed separately from combat transitions.
  auto& a=f.anim;float t=f.actionTime;
  a.gait+=std::abs(f.x-oldX)*4.6f;
  a.walk=reach(a.walk,f.floor>=0&&f.stun<=0?clamp(std::abs(f.vx)/5):0,13,dt);
  float facing=f.action==Action::Taunt||f.action==Action::Celebrate||f.action==Action::Defeat?f.face*.28f:f.face*.93f;
  a.yaw=reach(a.yaw,facing,12,dt);
  if(f.target>=0){const auto& target=s.fighters[f.target];
   a.look=reach(a.look,clamp((target.x-f.x)*f.face*.15f,-1,1),8,dt);
   a.lookUp=reach(a.lookUp,clamp((target.y-f.y)*.25f,-1,1),8,dt);
  }
  a.brake=reach(a.brake,clamp(f.vx*f.face/8,-1,1),9,dt);
  auto channel=[&](float& v,float to){v=reach(v,to,28,dt);};
  auto strike=[&](Action type){auto m=move(type,f.id);return f.action==type?ease(0,m.start,t)*(1-ease(m.end,m.duration,t)):0.f;};
  channel(a.punch,strike(Action::Jab));channel(a.heavy,strike(Action::Heavy));channel(a.aerial,strike(Action::Aerial));channel(a.grab,strike(Action::Grab));
  channel(a.shield,f.action==Action::Shield?1.f:0.f);channel(a.duck,f.action==Action::Dodge?std::sin(clamp(t/.42f)*pi):0.f);
  channel(a.recoil,f.stun>0?clamp(std::abs(f.vx)/22+.15f):0.f);channel(a.cast,strike(Action::Special));channel(a.recover,f.action==Action::Recovery?1.f:0.f);channel(a.air,f.floor<0?1.f:0.f);
  animateFighter(f,dt,s.roundTime);
  if(f.floor<0&&f.action==Action::Hurt&&f.launchPower>.25f){
   f.trailClock-=dt;if(f.trailClock<=0){f.trailClock=.055f;effect({f.x,f.y+.9f,-.18f},8,f.id,{-f.vx*.06f,.35f,0});}
  }
 }
 if(fighting){
  // Gather attacks before resolving them so a simultaneous trade is possible.
  struct Hit{int a,b;Move m;int dir;bool grab;};std::vector<Hit> pending;
  for(int i=0;i<s.active;i++){auto& a=s.fighters[i];if(a.stocks<=0||a.respawn>0||a.stun>0||!offensive(a.action))continue;auto m=move(a.action,a.id);
   if(a.actionTime<m.start||a.actionTime>m.end)continue;
   for(int j=0;j<s.active;j++){const auto& b=s.fighters[j];if(j==i||b.stocks<=0||b.respawn>0||(a.hits&(1u<<j)))continue;
    float dx=b.x-a.x-a.face*m.reach,dy=b.y-a.y;
    if(dx*dx/(m.radius+.52f)/(m.radius+.52f)+dy*dy/2.9f<1){a.hits|=1u<<j;pending.push_back({i,j,m,a.face,a.action==Action::Grab});}}
  }
  for(const auto& h:pending){float buff=s.fighters[h.a].overclock>0?1.25f:1;hit(h.a,h.b,h.m.damage*buff,h.m.launch,h.dir,h.m.lift,h.grab);}
  for(auto& shot:s.shots){float old=shot.p.x;shot.age+=dt;
   if(shot.id==3&&shot.age>.62f&&!shot.turned){shot.v.x=-shot.v.x;shot.turned=true;shot.hits=0;}
   if(shot.id==7)shot.v.y+=1.8f*dt;
   if(shot.id==5)for(int j=0;j<s.active;++j)if(j!=shot.owner){auto& b=s.fighters[j];float d=shot.p.x-b.x;if(std::abs(d)<4&&std::abs(shot.p.y-b.y-1)<3&&b.invulnerable<=0)b.vx+=clamp(d*9,-24,24)*dt;}
   shot.p=shot.p+shot.v*dt;
   for(int j=0;j<s.active;j++){auto& b=s.fighters[j];if(j==shot.owner||b.respawn>0||b.stocks<=0||(shot.hits&(1u<<j)))continue;
    float closest=clamp(b.x,std::min(old,shot.p.x),std::max(old,shot.p.x));
    if(std::abs(b.x-closest)<.65f+shot.radius&&std::abs(b.y+1.2f-shot.p.y)<1.0f){hit(shot.owner,j,shot.damage*(s.fighters[shot.owner].overclock>0?1.25f:1),shot.launch,shot.v.x>0?1:-1,shot.lift);
     shot.hits|=1u<<j;if(shot.id!=0&&shot.id!=3&&shot.id!=6)shot.age=shot.life;break;}}
  }
  std::erase_if(s.shots,[](const Projectile& p){return p.age>=p.life||std::abs(p.p.x)>28;});
  nextPickup_-=dt;if(nextPickup_<=0){nextPickup_=9;int p=rng_.next()%4;if(s.pickups.size()<3)s.pickups.push_back({ps[p].x,ps[p].y+1.0f,0,int(rng_.next()%3),p});}
  for(auto& item:s.pickups){item.x+=ps[item.platform].x-oldps[item.platform].x;item.y=ps[item.platform].y+1;item.age+=dt;for(int i=0;i<s.active;i++){auto& f=s.fighters[i];if(f.stocks<=0||f.respawn>0)continue;if(std::abs(f.x-item.x)<1&&std::abs(f.y+1-item.y)<1.5f){
   if(item.kind==0)f.damage=std::max(0.f,f.damage-25);else if(item.kind==1)f.overclock=8;else {f.shield=1;f.invulnerable=2.5f;}
   item.age=31;++counters.pickups;cue(saver::Cue::Pickup,i);if(f.stun<=0)react(i,Reaction::Happy,.8f);effect({item.x,item.y,0},4,f.id);break;}}}
  std::erase_if(s.pickups,[](const Pickup& p){return p.age>30;});
  int living=0;for(int i=0;i<s.active;i++)living+=s.fighters[i].stocks>0;
  if(living<=1||s.roundTime>=92.6){
   int winner=0;auto score=[](const Fighter& f){return f.stocks*10000.f+f.kos*500.f-f.damage;};
   for(int i=1;i<s.active;i++)if(score(s.fighters[i])>score(s.fighters[winner]))winner=i;
   s.winner=winner;s.finishedAt=s.roundTime;s.shots.clear();s.pickups.clear();
  }
 }
 // Following camera uses bounded, slowly converging framing rather than hard cuts.
 float lo=0,hi=0,top=5;int live=0;
 for(int i=0;i<s.active;i++){const auto& f=s.fighters[i];if(f.stocks<=0||f.respawn>0)continue;if(!live){lo=hi=f.x;}lo=std::min(lo,f.x);hi=std::max(hi,f.x);top=std::max(top,f.y+2.8f);++live;}
 V3 goal{clamp((lo+hi)*.5f,-3,3),clamp(top*.37f,2.8f,5.5f),0};
 s.cameraTarget=mix(s.cameraTarget,goal,1-std::exp(-dt*1.6f));
 s.cameraDistance=reach(s.cameraDistance,clamp(29.f+(hi-lo)*.20f+(top-7)*.6f,30,41),1.3f,dt);
}
void Battle::advance(double seconds){
 if(!std::isfinite(seconds)||seconds<0||seconds>86400)throw std::runtime_error("Invalid simulation interval");
 accumulator_+=seconds;
 const auto steps=std::uint64_t(std::floor(accumulator_/FixedStep+1e-7));
 accumulator_-=steps*FixedStep;if(accumulator_<0)accumulator_=0;
 for(std::uint64_t i=0;i<steps;i++)step();
}
void Battle::seek(double time){if(!std::isfinite(time)||time<0||time>86400)throw std::runtime_error("Invalid simulation time");double total=current_.time+accumulator_;if(time+1e-9<total){*this=Battle(seed_,stageChoice_,count_,firstStage_);total=0;}advance(std::max(0.,time-total));}
BattleState Battle::sample()const{
 auto s=current_;float a=clamp(float(accumulator_/FixedStep));if(s.round!=previous_.round)return s;
 auto lerp=[a](float x,float y){return x+(y-x)*a;};
 s.time=previous_.time+(current_.time-previous_.time)*a;s.roundTime=previous_.roundTime+(current_.roundTime-previous_.roundTime)*a;
 for(int i=0;i<s.active;i++){
  auto& f=s.fighters[i];auto& p=previous_.fighters[i];if(f.stocks!=p.stocks||(p.respawn>0)!=(f.respawn>0))continue;
  f.x=lerp(p.x,f.x);f.y=lerp(p.y,f.y);f.vx=lerp(p.vx,f.vx);f.vy=lerp(p.vy,f.vy);
  if(p.action==f.action)f.actionTime=lerp(p.actionTime,f.actionTime);
  auto& b=f.anim;auto& v=p.anim;
#define BLEND(field) b.field=lerp(v.field,b.field)
  BLEND(gait);BLEND(walk);BLEND(punch);BLEND(heavy);BLEND(aerial);BLEND(shield);BLEND(duck);BLEND(recoil);BLEND(cast);BLEND(recover);BLEND(grab);BLEND(air);BLEND(land);BLEND(yaw);
  BLEND(tumble);BLEND(impact);BLEND(taunt);BLEND(celebrate);BLEND(look);BLEND(lookUp);BLEND(brake);
#undef BLEND
  b.pose=blendPose(v.pose,b.pose,a);
 }
 for(auto& p:s.shots){float back=float(FixedStep)*(1-a);p.p=p.p-p.v*back;p.age=std::max(0.f,p.age-back);}
 s.cameraTarget=mix(previous_.cameraTarget,current_.cameraTarget,a);s.cameraDistance=lerp(previous_.cameraDistance,current_.cameraDistance);
 return s;
}
}
