#include <algorithm>
#include "world.hpp"
#include "metrics.hpp"
#include "renderer.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
using namespace patrol;
namespace {
int checks=0;
void check(bool b,const char* message){++checks;if(!b)throw std::runtime_error(message);}
void near(double x,double y,double epsilon,const char* message){check(std::abs(x-y)<epsilon,message);}
}
int main(){try{
    auto a=parseCpu("cpu 100 10 30 800 40 5 5 10 50 20\ncpu0 2 3 4 5\n");
    auto b=parseCpu("cpu 150 10 50 900 60 10 10 10 9999 9999\n");
    check(a.has_value()&&b.has_value(),"parse cpu");near(*cpuPercent(*a,*b),40,0.000001,"CPU excludes guest and counts iowait as idle");
    check(!parseCpu("cpu nope 1 2 3"),"malformed cpu");check(!parseCpu("cpu -1 2 3 4"),"negative cpu");check(!parseCpu("cpu0 1 2 3 4"),"must use aggregate cpu");check(!cpuPercent(*b,*a),"reset counters");check(!cpuPercent(*a,*a),"zero interval");
    Metrics m;parseMemory("MemTotal: 8388608 kB\nMemFree: 100 kB\nMemAvailable: 2097152 kB\nHugePages_Total: 0\n",m);near(m.memory,75,0.00001,"memory uses MemAvailable");near(m.memoryTotalGiB,8,0.00001,"GiB units");
    parseMemory("MemTotal: 100 kB\nMemFree: 90 kB\n",m);check(m.memory<0,"no invented MemAvailable fallback");parseMemory("MemTotal: 100 kB\nMemAvailable: 200 kB\n",m);check(m.memory<0,"invalid memory range");
    std::string route="Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT\nwlan0 00000000 01010101 0003 0 0 600 00000000 0 0 0\neth0 00000000 02020202 0003 0 0 100 00000000 0 0 0\n";
    check(defaultInterface(route)=="eth0","choose minimum default-route metric");
    auto net=parseNetwork(" eth0: 1024 2 0 0 0 0 0 0 4096 2 0 0 0 0 0 0\n","eth0");check(net&&(*net)[0]==1024&&(*net)[1]==4096,"network byte columns");check(!parseNetwork("eth0: 1 2 3","eth0"),"truncated network row");
    for(int gap:{32,64,96})for(int rise:{-64,-32,0,32,64}){auto j=planJump({100,416},{100.0+26+gap+36,416.0+rise});check(j.valid,"all generated joins reachable");near(416+j.vy*j.duration+0.5*Gravity*j.duration*j.duration,416+rise,1e-8,"ballistic landing endpoint");}
    check(!planJump({0,400},{400,300}).valid,"reject impossible jumps");
    World one(17),two(17);for(int i=0;i<12000;++i){one.step();two.step();}near(one.hero.pos.x,two.hero.pos.x,1e-8,"same-seed determinism");check(one.cleared==two.cleared,"combat determinism");
    for(std::uint64_t seed=0;seed<100;++seed){World w(seed);for(int i=0;i<36000;++i){w.step();if(i%500==0){check(std::isfinite(w.hero.pos.x)&&std::isfinite(w.hero.pos.y),"finite hero");check(w.hero.pos.y<=449&&w.hero.pos.y>150,"hero cannot fall out of level");check(w.platforms.size()<=18,"bounded platforms");check(w.enemies.size()<=36,"bounded enemies");check(w.bolts.size()<=48&&w.particles.size()<=180,"bounded effects");}}check(w.hero.platform>35,"autopilot makes progress");check(w.cleared>20,"duo actually defeats enemies");check(w.rescues==0,"no safety teleports on reachable generator");}
    {auto a=districtAt(DistrictLength-DistrictBlend-1);check(a.t==0&&a.from==a.to,"district interior is unblended");
     auto b=districtAt(DistrictLength);near(b.t,0.5,1e-9,"district line is the blend midpoint");check(b.from==Biome::Rooftops&&b.to==Biome::Memory,"districts cycle in order");
     check(districtAt(DistrictLength+DistrictBlend-0.001).t>0.999,"blend completes after the line");check(districtBiome(-1)==Biome::Network,"negative district index wraps");
     World v(5);int gates=0,catwalks=0;std::uint64_t seen=0;
     for(int i=0;i<120*300;++i){v.step();for(const auto& p:v.platforms)if(p.id>=seen){seen=p.id+1;gates+=p.gate;catwalks+=p.style==RoofStyle::Catwalk;
         double s=p.x+double(v.distanceBase),line=double(p.district+(p.gate?0:1))*DistrictLength;
         check(p.width>=5*Tile&&std::fmod(p.width,Tile)==0,"tile-aligned roof widths");
         if(p.gate)check(p.width>=8*Tile&&p.sign<0&&p.style==RoofStyle::Building&&s>line-6*Tile&&s<line+3*Tile,"gate roofs are wide, unsigned and sit on their district line");
         else check(s+p.width<=line-Tile+1e-6,"no roof straddles a district line");
         check(districtBiome(p.district)==p.biome,"roof biome follows its district");}}
     check(gates>=2&&catwalks>=5,"generator produces gates and catwalks");}
    World longRun(991);for(int i=0;i<120*2400;++i)longRun.step();check(longRun.distanceBase>0,"long-run coordinate rebase");check(longRun.rescues==0,"rebase preserves navigation");
    // Encounter progression: shuffled guardians, working machinery and bounded rewards.
    {World w(42);std::vector<int> bosses;std::uint64_t seen=0;bool active=false,telegraph=false,recovery=false;
     for(int i=0;i<120*900;++i){w.step();for(const auto& e:w.enemies)if(e.boss){if(e.id>=seen){seen=e.id+1;bosses.push_back(e.bossKind);}if(e.platform==w.hero.platform){if(!active)std::cout<<"Arena first reached at "<<w.time<<"s\n";active=true;telegraph|=e.stage==0;recovery|=e.stage==2;}}
      check(w.hazards.size()<=40&&w.pickups.size()<12,"bounded arena and pickup pools");}
     check(w.bossesDefeated>=8&&w.repairs>=8&&w.collected>=8,"guardians, objectives and rewards complete");
     check(active&&telegraph&&recovery&&w.dodges>0,"boss fights have attack and vulnerability phases");
     check(w.rescues==0,"arena exits preserve reachability");
     for(std::size_t i=0;i+3<bosses.size();i+=4){auto group=std::array<int,4>{bosses[i],bosses[i+1],bosses[i+2],bosses[i+3]};std::sort(group.begin(),group.end());check(group==std::array<int,4>{0,1,2,3},"each shuffled boss cycle contains all four guardians");}
    }
    {World shielded(3);shielded.hero.power=Power::Shield;shielded.hero.powerTime=10;shielded.hazards.push_back({{shielded.hero.pos.x+35,shielded.hero.pos.y-30},{0,0},1,8,2});shielded.step();check(shielded.shieldBlocks==1&&shielded.hero.stagger==0,"shield intercepts hazards before impact");
     World bare(3);bare.hazards.push_back({{bare.hero.pos.x,bare.hero.pos.y-30},{0,0},1,8,2});bare.step();check(bare.hero.stagger>0,"unshielded hazards interrupt firing briefly");}
    World invalid(1);auto previous=invalid.time;invalid.step(std::numeric_limits<double>::quiet_NaN());check(invalid.time==previous,"invalid dt rejected");
    Canvas tiny(16,16);tiny.rect(-4,-5,8,9,0xffff0000u);tiny.blit(tiny,100,100);check(tiny.pixels[0]==0xffff0000u,"clipping");
    Renderer renderer;World scene;const auto& image=renderer.render(scene,demoMetrics(0));check(image.width==960&&image.height==540,"renderer dimensions");check(renderer.assets().penguin.size()==32&&renderer.assets().duck.size()==16,"sprite counts");
    for(auto pixel:image.pixels)check((pixel>>24)==255,"opaque final frame");
    World mid(3);for(int i=0;i<120*30;++i)mid.step();const auto& blended=renderer.render(mid,demoMetrics(mid.time));
    for(auto pixel:blended.pixels)check((pixel>>24)==255,"opaque frame inside a district blend");check(renderer.assets().props.size()==13,"prop count");
    std::cout<<"PASS: "<<checks<<" checks; 100 seeds x 5 minutes + 40-minute rebase simulation; metric parsers and renderer.\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
