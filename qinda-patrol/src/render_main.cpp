#include "renderer.hpp"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

using namespace patrol;
namespace {
double numeric(const std::string& text,double min,double max){std::size_t n=0;double value=std::stod(text,&n);if(n!=text.size()||!std::isfinite(value)||value<min||value>max)throw std::runtime_error("numeric option out of range: "+text);return value;}
std::uint64_t seedValue(const std::string& text){if(text.empty()||text[0]=='-')throw std::runtime_error("invalid seed");std::size_t n=0;auto v=std::stoull(text,&n,0);if(n!=text.size())throw std::runtime_error("invalid seed");return v;}
}
int main(int argc,char** argv){
 try{
    std::uint64_t seed=551767;double time=2,seconds=12;int fps=20;bool overlay=true,live=false,hidden=false,bench=false;
    std::string output="patrol.png",frames,assets;
    for(int i=1;i<argc;++i){std::string a=argv[i];auto value=[&](){if(i+1>=argc)throw std::runtime_error("missing value for "+a);return std::string(argv[++i]);};
        if(a=="--help"){std::cout<<"Qinda Patrol headless renderer\n  --output scene.png --time 2 --seed 551767\n  --frames directory --seconds 12 --fps 20 [--time 0]\n  --export-assets directory  (exports and exits unless --frames is also set)\n  --live  (sample this machine; otherwise clearly labeled demo data)\n  --no-metrics --no-overlay --benchmark\n";return 0;}
        else if(a=="--output")output=value();else if(a=="--time")time=numeric(value(),0,36000);else if(a=="--seconds")seconds=numeric(value(),0.05,300);
        else if(a=="--fps")fps=int(numeric(value(),1,60));else if(a=="--seed")seed=seedValue(value());else if(a=="--frames")frames=value();else if(a=="--export-assets")assets=value();
        else if(a=="--no-overlay")overlay=false;else if(a=="--live")live=true;else if(a=="--no-metrics")hidden=true;else if(a=="--benchmark")bench=true;else throw std::runtime_error("unknown option: "+a);
    }
    if(live&&!frames.empty())throw std::runtime_error("--live is for a current still; animation exports use demo or hidden metrics");
    if(live&&hidden)throw std::runtime_error("choose --live or --no-metrics, not both");
    World w(seed);Renderer renderer;
    if(!assets.empty()){if(!renderer.exportAssets(assets))throw std::runtime_error("asset export failed");std::cout<<"Assets exported to "<<assets<<"\n";if(frames.empty()&&!bench)return 0;}
    for(std::uint64_t i=0,n=std::uint64_t(std::llround(time/FixedDt));i<n;++i)w.step();
    Metrics staticMetrics;
    if(live){MetricSampler sampler;std::this_thread::sleep_for(std::chrono::milliseconds(1250));staticMetrics=sampler.snapshot();}
    auto metrics=[&](){if(hidden){Metrics m;m.mode=MetricMode::Hidden;return m;}return live?staticMetrics:demoMetrics(w.time);};
    if(bench){auto begin=std::chrono::steady_clock::now();constexpr int n=200;for(int i=0;i<n;++i){for(int j=0;j<4;++j)w.step();renderer.render(w,metrics(),overlay);}double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()/n;std::cout<<"Render + 4 fixed steps: "<<ms<<" ms/frame at 960x540 (headless, CPU)\n";return 0;}
    if(!frames.empty()){
        std::filesystem::create_directories(frames);int count=int(seconds*fps);double target=w.time;
        for(int i=0;i<count;++i){target+=1.0/fps;while(w.time+FixedDt*0.5<target)w.step();std::ostringstream name;name<<frames<<"/"<<std::setw(5)<<std::setfill('0')<<i<<".png";if(!renderer.render(w,metrics(),overlay).png(name.str()))throw std::runtime_error("frame export failed");}
        std::cout<<count<<" frames exported; "<<biomeName(w.currentBiome())<<"\n";
    }else{if(!renderer.render(w,metrics(),overlay).png(output))throw std::runtime_error("cannot write "+output);std::cout<<output<<" / "<<biomeName(w.currentBiome())<<" / seed="<<seed<<" / t="<<w.time<<" / bosses="<<w.bossesDefeated<<" / services="<<w.repairs<<" / patched="<<w.cleared<<"\n";}
    return 0;
 }catch(const std::exception& e){std::cerr<<"qinda-patrol-render: "<<e.what()<<'\n';return 2;}
}
