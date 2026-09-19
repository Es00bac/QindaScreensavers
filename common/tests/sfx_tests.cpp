// SPDX-License-Identifier: GPL-3.0-or-later
#include "sfx.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace saver {struct SoundTest {
 static std::vector<float> render(Cue cue,int id,float pan=0){Sound sound(false);sound.queue_[0]={1,0,cue,id,pan};sound.write_.store(1);std::vector<float> samples(48000);sound.mix(samples.data(),24000);return samples;}
};}
int main(){try{
 std::vector<std::vector<float>> specials;
 for(int id=0;id<8;++id){auto samples=saver::SoundTest::render(saver::Cue::Special,id);double energy=0;for(float x:samples){if(!std::isfinite(x)||std::abs(x)>1)throw std::runtime_error("Invalid PCM");energy+=x*x;}if(energy<.1)throw std::runtime_error("Silent effect");specials.push_back(std::move(samples));}
 for(int a=0;a<8;++a)for(int b=a+1;b<8;++b)if(specials[a]==specials[b])throw std::runtime_error("Duplicate special timbres");
 for(auto cue:{saver::Cue::Hit,saver::Cue::Shield,saver::Cue::Pickup,saver::Cue::Knockout,saver::Cue::Boost}){auto data=saver::SoundTest::render(cue,0,-1);double energy=0;for(std::size_t i=0;i<data.size();i+=2){energy+=data[i]*data[i];if(data[i+1]!=0)throw std::runtime_error("Stereo pan failed");}if(energy<.01)throw std::runtime_error("Silent cue");}
 saver::Sound device(true);if(!device.available())throw std::runtime_error("SDL dummy audio device failed");
 std::cout<<"PASS: finite audible PCM, eight distinct specials, stereo pan and SDL audio device\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
