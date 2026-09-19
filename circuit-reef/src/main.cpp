// SPDX-License-Identifier: GPL-3.0-or-later
#include "reef/renderer.hpp"
#include "qt_display.hpp"
#include "native_renderer.hpp"
#ifdef REEF_SDL_SYSTEM_HEADERS
#include <SDL.h>
#else
#include "sdl_abi.hpp"
#endif
#include <atomic>
#include <charconv>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
namespace {
using namespace reef;
volatile std::sig_atomic_t stopping=0;
void signalHandler(int){stopping=1;}
struct Options {
 bool screenSet=false,listScreens=false;
 bool fullscreen=false,all=false,privateMode=false,demo=false,help=false,eco=false,raster=false;
 int screen=0,fps=60,width=1600,height=900,density=10,frames=0;
 double start=0,duration=24,quitAfter=0;
 std::uint64_t seed=0;bool seedSet=false;
 std::string scheme="lagoon",quality="balanced",interface,snapshot,raw,assets,capture,captureDir;
 RenderOptions render;
};
int integer(const std::string&s,int low,int high){int v{};auto [p,e]=std::from_chars(s.data(),s.data()+s.size(),v);if(e!=std::errc{}||p!=s.data()+s.size()||v<low||v>high)throw std::invalid_argument("Out-of-range integer: "+s);return v;}
double number(const std::string&s,double low,double high){std::size_t n{};double v=std::stod(s,&n);if(n!=s.size()||!std::isfinite(v)||v<low||v>high)throw std::invalid_argument("Out-of-range number: "+s);return v;}
Options options(int argc,char**argv){
 Options o;
 for(int i=1;i<argc;i++){
  std::string a=argv[i];auto take=[&](){if(++i>=argc)throw std::invalid_argument("Missing value for "+a);return std::string(argv[i]);};
  if(a=="--help"||a=="-h")o.help=true;
  else if(a=="--fullscreen"||a=="--screensaver")o.fullscreen=true;
  else if(a=="--capture-dir")o.captureDir=take();
  else if(a=="--list-screens")o.listScreens=true;
  else if(a=="--windowed")o.fullscreen=false;
  else if(a=="--all-screens"){o.all=true;o.fullscreen=true;}
  else if(a=="--screen"){o.screen=integer(take(),0,63);o.screenSet=true;}
  else if(a=="--private")o.privateMode=true;
  else if(a=="--no-metrics")o.render.metrics=false;
  else if(a=="--no-branding")o.render.branding=false;
  else if(a=="--demo")o.demo=true;
  else if(a=="--reduced-motion")o.render.reducedMotion=true;
  else if(a=="--brightness")o.render.brightness=number(take(),.05,1);
  else if(a=="--fps")o.fps=integer(take(),10,240);
  else if(a=="--eco")o.eco=true;
  else if(a=="--raster")o.raster=true;
  else if(a=="--capture-native")o.capture=take();
  else if(a=="--quality"){o.quality=take();if(o.quality!="balanced"&&o.quality!="high"&&o.quality!="native")throw std::invalid_argument("quality must be balanced, high or native");}
  else if(a=="--palette"){o.scheme=take();(void)palette(o.scheme);}
  else if(a=="--density")o.density=integer(take(),3,24);
  else if(a=="--seed"){auto s=take();auto[p,e]=std::from_chars(s.data(),s.data()+s.size(),o.seed);if(e!=std::errc{}||p!=s.data()+s.size())throw std::invalid_argument("Invalid unsigned seed");o.seedSet=true;}
  else if(a=="--interface")o.interface=take();
  else if(a=="--snapshot")o.snapshot=take();
  else if(a=="--raw-video")o.raw=take();
  else if(a=="--export-assets")o.assets=take();
  else if(a=="--start")o.start=number(take(),0,100000000);
  else if(a=="--duration")o.duration=number(take(),.01,86400);
  else if(a=="--frames")o.frames=integer(take(),1,10000000);
  else if(a=="--quit-after")o.quitAfter=number(take(),.01,86400);
  else if(a=="--size"){auto s=take();auto x=s.find('x');if(x==std::string::npos)throw std::invalid_argument("size must be WIDTHxHEIGHT");o.width=integer(s.substr(0,x),64,8192);o.height=integer(s.substr(x+1),64,8192);}
  else throw std::invalid_argument("Unknown option: "+a);
 }
 if(o.privateMode&&o.demo)throw std::invalid_argument("Choose --private or --demo, not both");
 if(o.all&&!o.capture.empty())throw std::invalid_argument("Native capture requires one display");
 if(o.all&&o.screen!=0)throw std::invalid_argument("Choose --screen or --all-screens");
 if(o.eco){o.fps=std::min(o.fps,30);o.density=std::min(o.density,7);}
 if(!o.seedSet){std::random_device r;o.seed=(std::uint64_t(r())<<32)^r();}
 return o;
}
void help(){std::cout<<R"(Circuit Reef 1.0 | Kind of Quiet for QindaQt
C++20 procedural cyberpunk aquarium. Visual screensaver, NOT a session lock.

  --windowed                 Preview window (default); Esc closes
  --fullscreen               Screensaver; input dismisses after 1 s grace
  --all-screens              Fullscreen on all displays, with hotplug
  --screensaver              Alias for --fullscreen (all monitors by default)
  --capture-dir DIR          Save a PNG from each native fullscreen output
  --list-screens              List monitor names, geometry and scale
  --screen N                 Select a display, starting at zero
  --seed N                   Reproducible ecology; random seed by default
  --palette lagoon|amethyst|ember
  --density N                3..24 large koi (default 10)
  --private                  No telemetry worker, reads, or metric panels
  --no-metrics                Hide metric panels but retain local CPU reactivity
  --interface NAME           Network readings from this single interface
  --fps N                    10..240 presentation cap (default 60)
  --eco                      30 fps cap, seven koi, 720-pixel render height cap
  --raster                   CPU Cairo fallback instead of GPU triangle rendering
  --capture-native FILE.png  Capture the first native frame before presentation
  --quality balanced|high|native  Render height cap: 900 / 1440 / output size
  --brightness 0.05..1        Overall image brightness (default 0.86)
  --reduced-motion           Slow the ecosystem to 40 percent speed
  --no-branding              Omit the startup title
  --size WIDTHxHEIGHT        Preview / export size (default 1600x900)
  --quit-after SECONDS       Exit automatically; useful for testing

Headless tools, no window or graphics server needed:
  --snapshot FILE.png        Render one frame at --start (default 0 seconds)
  --raw-video FILE|-          Write raw BGRA video, suitable for ffmpeg
  --duration SECONDS         Video duration (default 24)
  --frames N                 Override video frame count
  --start SECONDS            Start at this continuous animation time
  --export-assets DIR        Export reusable PNG, SVG, sheets and manifest
  --demo                     Explicitly use labeled synthetic telemetry

Exports omit system readings unless --demo is provided. They never sample your
machine. In a preview: F toggles fullscreen, M toggles labels, R slows motion.
No root, network connection, login hooks, or proprietary asset packs required.
)";}
struct View {
 SDL_Window* window{};SDL_Renderer* renderer{};SDL_Texture* texture{};
 std::unique_ptr<Renderer> reef;DrawList draw;std::vector<SDL_Vertex> vertices;
 std::map<std::uint64_t,SDL_Texture*> textures;
 int tw=0,th=0;bool visible=true,captured=false;
 View()=default;View(const View&)=delete;
 ~View(){for(auto [id,tex]:textures){(void)id;SDL_DestroyTexture(tex);}if(texture)SDL_DestroyTexture(texture);if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);}
};
std::unique_ptr<View> createView(const Options&o,int screen,int index){
 auto v=std::make_unique<View>();SDL_Rect bounds{};if(SDL_GetDisplayBounds(screen,&bounds))throw std::runtime_error(SDL_GetError());
 Uint32 flags=SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE;if(o.fullscreen)flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;
 int w=o.fullscreen?bounds.w:std::min(o.width,std::max(480,bounds.w-90));
 int h=o.fullscreen?bounds.h:std::min(o.height,std::max(320,bounds.h-90));
 v->window=SDL_CreateWindow("Circuit Reef | QindaQt",SDL_WINDOWPOS_CENTERED_DISPLAY(screen),SDL_WINDOWPOS_CENTERED_DISPLAY(screen),w,h,flags);
 if(!v->window)throw std::runtime_error(SDL_GetError());
 // Only the lead display requests blocking vsync. Independent per-output
 // blocking swaps would serially stall a multi-monitor presentation loop.
 v->renderer=SDL_CreateRenderer(v->window,-1,SDL_RENDERER_ACCELERATED|(index==0?SDL_RENDERER_PRESENTVSYNC:0));
 if(!v->renderer)v->renderer=SDL_CreateRenderer(v->window,-1,SDL_RENDERER_SOFTWARE);
 if(!v->renderer)throw std::runtime_error(SDL_GetError());
 SDL_SetRenderDrawColor(v->renderer,2,10,20,255);
 v->reef=std::make_unique<Renderer>(o.seed+std::uint64_t(screen)*0x9e3779b97f4a7c15ULL,o.scheme,o.density);
 return v;
}
void present(View&v,const Options&o,double t,const Metrics&m){
 int w{},h{};if(SDL_GetRendererOutputSize(v.renderer,&w,&h)||w<32||h<32)return;
 int cap=o.eco?720:(o.quality=="balanced"?900:o.quality=="high"?1440:8192);
 double ratio=std::min({1.0,double(cap)/h,8192.0/w});
 int rw=std::max(32,int(w*ratio)),rh=std::max(32,int(h*ratio));
 if(v.tw!=rw||v.th!=rh){if(v.texture)SDL_DestroyTexture(v.texture);v.texture=SDL_CreateTexture(v.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,rw,rh);if(!v.texture)throw std::runtime_error(SDL_GetError());v.tw=rw;v.th=rh;}
 if(o.raster){
  SDL_RenderSetScale(v.renderer,1,1);
  const auto& frame=v.reef->render(rw,rh,t,m,o.render);
  if(SDL_UpdateTexture(v.texture,nullptr,frame.data(),frame.stride())||SDL_RenderClear(v.renderer)||SDL_RenderCopy(v.renderer,v.texture,nullptr,nullptr))throw std::runtime_error(SDL_GetError());
 }else{
  v.reef->drawGpu(v.draw,rw,rh,t,m,o.render);
  SDL_RenderSetScale(v.renderer,float(w)/rw,float(h)/rh);SDL_SetRenderDrawBlendMode(v.renderer,SDL_BLENDMODE_BLEND);
  if(SDL_RenderClear(v.renderer))throw std::runtime_error(SDL_GetError());
  if(v.textures.size()>160){for(auto [id,tex]:v.textures){(void)id;SDL_DestroyTexture(tex);}v.textures.clear();}
  v.vertices.clear();v.vertices.reserve(v.draw.vertices.size());
  auto byte=[](double c){return Uint8(std::lround(clamp(c)*255));};
  for(const auto& vertex:v.draw.vertices)v.vertices.push_back({{float(vertex.position.x),float(vertex.position.y)},{byte(vertex.color.r),byte(vertex.color.g),byte(vertex.color.b),byte(vertex.color.a)},{float(vertex.uv.x),float(vertex.uv.y)}});
  for(const auto& batch:v.draw.batches){
   SDL_Texture* texture=nullptr;
   if(batch.texture){
    auto id=batch.texture->serial;auto it=v.textures.find(id);
    if(it!=v.textures.end())texture=it->second;
    else{
     const auto& im=*batch.texture;texture=SDL_CreateTexture(v.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,im.width,im.height);
     if(!texture)throw std::runtime_error(SDL_GetError());
     std::vector<Uint32> straight(std::size_t(im.width)*im.height);const auto* pixels=im.data();
     for(int y=0;y<im.height;y++)for(int x=0;x<im.width;x++){
      Uint32 pixel{};std::memcpy(&pixel,pixels+y*im.stride()+x*4,4);Uint32 a=pixel>>24;
      auto un=[&](int shift){auto c=(pixel>>shift)&255;return a?std::min(255u,(c*255+a/2)/a):0;};
      straight[std::size_t(y)*im.width+x]=(a<<24)|(un(16)<<16)|(un(8)<<8)|un(0);
     }
     if(SDL_UpdateTexture(texture,nullptr,straight.data(),im.width*4)||SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND)){SDL_DestroyTexture(texture);throw std::runtime_error(SDL_GetError());}
     v.textures.emplace(id,texture);
    }
   }
   if(SDL_RenderGeometry(v.renderer,texture,v.vertices.data()+batch.first,int(batch.count),nullptr,0))throw std::runtime_error(std::string(SDL_GetError())+"; try --raster for the reference backend");
  }
 }
 if(!o.capture.empty()&&!v.captured){
  Image capture(w,h);if(SDL_RenderReadPixels(v.renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,capture.data(),capture.stride()))throw std::runtime_error(SDL_GetError());
  cairo_surface_mark_dirty(capture.surface);capture.png(o.capture);v.captured=true;
 }
 SDL_RenderPresent(v.renderer);
}
int native(Options o){
 SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1");
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))throw std::runtime_error(SDL_GetError());
 struct Shutdown{~Shutdown(){SDL_ShowCursor(SDL_ENABLE);SDL_Quit();}} shutdown;
 SDL_EnableScreenSaver();if(o.fullscreen)SDL_ShowCursor(SDL_DISABLE);
 std::cerr<<"Circuit Reef seed="<<o.seed<<", driver="<<(SDL_GetCurrentVideoDriver()?SDL_GetCurrentVideoDriver():"unknown")<<", desktop blanking allowed="<<SDL_IsScreenSaverEnabled()<<"\n";
 int screens=SDL_GetNumVideoDisplays();if(screens<=0||o.screen>=screens)throw std::runtime_error("Selected display does not exist");
 std::vector<std::unique_ptr<View>> views;
 if(o.all){for(int i=0;i<screens;i++)views.push_back(createView(o,i,i));}else views.push_back(createView(o,o.screen,0));
 std::unique_ptr<Monitor> monitor;if(!o.privateMode&&!o.demo)monitor=std::make_unique<Monitor>(o.interface);
 using Clock=std::chrono::steady_clock;auto begun=Clock::now(),last=begun,next=begun;double scene=o.start;bool done=false;int dx=0,dy=0;std::uint64_t frames=0;
 while(!done&&!stopping){
  auto now=Clock::now();double age=std::chrono::duration<double>(now-begun).count();double dt=std::chrono::duration<double>(now-last).count();last=now;
  SDL_Event e{};
  while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT)done=true;
   if(e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE)done=true;
   if(e.type==SDL_DISPLAYEVENT&&age>1){std::cerr<<"Display topology changed; exiting for host-managed relaunch.\n";done=true;}
   if(e.type==SDL_WINDOWEVENT){
    for(auto& v:views)if(SDL_GetWindowID(v->window)==e.window.windowID){
     if(e.window.event==SDL_WINDOWEVENT_HIDDEN||e.window.event==SDL_WINDOWEVENT_MINIMIZED)v->visible=false;
     if(e.window.event==SDL_WINDOWEVENT_SHOWN||e.window.event==SDL_WINDOWEVENT_RESTORED)v->visible=true;
     if(e.window.event==SDL_WINDOWEVENT_CLOSE)done=true;
     if(o.fullscreen&&age>1&&e.window.event==SDL_WINDOWEVENT_FOCUS_LOST&&!o.all)done=true;
    }
   }
   if(o.fullscreen&&age>1){
    if(e.type==SDL_KEYDOWN||e.type==SDL_MOUSEBUTTONDOWN||e.type==SDL_MOUSEWHEEL||e.type==SDL_FINGERDOWN)done=true;
    if(e.type==SDL_MOUSEMOTION){dx+=e.motion.xrel;dy+=e.motion.yrel;if(std::hypot(dx,dy)>12)done=true;}
   }else if(!o.fullscreen&&e.type==SDL_KEYDOWN){
    if(e.key.keysym.sym==SDLK_m)o.render.metrics=!o.render.metrics;
    if(e.key.keysym.sym==SDLK_r)o.render.reducedMotion=!o.render.reducedMotion;
    if(e.key.keysym.sym==SDLK_f){auto*w=views.front()->window;const auto current=SDL_GetWindowFlags(w);if(SDL_SetWindowFullscreen(w,(current&1)?0:SDL_WINDOW_FULLSCREEN_DESKTOP))std::cerr<<SDL_GetError()<<'\n';}
   }
  }
  bool visible=false;for(const auto&v:views)visible|=v->visible;
  if(visible){
   // After stalls/suspend, resume gently rather than advancing across minutes.
   scene+=std::min(dt,.10)*(o.render.reducedMotion?.4:1.0);Metrics m;
   if(monitor)m=monitor->snapshot();else if(o.demo)m=demoMetrics(scene);else m.privateMode=true;
   auto drawOptions=o;drawOptions.render.reducedMotion=false;
   for(auto&v:views)if(v->visible)present(*v,drawOptions,scene,m);
   ++frames;
  }
  if(o.quitAfter>0&&age>=o.quitAfter)done=true;
  next+=std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(visible?1.0/o.fps:.1));
  if(next<Clock::now())next=Clock::now();else std::this_thread::sleep_until(next);
 }
 double elapsed=std::chrono::duration<double>(Clock::now()-begun).count();
 std::cerr<<"Presented "<<frames<<" frames in "<<elapsed<<" s ("<<(elapsed>0?frames/elapsed:0)<<" fps average).\n";
 return 0;
}
}
int main(int argc,char**argv){
 try{
  std::signal(SIGINT,signalHandler);std::signal(SIGTERM,signalHandler);std::signal(SIGPIPE,SIG_IGN);
  auto o=options(argc,argv);if(o.help){help();return 0;}
  if(!o.snapshot.empty()||!o.raw.empty()||!o.assets.empty()){
   Renderer renderer(o.seed,o.scheme,o.density);
   if(!o.assets.empty()){renderer.exportAssets(o.assets);std::cerr<<"Assets exported to "<<o.assets<<'\n';}
   auto metric=[&](double t){if(o.demo)return demoMetrics(t);Metrics m;m.privateMode=true;return m;};
   if(!o.snapshot.empty())renderer.render(o.width,o.height,o.start,metric(o.start),o.render).png(o.snapshot);
   if(!o.raw.empty()){
    std::ofstream file;std::ostream*out=&std::cout;if(o.raw!="-"){file.open(o.raw,std::ios::binary);if(!file)throw std::runtime_error("Cannot open raw output");out=&file;}
    int count=o.frames?o.frames:std::max(1,int(o.duration*o.fps));
    for(int i=0;i<count&&!stopping;i++){
     double t=o.start+i/double(o.fps);const auto& image=renderer.render(o.width,o.height,t,metric(t),o.render);
     const auto* data=image.data();for(int row=0;row<o.height;row++)out->write(reinterpret_cast<const char*>(data+row*image.stride()),o.width*4);
     if(!*out)throw std::runtime_error("Raw output stream closed or write failed");
    }
   }
   return 0;
  }
  if(o.fullscreen||o.listScreens){
   std::unique_ptr<Monitor> monitor;if(!o.privateMode&&!o.demo)monitor=std::make_unique<Monitor>(o.interface);
   Metrics readings;double scene=o.start;
   saver::Presentation presentation;presentation.title="QindaQt / Circuit Reef";presentation.screen=o.screenSet?o.screen:-1;presentation.fps=o.fps;presentation.duration=o.quitAfter;presentation.list=o.listScreens;presentation.captureDir=o.captureDir;
   int cap=o.eco?720:o.quality=="balanced"?900:o.quality=="high"?1440:8192;
   return saver::run(argc,argv,presentation,[&](QScreen* output,int index){auto seed=o.seed+std::uint64_t(index)*0x9e3779b97f4a7c15ULL;
     return std::make_unique<saver::GLWindow<NativeRenderer>>(output,[&,seed](int w,int h){return std::make_unique<NativeRenderer>(w,h,seed,o.scheme,o.density,cap);},[&](NativeRenderer& r){r.draw(scene,readings,o.render);});},
     [&](double dt){scene+=dt;if(monitor)readings=monitor->snapshot();else if(o.demo)readings=demoMetrics(scene);else readings.privateMode=true;},[]{return stopping!=0;});
  }
  return native(o);
 }catch(const std::exception&e){std::cerr<<"Circuit Reef: "<<e.what()<<"\nTry --help.\n";return 1;}
}
