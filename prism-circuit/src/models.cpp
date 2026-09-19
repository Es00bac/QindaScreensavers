// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
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
void animal(Frame& f,M4 m,int id,double time,float steering){
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
 // Tail silhouettes make the species readable from rear chase views.
 if(id==2||id==3||id==6){
  for(int k=0;k<6;k++){
   float a=k/5.f;V3 p{.25f+std::sin(a*2.1f)*.44f,.13f+a*.36f,-.30f-a*1.0f};
   Material color=coat;if(id==3)color=k%2?onyx:coat;if(id==6)color=k%2?cream:coat;if(id==2&&k>3)color=cream;
   ell(f,m*ry(.09f*std::sin(t*1.5f)),p,{.22f*(1-a*.35f),.23f*(1-a*.3f),.22f},color);
  }
 }
 if(id==5){for(int k=0;k<7;k++)ell(f,m,{.3f+float(k)*.045f,.10f+std::pow(k/6.f,2.f)*.7f,-.38f-k*.12f},{.09f,.12f,.12f},coat);}
 // Racing suit and seated feet. Metallic buckles and piping remain non-emissive.
 for(int side:{-1,1}){
  plate(f,m,{side*.30f,.30f,.42f},{.087f,.30f,.06f},carbon);
  plate(f,m,{side*.30f,.57f,.41f},{.099f,.063f,.061f},copper);
  ell(f,m,{side*.25f,-.22f,.34f},{.23f,.13f,.34f},id<2?copper:carbon);
  plate(f,m,{side*.25f,-.23f,.53f},{.18f,.057f,.095f},steel);
  V3 shoulder{side*.48f,.50f,.01f};
  V3 elbow{side*.57f,.25f,.39f};
  V3 hand{side*.34f+steering*.18f,.30f+side*steering*.12f,.72f};
  tube(f,m,shoulder,elbow,.12f,coat);ell(f,m,elbow,{.15f,.15f,.15f},coat);
  tube(f,m,elbow,hand,.105f,carbon);ell(f,m,hand,{.14f,.12f,.15f},carbon);
 }
 plate(f,m,{0,.24f,.46f},{.027f,.23f,.018f},steel);
 badge(f,m*translate({-.25f,.43f,.492f}),.055f,glow(accent,.9f));
 float lean=.025f*std::sin(t*2.1f+id);
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
void kart(Frame& f,M4 m,int id,double time,float wheel,float steer,float drift,float boost){
 const V3 accent=racerColors[id];Material enamel=paint(accent*.72f+V3{.045f,.045f,.045f});
 // Wheels stay on the track while the chassis leans on its suspension.
 M4 chassis=m*translate({0,float(.018f*std::sin(time*6.5+id)),0})*rz(-drift*.16f);
 plate(f,chassis,{0,.57f,0},{1.04f,.19f,1.48f},carbon);
 plate(f,chassis,{0,.76f,.17f},{.89f,.16f,1.29f},enamel);
 add(f,Hull,chassis*translate({0,.82f,.86f})*ry(-pi/2)*scale({.30f,.68f,.81f}),ceramic);
 // Layered nose cone and contrasting center stripe.
 add(f,Hull,chassis*translate({0,.953f,.76f})*ry(-pi/2)*scale({.245f,.42f,.47f}),enamel);
 add(f,Hull,chassis*translate({0,1.005f,.81f})*ry(-pi/2)*scale({.225f,.41f,.09f}),carbon);
 plate(f,chassis,{0,.65f,1.815f},{.16f,.095f,.017f},Material{{.055f,.085f,.1f},0,.30f,.18f,16.f+id*.1f});
 for(int side:{-1,1}){
  tube(f,chassis,{side*.86f,.67f,1.67f},{side*1.10f,.58f,1.47f},.072f,steel);
  tube(f,chassis,{side*.86f,.67f,1.67f},{0,.69f,1.78f},.072f,carbon);
  plate(f,chassis,{side*.66f,.87f,1.36f},{.25f,.065f,.09f},carbon);
  plate(f,chassis,{side*.66f,.881f,1.44f},{.19f,.033f,.02f},glow({.20f,.85f,.61f},1.3f));
  plate(f,chassis,{side*.985f,.74f,.36f},{.078f,.12f,.79f},steel);
  plate(f,chassis,{side*1.019f,.75f,.45f},{.028f,.05f,.68f},glow(accent,1.0f));
  // Side pod vents, fasteners and exposed frame rails.
  for(int k=0;k<4;k++)plate(f,chassis,{side*1.059f,.82f,-.05f-k*.14f},{.018f,.072f,.034f},carbon);
  for(float z:{-.67f,.85f})ell(f,chassis,{side*1.069f,.84f,z},{.028f,.032f,.032f},copper);
  tube(f,chassis,{side*.77f,.88f,-1.04f},{side*.78f,1.40f,-.88f},.046f,steel);
  tube(f,chassis,{side*.78f,1.40f,-.88f},{side*.46f,1.43f,-.82f},.046f,steel);
  plate(f,chassis,{side*.68f,.64f,-1.56f},{.18f,.069f,.054f},glow({1,.13f,.045f},1.7f));
 }
 // Seat, shoulder surround and steering wheel.
 plate(f,chassis,{0,1.15f,-.41f},{.53f,.42f,.17f},carbon);
 plate(f,chassis,{0,1.46f,-.43f},{.40f,.16f,.17f},enamel);
 plate(f,chassis,{0,.99f,-.10f},{.49f,.095f,.44f},carbon);
 tube(f,chassis,{0,1.02f,.62f},{0,1.27f,.56f},.034f,steel);
 M4 steering=chassis*translate({0,1.27f,.56f})*rx(.70f)*ry(-steer);
 ring(f,steering,{0,0,0},{.33f,.6f,.33f},carbon);
 tube(f,steering,{-.29f,0,0},{.29f,0,0},.027f,steel);
 ell(f,steering,{0,0,0},{.089f,.045f,.089f},enamel);
 // Rear power unit, ceramic manifold, twin short boost exhausts.
 plate(f,chassis,{0,.89f,-1.19f},{.64f,.26f,.34f},steel);
 for(int k=0;k<5;k++)plate(f,chassis,{(k-2)*.19f,1.17f,-1.18f},{.055f,.06f,.30f},carbon);
 for(int side:{-1,1}){
  M4 e=chassis*translate({side*.55f,.69f,-1.43f})*rx(pi/2);
  add(f,Cylinder,e*scale({.18f,.30f,.18f}),steel);
  ring(f,e,{0,.31f,0},{.15f,.4f,.15f},glow(accent,boost>0?3:1));
  ell(f,chassis,{side*.55f,.69f,-1.79f-boost*.15f},{.095f,.095f,.16f+boost*.32f},glow(boost>0?V3{.33f,.77f,1}:accent,boost>0?4:1.4f));
 }
 // Spoiler: a material wing with a luminous edge, not a floating text billboard.
 for(int side:{-1,1})plate(f,chassis,{side*.62f,1.18f,-1.38f},{.055f,.37f,.08f},carbon);
 plate(f,chassis*rx(-.1f),{0,1.47f,-1.42f},{1.09f,.069f,.25f},enamel);
 plate(f,chassis,{0,1.51f,-1.18f},{.85f,.029f,.039f},ceramic);
 // Tire rubber uses its own procedural tread material. The sidewall has modeled rims.
 for(int side:{-1,1})for(int axle:{-1,1}){
  float z=axle>0?1.03f:-1.02f;
  M4 w=m*translate({side*1.17f,.49f,z})*ry(axle>0?steer:0)*rx(wheel);
  M4 cyl=w*rz(pi/2);
  add(f,Cylinder,cyl*scale({.48f,.245f,.48f}),rubber);
  for(float x:{-.235f,.235f}){
   M4 sidewall=w*translate({x,0,0})*rz(pi/2);
   ring(f,sidewall,{0,0,0},{.36f,.80f,.36f},carbon);
   ring(f,sidewall,{0,0,0},{.252f,.50f,.252f},steel);
   ring(f,sidewall,{0,0,0},{.293f,.28f,.293f},glow(accent,.65f));
   add(f,Cylinder,sidewall*scale({.095f,.025f,.095f}),copper);
   for(int spoke=0;spoke<5;spoke++){
    float a=spoke*2*pi/5;
    tube(f,w,{x,std::cos(a)*.10f,std::sin(a)*.10f},{x,std::cos(a+.19f)*.235f,std::sin(a+.19f)*.235f},.027f,ceramic);
   }
  }
  // Suspension arms and spring collars attach to the wheel carrier.
  tube(f,m,{side*.71f,.59f,z},{side*1.02f,.49f,z},.051f,steel);
 }
 badge(f,chassis*translate({-.38f,.665f,1.82f}),.074f,steel);
 animal(f,chassis*translate({0,.90f,-.10f})*scale({.82f,.82f,.82f}),id,time,steer);
 if(boost>.02f){
  for(int side:{-1,1})for(int j=0;j<5;j++){
   float z=-2.0f-j*.55f;
   tube(f,m,{side*.85f,.22f,z},{side*.85f,.22f,z-.32f},.025f*(1-j*.13f),glow(accent,2.4f*clamp(boost)));
  }
 }
}
void arch(Frame& f,M4 m,int sector,double time){
 V3 c=sector==0?V3{.14f,.93f,.69f}:sector==1?V3{1,.30f,.51f}:sector==2?V3{.45f,.31f,1}:V3{.20f,.67f,1};
 for(int side:{-1,1}){
  plate(f,m,{side*7.65f,2.15f,0},{.26f,2.4f,.47f},carbon);
  tube(f,m,{side*7.60f,4.2f,0},{side*5.5f,6.5f,0},.23f,steel);
  tube(f,m,{side*7.62f,.25f,-.49f},{side*7.62f,4.3f,-.49f},.063f,glow(c,2));
  for(int j=0;j<6;j++)plate(f,m,{side*7.96f,.7f+j*.57f,-.07f},{.067f,.14f,.32f},paint(c*.5f));
 }
 plate(f,m,{0,6.5f,0},{5.7f,.24f,.45f},carbon);
 tube(f,m,{-5.6f,6.70f,-.48f},{5.6f,6.70f,-.48f},.064f,glow(c,2.2f));
 badge(f,m*translate({0,6.46f,.49f}),.35f,glow(c,1.1f));
 for(int k=0;k<3;k++)ell(f,m,{(k-1)*1.5f,5.85f,0},{.16f,.16f,.16f},glow(c,1.5f+.18f*std::sin(time*.5+k)));
}
}
