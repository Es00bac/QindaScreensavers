#include "metrics.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <utility>

namespace patrol {
namespace {
std::string readFile(const char* path) {
    std::ifstream f(path);
    if(!f) return {};
    std::string out,line;
    while(std::getline(f,line) && out.size()<256*1024) {out+=line;out+='\n';}
    return out;
}
}
std::optional<CpuCounters> parseCpu(const std::string& text) {
    std::istringstream all(text); std::string line; std::getline(all,line);
    std::istringstream in(line); std::string name; in>>name;
    if(name!="cpu") return {};
    CpuCounters c;
    for(std::size_t i=0;i<8;++i) {
        std::string token; if(!(in>>token)) {if(i<4)return {};break;}
        if(token.empty() || token.find_first_not_of("0123456789")!=std::string::npos) return {};
        try { c.values[i]=std::stoull(token); } catch(...) {return {};}
    }
    return c; // guest/guest_nice are already included in user/nice; do not add.
}
std::optional<double> cpuPercent(const CpuCounters& a,const CpuCounters& b) {
    long double total=0,idle=0;
    for(std::size_t i=0;i<8;++i) {
        // Counter reset / unreliable decreasing iowait: establish a new baseline.
        if(b.values[i]<a.values[i]) return {};
        auto d=b.values[i]-a.values[i];total+=d;
        if(i==3||i==4) idle+=d;
    }
    if(total<=0) return {};
    return std::clamp(double((total-idle)*100/total),0.0,100.0);
}
void parseMemory(const std::string& text,Metrics& out) {
    std::istringstream all(text); std::string line; double total=-1,available=-1;
    while(std::getline(all,line)) {
        std::istringstream in(line);std::string key,unit;double value=-1;
        if(!(in>>key>>value>>unit) || unit!="kB" || !std::isfinite(value)||value<0) continue;
        if(key=="MemTotal:") total=value;
        else if(key=="MemAvailable:") available=value;
    }
    out.memory=out.memoryUsedGiB=out.memoryTotalGiB=-1;
    // Do not relabel MemFree as MemAvailable when the latter is missing.
    if(total>0 && available>=0 && available<=total) {
        out.memory=(total-available)*100/total;
        out.memoryTotalGiB=total/(1024*1024);
        out.memoryUsedGiB=(total-available)/(1024*1024);
    }
}
std::string defaultInterface(const std::string& text) {
    std::istringstream all(text);std::string line,result;
    unsigned long best=std::numeric_limits<unsigned long>::max();
    while(std::getline(all,line)) {
        std::istringstream in(line);std::string name,destination,gateway,flags;
        unsigned long ref,use,metric;
        if(!(in>>name>>destination>>gateway>>flags>>ref>>use>>metric))continue;
        if(destination!="00000000" || name=="lo")continue;
        try {if((std::stoul(flags,nullptr,16)&1) && metric<best){best=metric;result=name;}}catch(...){}
    }
    return result;
}
std::optional<std::array<std::uint64_t,2>> parseNetwork(const std::string& text,const std::string& iface) {
    if(iface.empty())return {};
    std::istringstream all(text);std::string line;
    while(std::getline(all,line)) {
        auto colon=line.find(':');if(colon==std::string::npos)continue;
        std::istringstream name(line.substr(0,colon));std::string n;name>>n;if(n!=iface)continue;
        std::istringstream in(line.substr(colon+1));std::array<std::uint64_t,16> v{};
        for(auto& x:v) {std::string s;if(!(in>>s)||s.find_first_not_of("0123456789")!=std::string::npos)return {};try{x=std::stoull(s);}catch(...){return {};}}
        return std::array<std::uint64_t,2>{v[0],v[8]};
    }
    return {};
}
Metrics demoMetrics(double t) {
    Metrics m;m.mode=MetricMode::Demo;
    m.cpu=28+17*std::sin(t*0.23)+7*std::sin(t*0.8);
    m.memory=43+9*std::sin(t*0.08);m.memoryTotalGiB=32;m.memoryUsedGiB=m.memory*0.32;
    m.load1=1.24+0.6*std::sin(t*0.09);m.uptime=343200+t;
    m.rxKiB=450+380*std::sin(t*0.37);m.txKiB=90+65*std::sin(t*0.24);m.interface="DEMO";
    m.sampled=std::chrono::steady_clock::now();return m;
}
MetricSampler::MetricSampler(std::string interface):requestedInterface_(std::move(interface)) {worker_=std::thread([this]{run();});}
MetricSampler::~MetricSampler() {{std::lock_guard lock(mutex_);stop_=true;}condition_.notify_all();if(worker_.joinable())worker_.join();}
Metrics MetricSampler::snapshot() const {std::lock_guard lock(mutex_);return latest_;}
void MetricSampler::setActive(bool a) {{std::lock_guard lock(mutex_);active_=a;}condition_.notify_all();}
void MetricSampler::run() {
    std::optional<CpuCounters> lastCpu;
    std::optional<std::array<std::uint64_t,2>> lastNet;
    std::string lastIface;
    auto lastTime=std::chrono::steady_clock::now();
    while(true) {
        {std::unique_lock lock(mutex_);condition_.wait(lock,[&]{return stop_||active_;});if(stop_)return;}
        Metrics m;m.sampled=std::chrono::steady_clock::now();
        auto cpu=parseCpu(readFile("/proc/stat"));
        if(cpu&&lastCpu) if(auto p=cpuPercent(*lastCpu,*cpu))m.cpu=*p;
        lastCpu=cpu;
        parseMemory(readFile("/proc/meminfo"),m);
        {std::istringstream s(readFile("/proc/uptime"));double x;if(s>>x && std::isfinite(x)&&x>=0)m.uptime=x;}
        {std::istringstream s(readFile("/proc/loadavg"));double x;if(s>>x && std::isfinite(x)&&x>=0)m.load1=x;}
        m.interface=requestedInterface_.empty()?defaultInterface(readFile("/proc/net/route")):requestedInterface_;
        auto network=parseNetwork(readFile("/proc/net/dev"),m.interface);
        double elapsed=std::chrono::duration<double>(m.sampled-lastTime).count();
        if(network&&lastNet&&lastIface==m.interface&&elapsed>0.001) {
            if((*network)[0]>=(*lastNet)[0] && (*network)[1]>=(*lastNet)[1]) {
                m.rxKiB=double((*network)[0]-(*lastNet)[0])/elapsed/1024;
                m.txKiB=double((*network)[1]-(*lastNet)[1])/elapsed/1024;
            }
        }
        lastTime=m.sampled;lastNet=network;lastIface=m.interface;
        {std::lock_guard lock(mutex_);latest_=std::move(m);}
        std::unique_lock lock(mutex_);
        condition_.wait_for(lock,std::chrono::seconds(1),[&]{return stop_||!active_;});
        if(stop_)return;
        if(!active_){lastCpu.reset();lastNet.reset();}
    }
}
}
