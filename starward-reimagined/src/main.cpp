// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer.hpp"
#include "sdl_abi.hpp"
#include "qt_display.hpp"
#include "starward/metrics.hpp"
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
using namespace sw;
namespace {
volatile std::sig_atomic_t stopped=0;
void stop(int){stopped=1;}
double number(const std::string& s,double lo,double hi){size_t n=0;double v;try{v=std::stod(s,&n);}catch(...){throw std::runtime_error("Invalid numeric value: "+s);}if(n!=s.size()||!std::isfinite(v)||v<lo||v>hi)throw std::runtime_error("Number outside range: "+s);return v;}
int integer(const std::string& s,int lo,int hi){double v=number(s,lo,hi);if(v!=std::floor(v))throw std::runtime_error("Expected an integer: "+s);return int(v);}
struct View {
 SDL_Window* window=nullptr;SDL_GLContext context=nullptr;std::unique_ptr<Renderer> renderer;
 ~View(){if(context&&window)SDL_GL_MakeCurrent(window,context);renderer.reset();if(context)SDL_GL_DeleteContext(context);if(window)SDL_DestroyWindow(window);}
};
std::string stats(const starward::Metrics& m){std::ostringstream s;s<<"CPU ";if(m.cpu)s<<int(std::round(*m.cpu))<<'%';else s<<"--";s<<"   /   RAM ";if(m.ramUsed)s<<std::fixed<<std::setprecision(1)<<*m.ramUsed<<" GiB";else s<<"--";return s.str();}
void help(){std::cout<<R"HELP(STARWARD / REIMAGINED 2.0
C++20 real-time 3D screensaver for QindaQt. Not a security locker.

  --windowed             Preview window (default)
  --fullscreen           Fullscreen on every monitor (hotplug supported)
  --all-screens          One independent GL view on each connected display
  --capture-dir DIR       Save a PNG from each native fullscreen output
  --list-screens          List output names, geometry and scale
  --screensaver           Alias for --fullscreen
  --screen N             Select a display, zero based
  --size WxH             Preview/export size, default 1440x810
  --seed N               Reproducible scenery seed
  --chapter NAME         Hold harbor, asteroids, rescue, relic, pursuit,
                         contact, or home
  --start SECONDS        Start at an animation time
  --chapter-seconds N    Default 38; 7 chapters per voyage
  --tour                 12-second chapters, 84-second voyage
  --fps N                Frame cap, 10..240, default 60
  --eco                  30 fps; 2x MSAA; bloom reduced
  --no-msaa              Disable multisample antialiasing (default 4x)
  --reduced-motion       Slower motion, no camera shake in either mode
  --brightness N         0.1..1.0
  --no-titles            Hide the brief chapter captions
  --telemetry            Opt in to local CPU/RAM cockpit readout
  --private              Never read local telemetry (default without opt-in)
  --demo                 Explicitly labeled demonstration telemetry
  --snapshot FILE.png    Save the same OpenGL frame drawn by the app, then exit
  --frames N --raw FILE  Write N RGB24 frames at --fps (not real-time paced)
  --benchmark N         Draw N frames without delays, report throughput
  --quit-after N         Exit after N wall-clock seconds (smoke testing)
  --export-models DIR    Export modeled assets as OBJ/MTL; needs no GL context
  --help

Escape always exits. In fullscreen, activity dismisses after a one-second
launch grace period. Existing lock, idle, DPMS and suspend policy stays with
the desktop. No network requests, sound, or changes to desktop settings.
)HELP";}
}
int main(int argc,char** argv){
 try{
  Director director;RenderOptions renderOpt;renderOpt.metrics=false;
  int width=1440,height=810,screen=0,fps=60,frames=0,benchmark=0;double start=0,quit=0;
  bool screenSet=false,listScreens=false;
  bool fullscreen=false,all=false,telemetry=false,privateMode=true,demo=false,haveSeed=false;
  std::string snapshot,raw,exportDir,captureDir;
  for(int i=1;i<argc;i++){
   std::string a=argv[i];auto val=[&](){if(i+1>=argc)throw std::runtime_error("Missing value after "+a);return std::string(argv[++i]);};
   if(a=="--help"){help();return 0;}else if(a=="--windowed")fullscreen=false;
   else if(a=="--capture-dir")captureDir=val();
   else if(a=="--list-screens")listScreens=true;
   else if(a=="--fullscreen"||a=="--screensaver")fullscreen=true;else if(a=="--all-screens"){fullscreen=true;all=true;}
   else if(a=="--screen"){screen=integer(val(),0,63);screenSet=true;}else if(a=="--fps")fps=integer(val(),10,240);
   else if(a=="--size"){std::string s=val();auto p=s.find('x');if(p==std::string::npos)throw std::runtime_error("Use --size WIDTHxHEIGHT");width=integer(s.substr(0,p),320,8192);height=integer(s.substr(p+1),180,8192);}
   else if(a=="--seed"){director.seed=std::uint64_t(number(val(),0,9007199254740991.));haveSeed=true;}
   else if(a=="--chapter"){std::string s=val();int j=0;for(;j<7;j++)if(s==names[j])break;if(j==7)throw std::runtime_error("Unknown chapter: "+s);director.fixedChapter=j;}
   else if(a=="--start")start=number(val(),0,1e7);else if(a=="--chapter-seconds")director.chapterSeconds=number(val(),10,240);
   else if(a=="--tour")director.chapterSeconds=12;
   else if(a=="--eco"){fps=30;renderOpt.bloom=.16f;renderOpt.samples=2;}else if(a=="--no-msaa")renderOpt.samples=1;else if(a=="--reduced-motion")director.reduced=true;
   else if(a=="--brightness")renderOpt.brightness=number(val(),.1,1);else if(a=="--no-titles")renderOpt.titles=false;
   else if(a=="--telemetry"){telemetry=true;privateMode=false;}
   else if(a=="--private"){privateMode=true;telemetry=false;}
   else if(a=="--demo"){demo=true;privateMode=true;telemetry=true;}
   else if(a=="--snapshot")snapshot=val();else if(a=="--raw")raw=val();
   else if(a=="--frames")frames=integer(val(),1,1000000);else if(a=="--benchmark")benchmark=integer(val(),1,1000000);
   else if(a=="--quit-after")quit=number(val(),.01,3600);else if(a=="--export-models")exportDir=val();
   else throw std::runtime_error("Unknown option: "+a);
  }
  if(!haveSeed)director.seed=std::random_device{}();
  if(!exportDir.empty()){exportModels(exportDir);std::cout<<"Models exported to "<<exportDir<<'\n';return 0;}
  if((frames>0)!=(!raw.empty()))throw std::runtime_error("Use --frames and --raw together");
  bool exporting=!snapshot.empty()||frames>0;
  if(exporting){all=false;fullscreen=false;privateMode=true;}
  renderOpt.metrics=telemetry;renderOpt.demo=demo;
  std::unique_ptr<starward::Monitor> monitor;
  if(telemetry&&!privateMode&&!exporting)monitor=std::make_unique<starward::Monitor>();
  if(fullscreen||listScreens){
   std::signal(SIGINT,stop);std::signal(SIGTERM,stop);
   Frame nativeFrame;initialize(nativeFrame);compose(nativeFrame,start,director);
   double time=start;std::string readings;
   saver::Presentation presentation;presentation.title="QindaQt / starward";presentation.screen=screenSet?screen:-1;presentation.fps=fps;presentation.duration=quit;presentation.list=listScreens;presentation.captureDir=captureDir;
   return saver::run(argc,argv,presentation,[&](QScreen* output,int){return std::make_unique<saver::GLWindow<Renderer>>(output,
      [&](int w,int h){return std::make_unique<Renderer>(nativeFrame,w,h,renderOpt);},[&](Renderer& r){r.draw(nativeFrame,readings);});},
      [&](double dt){time+=dt;compose(nativeFrame,time,director);if(demo)readings=stats(starward::demoMetrics(time));else if(monitor)readings=stats(monitor->snapshot());},[]{return stopped!=0;});
  }
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))throw std::runtime_error(SDL_GetError());
  struct Quit{~Quit(){SDL_Quit();}} sdlQuit;
  SDL_EnableScreenSaver();std::signal(SIGINT,stop);std::signal(SIGTERM,stop);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
  Frame frame;initialize(frame);compose(frame,start,director);
  int displays=SDL_GetNumVideoDisplays();if(displays<1)throw std::runtime_error("No video displays");if(screen>=displays)throw std::runtime_error("Selected display is not connected");
  std::vector<std::unique_ptr<View>> views;
  int count=all?displays:1;
  for(int i=0;i<count;i++){
   int which=all?i:screen;auto v=std::make_unique<View>();
   Uint32 flags=SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE;
   if(fullscreen){flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;}
   if(exporting){flags|=SDL_WINDOW_HIDDEN;}
   v->window=SDL_CreateWindow("QindaQt / Starward Reimagined",SDL_WINDOWPOS_CENTERED_DISPLAY(which),SDL_WINDOWPOS_CENTERED_DISPLAY(which),width,height,flags);
   if(!v->window){throw std::runtime_error(SDL_GetError());}
   v->context=SDL_GL_CreateContext(v->window);
   if(!v->context){throw std::runtime_error(SDL_GetError());}
   SDL_GL_SetSwapInterval(exporting||benchmark||all?0:1);
   int w=width,h=height;SDL_GL_GetDrawableSize(v->window,&w,&h);
   v->renderer=std::make_unique<Renderer>(frame,w,h,renderOpt);views.push_back(std::move(v));
  }
  std::cerr<<"OpenGL "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<"\nSDL backend: "<<SDL_GetCurrentVideoDriver()<<" | seed "<<director.seed<<'\n';
  if(fullscreen)SDL_ShowCursor(SDL_DISABLE);
  std::ofstream rawFile;if(!raw.empty()){rawFile.open(raw,std::ios::binary);if(!rawFile)throw std::runtime_error("Cannot open raw output: "+raw);}
  using Clock=std::chrono::steady_clock;auto born=Clock::now(),previous=born;double time=start;std::uint64_t rendered=0;int movement=0;
  while(!stopped){
   auto begin=Clock::now();double elapsed=std::chrono::duration<double>(begin-born).count();
   SDL_Event ev;bool done=false;
   while(SDL_PollEvent(&ev)){
    if(ev.type==SDL_QUIT||(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE))done=true;
    if(ev.type==SDL_DISPLAYEVENT)done=true; // Relaunch through the host on topology changes.
    if(ev.type==SDL_WINDOWEVENT&&ev.window.event==SDL_WINDOWEVENT_CLOSE)done=true;
    if(fullscreen&&elapsed>1){if(ev.type==SDL_MOUSEMOTION){movement+=std::abs(ev.motion.xrel)+std::abs(ev.motion.yrel);if(movement>12)done=true;}if(ev.type==SDL_KEYDOWN||ev.type==SDL_MOUSEBUTTONDOWN||ev.type==SDL_MOUSEWHEEL||ev.type==SDL_FINGERDOWN)done=true;}
   }
   if(done||(quit>0&&elapsed>=quit))break;
   bool visible=false;for(auto& v:views)if(!(SDL_GetWindowFlags(v->window)&(SDL_WINDOW_HIDDEN|0x40)))visible=true;
   if(!visible&&!exporting){previous=begin;SDL_Delay(100);continue;}
   if(exporting||benchmark)time=start+double(rendered)/fps;else{time+=std::min(.10,std::chrono::duration<double>(begin-previous).count());previous=begin;}
   compose(frame,time,director);
   std::string readings;if(demo)readings=stats(starward::demoMetrics(time));else if(monitor)readings=stats(monitor->snapshot());
   for(auto& v:views){
    SDL_GL_MakeCurrent(v->window,v->context);int w,h;SDL_GL_GetDrawableSize(v->window,&w,&h);if(w<1||h<1)continue;
    if(w!=v->renderer->width()||h!=v->renderer->height())v->renderer->resize(w,h);
    v->renderer->draw(frame,readings);
    if(!snapshot.empty())png(snapshot,v->renderer->pixels(),w,h);
    if(rawFile.is_open()){auto rgb=v->renderer->pixels();rawFile.write(reinterpret_cast<const char*>(rgb.data()),std::streamsize(rgb.size()));if(!rawFile)throw std::runtime_error("Raw frame write failed");}
    SDL_GL_SwapWindow(v->window);
   }
   rendered++;
   if(!snapshot.empty()||(frames&&rendered>=unsigned(frames))||(benchmark&&rendered>=unsigned(benchmark)))break;
   if(!exporting&&!benchmark)std::this_thread::sleep_until(begin+std::chrono::microseconds(1000000/fps));
  }
  glFinish();double total=std::chrono::duration<double>(Clock::now()-born).count();std::cerr<<"Rendered "<<rendered<<" frames in "<<total<<" s ("<<rendered/std::max(.0001,total)<<" fps measured).\n";
  if(fullscreen){SDL_ShowCursor(SDL_ENABLE);}
  return 0;
 }catch(const std::exception& e){std::cerr<<"starward: "<<e.what()<<'\n';return 1;}
}
