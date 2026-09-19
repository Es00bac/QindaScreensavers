// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
namespace sw {
void compose(Frame& f,double time,const Director& dir){
 for(auto& b:f.batches)b.instances.clear();
 f.time=time;
 double per=dir.chapterSeconds;f.voyage=std::uint64_t(time/(per*7));
 f.chapter=dir.fixedChapter>=0?dir.fixedChapter:int(time/per)%7;
 f.local=float(std::fmod(time,per)/per);float p=f.local;
 // Black-space transitions mask scene replacement. C2 easing suppresses motion discontinuities.
 f.fade=dir.fixedChapter>=0?1:ease(0,.05f,p)*(1-ease(.95f,1,p));
 double t=time*(dir.reduced?.30:1.0);f.motionTime=t;float action=ease(.14f,.30f,p)*(1-ease(.70f,.84f,p));
 f.eye={7.5f+.5f*float(std::sin(t*.039f)),5.6f+.27f*float(std::sin(t*.031f)),23.0f};
 f.target={.2f,.4f,-1.8f};
 // Flight is continuous in time. Formations use delayed signals, rather than random destinations.
 V3 pp{-3.9f+.65f*float(std::sin(t*.18f)),-.6f+.36f*float(std::sin(t*.24f)),6.1f+.30f*float(std::cos(t*.17f))};
 V3 dp{-5.6f+.75f*float(std::sin((t-.8)*.18f)),3.20f+.31f*float(std::sin((t-.8)*.24f)),1.0f+.36f*float(std::cos((t-.8)*.17f))};
 float bank=.045f*std::cos(t*.24f),dbank=.065f*std::cos((t-.8)*.24f);
 if(f.chapter==4){pp.y+=.63f*std::sin(t*.55f);pp.z+=.45f*std::cos(t*.47f);dp.y+=.60f*std::sin((t-.8)*.55f);bank+=.12f*std::cos(t*.55f);dbank+=.12f*std::cos((t-.8)*.55f);}
 if(f.chapter==2){float e=ease(.16f,.34f,p)*(1-ease(.71f,.90f,p));dp=mix(dp,{1.9f,2.2f,-1.8f},e);}
 f.pengu=pp;f.ducke=dp;
 f.pship=translate(pp)*ry(-.11f+.045f*std::sin(t*.18f))*rz(bank)*scale({1.17f,1.17f,1.17f});
 f.dship=translate(dp)*ry(.09f+.065f*std::sin((t-.8)*.18f))*rz(dbank)*scale({.80f,.80f,.80f});
 Random rng(dir.seed+f.voyage*7919);
 if(f.chapter==0||f.chapter==6){
  station(f,translate({6.5f,1.4f,-13.0f})*rz(-.10f)*scale({1.05f,1.05f,1.05f}),t,f.chapter==6);
 }else if(f.chapter==1){
  for(int i=0;i<40;i++){V3 a{rng.range(-27,26),rng.range(-15,14),rng.range(-32,-5)};float r=rng.range(.30f,2.25f);a.x+=1.1f*std::sin(t*.04f+i);a.y+=.6f*std::cos(t*.025f+i);
   asteroid(f,translate(a)*ry(t*.02f+i)*rz(t*.011f+i*.41f)*scale({r,r,r}),i%6);
  }
  asteroid(f,translate({-12,7.4f,7})*rx(t*.02f)*scale({3.1f,3.1f,3.1f}),2);
  asteroid(f,translate({12.8f,-7.2f,5})*ry(t*.018f)*scale({4.2f,3.8f,4}),3);
  V3 target{4.4f,-.27f,.1f};float broken=ease(.56f,.69f,p);
  if(broken<.999f)add(f,Rock4,translate(target)*ry(t*.03f)*scale({1.28f*(1-broken),1.28f*(1-broken),1.28f*(1-broken)}),Material{{.20f,.16f,.12f},action*.5f,.83f,.1f,7});
  if(p>.28f&&p<.57f){V3 gun=point(f.pship,{2.39f,-.43f,.73f});beam(f,gun,target,Material{{.08f,1,.73f},6,.2f,0},.018f);
   for(int i=0;i<20;i++){float q=std::fmod(float(t)*1.4f+i*.113f,1.f);float a=i*2.399f;V3 v{std::cos(a)*q*1.8f,std::sin(a)*q*1.8f,q*.5f};add(f,Sphere,translate(target+v)*scale({.028f*(1-q),.028f*(1-q),.028f*(1-q)}),Material{{1,.39f,.065f},4,.4f,0});}
  }
  if(broken>.001f){for(int i=0;i<12;i++){float a=i*2.399f;V3 d{std::cos(a),std::sin(a),std::sin(a*.7f)};float r=.12f+(i%3)*.04f;asteroid(f,translate(target+d*(.4f+broken*2.6f))*rx(i+broken)*scale({r,r*.8f,r}),i%6);}
   V3 carry=point(f.pship,{1.25f,.79f,-.3f});float lift=ease(.69f,.87f,p);V3 cap=mix(target,carry,lift);capsule(f,translate(cap)*scale({.62f,.62f,.62f}),t);if(lift>0&&lift<1)beam(f,carry,cap,Material{{.07f,.38f,1},2,.3f,0},.026f);
  }
 }else if(f.chapter==2){
  freighter(f,translate({4.4f,.7f,-8.3f})*ry(-.10f)*rz(.08f),t,ease(.55f,.72f,p));
  V3 repair{6.9f,4.2f,-8.3f};
  if(p>.34f&&p<.70f){beam(f,point(f.dship,{2.39f,-.43f,.73f}),repair,Material{{.15f,.55f,1},4,.2f,0},.012f);
   for(int i=0;i<22;i++){float q=std::fmod(float(t)*1.5f+i*.137f,1.f);add(f,Sphere,translate(repair+V3{std::sin(i*2.4f)*q*1.2f,std::cos(i*2.4f)*q*1.2f,q})*scale({.036f*(1-q),.036f*(1-q),.036f*(1-q)}),Material{{1,.65f,.18f},4,.3f,0});}
  }
 }else if(f.chapter==3||f.chapter==4){
  gate(f,translate({6.2f,1.2f,-11.4f})*ry(-.26f)*rx(.05f),t,f.chapter==4?1:ease(.34f,.70f,p));
  for(int i=0;i<17;i++){V3 a{rng.range(-23,25),rng.range(-13,13),rng.range(-32,-18)};add(f,Monolith,translate(a)*rz(i*.32f)*rx(t*.005f+i)*scale({.18f,.48f,.18f}),Material{{.14f,.11f,.19f},0,.4f,.5f,5});}
  if(f.chapter==3&&p>.24f&&p<.67f)beam(f,point(f.pship,{2.39f,-.43f,.73f}),{6.2f,1.2f,-11.4f},Material{{.10f,.73f,1},1.7f,.3f,0},.012f);
  if(f.chapter==4){
   for(int i=0;i<5;i++){V3 a{3.9f+i*2.1f+float(std::sin(t*.33f+i)),3.4f*float(std::sin(t*.26f+i*1.1f))+1,-4.1f-i*.9f};M4 s=translate(a)*ry(-.10f)*rz(.15f*std::sin(t*.65f+i));sentinel(f,s,t);
    float shot=std::fmod(float(t)*.67f+i*.31f,1.f);if(shot<.62f){V3 aim=point(f.pship,{1.5f,.4f,1.4f});float q=ease(0,.62f,shot);V3 head=mix(a,aim,q),tail=mix(a,aim,clamp(q-.13f));beam(f,tail,head,Material{{1,.07f,.13f},5,.2f,0},.019f);
     if(q>.82f){M4 sh=translate(aim)*ry(pi/2)*rx(pi/2);add(f,Torus,sh*scale({.5f+(q-.82f)*2,.07f,.5f+(q-.82f)*2}),Material{{.05f,.7f,1},2*(1-q)/.18f,.3f,.1f});}
    }
   }
   if(p>.4f&&p<.72f)beam(f,point(f.dship,{2.39f,-.43f,.73f}),{6.2f,1.2f,-11.4f},Material{{.1f,.55f,1},2.5f,.3f,0},.017f);
  }
 }else if(f.chapter==5){
  M4 c=translate({5.5f,2.0f+float(std::sin(t*.21f))*.35f,-7.4f})*ry(-.30f)*rz(.13f)*scale({1.95f,1.95f,1.95f});creature(f,c,t);
  for(int i=0;i<7;i++){float a=i*.70f+t*.22f;creature(f,translate({7.6f+std::sin(a)*5,4.4f+std::cos(a*.6f)*2,-18.0f-i*1.2f})*ry(.2f)*scale({.20f,.20f,.20f}),t+i);}
 }
 // Small debris glints and dust convey forward travel between the larger set pieces.
 for(int i=0;i<64;++i){float z=-38+std::fmod(float(t)*(f.chapter==4?3.f:1.0f)+i*2.7f,52.f);V3 pos{rng.range(-24,25),rng.range(-12,15),z};float r=.012f+(i%3)*.009f;
  add(f,Sphere,translate(pos)*scale({r,r,r}),{{.28f,.60f,.74f},.35f,.4f,0});}
 if(f.chapter==0||f.chapter==6){
  for(int i=0;i<3;++i){float u=std::fmod(float(t)*.011f+i*.33f,1.f);V3 p{-22+u*48,7.f+i*2,-32.f-i*7};M4 shuttle=translate(p)*ry(-.2f)*scale({.20f,.20f,.20f});sentinel(f,shuttle,t);
   beam(f,p+V3{-1,0,0},p+V3{-2.8f,0,0},{{.12f,.65f,1},1.4f,.3f,0},.026f);}
 }
 ship(f,f.pship,false,t,f.chapter==4?1.4f:1);ship(f,f.dship,true,t,f.chapter==4?1.4f:1);
 if(f.chapter>=2&&f.chapter<=6){V3 pos=point(f.pship,{1.25f,.79f,-.3f});if(f.chapter==6)pos=mix(pos,{4.6f,-3.3f,-3.5f},ease(.44f,.78f,p));capsule(f,translate(pos)*scale({.62f,.62f,.62f}),t);}
 if(f.chapter==6){creature(f,translate(pp+V3{-2.7f,2.0f,-.5f})*ry(.3f)*scale({.19f,.19f,.19f}),t);}
}
}
