// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
namespace sw {
namespace {
const Material carbon{{.021f,.031f,.048f},0,.30f,.65f};
const Material steel{{.21f,.29f,.36f},0,.24f,.84f};
const Material ceramic{{.81f,.86f,.87f},0,.24f,.18f};
const Material copper{{.76f,.32f,.12f},0,.27f,.75f};
const Material rubber{{.009f,.014f,.021f},0,.75f,.03f,15};
const Material onyx{{.008f,.013f,.025f},0,.14f,.15f};
const Material cream{{.96f,.90f,.74f},0,.52f,.02f};
const Material iris{{.003f,.01f,.018f},0,.06f,.40f};
Material paint(V3 c){return {c,0,.26f,.42f};}
Material glow(V3 c,float e=2){return {c,e,.23f,.2f};}
void ell(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Sphere,m*translate(p)*scale(s),c);}
void plate(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Box,m*translate(p)*scale(s),c);}
void tube(Frame& f,M4 m,V3 a,V3 b,float r,Material c){add(f,Cylinder,m*segment(a,b,r),c);}
void ring(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Torus,m*translate(p)*scale(s),c);}
void badge(Frame& f,M4 m,float size,Material c){
 ring(f,m*rx(pi/2),{0,0,0},{size,.35f,size},c);
 tube(f,m,{size*.45f,-size*.55f,.02f},{size*.95f,-size*1.04f,.02f},size*.11f,c);
}
}
void fighterModel(Frame& f,M4 m,int id,double time,const Anim& a){
 const auto& pose=a.pose;
 M4 base=m;
 M4 torso=translate(pose.body)*rx(pose.torso.x)*ry(pose.torso.y)*rz(pose.torso.z)
  *scale({1+(1-pose.squash)*.32f,pose.squash,1+(1-pose.squash)*.2f});
 m=m*torso;
 float t=float(std::fmod(time,4096.0));
 const V3 accent=racerColors[id];Material coat=onyx;
 if(id==1)coat={{1.f,.62f,.12f},0,.46f,.02f};
 if(id==2||id==6)coat={{id==2?.84f:.61f,.19f,.074f},0,.60f,.02f};
 if(id==3)coat={{.29f,.33f,.39f},0,.62f,.03f};
 if(id==4)coat={{.83f,.79f,.90f},0,.55f,.02f};
 if(id==7)coat={{1.f,.56f,.70f},0,.44f,.03f};
 const Material belly=id==1?Material{{1.f,.82f,.35f},0,.5f,.01f}:cream;
 ell(f,m,{0,.30f,0},{.52f,.59f,.42f},coat);
 ell(f,m,{0,.22f,.34f},{.40f,.40f,.17f},belly);
 // The same modeled tails and species details as Prism Circuit.
 if(id==2||id==3||id==6){
  for(int k=0;k<6;k++){
   float along=k/5.f;
   float flick=std::sin(t*3.6f-along*2.5f+id)*(.12f+.20f*a.walk+.28f*a.taunt);
   V3 p{.20f+std::sin(along*2.1f)*.39f+flick*along,.13f+along*(.36f+pose.tail*.30f)-a.impact*along*.32f,-.30f-along};
   Material color=coat;if(id==3)color=k%2?onyx:coat;if(id==6)color=k%2?cream:coat;if(id==2&&k>3)color=cream;
   ell(f,m,p,{.22f*(1-along*.35f),.23f*(1-along*.3f),.22f},color);
  }
 }
 if(id==5){for(int k=0;k<7;k++){float along=k/6.f;ell(f,m,{.3f+k*.045f+along*.19f*std::sin(t*3-along*3),.10f+along*along*(.7f+pose.tail*.23f),-.38f-k*.12f},{.09f,.12f,.12f},coat);}}
 // An upright articulated fight rig. Legs and hands are not seated-kart geometry.
 V3 rightHand{};
 for(int side:{-1,1}){
  plate(f,m,{side*.30f,.30f,.42f},{.087f,.30f,.06f},carbon);
  plate(f,m,{side*.30f,.57f,.41f},{.099f,.063f,.061f},copper);
  int joint=side>0?1:0;
  V3 hip=point(torso,{side*.23f,-.10f,0}),knee=pose.knees[joint],ankle=pose.feet[joint];
  knee.y+=pose.body.y*.4f;
  tube(f,base,hip,knee,.12f,carbon);ell(f,base,knee,{.16f,.16f,.15f},steel);
  tube(f,base,knee,ankle,.11f,coat);
  M4 boot=base*translate(ankle)*rx(pose.footPitch[joint]);
  ell(f,boot,{0,-.047f,.10f},{.22f,.135f,.29f},id<2?copper:carbon);
  plate(f,boot,{0,-.13f,.07f},{.205f,.04f,.23f},steel);
  plate(f,boot,{0,.003f,.34f},{.135f,.02f,.016f},glow(accent,.7f));
  V3 shoulder{side*.48f,.50f,.01f};
  V3 hand=pose.hands[joint],elbow=pose.elbows[joint];
  tube(f,m,shoulder,elbow,.115f,coat);ell(f,m,shoulder,{.18f,.20f,.18f},steel);
  ell(f,m,elbow,{.15f,.15f,.15f},carbon);tube(f,m,elbow,hand,.105f,carbon);
  M4 palm=m*translate(hand)*rx(pose.wrist[joint]);
  ell(f,palm,{0,0,0},{.18f,.15f,.18f},carbon);
  ring(f,palm*rx(pi/2),{0,0,0},{.13f,.25f,.13f},glow(accent,.8f));
  if(pose.open[joint]>.08f){
   for(int digit=0;digit<3;++digit){float x=(digit-1)*.095f;
    tube(f,palm,{x,.02f,.10f},{x*(1+.45f*pose.open[joint]),.01f,.18f+.15f*pose.open[joint]},.038f,carbon);
   }
  }
  if(side>0)rightHand=hand;
 }
 // CyberPengu's patch hammer stays attached to the animated hand, even during recoil.
 if(id==0){
  M4 grip=m*translate(rightHand)*rx(pose.wrist[1])*rz(-.25f);
  tube(f,grip,{0,-.19f,0},{0,.55f,0},.054f,steel);
  M4 head=grip*translate({0,.55f,0});
  add(f,Box,head*scale({.42f,.24f,.23f}),paint({.17f,.30f,.34f}));
  for(int side:{-1,1}){plate(f,head,{side*.41f,0,0},{.028f,.21f,.21f},ceramic);plate(f,head,{side*.446f,0,0},{.013f,.14f,.13f},glow(accent,1.4f));}
  badge(f,head*translate({0,0,.242f}),.095f,glow(accent,1));
 } else if(id==1){
  // Gold-edged forearm shield: distinct from the spherical active guard effect.
  M4 buckler=m*translate(pose.hands[0]+V3{0,0,.19f})*rx(pi/2+pose.wrist[0]);
  add(f,Cylinder,buckler*scale({.29f,.04f,.29f}),carbon);
  ring(f,buckler,{0,-.041f,0},{.29f,.24f,.29f},glow(accent,.85f));
 } else {
  // Other fighters wear individual emitter gauntlets rather than sharing the hammer.
  ell(f,m,rightHand+V3{0,.04f,.17f},{.085f,.095f,.04f},glow(accent,1.0f+a.cast*2));
 }
 plate(f,m,{0,.24f,.46f},{.027f,.23f,.018f},steel);
 badge(f,m*translate({-.25f,.43f,.492f}),.055f,glow(accent,.9f));
 M4 head=m*translate({0,1.08f,0})*rx(pose.head.x)*ry(pose.head.y)*rz(pose.head.z);
 ell(f,head,{0,0,0},{.69f,.70f,.60f},coat);
 if(id==0){
  for(int side:{-1,1})ell(f,head,{side*.225f,-.02f,.46f},{.35f,.44f,.20f},cream);
  ell(f,head,{0,-.27f,.48f},{.42f,.24f,.18f},cream);
 }else if(id==2||id==6){
  for(int side:{-1,1}){
   ell(f,head*rz(side*.25f),{side*.28f,-.16f,.45f},{.35f,.25f,.21f},cream);
   if(id==6)ell(f,head,{side*.30f,.105f,.53f},{.20f,.245f,.096f},Material{{.32f,.08f,.034f},0,.6f,0});
  }
 }else if(id==3){
  for(int side:{-1,1})ell(f,head*rz(side*-.16f),{side*.26f,.015f,.486f},{.33f,.245f,.15f},onyx);
  ell(f,head,{0,-.24f,.52f},{.26f,.20f,.15f},cream);
 }else if(id==4){
  for(int side:{-1,1})ell(f,head,{side*.19f,-.20f,.53f},{.22f,.20f,.11f},cream);
 }else if(id==7){
  for(int side:{-1,1})ell(f,head,{side*.43f,-.13f,.49f},{.16f,.10f,.05f},Material{{1.f,.26f,.49f},0,.5f,0});
 }
 // Species-specific ears and gills are actual articulated geometry.
 for(int side:{-1,1}){
  if(id==2||id==5){
   M4 e=head*translate({side*.43f,.42f,-.05f})*rz(side*(-.18f-.27f*pose.ears)+std::sin(t*3.8f+id+side)*(.025f+.07f*a.impact))*rx(-.28f*pose.pain);
   add(f,Ear,e*scale({.72f,.51f,.86f}),coat);
   add(f,Ear,e*translate({0,.09f,.075f})*scale({.43f,.36f,.62f}),id==2?cream:paint({.26f,.12f,.28f}));
   tube(f,e,{-.16f,.27f,.18f},{-.055f,.66f,.08f},.017f,glow(accent,1));
  }else if(id==3||id==6){
   M4 ear=head*translate({side*.51f,.49f,-.035f})*rz(-side*pose.ears*.25f);
   ell(f,ear,{0,0,0},{.23f,.27f,.13f},cream);
   ell(f,ear,{0,0,.106f},{.145f,.185f,.04f},coat);
  }else if(id==4){
   M4 e=head*translate({side*.28f,.46f,-.08f})*rz(-side*(.17f+.33f*pose.ears)+(.045f+.13f*a.walk+.19f*a.impact)*std::sin(t*5.7f+side))*rx(-.35f*pose.pain+.12f*std::sin(t*4.2f+side));
   ell(f,e,{0,.44f,0},{.22f,.74f,.17f},coat);
   ell(f,e,{0,.48f,.13f},{.115f,.49f,.047f},paint({.70f,.31f,.53f}));
   tube(f,e,{0,.12f,.17f},{0,.73f,.13f},.017f,glow(accent,1));
  }else if(id==7){
   for(int k=0;k<3;k++){
    float a=(k-1)*(.48f+.22f*pose.ears)+(.05f+.10f*pose.joy+.16f*pose.shock)*std::sin(t*4.4f+k);
    V3 base{side*.54f,.10f,0},tip{side*(.89f+.08f*std::cos(a)),.12f+std::sin(a)*.68f,.08f};
    tube(f,head,base,tip,.065f,paint({.9f,.25f,.47f}));
    ell(f,head,tip,{.15f,.11f,.095f},paint({1.f,.32f,.56f}));
    ell(f,head,tip+V3{side*.06f,0,.07f},{.04f,.037f,.025f},glow(accent,1.3f));
   }
  }
 }
 float phase=std::fmod(t+id*.68f,5.8f),blink=phase<.21f?std::pow(std::sin(phase*pi/.21f),2.f):0;
 float eyeHeight=clamp(pose.eyes*(1-.35f*pose.pain)+.28f*pose.shock,.20f,1.3f);
 float gaze=clamp(a.look,-1,1)*.037f;
 if(id!=1){
  for(int side:{-1,1}){
   float wink=side<0?1-.6f*pose.smug:1;
   ell(f,head,{side*.25f,.045f,.665f},{.16f,.199f*eyeHeight*wink,.073f},iris);
   ell(f,head,{side*.25f-.043f+gaze,.045f+.067f*eyeHeight*wink,.731f},{.035f,.046f*eyeHeight*wink,.018f},cream);
   ell(f,head,{side*.25f+.024f,-.003f,.735f},{.014f,.018f,.012f},glow(accent,.45f));
   if(blink>.002f)ell(f,head,{side*.25f,.236f-blink*.19f,.747f},{.17f,.209f*blink,.022f},id==0?cream:coat);
   float tilt=pose.anger*.09f-pose.shock*.07f-pose.pain*.055f;
   float brow=.29f+.08f*pose.shock-.04f*pose.smug;
   auto browDepth=[&](float x,float y){
    float z=.60f*std::sqrt(std::max(.02f,1-x*x/(.69f*.69f)-y*y/(.70f*.70f)));
    if(id==0){float cheek=(x-side*.225f)/.35f,up=(y+.02f)/.44f;
     z=std::max(z,.46f+.20f*std::sqrt(std::max(0.f,1-cheek*cheek-up*up)));}
    return z+.016f;
   };
   tube(f,head,{side*.10f,brow-tilt,browDepth(side*.10f,brow-tilt)},
    {side*.39f,brow+tilt,browDepth(side*.39f,brow+tilt)},.028f,id==0?copper:onyx);
  }
 }else{
  for(int side:{-1,1}){
   M4 glasses=head*rz(-.08f*pose.smug+.10f*pose.shock);
   plate(f,glasses,{side*.265f,.085f+.05f*pose.shock,.553f},{.275f,.225f,.087f},copper);
   plate(f,glasses,{side*.265f,.09f+.05f*pose.shock,.631f},{.232f,.178f,.04f},Material{{.013f,.028f,.064f},0,.055f,.72f});
   plate(f,glasses*rz(-.18f),{side*.265f,.16f,.677f},{.15f,.012f,.011f},paint({.34f,.59f,.81f}));
  }
  plate(f,head,{0,.13f,.62f},{.09f,.043f,.04f},onyx);
 }
 if(id==0){
  M4 lens=head*translate({.255f,.065f,.753f})*rx(pi/2);
  ring(f,lens,{0,0,0},{.245f,.45f,.245f},steel);
  ring(f,lens,{0,-.036f,0},{.209f,.18f,.209f},glow({.04f,.92f,.62f},1.8f));
  plate(f,head,{.52f,.09f,.65f},{.12f,.055f,.04f},carbon);
 }
 if(id<2){
  float gape=pose.mouth*.16f;
  ell(f,head,{0,-.25f,.67f},id==0?V3{.21f,.13f,.24f}:V3{.34f,.125f,.29f},paint({1.f,.43f,.055f}));
  ell(f,head,{0,-.314f-gape*.45f,.72f},id==0?V3{.19f,.033f+gape*.55f,.18f}:V3{.32f,.027f+gape*.55f,.24f},paint({.25f,.05f,.025f}));
  ell(f,head,{0,-.35f-gape,.68f},id==0?V3{.16f,.055f,.19f}:V3{.27f,.052f,.23f},paint({1.f,.65f,.14f}));
 }else if(id==7){
  if(pose.mouth>.15f)ell(f,head,{0,-.29f,.587f},{.12f,.045f+.09f*pose.mouth,.025f},paint({.35f,.05f,.13f}));
  else {float smile=.025f+.04f*pose.joy;
   tube(f,head,{-.15f,-.26f,.581f},{0,-.26f-smile,.60f},.016f,paint({.45f,.09f,.18f}));
   tube(f,head,{0,-.26f-smile,.60f},{.15f,-.26f,.581f},.016f,paint({.45f,.09f,.18f}));}
 }else{
  ell(f,head,{0,-.21f,.59f},{.22f,.145f,id==2?.24f:.13f},id==5?coat:cream);
  ell(f,head,{0,-.17f,id==2?.80f:.714f},{.10f,.072f,.07f},id==4?paint({.75f,.29f,.40f}):iris);
  tube(f,head,{0,-.22f,.70f},{0,-.29f,.675f},.011f,onyx);
  float lip=id==2?.78f:.66f;
  if(pose.mouth>.16f){
   ell(f,head,{0,-.345f,lip},{.115f+.04f*pose.shock,.035f+.10f*pose.mouth,.035f},onyx);
   if(pose.joy>.35f)plate(f,head,{0,-.315f,lip+.034f},{.080f,.018f,.012f},cream);
  }else{
   float smile=.026f+.035f*pose.joy-.015f*pose.pain;
   tube(f,head,{-.13f,-.30f,lip-.02f},{0,-.30f-smile,lip},.015f,onyx);
   tube(f,head,{0,-.30f-smile,lip},{.13f,-.30f+.03f*pose.smug,lip-.02f},.015f,onyx);
  }
 }
 // Shared cyberpunk headset language, individual color-coded ear lights.
 for(int side:{-1,1}){
  ell(f,head,{side*.65f,-.03f,-.09f},{.10f,.27f,.28f},carbon);
  M4 cup=head*translate({side*.742f,-.025f,-.09f})*rz(pi/2);
  ring(f,cup,{0,0,0},{.185f,.36f,.185f},steel);
  ring(f,cup,{0,-side*.018f,0},{.14f,.15f,.14f},glow(accent,.85f));
 }
 tube(f,head,{-.73f,.13f,-.13f},{-.80f,.63f,-.15f},.018f,steel);
 ell(f,head,{-.80f,.63f,-.15f},{.037f,.045f,.037f},glow(accent,1.6f));
 if(id==0){
  for(int side:{-1,1})add(f,Ribbon,m*translate({side*.15f,.68f,-.37f})*rx(-a.walk*.28f-a.impact*.5f)*rz(.12f*std::sin(t*5+side))*ry(-pi/2)*scale({.6f,.5f,.55f}),Material{{.77f,.21f,.055f},0,.64f,.03f,11});
 }
}
}
