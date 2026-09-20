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
};
}
int main(){try{
 check(std::abs(length(unit({3,4,0}))-1)<1e-6,"normalization");
 for(int stage=0;stage<3;stage++)for(int n=0;n<100;n++){
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
 }
 for(int stage=0;stage<3;stage++)for(int seed=1;seed<5;seed++){
  Battle b(seed,stage,4);for(int tick=0;tick<1800;tick++){b.advance(.05);auto s=b.sample();
   check(s.shots.size()<=64&&s.effects.size()<=192&&s.pickups.size()<=3,"bounded transient pools");
   for(int i=0;i<4;i++){const auto& f=s.fighters[i];check(std::isfinite(f.x)&&std::isfinite(f.y)&&std::isfinite(f.anim.yaw),"finite motion");check(f.stocks>=0&&f.stocks<=3,"stock limits");check(f.damage>=0&&f.damage<=350,"damage bounds");check(f.shield>=0&&f.shield<=1,"shield bounds");}
  }
 }
 {Frame frame;initialize(frame);Battle b;SceneOptions opt;b.advance(15);compose(frame,b.sample(),opt);
  for(auto& batch:frame.batches){for(auto index:batch.mesh.ix)check(index<batch.mesh.v.size(),"mesh indices");for(auto instance:batch.instances)for(float v:instance.model.a)check(std::isfinite(v),"finite pose matrices");}
 }
 std::cout<<"PASS "<<checks<<" assertions\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
