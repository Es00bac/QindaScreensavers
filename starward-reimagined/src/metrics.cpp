// SPDX-License-Identifier: GPL-3.0-or-later
#include "starward/metrics.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
namespace starward {
static std::string read(const char* p){std::ifstream f(p);if(!f)return{}; std::ostringstream b;b<<f.rdbuf();return b.str();}
std::optional<CpuCount> parseCpu(const std::string& s){
 std::istringstream in(s);std::string label;in>>label;if(label!="cpu")return{};
 std::array<std::uint64_t,8> v{};for(auto& n:v)if(!(in>>n))return{};
 std::uint64_t total=0;for(auto n:v){if(n>std::numeric_limits<std::uint64_t>::max()-total)return{};total+=n;}
 return CpuCount{total,v[3]+v[4]}; // guest fields are already in user/nice.
}
std::optional<double> cpuUsage(CpuCount a,CpuCount b){
 if(b.total<=a.total||b.idle<a.idle)return{};
 const auto dt=b.total-a.total,di=b.idle-a.idle;if(di>dt)return{};
 return 100.0*(1-double(di)/double(dt));
}
std::optional<std::pair<double,double>> parseMemory(const std::string& s){
 std::istringstream input(s);std::string line;double total=-1,available=-1;
 while(std::getline(input,line)){std::istringstream row(line);std::string k,unit;double v; if(!(row>>k>>v>>unit)||unit!="kB"||v<0)continue;if(k=="MemTotal:")total=v;if(k=="MemAvailable:")available=v;}
 if(total<=0||available<0||available>total)return{};
 return std::pair{(total-available)/1048576.0,total/1048576.0};
}
std::map<std::string,NetCount> parseNetwork(const std::string& s){
 std::map<std::string,NetCount> out;std::istringstream in(s);std::string row;
 while(std::getline(in,row)){
  const auto colon=row.find(':');if(colon==std::string::npos)continue;
  auto name=row.substr(0,colon);auto first=name.find_first_not_of(" \t");if(first==std::string::npos)continue;name.erase(0,first);name.erase(name.find_last_not_of(" \t")+1);
  std::istringstream values(row.substr(colon+1));std::array<std::uint64_t,16> v{};bool ok=true;
  for(auto& value:v)if(!(values>>value)){ok=false;break;}
  if(ok)out[name]={v[0],v[8]};
 }
 return out;
}
std::string defaultInterface(const std::string& s){
 std::istringstream input(s);std::string row,best;unsigned bestMetric=~0u;
 while(std::getline(input,row)){std::istringstream in(row);std::string name,dest,gw,flags,ref,use;unsigned metric;
  if(!(in>>name>>dest>>gw>>flags>>ref>>use>>metric)||dest!="00000000")continue;
  unsigned f{};std::istringstream(flags)>>std::hex>>f;
  if((f&1)&&metric<bestMetric){best=name;bestMetric=metric;}
 }
 return best;
}
Metrics demoMetrics(double t){Metrics m;m.demo=true;m.cpu=34+18*std::sin(t*.15);m.ramTotal=32;m.ramUsed=9.8+1.3*std::sin(t*.06);m.rx=144000+92000*std::sin(t*.35);m.tx=47000+22000*std::sin(t*.21);m.uptime=301220+t;m.interface="DEMO";return m;}
Monitor::Monitor(std::string s):selected_(std::move(s)),worker_([this]{run();}){}
Monitor::~Monitor(){{std::lock_guard lock(mutex_);stop_=true;}wake_.notify_one();worker_.join();}
Metrics Monitor::snapshot()const{std::lock_guard lock(mutex_);auto m=current_; if(std::chrono::steady_clock::now()-m.sampled>std::chrono::seconds(4)){m.cpu.reset();m.ramUsed.reset();m.ramTotal.reset();m.rx.reset();m.tx.reset();m.uptime.reset();}return m;}
void Monitor::run(){
 std::optional<CpuCount> oldCpu;std::optional<NetCount> oldNet;std::string oldIf;auto oldTime=std::chrono::steady_clock::now();
 while(true){
  const auto now=std::chrono::steady_clock::now();Metrics m;m.sampled=now;
  auto cpu=parseCpu(read("/proc/stat"));if(cpu&&oldCpu)m.cpu=cpuUsage(*oldCpu,*cpu);oldCpu=cpu;
  auto mem=parseMemory(read("/proc/meminfo"));if(mem){m.ramUsed=mem->first;m.ramTotal=mem->second;}
  std::istringstream uptime(read("/proc/uptime"));double seconds;if(uptime>>seconds&&std::isfinite(seconds)&&seconds>=0)m.uptime=seconds;
  auto net=parseNetwork(read("/proc/net/dev"));auto name=selected_.empty()?defaultInterface(read("/proc/net/route")):selected_;
  // IPv6-only/no IPv4 default: choose one stable non-loopback interface, not a double-counted sum.
  if(name.empty())for(const auto& [n,v]:net){(void)v;if(n!="lo"){name=n;break;}}
  m.interface=name;const auto it=net.find(name);double dt=std::chrono::duration<double>(now-oldTime).count();
  if(it!=net.end()){
   if(oldNet&&oldIf==name&&dt>.05&&it->second.rx>=oldNet->rx&&it->second.tx>=oldNet->tx){m.rx=double(it->second.rx-oldNet->rx)/dt;m.tx=double(it->second.tx-oldNet->tx)/dt;}
   oldNet=it->second;
  }else oldNet.reset();oldIf=name;oldTime=now;
  std::unique_lock lock(mutex_);current_=m;if(wake_.wait_for(lock,std::chrono::seconds(1),[this]{return stop_;}))break;
 }
}
}
