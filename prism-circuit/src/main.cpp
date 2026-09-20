// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer.hpp"
#include "race.hpp"
#include "sdl_abi.hpp"
#include "qt_display.hpp"
#include "sfx.hpp"
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>
using namespace sw;
namespace {
volatile std::sig_atomic_t stopped=0;
void stop(int){stopped=1;}
double number(const std::string& s,double lo,double hi){std::size_t n=0;double v;try{v=std::stod(s,&n);}catch(...){throw std::runtime_error("Invalid number: "+s);}if(n!=s.size()||!std::isfinite(v)||v<lo||v>hi)throw std::runtime_error("Number outside range: "+s);return v;}
int integer(const std::string& s,int lo,int hi){double v=number(s,lo,hi);if(v!=std::floor(v))throw std::runtime_error("Expected integer: "+s);return int(v);}
struct View {
 SDL_Window* window=nullptr;SDL_GLContext context=nullptr;std::unique_ptr<Renderer> renderer;
 ~View(){if(context&&window)SDL_GL_MakeCurrent(window,context);renderer.reset();if(context)SDL_GL_DeleteContext(context);if(window)SDL_DestroyWindow(window);}
};
void help(){std::cout<<R"HELP(PRISM CIRCUIT / AFTERGLOW GRAND PRIX 1.2
C++20 autonomous 3D racing screensaver for QindaQt. Not a security locker.

  --windowed             Preview window (default)
  --fullscreen           Fullscreen on every monitor (hotplug supported)
  --all-screens          Present the same live race on all connected outputs
  --capture-dir DIR       Save a PNG from each native fullscreen output
  --list-screens          List output names, geometry and scale
  --screensaver           Alias for --fullscreen
  --screen N             Select a display, zero based
  --size WxH             Preview/export size, default 1440x810
  --seed N               Reproduce course details and driver decisions
  --course NAME          prism, eight, aurora, bliss, city, or random
  --camera NAME          auto (default), chase, cockpit, front, orbit,
                         trackside, pack, overview
  --driver N             Follow a specific racer, 0..7
  --showcase N           Isolated 3D model turntable, 0..7
  --start SECONDS        Simulate to a specific time, 0..7200
  --fps N                Presentation cap, 10..240, default 60
  --eco                  30 fps; 2x MSAA; reduced bloom
  --no-msaa              Disable multisample antialiasing (default 4x)
  --reduced-motion       Half-speed race and a steadier, more distant camera
  --brightness N         0.1..1.0
  --mute                  Disable synthesized sound effects
  --sound                 Enable sound (default, one mixer across monitors)
  --volume N              Effects volume, 0..1 (default 0.20)
  --no-hud               Hide all broadcast graphics and map
  --private              Accepted for suite compatibility; no telemetry exists
  --snapshot FILE.png    Save a frame from the actual OpenGL renderer
  --frames N --raw FILE  Export N RGB24 frames at --fps, not realtime paced
  --benchmark N         Draw N frames unpaced; report throughput
  --quit-after N         Exit after N wall-clock seconds (smoke tests)
  --export-models DIR    Export OBJ/MTL assets without opening a display
  --race-log SECONDS    Headless simulation statistics as JSON
  --help

Drivers: 0 CyberPengu, 1 Ducké, 2 Vix (fox), 3 Cache (raccoon),
4 Mochi (rabbit), 5 Hex (cat), 6 Patches (red panda), 7 Axi (axolotl).
Escape exits immediately. Fullscreen also dismisses on activity after a
one-second startup grace period. No player controls, network or
telemetry. Existing idle, lock, DPMS and suspend policy stays with QindaQt.
Preview cameras: C cycles views, A returns to auto, V selects cockpit,
1..8 select a racer, [ / ] change racer, left-drag orbits, wheel zooms.
)HELP";}
}
int main(int argc,char** argv){
 try{
  RenderOptions renderOpt;renderOpt.metrics=false;SceneOptions options;
  int width=1440,height=810,screen=0,fps=60,frames=0,benchmark=0,course=0;
  double start=0,quit=0,logSeconds=-1;
  bool screenSet=false,listScreens=false,sound=true;float volume=.20f;
  bool fullscreen=false,all=false,haveSeed=false;
  std::uint64_t seed=41;std::string snapshot,raw,exportDir,captureDir;
  for(int i=1;i<argc;i++){
   std::string a=argv[i];auto val=[&](){if(i+1>=argc)throw std::runtime_error("Missing value after "+a);return std::string(argv[++i]);};
   if(a=="--help"){help();return 0;}
   else if(a=="--windowed"){fullscreen=false;all=false;}
   else if(a=="--capture-dir")captureDir=val();
   else if(a=="--list-screens")listScreens=true;
   else if(a=="--fullscreen"||a=="--screensaver")fullscreen=true;
   else if(a=="--all-screens"){fullscreen=true;all=true;}
   else if(a=="--screen"){screen=integer(val(),0,63);screenSet=true;}
   else if(a=="--mute")sound=false;
   else if(a=="--sound")sound=true;
   else if(a=="--volume")volume=float(number(val(),0,1));
   else if(a=="--fps")fps=integer(val(),10,240);
   else if(a=="--size"){auto s=val();auto p=s.find('x');if(p==std::string::npos)throw std::runtime_error("Use --size WIDTHxHEIGHT");width=integer(s.substr(0,p),320,8192);height=integer(s.substr(p+1),180,8192);}
   else if(a=="--seed"){seed=std::uint64_t(number(val(),0,9007199254740991.));haveSeed=true;}
   else if(a=="--course"){auto s=val();if(s=="prism")course=0;else if(s=="eight")course=1;else if(s=="aurora")course=2;else if(s=="bliss")course=3;else if(s=="city")course=4;else if(s=="random")course=-1;else throw std::runtime_error("Unknown course: "+s);}
   else if(a=="--camera"){auto s=val();if(s=="auto")options.camera=Camera::Director;else if(s=="chase")options.camera=Camera::Chase;else if(s=="front")options.camera=Camera::Front;else if(s=="orbit")options.camera=Camera::Orbit;else if(s=="overview")options.camera=Camera::Overview;else if(s=="cockpit")options.camera=Camera::Cockpit;else if(s=="trackside")options.camera=Camera::Trackside;else if(s=="pack")options.camera=Camera::Pack;else throw std::runtime_error("Unknown camera: "+s);}
   else if(a=="--driver")options.focus=integer(val(),0,7);
   else if(a=="--showcase"){options.gallery=true;options.galleryId=integer(val(),0,7);}
   else if(a=="--start")start=number(val(),0,7200);
   else if(a=="--eco"){fps=30;renderOpt.bloom=.16f;renderOpt.samples=2;}
   else if(a=="--no-msaa")renderOpt.samples=1;
   else if(a=="--reduced-motion")options.reduced=true;
   else if(a=="--brightness")renderOpt.brightness=number(val(),.1,1);
   else if(a=="--no-hud")renderOpt.titles=false;
   else if(a=="--private"){} // There is intentionally no telemetry collector.
   else if(a=="--snapshot")snapshot=val();
   else if(a=="--raw")raw=val();
   else if(a=="--frames")frames=integer(val(),1,1000000);
   else if(a=="--benchmark")benchmark=integer(val(),1,1000000);
   else if(a=="--quit-after")quit=number(val(),.01,3600);
   else if(a=="--export-models")exportDir=val();
   else if(a=="--race-log")logSeconds=number(val(),0,86400);
   else throw std::runtime_error("Unknown option: "+a);
  }
  if(!haveSeed)seed=std::random_device{}();
  if(course<0)course=int(seed%CourseCount);
  if((frames>0)!=(!raw.empty()))throw std::runtime_error("Use --frames and --raw together");
  if(!exportDir.empty()){exportModels(exportDir,seed,course);std::cout<<"Models exported to "<<exportDir<<'\n';return 0;}
  Race race(seed,course);
  if(logSeconds>=0){
   race.advance(logSeconds);const auto& c=race.counters;
   std::cout<<"{\"seed\":"<<seed<<",\"seconds\":"<<logSeconds<<",\"course_length_m\":"<<race.track.length<<",\"ticks\":"<<c.ticks<<",\"overtakes\":"<<c.passes<<",\"boosts\":"<<c.boosts<<",\"rounds\":"<<c.rounds<<",\"soft_contacts\":"<<c.contacts<<",\"pickups\":"<<c.pickups<<",\"items_used\":"<<c.itemsUsed<<",\"shield_blocks\":"<<c.blocks<<",\"hits\":"<<c.hits<<",\"spinouts\":"<<c.spinouts<<",\"knockouts\":"<<c.knockouts<<",\"rescues\":"<<c.rescues<<",\"projectiles_fired\":"<<c.shots<<",\"traps_hit\":"<<c.trapsHit<<",\"camera_cuts\":"<<c.cameraCuts<<"}\n";return 0;
  }
  bool exporting=!snapshot.empty()||frames>0;if(exporting){all=false;fullscreen=false;}
  if(fullscreen||listScreens){
   saver::Sound audio(sound&&!listScreens,volume);
   std::signal(SIGINT,stop);std::signal(SIGTERM,stop);
   Frame nativeFrame;initialize(nativeFrame);buildTrackMeshes(nativeFrame,race.track);race.seek(start);compose(nativeFrame,race.sample(),race.track,options);
   double time=start;std::string readings;
   saver::Presentation presentation;presentation.title="QindaQt / prism-circuit";presentation.screen=screenSet?screen:-1;presentation.fps=fps;presentation.duration=quit;presentation.list=listScreens;presentation.captureDir=captureDir;
   return saver::run(argc,argv,presentation,[&](QScreen* output,int){return std::make_unique<saver::GLWindow<Renderer>>(output,
      [&](int w,int h){return std::make_unique<Renderer>(nativeFrame,w,h,renderOpt);},[&](Renderer& r){r.draw(nativeFrame);});},
      [&](double dt){time+=dt*(options.reduced?.5:1);race.seek(time);compose(nativeFrame,race.sample(),race.track,options);audio.play(race.state().sounds);},[]{return stopped!=0;});
  }
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER,"1");
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))throw std::runtime_error(SDL_GetError());
  struct Quit{~Quit(){SDL_Quit();}} sdlQuit;
  saver::Sound audio(sound&&!exporting&&!benchmark,volume);
  SDL_EnableScreenSaver();std::signal(SIGINT,stop);std::signal(SIGTERM,stop);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
  Frame frame;initialize(frame);buildTrackMeshes(frame,race.track);race.seek(start);compose(frame,race.sample(),race.track,options);
  int displays=SDL_GetNumVideoDisplays();if(displays<1)throw std::runtime_error("No video displays");if(screen>=displays)throw std::runtime_error("Selected display is not connected");
  std::vector<std::unique_ptr<View>> views;
  for(int i=0;i<(all?displays:1);i++){
   int which=all?i:screen;auto v=std::make_unique<View>();
   Uint32 flags=SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE;
   if(fullscreen)flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;
   if(exporting)flags|=SDL_WINDOW_HIDDEN;
   v->window=SDL_CreateWindow("QindaQt / Prism Circuit",SDL_WINDOWPOS_CENTERED_DISPLAY(which),SDL_WINDOWPOS_CENTERED_DISPLAY(which),width,height,flags);
   if(!v->window)throw std::runtime_error(SDL_GetError());
   v->context=SDL_GL_CreateContext(v->window);
   if(!v->context)throw std::runtime_error(SDL_GetError());
   SDL_GL_SetSwapInterval(exporting||benchmark||all?0:1);
   int w=width,h=height;SDL_GL_GetDrawableSize(v->window,&w,&h);v->renderer=std::make_unique<Renderer>(frame,w,h,renderOpt);views.push_back(std::move(v));
  }
  std::cerr<<"OpenGL "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<"\nSDL backend: "<<SDL_GetCurrentVideoDriver()<<" | seed "<<seed<<" | "<<race.track.name()<<'\n';
  if(fullscreen)SDL_ShowCursor(SDL_DISABLE);
  std::ofstream rawFile;if(!raw.empty()){rawFile.open(raw,std::ios::binary);if(!rawFile)throw std::runtime_error("Cannot open raw output: "+raw);}
  using Clock=std::chrono::steady_clock;auto born=Clock::now(),previous=born;double time=start;std::uint64_t rendered=0;int movement=0;
  while(!stopped){
   auto begin=Clock::now();double elapsed=std::chrono::duration<double>(begin-born).count();SDL_Event ev;bool done=false;
   while(SDL_PollEvent(&ev)){
    if(ev.type==SDL_QUIT||(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE))done=true;
    if(ev.type==SDL_DISPLAYEVENT)done=true;
    if(ev.type==SDL_WINDOWEVENT&&ev.window.event==SDL_WINDOWEVENT_CLOSE)done=true;
    if(!fullscreen&&!exporting){
     if(ev.type==SDL_KEYDOWN&&!ev.key.repeat){int key=ev.key.keysym.sym;
      if(key=='c')options.camera=Camera((int(options.camera)+1)%8);
      if(key=='a'){options.camera=Camera::Director;options.focus=-1;options.zoom=1;}
      if(key=='v')options.camera=Camera::Cockpit;
      if(key>='1'&&key<='8')options.focus=key-'1';
      if(key==']')options.focus=(frame.focus+1)%8;
      if(key=='[')options.focus=(frame.focus+7)%8;
     }
     if(ev.type==SDL_MOUSEMOTION&&(ev.motion.state&1)){
      options.camera=Camera::Orbit;options.orbitYaw-=ev.motion.xrel*.006f;options.orbitPitch=clamp(options.orbitPitch+ev.motion.yrel*.004f,-.4f,1.2f);
     }
     if(ev.type==SDL_MOUSEWHEEL)options.zoom=clamp(options.zoom*std::exp(-ev.wheel.y*.10f),.55f,2.6f);
    }
    if(fullscreen&&elapsed>1){
     if(ev.type==SDL_MOUSEMOTION){movement+=std::abs(ev.motion.xrel)+std::abs(ev.motion.yrel);if(movement>12)done=true;}
     if(ev.type==SDL_KEYDOWN||ev.type==SDL_MOUSEBUTTONDOWN||ev.type==SDL_MOUSEWHEEL||ev.type==SDL_FINGERDOWN)done=true;
    }
   }
   if(done||(quit>0&&elapsed>=quit))break;
   bool visible=false;for(auto& v:views)if(!(SDL_GetWindowFlags(v->window)&(SDL_WINDOW_HIDDEN|0x40)))visible=true;
   if(!visible&&!exporting){previous=begin;SDL_Delay(100);continue;}
   double rate=options.reduced?.5:1;
   if(exporting||benchmark)time=start+double(rendered)/fps*rate;
   else{time+=std::min(.1,std::chrono::duration<double>(begin-previous).count())*rate;previous=begin;}
   race.seek(time);compose(frame,race.sample(),race.track,options);audio.play(race.state().sounds);
   for(auto& v:views){
    SDL_GL_MakeCurrent(v->window,v->context);int w,h;SDL_GL_GetDrawableSize(v->window,&w,&h);if(w<1||h<1)continue;
    if(w!=v->renderer->width()||h!=v->renderer->height())v->renderer->resize(w,h);
    v->renderer->draw(frame);
    if(!snapshot.empty())png(snapshot,v->renderer->pixels(),w,h);
    if(rawFile.is_open()){auto rgb=v->renderer->pixels();rawFile.write(reinterpret_cast<const char*>(rgb.data()),std::streamsize(rgb.size()));if(!rawFile)throw std::runtime_error("Frame output failed");}
    SDL_GL_SwapWindow(v->window);
   }
   rendered++;
   if(!snapshot.empty()||(frames&&rendered>=std::uint64_t(frames))||(benchmark&&rendered>=std::uint64_t(benchmark)))break;
   if(!exporting&&!benchmark){auto deadline=begin+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0/fps));std::this_thread::sleep_until(deadline);}
  }
  double took=std::chrono::duration<double>(Clock::now()-born).count();
  std::cerr<<"Rendered "<<rendered<<" frames in "<<std::fixed<<std::setprecision(3)<<took<<" s";if(benchmark)std::cerr<<" ("<<rendered/std::max(.001,took)<<" fps, this machine only)";std::cerr<<'\n';
  if(fullscreen)SDL_ShowCursor(SDL_ENABLE);
  return 0;
 }catch(const std::exception& e){std::cerr<<"prism-circuit: "<<e.what()<<'\n';return 1;}
}
