// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
#include <iostream>
#include <stdexcept>
using namespace sw;
namespace { unsigned long checks=0;void check(bool p,const char* msg){++checks;if(!p)throw std::runtime_error(msg);} }
namespace sw {
struct BattleTest {
 static BattleState& state(Battle& b){return b.current_;}
 static void hit(Battle& b,int from,int to,float dmg,float power=6,float dir=1,bool bypass=false){b.hit(from,to,dmg,power,dir,.5f,bypass);}
 static void action(Battle& b,int i,Action a){b.begin(i,a);}
 static void special(Battle& b,int i){b.special(i);}
 static void think(Battle& b,int i){b.think(i);}
 static const BattleState& previous(const Battle& b){return b.previous_;}
 static void nextRound(Battle& b){b.reset(b.current_.round+1,b.current_.time);}
};
}
int main(){try{
 check(std::abs(length(unit({3,4,0}))-1)<1e-6,"normalization");
 for(int stage=0;stage<StageCount;stage++)for(int n=0;n<100;n++){
  auto ps=platforms(stage,n*.17);for(auto p:ps)check(p.half>1&&std::isfinite(p.y),"platform geometry");
 }
 // Damage growth increases launch power; shield stops damage; grabs bypass guards.
 {Battle b;auto& s=BattleTest::state(b);for(auto& f:s.fighters)f.invulnerable=0;
  BattleTest::hit(b,0,1,10);float launch=s.fighters[1].vx;check(s.fighters[1].damage==10,"damage application");
  s.fighters[1].invulnerable=0;BattleTest::hit(b,0,1,50);check(s.fighters[1].vx>launch,"damage-scaled knockback");
  s.fighters[2].action=Action::Shield;BattleTest::hit(b,0,2,10);check(s.fighters[2].damage==0&&s.fighters[2].shield<1,"shield defense");
  BattleTest::hit(b,0,2,10,6,1,true);check(s.fighters[2].damage==10,"grab bypass");
  s.fighters[3].action=Action::Dodge;s.fighters[3].actionTime=.1f;BattleTest::hit(b,0,3,10);check(s.fighters[3].damage==0,"dodge invulnerability");
 }
 // Every species has a distinct special, with authoritative projectile properties.
 for(int id=0;id<8;++id){Battle b;auto& s=BattleTest::state(b);auto& f=s.fighters[0];f.id=id;f.damage=40;f.shield=.4f;
  BattleTest::special(b,0);check(b.counters.specials==1&&!s.sounds.empty(),"special emits a sound event");check(!s.shots.empty(),"special produces collision geometry");
  if(id==0)check(s.shots[0].damage==16&&s.shots[0].radius>.6f,"hammer launches a ground shockwave");
  if(id==1)check(s.shots.size()==2&&s.shots[0].v.y!=s.shots[1].v.y,"dual thruster spread");
  if(id==2)check(std::abs(f.vx)==19&&f.invulnerable>0,"fox dash moves and guards the fighter");
  if(id==4)check(f.vy>10&&s.shots[0].lift>1,"uppercut lifts fighter and victim");
  if(id==6)check(s.shots.size()==2&&s.shots[0].v.x<0&&s.shots[1].v.x>0,"stomp has bilateral shockwaves");
  if(id==7)check(f.damage==28&&f.shield>.4f,"tidal mend actually heals and restores shield");
 }
 {Battle b;auto& s=BattleTest::state(b);s.fighters[0].id=3;s.roundTime=4;BattleTest::special(b,0);b.advance(.65);bool returned=false;for(const auto& shot:b.state().shots)if(shot.id==3&&shot.turned)returned=true;check(returned,"boomerang reverses direction");}
 {Battle b(42);b.advance(180);check(b.counters.pickups>4,"fighters actively pursue and use items");}
 // Swept platform landing and ring-out/respawn behavior.
 {Battle b;auto& s=BattleTest::state(b);s.roundTime=4;auto& f=s.fighters[0];f.x=0;f.y=.03f;f.floor=-1;f.vy=-20;f.stun=1;f.invulnerable=0;
  b.advance(FixedStep);check(b.state().fighters[0].floor==0,"swept floor prevents tunneling");
  auto& ff=BattleTest::state(b).fighters[0];ff.x=25;ff.lastHit=1;b.advance(FixedStep);check(b.state().fighters[0].stocks==2,"ringout consumes stock");check(b.state().fighters[1].kos==1,"KO attribution");
  b.advance(1.25);check(b.state().fighters[0].invulnerable>0&&b.state().fighters[0].damage==0,"protected respawn");
 }
 {Battle a(77),b(77);a.advance(60);for(int i=0;i<3600;i++)b.advance(1./60);
  check(a.counters.hits==b.counters.hits,"render-cadence independent hits");
  for(int i=0;i<4;i++)check(std::abs(a.state().fighters[i].x-b.state().fighters[i].x)<.0001,"deterministic positions");
  check(a.counters.hits>40,"active combat, not just idle motion");check(a.counters.specials>0,"specials happen");
  check(a.counters.taunts>4&&a.counters.tumbles>10&&a.counters.hardLandings>5,"new animation events occur in real matches");
  for(int i=0;i<4;i++){
   check(std::abs(a.state().fighters[i].anim.tumble-b.state().fighters[i].anim.tumble)<.0001f,"render-cadence independent tumbling");
   check(length(a.state().fighters[i].anim.pose.hands[1]-b.state().fighters[i].anim.pose.hands[1])<.0001f,"render-cadence independent joint poses");
  }
 }
 // A launch interrupts bravado, freezes its first flinch, then rotates the whole
 // rig. The facing direction changes the spin direction, not its magnitude.
 for(float direction:{-1.f,1.f}){Battle b(41,0,2);auto& s=BattleTest::state(b);s.roundTime=4;
  for(auto& fighter:s.fighters){fighter.invulnerable=0;fighter.decision=100;fighter.action=Action::Idle;}
  auto& f=s.fighters[1];f.x=0;f.y=3;f.floor=-1;f.action=Action::Taunt;
  BattleTest::hit(b,0,1,80,15,direction);
  check(f.action==Action::Hurt&&f.reaction==Reaction::Launched,"hard hit cancels taunt and triggers launch reaction");
  check(f.anim.pose.torso.x<-.1f&&f.anim.pose.open[0]>.2f,"impact immediately poses torso and flailing hands");
  float spin=f.anim.tumble,x=f.x;b.advance(FixedStep*2);
  check(f.anim.tumble==spin&&f.x==x,"impact hold freezes root and skeleton together");
  b.advance(FixedStep*22);check((f.anim.tumble-spin)*direction<-.5f,"launch rotates away from attacker");
  check(std::any_of(s.effects.begin(),s.effects.end(),[](const Effect& e){return e.kind==8;}),"hard launches leave world-space trails");
  b.advance(FixedStep*.5);auto sampled=b.sample();const auto& old=BattleTest::previous(b).fighters[1];
  check(std::abs(sampled.fighters[1].anim.tumble-(old.anim.tumble+f.anim.tumble)*.5f)<.001f,"tumbling interpolates between simulation ticks");
  check(length(sampled.fighters[1].anim.pose.hands[0]-(old.anim.pose.hands[0]+f.anim.pose.hands[0])*.5f)<.001f,"joint targets interpolate with the root");
 }
 {Battle b(41,0,2);auto& s=BattleTest::state(b);s.roundTime=4;auto& f=s.fighters[0];
  for(auto& fighter:s.fighters){fighter.action=Action::Idle;fighter.decision=100;}
  f.x=0;f.face=1;BattleTest::action(b,0,Action::Dodge);b.advance(.22);
  check(std::abs(f.anim.tumble)>2.5f,"dodge rolls the entire rig");float last=f.anim.tumble;
  for(int i=0;i<100;++i){b.advance(FixedStep);check(std::abs(f.anim.tumble-last)<.5f,"roll never snaps across a full-turn angle wrap");last=f.anim.tumble;}
  check(std::abs(std::remainder(f.anim.tumble,2*pi))<.01f,"roll returns to an equivalent upright orientation");
 }
 {Battle b(41,0,2);auto& s=BattleTest::state(b);s.roundTime=4;
  auto& f=s.fighters[0];f.x=0;f.y=.03f;f.floor=-1;f.vy=-23;f.action=Action::Hurt;f.stun=.4f;f.decision=100;
  b.advance(FixedStep);check(f.action==Action::Land&&f.anim.land>.9f,"hard impact braces on the swept platform contact");
  check(b.counters.hardLandings==1&&std::any_of(s.effects.begin(),s.effects.end(),[](const Effect& e){return e.kind==7;}),"hard landing emits one dust event");
 }
 {Battle b(41,0,2);auto& s=BattleTest::state(b);s.roundTime=4;auto& f=s.fighters[0];
  f.invulnerable=0;f.action=Action::Shield;f.shield=.10f;BattleTest::hit(b,1,0,15);
  check(f.action==Action::Hurt&&f.reaction==Reaction::Dizzy,"broken shields use a dizzy reaction");
  f.stun=0;f.action=Action::Taunt;f.actionTime=.5f;f.x=0;s.fighters[1].x=2;
  BattleTest::think(b,0);check(f.action!=Action::Taunt,"taunt aborts when an opponent closes in");
  f.action=Action::Taunt;s.fighters[1].x=8;s.shots.push_back({{2,1.2f,0},{-10,0,0},0,2,.25f,1,1});
  BattleTest::think(b,0);check(f.action!=Action::Taunt,"taunt aborts for an incoming projectile");
 }
 // Every species and gesture can traverse the new animation vocabulary without
 // invalid transforms, detached equipment or a full-turn reset discontinuity.
 {Frame frame;initialize(frame);
  for(int id=0;id<8;++id)for(int clip=0;clip<16;++clip)for(float phase:{.08f,.25f,.55f,1.12f,1.6f}){
   auto fighter=animationDemo(id,clip*2.4+phase);for(auto& batch:frame.batches)batch.instances.clear();
   fighterModel(frame,M4::identity(),id,clip*2.4+phase,fighter.anim);
   for(const auto& batch:frame.batches)for(const auto& instance:batch.instances)for(float value:instance.model.a)check(std::isfinite(value),"all animated rig matrices stay finite");
   for(const auto& hand:fighter.anim.pose.hands)check(length(hand)<2.6f,"hands stay within articulated reach");
  }
  auto salute=animationDemo(0,3.1),bow=animationDemo(2,3.1),wave=animationDemo(7,3.1);
  check(length(salute.anim.pose.hands[1]-bow.anim.pose.hands[1])>.35f&&length(bow.anim.pose.hands[0]-wave.anim.pose.hands[0])>.35f,"signature taunts have distinct body silhouettes");
 }
 for(int stage=0;stage<StageCount;stage++)for(int seed=1;seed<5;seed++){
  Battle b(seed,stage,4);for(int tick=0;tick<1800;tick++){b.advance(.05);auto s=b.sample();
   check(s.shots.size()<=64&&s.effects.size()<=192&&s.pickups.size()<=3,"bounded transient pools");
   for(int i=0;i<4;i++){const auto& f=s.fighters[i];check(std::isfinite(f.x)&&std::isfinite(f.y)&&std::isfinite(f.anim.yaw),"finite motion");check(f.stocks>=0&&f.stocks<=3,"stock limits");check(f.damage>=0&&f.damage<=350,"damage bounds");check(f.shield>=0&&f.shield<=1,"shield bounds");}
  }
 }
 {Frame frame;initialize(frame);Battle b;SceneOptions opt;b.advance(15);compose(frame,b.sample(),opt);
  for(auto& batch:frame.batches){for(auto index:batch.mesh.ix)check(index<batch.mesh.v.size(),"mesh indices");for(auto instance:batch.instances)for(float v:instance.model.a)check(std::isfinite(v),"finite pose matrices");}
 }
 // The stage registry drives selection and the complete automatic rotation.
 {Battle b(41);std::array<bool,StageCount> visited{};
  for(int round=0;round<StageCount;++round){int stage=b.state().stage;check(!visited[stage],"automatic rotation does not repeat before all seven arenas");visited[stage]=true;BattleTest::nextRound(b);}
  for(bool shown:visited)check(shown,"automatic rotation visits every arena");
  bool rejected=false;try{Battle invalid(41,StageCount);}catch(const std::runtime_error&){rejected=true;}check(rejected,"stage constructor rejects out-of-range indices");
 }
 {Frame frame;initialize(frame);
  for(int stage=0;stage<StageCount;++stage)for(double time:{0.,11.7,79.2}){
   for(auto& batch:frame.batches)batch.instances.clear();stageModel(frame,stage,time,777);
   const auto ps=platforms(stage,time);const auto& decks=frame.batches[RoadSurface].instances;
   check(decks.size()==ps.size(),"every collision platform has exactly one visible deck");
   for(unsigned p=0;p<ps.size();++p){
    for(const auto& vertex:frame.batches[RoadSurface].mesh.v){V3 position=point(decks[p].model,vertex.p);
     check(std::abs(position.y-ps[p].y)<.0001f,"visible deck height exactly matches platform collision");
     check(std::abs(position.x-ps[p].x)<=ps[p].half+.0001f,"visible deck matches the collision span");
    }
   }
   size_t instances=0;for(const auto& batch:frame.batches){instances+=batch.instances.size();
    for(const auto& instance:batch.instances)for(float value:instance.model.a)check(std::isfinite(value),"every stage has finite geometry transforms");}
   check(instances<3000,"stage geometry remains bounded");
  }
 }
 // Vertical lifts carry fighters and supplies with the same moving top.
 {Battle b(41,4,2);auto& s=BattleTest::state(b);s.roundTime=4;auto surface=platforms(4,4)[3];
  auto& f=s.fighters[0];f.x=surface.x;f.y=surface.y;f.floor=3;f.vx=f.vy=0;f.decision=100;f.action=Action::Idle;
  s.pickups.push_back({surface.x+2,surface.y+1,0,0,3});
  b.advance(FixedStep);auto now=platforms(4,s.roundTime)[3];
  check(f.floor==3&&std::abs(f.y-now.y)<.0001f,"fighter follows the workshop lift");
  check(!s.pickups.empty()&&std::abs(s.pickups[0].y-now.y-1)<.0001f,"pickup follows the workshop lift");
 }
 std::cout<<"PASS "<<checks<<" assertions\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
