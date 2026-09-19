// SPDX-License-Identifier: GPL-3.0-or-later
#include "reef/renderer.hpp"
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace reef;
static std::uint64_t checks=0;
void require(bool v,const char* name){++checks;if(!v)throw std::runtime_error(name);}
bool near(double a,double b,double epsilon=1e-8){return std::abs(a-b)<epsilon;}
int main(){try{
 auto cpu=parseCpu("cpu 20 5 15 100 5 3 2 0 999 888\ncpu0 1 2 3");
 require(cpu&&cpu->total==150&&cpu->idle==105,"CPU excludes duplicate guest accounting");
 require(!parseCpu("cpu0 1 2 3 4 5 6 7 8"),"aggregate CPU only");
 require(!parseCpu("cpu x"),"bad CPU input");require(!parseCpu(""),"missing CPU input");
 require(near(*cpuUsage({150,105},{250,155}),50),"CPU interval percentage");
 require(!cpuUsage({100,50},{90,55}),"CPU counter rollback");require(!cpuUsage({100,50},{100,50}),"zero CPU interval");require(!cpuUsage({100,50},{120,90}),"impossible CPU idle delta");require(!cpuUsage({100,50},{120,40}),"iowait rollback is unavailable");
 auto mem=parseMemory("MemTotal: 33554432 kB\nMemFree: 1000 kB\nMemAvailable: 25165824 kB\n");
 require(mem&&near(mem->first,8)&&near(mem->second,32),"MemAvailable based usage, GiB units");require(!parseMemory("MemTotal: 123 kB"),"missing available memory");require(!parseMemory("MemTotal: 1 kB\nMemAvailable: 2 kB"),"invalid memory counters");
 auto net=parseNetwork("Inter-| Receive\n  eth0: 123 2 3 4 5 6 7 8 456 10 11 12 13 14 15 16\n  lo: 20 0 0 0 0 0 0 0 20 0 0 0 0 0 0 0\n  wlan0: rubbish");
 require(net.size()==2&&net["eth0"].rx==123&&net["eth0"].tx==456,"network columns");
 require(defaultInterface("Iface Destination Gateway Flags RefCnt Use Metric\neth0 00000000 01010101 0003 0 0 100\nwlan0 00000000 01010101 0003 0 0 600\n")=="eth0","lowest metric default route");
 require(defaultInterface("eth0 00000000 00 0000 0 0 0\n").empty(),"down route not selected");
 for(std::uint64_t seed=0;seed<48;seed++){
  World a(seed),b(seed);require(a.fish.size()==10&&a.plants.size()==54,"bounded ecology allocations");
  for(std::size_t i=0;i<a.fish.size();i++){
   for(int step=0;step<720;step++){
    double t=step*5.0;auto p=a.fishPose(i,t),q=b.fishPose(i,t);
    require(near(p.p.x,q.p.x)&&near(p.p.y,q.p.y)&&near(p.yaw,q.yaw),"seed determinism");
    require(std::isfinite(p.p.x)&&std::isfinite(p.p.y)&&std::isfinite(p.yaw)&&p.scale>.2,"finite positive poses");
    require(p.p.x>-400&&p.p.x<2000&&p.p.y>0&&p.p.y<850,"fish excursions stay in bounded offscreen lanes");
    auto n=a.fishPose(i,t+.001),before=a.fishPose(i,std::max(0.0,t-.001));
    double speed=std::hypot(n.p.x-p.p.x,n.p.y-p.p.y)/.001;
    double turn=std::abs(std::remainder(n.yaw-p.yaw,tau))/.001;
    require(speed<100,"bounded swimming speed");require(turn<.5,"continuous turning without mirror flips");
    if(t>0){double accel=std::hypot(n.p.x-2*p.p.x+before.p.x,n.p.y-2*p.p.y+before.p.y)/1e-6;require(accel<20,"bounded continuous path acceleration");}
   }
  }
 }
 World w(7);
 require(w.creatures.size()==9,"sharks, octopi, rays, turtle and seahorses present");
 for(std::size_t i=0;i<w.creatures.size();++i){bool off=false,on=false;for(int step=0;step<600;++step){auto p=w.creaturePose(i,step);require(std::isfinite(p.p.x)&&p.scale>0,"finite creature rig");off|=p.p.x<0||p.p.x>1600;on|=p.p.x>200&&p.p.x<1400;}require(off&&on,"creatures cross the visible boundary");}
 for(std::size_t i=0;i<w.jellies.size();++i){auto a=w.jellyPose(i,0),b=w.jellyPose(i,90);require(std::hypot(a.x-b.x,a.y-b.y)>100,"jellies propel through the aquarium");}
 for(int j=0;j<=100;j++)for(int k=0;k<30;k++){
  double u=j/100.0,a=k*tau/30;auto p=w.fishLocal(u,a,1),q=w.fishLocal(u,a,1+tau);
  require(length(p-q)<1e-8,"mesh deformation has a continuous cycle");
 }
 for(const auto& scheme:{"lagoon","amethyst","ember"}){
  Renderer r(42,scheme);Metrics m=demoMetrics(5);
  for(const auto& sz:{Vec2{640,360},Vec2{800,600},Vec2{360,640},Vec2{1600,600}}){
   const auto& im=r.render(int(sz.x),int(sz.y),5,m);
   require(im.data()!=nullptr&&im.stride()>=int(sz.x)*4,"render surface valid");
   auto*p=im.data()+int(sz.y/2)*im.stride()+int(sz.x/2)*4;
   require(p[3]==255,"opaque surface (no transparent screen leaks)");
  }
 }
 {
  Renderer r(42);DrawList list;std::size_t maximum=0;
  for(int i=0;i<80;i++){
   r.drawGpu(list,960,540,i*.5,demoMetrics(i*.5));
   require(!list.vertices.empty()&&list.vertices.size()%3==0,"GPU triangles are complete");
   require(list.vertices.size()<750000,"bounded per-frame GPU geometry");
   require(list.batches.size()<24&&list.textCacheSize()<=140,"bounded draw and text caches");
   for(const auto& v:list.vertices)require(std::isfinite(v.position.x)&&std::isfinite(v.position.y)&&std::isfinite(v.color.a),"finite GPU vertices");
   for(const auto& batch:list.batches)require(batch.first+batch.count<=list.vertices.size(),"GPU batch in range");
   maximum=std::max(maximum,list.vertices.size());
  }
  std::cout<<"Peak triangle stream: "<<maximum<<" vertices.\n";
 }
 bool rejected=false;try{Renderer r;r.render(0,100,0,{});}catch(const std::invalid_argument&){rejected=true;}require(rejected,"invalid render size rejected");
 rejected=false;try{Renderer r;r.render(64,64,std::numeric_limits<double>::quiet_NaN(),{});}catch(const std::invalid_argument&){rejected=true;}require(rejected,"NaN time rejected");
 std::cout<<checks<<" checks passed; 48 seeded one-hour pose sweeps, continuous turns, telemetry parsers, 3 palettes, 4 aspect ratios.\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}}
