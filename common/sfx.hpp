// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "sound_events.hpp"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>
namespace saver {
// Original, synthesized stereo effects. No asset downloads and no audio thread
// allocations. One mixer belongs to the process, so multiple screens stay quiet.
class Sound {
 friend struct SoundTest;
 struct Voice {Cue cue=Cue::Hit;int id=0;float pan=0,age=99,life=.2f,phase=0;std::uint32_t noise=1;};
 std::array<Voice,24> voices_{};std::array<SoundEvent,128> queue_{};
 std::atomic<unsigned> write_{0},read_{0};SDL_AudioDeviceID device_=0;float volume_=.2f;std::uint64_t consumed_=0;bool initialized_=false;
 static void callback(void* data,Uint8* stream,int length){static_cast<Sound*>(data)->mix(reinterpret_cast<float*>(stream),length/(2*sizeof(float)));}
 void mix(float* out,int frames){
  unsigned r=read_.load(std::memory_order_relaxed),w=write_.load(std::memory_order_acquire);
  while(r!=w){const auto e=queue_[r%queue_.size()];auto* v=&*std::max_element(voices_.begin(),voices_.end(),[](const Voice& a,const Voice& b){return a.age/a.life<b.age/b.life;});
   *v={e.cue,e.character,std::clamp(e.pan,-1.f,1.f),0,e.cue==Cue::Knockout?.65f:e.cue==Cue::Special?.38f:e.cue==Cue::Pickup?.46f:.18f,0,std::uint32_t(e.serial+1)};++r;}
  read_.store(r,std::memory_order_release);
  constexpr float dt=1.f/48000,pi=3.14159265359f;
  for(int i=0;i<frames;++i){float l=0,rch=0;
   for(auto& v:voices_)if(v.age<v.life){float u=v.age/v.life,freq=180,noiseMix=0,amp=1-u;
    switch(v.cue){
     case Cue::Hit:freq=130*(1-u)+45;noiseMix=.45f;amp*=amp;break;
     case Cue::Shield:freq=900+380*std::sin(u*7);amp*=.4f;break;
     case Cue::Special:freq=(190+v.id*71)*(v.id%2?1+u*2:2-u*1.6f);noiseMix=.06f*(v.id%4);break;
     case Cue::Jump:freq=250+u*650;amp*=.38f;break;
     case Cue::Pickup:freq=440*std::pow(2.f,std::floor(u*4)*.25f);amp*=.48f;break;
     case Cue::Knockout:freq=260*(1-u)+36;noiseMix=.38f;amp*=.8f;break;
     case Cue::Boost:freq=140+u*270;noiseMix=.34f;amp*=.55f;break;
     case Cue::Start:freq=660; amp*=.38f;break;
    }
    v.phase+=freq*dt*2*pi;if(v.phase>2*pi)v.phase-=2*pi;
    v.noise=v.noise*1664525u+1013904223u;float noise=float(v.noise>>8)*(2.f/16777216)-1;
    float wave=std::sin(v.phase)+.21f*std::sin(v.phase*2)+.08f*std::sin(v.phase*3);
    float envelope=std::min(1.f,v.age/.006f)*amp;
    float x=(wave*(1-noiseMix)+noise*noiseMix)*envelope*volume_*.28f;
    l+=x*std::sqrt((1-v.pan)*.5f);rch+=x*std::sqrt((1+v.pan)*.5f);v.age+=dt;
   }
   out[i*2]=std::tanh(l);out[i*2+1]=std::tanh(rch);
  }
 }
public:
 explicit Sound(bool enabled=true,float volume=.2f):volume_(volume){
  if(!enabled)return;
  if(SDL_InitSubSystem(SDL_INIT_AUDIO)!=0){std::cerr<<"SFX unavailable: "<<SDL_GetError()<<"\n";return;}initialized_=true;
  SDL_AudioSpec want{};want.freq=48000;want.format=AUDIO_F32SYS;want.channels=2;want.samples=1024;want.callback=callback;want.userdata=this;
  device_=SDL_OpenAudioDevice(nullptr,0,&want,nullptr,0);if(device_)SDL_PauseAudioDevice(device_,0);else std::cerr<<"SFX unavailable: "<<SDL_GetError()<<"\n";
 }
 ~Sound(){if(device_)SDL_CloseAudioDevice(device_);if(initialized_)SDL_QuitSubSystem(SDL_INIT_AUDIO);}
 template<class Events> void play(const Events& events){for(const auto& e:events)if(e.serial>consumed_){consumed_=e.serial;if(!device_)continue;unsigned w=write_.load(std::memory_order_relaxed),r=read_.load(std::memory_order_acquire);if(w-r<queue_.size()){queue_[w%queue_.size()]=e;write_.store(w+1,std::memory_order_release);}}}
 bool available()const{return device_!=0;}
};
}
