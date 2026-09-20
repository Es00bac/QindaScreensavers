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
 float steering=-a.recoil*.2f+a.cast*.1f;
 // Readable anticipation, weight transfer and species-specific casting poses.
 float squash=1-a.duck*.18f-a.land*.25f;
 m=m*scale({1+(1-squash)*.35f,squash,1+(1-squash)*.2f});
 if(id==2)m=m*rx(a.cast*.28f);
 if(id==4)m=m*rz(a.cast*-.20f);
 if(id==6)m=m*rx(-a.cast*.20f);
 if(id==7)m=m*translate({0,a.cast*.10f,0});
 m=m*translate({0,-a.duck*.32f-a.land+std::sin(a.gait*2)*a.walk*.025f,0})*rx(a.recoil*-.23f+a.heavy*.11f);
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
   float a=k/5.f;V3 p{.25f+std::sin(a*2.1f)*.44f,.13f+a*.36f,-.30f-a*1.0f};
   Material color=coat;if(id==3)color=k%2?onyx:coat;if(id==6)color=k%2?cream:coat;if(id==2&&k>3)color=cream;
   ell(f,m*ry(.09f*std::sin(t*1.5f)),p,{.22f*(1-a*.35f),.23f*(1-a*.3f),.22f},color);
  }
 }
 if(id==5){for(int k=0;k<7;k++)ell(f,m,{.3f+float(k)*.045f,.10f+std::pow(k/6.f,2.f)*.7f,-.38f-k*.12f},{.09f,.12f,.12f},coat);}
 // An upright articulated fight rig. Legs and hands are not seated-kart geometry.
 V3 rightHand{};
 for(int side:{-1,1}){
  plate(f,m,{side*.30f,.30f,.42f},{.087f,.30f,.06f},carbon);
  plate(f,m,{side*.30f,.57f,.41f},{.099f,.063f,.061f},copper);
  float phase=a.gait+(side<0?pi:0);
  float stride=std::cos(phase)*.34f*a.walk;
  float lift=std::max(0.f,std::sin(phase))*.15f*a.walk;
  V3 hip{side*.23f,-.10f,0},knee{side*.26f,-.35f+lift*.5f+a.air*.07f,.06f+stride*.55f};
  V3 ankle{side*.28f,-.54f+lift+a.air*.12f,.08f+stride+a.aerial*.42f*side};
  tube(f,m,hip,knee,.12f,carbon);ell(f,m,knee,{.16f,.16f,.15f},steel);
  tube(f,m,knee,ankle,.11f,coat);
  ell(f,m,ankle+V3{0,-.047f,.10f},{.22f,.135f,.29f},id<2?copper:carbon);
  plate(f,m,ankle+V3{0,-.13f,.07f},{.205f,.04f,.23f},steel);
  plate(f,m,ankle+V3{0,.003f,.34f},{.135f,.02f,.016f},glow(accent,.7f));
  V3 shoulder{side*.48f,.50f,.01f};
  float punch=side>0?a.punch:0;
  V3 hand{side*(.49f+a.aerial*.25f)-side*a.shield*.09f,
          .24f+.42f*a.shield+.18f*punch+.22f*a.heavy+.45f*a.cast+.68f*a.recover-.14f*a.recoil,
          .39f+1.08f*punch+1.12f*a.heavy+.43f*a.cast+.7f*a.grab-.12f*a.recoil};
  hand.y+=std::cos(phase)*a.walk*.085f;
  if(id==0)hand.y+=a.cast*.6f;
  if(id==3){hand.x+=side*a.cast*.36f;hand.z+=a.cast*.24f;}
  if(id==4&&side>0)hand.y+=a.cast*.85f;
  if(id==6){hand.y+=a.cast*.70f;hand.z-=a.cast*.20f;}
  if(id==7){hand.x+=side*a.cast*.22f;hand.z+=a.cast*.2f;}
  V3 elbow{side*(.62f+.05f*a.heavy),.19f+.18f*a.shield+.28f*a.cast,
           .23f+.47f*punch+.45f*a.heavy};
  tube(f,m,shoulder,elbow,.115f,coat);ell(f,m,shoulder,{.18f,.20f,.18f},steel);
  ell(f,m,elbow,{.15f,.15f,.15f},carbon);tube(f,m,elbow,hand,.105f,carbon);
  ell(f,m,hand,{.18f,.15f,.18f},carbon);
  ring(f,m*translate(hand)*rx(pi/2),{0,0,0},{.13f,.25f,.13f},glow(accent,.8f));
  if(side>0)rightHand=hand;
 }
 // CyberPengu's patch hammer stays attached to the animated hand, even during recoil.
 if(id==0){
  V3 end=rightHand+V3{.32f*(1-a.heavy),.45f-.25f*a.heavy,.04f+.33f*a.heavy};
  tube(f,m,rightHand-V3{0,.18f,0},end,.054f,steel);
  M4 head=m*translate(end)*rx(a.heavy*.7f);
  add(f,Box,head*scale({.42f,.24f,.23f}),paint({.17f,.30f,.34f}));
  for(int side:{-1,1}){plate(f,head,{side*.41f,0,0},{.028f,.21f,.21f},ceramic);plate(f,head,{side*.446f,0,0},{.013f,.14f,.13f},glow(accent,1.4f));}
  badge(f,head*translate({0,0,.242f}),.095f,glow(accent,1));
 } else if(id==1){
  // Gold-edged forearm shield: distinct from the spherical active guard effect.
  M4 buckler=m*translate({-.50f,.32f+.43f*a.shield,.67f+.1f*a.shield})*rx(pi/2);
  add(f,Cylinder,buckler*scale({.29f,.04f,.29f}),carbon);
  ring(f,buckler,{0,-.041f,0},{.29f,.24f,.29f},glow(accent,.85f));
 } else {
  // Other fighters wear individual emitter gauntlets rather than sharing the hammer.
  ell(f,m,rightHand+V3{0,.04f,.17f},{.085f,.095f,.04f},glow(accent,1.0f+a.cast*2));
 }
 plate(f,m,{0,.24f,.46f},{.027f,.23f,.018f},steel);
 badge(f,m*translate({-.25f,.43f,.492f}),.055f,glow(accent,.9f));
 float lean=.025f*std::sin(t*2.1f+id)-a.recoil*.11f+a.heavy*.065f;
 M4 head=m*translate({0,1.08f+.012f*std::sin(t*1.8f+id),0})*ry(steering*.45f+.035f*std::sin(t*.4f+id))*rz(lean);
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
   M4 e=head*translate({side*.43f,.42f,-.05f})*rz(side*-.18f+std::sin(t*1.8f+id)*.025f);
   add(f,Ear,e*scale({.72f,.51f,.86f}),coat);
   add(f,Ear,e*translate({0,.09f,.075f})*scale({.43f,.36f,.62f}),id==2?cream:paint({.26f,.12f,.28f}));
   tube(f,e,{-.16f,.27f,.18f},{-.055f,.66f,.08f},.017f,glow(accent,1));
  }else if(id==3||id==6){
   ell(f,head,{side*.51f,.49f,-.035f},{.23f,.27f,.13f},cream);
   ell(f,head,{side*.51f,.49f,.071f},{.145f,.185f,.04f},coat);
  }else if(id==4){
   M4 e=head*translate({side*.28f,.46f,-.08f})*rz(-side*.17f+.045f*std::sin(t*1.7f+side));
   ell(f,e,{0,.44f,0},{.22f,.74f,.17f},coat);
   ell(f,e,{0,.48f,.13f},{.115f,.49f,.047f},paint({.70f,.31f,.53f}));
   tube(f,e,{0,.12f,.17f},{0,.73f,.13f},.017f,glow(accent,1));
  }else if(id==7){
   for(int k=0;k<3;k++){
    float a=(k-1)*.48f+.05f*std::sin(t*1.4f+k);
    V3 base{side*.54f,.10f,0},tip{side*(.89f+.08f*std::cos(a)),.12f+std::sin(a)*.68f,.08f};
    tube(f,head,base,tip,.065f,paint({.9f,.25f,.47f}));
    ell(f,head,tip,{.15f,.11f,.095f},paint({1.f,.32f,.56f}));
    ell(f,head,tip+V3{side*.06f,0,.07f},{.04f,.037f,.025f},glow(accent,1.3f));
   }
  }
 }
 float phase=std::fmod(t+id*.68f,5.8f),blink=phase<.21f?std::pow(std::sin(phase*pi/.21f),2.f):0;
 if(id!=1){
  for(int side:{-1,1}){
   ell(f,head,{side*.25f,.045f,.665f},{.16f,.199f,.073f},iris);
   ell(f,head,{side*.25f-.043f,.112f,.731f},{.035f,.046f,.018f},cream);
   ell(f,head,{side*.25f+.024f,-.003f,.735f},{.014f,.018f,.012f},glow(accent,.45f));
   if(blink>.002f)ell(f,head,{side*.25f,.236f-blink*.19f,.747f},{.17f,.209f*blink,.022f},id==0?cream:coat);
  }
 }else{
  for(int side:{-1,1}){
   plate(f,head,{side*.265f,.085f,.553f},{.275f,.225f,.087f},copper);
   plate(f,head,{side*.265f,.09f,.631f},{.232f,.178f,.04f},Material{{.013f,.028f,.064f},0,.055f,.72f});
   plate(f,head*rz(-.18f),{side*.265f,.16f,.677f},{.15f,.012f,.011f},paint({.34f,.59f,.81f}));
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
  ell(f,head,{0,-.25f,.67f},id==0?V3{.21f,.13f,.24f}:V3{.34f,.125f,.29f},paint({1.f,.43f,.055f}));
  ell(f,head,{0,-.314f,.72f},id==0?V3{.19f,.033f,.18f}:V3{.32f,.027f,.24f},paint({.54f,.17f,.035f}));
  ell(f,head,{0,-.35f,.68f},id==0?V3{.16f,.055f,.19f}:V3{.27f,.052f,.23f},paint({1.f,.65f,.14f}));
 }else if(id==7){
  tube(f,head,{-.12f,-.26f,.581f},{0,-.285f,.59f},.014f,paint({.45f,.09f,.18f}));
  tube(f,head,{0,-.285f,.59f},{.12f,-.26f,.581f},.014f,paint({.45f,.09f,.18f}));
 }else{
  ell(f,head,{0,-.21f,.59f},{.22f,.145f,id==2?.24f:.13f},id==5?coat:cream);
  ell(f,head,{0,-.17f,id==2?.80f:.714f},{.10f,.072f,.07f},id==4?paint({.75f,.29f,.40f}):iris);
  tube(f,head,{0,-.22f,.70f},{0,-.29f,.675f},.011f,onyx);
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
  for(int side:{-1,1})add(f,Ribbon,m*translate({side*.15f,.68f,-.37f})*ry(-pi/2)*scale({.6f,.5f,.55f}),Material{{.77f,.21f,.055f},0,.64f,.03f,11});
 }
}
}
