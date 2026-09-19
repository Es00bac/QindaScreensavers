// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
namespace sw {
namespace {
const Material ink{{.027f,.046f,.066f},0,.26f,.68f},steel{{.18f,.25f,.30f},0,.32f,.78f},ivory{{.68f,.76f,.73f},0,.26f,.36f},copper{{.72f,.29f,.09f},0,.26f,.75f},gold{{.92f,.58f,.12f},0,.28f,.4f},white{{.95f,.92f,.77f},0,.46f,0},black{{.012f,.019f,.028f},0,.18f,.14f},eye{{.004f,.009f,.016f},0,.065f,.45f},mint{{.06f,.92f,.70f},3.5f,.23f,.25f},blue{{.05f,.44f,1.f},4,.18f,.2f},amber{{1.f,.37f,.06f},3,.3f,.1f},purple{{.42f,.20f,1.f},2.5f,.3f,.2f};
void ell(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Sphere,m*translate(p)*scale(s),c);}
void box(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Box,m*translate(p)*scale(s),c);}
void ring(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Torus,m*translate(p)*scale(s),c);}
void rod(Frame& f,M4 m,V3 a,V3 b,float r,Material c){add(f,Cylinder,m*segment(a,b,r),c);}
void pilot(Frame& f,M4 m,bool duck,double t){
 float breath=.015f*std::sin(t*1.7);Material coat=duck?gold:black;
 ell(f,m,{0,.36f,0},{.55f,.65f,.46f},coat);
 ell(f,m,{0,.27f,.38f},{.42f,.46f,.15f},duck?Material{{.95f,.65f,.17f},0,.4f,0}:white);
 // Padded flight vest with seams, zip, copper shoulder harness and glowing insignia.
 for(int s:{-1,1}){box(f,m,{s*.29f,.28f,.46f},{.105f,.36f,.07f},ink);box(f,m,{s*.30f,.57f,.43f},{.12f,.09f,.085f},copper);
 ell(f,m*rz(s*.33f),{s*.58f,.20f,.16f},{.18f,.45f,.20f},coat);ell(f,m,{s*.58f,-.08f,.35f},{.19f,.15f,.20f},ink);
 ell(f,m,{s*.24f,-.27f,.28f},{.24f,.13f,.37f},copper);box(f,m,{s*.24f,-.29f,.51f},{.18f,.07f,.12f},ink);
 }
 box(f,m,{0,.25f,.55f},{.04f,.27f,.018f},steel);box(f,m,{-.26f,.42f,.545f},{.05f,.055f,.025f},duck?blue:mint);
 M4 head=m*translate({0,1.11f+breath,0})*ry(.07f*std::sin(t*.47));
 ell(f,head,{0,0,0},{.73f,.76f,.65f},coat);
 if(!duck){
  ell(f,head,{-.23f,-.02f,.48f},{.36f,.45f,.22f},white);ell(f,head,{.23f,-.02f,.48f},{.36f,.45f,.22f},white);
  ell(f,head,{0,-.28f,.50f},{.43f,.25f,.19f},white);
  for(int s:{-1,1}){
   ell(f,head,{s*.26f,.08f,.661f},{.15f,.195f,.053f},eye);
   ell(f,head,{s*.26f-.041f,.14f,.712f},{.036f,.047f,.014f},white);
   ell(f,head,{s*.26f+.031f,.01f,.711f},{.012f,.019f,.009f},mint);
  }
  // The cyber-eye has a solid optically dark lens, metallic housing and a luminous reticle.
  ring(f,head*rx(pi/2),{.27f,.735f,-.10f},{.258f,.29f,.258f},steel);
  ring(f,head*rx(pi/2),{.27f,.779f,-.10f},{.222f,.17f,.222f},Material{{.04f,.70f,.43f},1.4f,.23f,.35f});
  box(f,head,{.52f,.12f,.72f},{.16f,.054f,.036f},ink);
  for(int i=0;i<3;i++)box(f,head,{.56f,.20f-i*.06f,.77f},{.021f,.012f,.02f},mint);
 }else{
  // Sunglasses sit on a copper bridge; two violet-black lenses, not a flat face sticker.
  for(int s:{-1,1}){box(f,head,{s*.27f,.10f,.578f},{.269f,.225f,.095f},copper);box(f,head,{s*.27f,.11f,.657f},{.231f,.180f,.040f},Material{{.022f,.039f,.085f},0,.055f,.70f});
   box(f,head*rz(-.15f),{s*.27f,.20f,.704f},{.15f,.012f,.012f},Material{{.27f,.45f,.73f},.3f,.2f,.2f});}
  box(f,head,{0,.15f,.65f},{.09f,.048f,.037f},ink);
 }
 // Blinks are eased lids that grow from the upper edge. No scale-flip or frame switching.
 float phase=std::fmod(float(t)+ (duck?2.1f:0.f),5.4f);float blink=phase<.20f?std::pow(std::sin(phase*pi/.20f),2.f):0;
 if(!duck&&blink>.001f)for(int s:{-1,1})ell(f,head,{s*.26f,.26f-blink*.18f,.727f},{.168f,.21f*blink,.025f},white);
 ell(f,head,{0,-.235f,.685f},duck?V3{.34f,.13f,.33f}:V3{.20f,.18f,.24f},Material{{1.f,.42f,.045f},0,.31f,.07f});
 ell(f,head,{0,-.293f,.705f},duck?V3{.31f,.047f,.30f}:V3{.16f,.035f,.21f},Material{{.48f,.105f,.015f},0,.4f,0});
 // Headset earcups, circular engraved rings and flexible antenna.
 for(int s:{-1,1}){
 M4 ear=head*translate({s*.715f,.015f,-.015f})*rz(pi/2);
 add(f,Cylinder,ear*scale({.29f,.095f,.29f}),ink);add(f,Torus,ear*translate({0,s*.102f,0})*scale({.21f,.16f,.21f}),steel);
 add(f,Cylinder,ear*translate({0,s*.12f,0})*scale({.145f,.018f,.145f}),duck?copper:steel);
 }
 for(int s:{-1,1}){box(f,head*rz(s*.16f),{s*.48f,.54f,-.20f},{.12f,.15f,.31f},ink);box(f,head*rz(s*.16f),{s*.48f,.68f,-.15f},{.035f,.02f,.21f},copper);}
 rod(f,head,{-.71f,.22f,0},{-.84f,.88f,-.02f},.023f,steel);ell(f,head,{-.84f,.88f,-.02f},{.048f,.048f,.048f},duck?amber:mint);
 rod(f,head,{-.77f,-.02f,.20f},{-.46f,-.39f,.56f},.033f,ink);ell(f,head,{-.46f,-.39f,.56f},{.10f,.055f,.055f},steel);
}
}
void beam(Frame& f,V3 a,V3 b,Material c,float r){if(length(b-a)<.001f)return;add(f,Cylinder,segment(a,b,r),c);ell(f,M4::identity(),b,{r*4,r*4,r*4},c);}
void ship(Frame& f,M4 m,bool duck,double t,float power){
 Material light=duck?amber:mint;
 // The airframe is modeled in three independently surfaced layers.
 add(f,Hull,m*translate({0,-.37f,0})*scale({1,.80f,1.07f}),ink);
 add(f,Hull,m,duck?Material{{.66f,.65f,.52f},0,.32f,.40f,9}:Material{{.68f,.76f,.73f},0,.30f,.38f,9});
 add(f,Hull,m*translate({.31f,.28f,0})*scale({.73f,.55f,.58f}),ink);
 // Layered forward cheek armor breaks the body into directional hard-surface forms.
 for(int side:{-1,1}){
  M4 cheek=m*translate({1.20f,.26f,side*.62f})*rz(-.16f)*ry(side*.16f);
  add(f,Hull,cheek*scale({.50f,.31f,.27f}),steel);
  add(f,Hull,cheek*translate({.06f,.055f,0})*scale({.43f,.20f,.235f}),ivory);
  rod(f,m,{.13f,.52f,side*.67f},{1.93f,.24f,side*.48f},.042f,duck?copper:steel);
  for(int k=0;k<5;k++){
   box(f,m*rz(-.12f),{.62f+k*.20f,.42f,side*.82f},{.035f,.042f,.09f},ink);
  }
  box(f,m,{-.85f,.43f,side*.79f},{.28f,.07f,.16f},copper);
 }
 // Nose armor, recessed light channels and collision bumper.
 ell(f,m,{2.76f,-.34f,0},{.27f,.075f,.18f},steel);
 for(int s:{-1,1}){
  box(f,m,{1.57f,.33f,s*.62f},{.43f,.035f,.051f},light);
  add(f,Wing,m*translate({-1.20f,-.10f,s*.80f})*scale({.92f,.85f,float(s)}),steel);
  add(f,Wing,m*translate({-1.18f,-.05f,s*.81f})*scale({.65f,.55f,s*.89f}),duck?copper:ivory);
  rod(f,m,{-1.68f,.06f,s*1.1f},{-2.68f,.09f,s*2.48f},.026f,light);
  // Exposed engine assembly: rings, dark throat, nested glow and longitudinal vents.
  M4 e=m*translate({-1.82f,-.12f,s*1.19f})*rz(pi/2);
  add(f,Cylinder,e*scale({.46f,.88f,.46f}),ink);
  for(float y:{-.65f,-.42f,.18f,.52f,.78f})add(f,Torus,e*translate({0,y,0})*scale({.46f,.8f,.46f}),y>.5?copper:steel);
  add(f,Cylinder,e*translate({0,.89f,0})*scale({.36f,.035f,.36f}),eye);
  add(f,Torus,e*translate({0,.94f,0})*scale({.30f,.65f,.30f}),light);
  add(f,Cylinder,e*translate({0,.94f,0})*scale({.19f,.015f,.19f}),light);
  for(int k=0;k<10;k++){float a=k*2*pi/10;box(f,e*ry(a),{.38f,-.03f,0},{.10f,.45f,.05f},steel);}
  float plume=(.68f+.07f*std::sin(t*7.4+s))*(.55f+.45f*power);
  ell(f,m,{-3.07f-plume*.4f,-.12f,s*1.19f},{plume,.165f,.165f},duck?blue:mint);
  ell(f,m,{-3.03f,-.12f,s*1.19f},{.48f,.088f,.088f},Material{{.7f,1.f,.97f},4,.2f,0});
  for(int k=0;k<6;k++)box(f,m,{.90f-k*.24f,-.19f,s*1.034f},{.055f,.16f,.03f},ink);
  // Large layered directional flank armor with stenciled markings and dark edges.
  M4 flank=m*scale({1,1,float(s)});
  add(f,Sideplate,flank*translate({-.04f,-.05f,.02f})*scale({1.04f,1.06f,1.02f}),ink);
  add(f,Sideplate,flank*translate({0,-.025f,.045f}),duck?Material{{.71f,.48f,.21f},0,.29f,.45f,9}:Material{{.61f,.70f,.72f},0,.28f,.55f,9});
  rod(f,m,{-.9f,-.10f,s*1.24f},{.2f,-.22f,s*1.055f},.022f,light);
  // Flush fasteners and ceramic panel breaks.
  for(int k=0;k<8;k++)ell(f,m,{-.90f+k*.36f,.0f,s*(1.025f-.10f*k)},{.025f,.026f,.025f},steel);
  rod(f,m,{-.71f,.56f,s*.66f},{.91f,.44f,s*.66f},.024f,copper);
  rod(f,m,{1.08f,-.20f,s*.93f},{2.63f,-.10f,s*.37f},.052f,ink);
  // Slender laser mounts are attached to the frame, so beams never detach during banking.
  rod(f,m,{.47f,-.50f,s*.80f},{2.38f,-.43f,s*.73f},.092f,steel);
  ell(f,m,{2.39f,-.43f,s*.73f},{.044f,.065f,.065f},light);
 }
 // Cockpit rim, seat, joystick and little forearm supports.
 ring(f,m,{-.58f,.49f,0},{.85f,.60f,.69f},steel);
 box(f,m,{-.72f,.77f,-.12f},{.52f,.31f,.52f},ink);
 box(f,m,{.32f,.64f,.20f},{.19f,.20f,.39f},steel);
 box(f,m*rz(-.17f),{.42f,.99f,.14f},{.10f,.035f,.30f},Material{{.09f,.42f,.41f},1,.23f,.3f});
 // Chunky Q badge on the starboard fuselage, created as geometry.
 M4 logo=m*translate({.12f,-.12f,1.0f})*rx(pi/2);
 ring(f,logo,{0,0,0},{.19f,.3f,.19f},ink);rod(f,m,{.22f,-.24f,1.032f},{.32f,-.36f,1.03f},.035f,copper);
 // A restrained copper scarf adds a living, flexible silhouette to the pilot.
 if(!duck)for(int side:{-1,1})add(f,Ribbon,m*translate({-.98f,1.31f,side*.26f})*rz(-.16f)*scale({1,.8f,1}),Material{{.70f,.20f,.045f},0,.68f,.04f,11});
 pilot(f,m*translate({-.66f,.72f,0})*ry(.38f)*rz(.025f*std::sin(t*1.4)),duck,t);
}
void asteroid(Frame& f,M4 m,int type){add(f,Primitive(Rock0+type%6),m,Material{{.21f,.18f,.15f},0,.88f,.09f,2});}
void capsule(Frame& f,M4 m,double t){add(f,Monolith,m*ry(t*.18f)*scale({.27f,.27f,.27f}),steel);ring(f,m*rz(.5f)*ry(t*.23f),{0,0,0},{.41f,.35f,.41f},mint);ell(f,m,{0,0,0},{.12f,.42f,.12f},amber);}
void station(Frame& f,M4 m,double t,bool home){
 // Ring habitat uses repeating structural bays; the negative space stays open.
 M4 rot=m*ry(t*.012f);
 ring(f,rot,{0,0,0},{6.2f,2.0f,6.2f},ink);ring(f,rot,{0,.19f,0},{6.3f,.6f,6.3f},steel);ring(f,rot,{0,-.24f,0},{6.5f,.6f,6.5f},copper);
 ring(f,rot,{0,.31f,0},{5.9f,.10f,5.9f},mint);
 for(int i=0;i<36;i++){M4 b=rot*ry(i*2*pi/36)*translate({6.2f,0,0});box(f,b,{0,0,0},{.43f,.26f,.33f},i%3?ivory:steel);
  box(f,b,{.46f,.025f,0},{.017f,.079f,.22f},i%5?Material{{.75f,.45f,.13f},1.4f,.4f,0}:mint);
  for(int k=0;k<3;k++)box(f,b,{.02f,.29f,k*.14f-.14f},{.31f,.027f,.018f},ink);
  if(i%3==0){box(f,b,{-.10f,.61f,0},{.21f,.37f,.20f},steel);box(f,b,{.13f,.60f,0},{.017f,.17f,.12f},Material{{.37f,.68f,.8f},.8f,.2f,.3f});rod(f,b,{-.1f,.9f,0},{-.1f,1.65f,0},.025f,copper);ell(f,b,{-.1f,1.65f,0},{.043f,.043f,.043f},amber);}
  if(i%6==0){rod(f,b,{.35f,-.2f,0},{2.3f,-1.2f,0},.12f,steel);box(f,b,{2.2f,-1.24f,0},{.68f,.09f,.46f},ink);for(int n=0;n<4;n++)box(f,b,{1.73f+n*.3f,-1.13f,.39f},{.035f,.02f,.023f},amber);}

 }
 for(int i=0;i<6;i++){M4 a=rot*ry(i*pi/3);rod(f,a,{0,0,0},{5.7f,0,0},.16f,steel);rod(f,a,{0,-1.5f,0},{5.7f,-.2f,0},.065f,copper);}
 add(f,Cylinder,m*scale({1.16f,3.9f,1.16f}),ink);
 for(float y:{-3.6f,-2.7f,-1.7f,.8f,1.9f,3.0f}){ring(f,m,{0,y,0},{1.23f,.6f,1.23f},steel);ring(f,m,{0,y+.13f,0},{1.21f,.16f,1.21f},amber);}
 for(int i=0;i<8;i++){M4 b=m*ry(i*pi/4);box(f,b,{1.10f,1.5f,0},{.18f,.72f,.36f},ivory);box(f,b,{1.285f,1.6f,0},{.018f,.13f,.28f},mint);}
 rod(f,m,{0,3.5f,0},{0,7,0},.12f,steel);ell(f,m,{0,7,0},{.09f,.09f,.09f},amber);
 // Solar fins and communications comb, distinctly human engineered.
 for(int s:{-1,1})for(int row=0;row<3;row++){
  M4 a=m*translate({s*3.2f,-2.8f+row*.75f,-1.2f});box(f,a,{0,0,0},{1.9f,.035f,.9f},steel);box(f,a,{0,.06f,0},{1.8f,.022f,.82f},Material{{.025f,.085f,.18f},0,.18f,.6f});
  for(int j=0;j<12;j++)box(f,a,{j*.30f-1.65f,.093f,0},{.007f,.008f,.82f},copper);
  rod(f,m,{s*.7f,-2.8f,0},{s*4.8f,-2.8f,-1.2f},.055f,steel);
 }
 // Traffic is small and separate from the large hero ships.
 for(int i=0;i<5;i++){float a=t*.06f+i*1.256f;M4 tug=m*translate({std::cos(a)*9,-1.8f+.4f*i,std::sin(a)*8})*ry(-a)*scale({.35f,.35f,.35f});add(f,Hull,tug,steel);ell(f,tug,{-2.9f,0,0},{.45f,.10f,.10f},amber);}
 if(home)for(int i=0;i<7;i++)ell(f,m,{float(std::sin(t*.4+i))*3,2+float(std::cos(t*.3+i)),float(std::cos(t*.4+i))*3},{.045f,.045f,.045f},mint);
}
void freighter(Frame& f,M4 m,double t,float repair){
 box(f,m,{0,0,0},{6.0f,.85f,1.2f},ink);
 for(int i=0;i<5;i++)for(int s:{-1,1}){M4 b=m*translate({-3.7f+i*1.85f,.25f,s*1.1f});box(f,b,{0,0,0},{.83f,.78f,.62f},i%2?ivory:copper);
 for(int j=0;j<6;j++)box(f,b,{-.65f+j*.26f,0,s*.66f},{.031f,.61f,.019f},steel);
 box(f,b,{0,.55f,s*.672f},{.18f,.047f,.025f},amber);}
 box(f,m,{5.4f,.5f,0},{.9f,1.05f,1.6f},steel);box(f,m,{5.8f,1.29f,0},{.68f,.17f,1.46f},Material{{.055f,.18f,.22f},.35f,.12f,.4f});
 for(int s:{-1,1}){M4 e=m*translate({-6.1f,0,s*1.1f})*rz(pi/2);add(f,Cylinder,e*scale({.6f,.6f,.6f}),steel);ring(f,e,{0,.67f,0},{.42f,.4f,.42f},repair>.5?mint:amber);}
 rod(f,m,{2.5f,1.0f,0},{2.5f,3.5f,0},.06f,steel);ring(f,m*rx(.6f),{2.5f,2.6f,-1.6f},{.69f,.3f,.69f},steel);
 ell(f,m,{2.5f,3.50f,0},{.13f,.13f,.13f},repair>.5?mint:Material{{1,.12f,.015f},1.5f+.5f*float(std::sin(t*4)),.3f,0});
}
void gate(Frame& f,M4 m,double t,float awake){
 for(int j=0;j<3;j++){M4 r=m*rz(float(t)*(.035f+j*.013f)*(j==1?-1:1))*rx(.07f*std::sin(t*.18+j));float radius=3.8f+j*.95f;
  ring(f,r*rx(pi/2),{0,0,0},{radius,.7f,radius},Material{{.09f,.11f,.15f},0,.23f,.8f,5});
  ring(f,r*rx(pi/2),{0,-.10f,0},{radius-.20f,.16f,radius-.20f},Material{{.09f,.62f,1.f},.3f+awake*3,.3f,.4f});
  for(int i=0;i<14;i++){M4 p=r*rz(i*2*pi/14)*translate({0,radius,0});box(f,p,{0,0,0},{.46f,.33f,.35f},Material{{.045f,.065f,.095f},0,.30f,.85f,5});
   for(int k=0;k<3;k++){float x=(k-1)*.14f;rod(f,p,{x,-.14f,.37f},{x+.07f,.07f,.37f},.019f,purple);}
  }
 }
 for(int i=0;i<7;i++){float a=i*2*pi/7+.4f;M4 p=m*translate({std::cos(a)*7,std::sin(a)*7,1.2f*std::sin(a)})*rz(-a+.5f)*ry(.5f)*rx(.2f*std::sin(t*.12+i));
  add(f,Monolith,p*scale({.70f,1.16f,.7f}),Material{{.018f,.028f,.047f},0,.28f,.83f,5});
  rod(f,p,{0,-1.5f,.57f},{0,1.4f,.40f},.032f,mint);
  for(int k=0;k<7;k++)rod(f,p,{-.21f,-1.0f+k*.30f,.61f},{.13f,-.94f+k*.30f,.61f},.016f,purple);
 }
 if(awake>.03f)add(f,Cylinder,m*translate({0,0,-.33f})*rx(pi/2)*scale({3.20f,.018f,3.20f}),Material{{.035f,.09f,.25f},awake,.25f,.4f,12});
 ell(f,m,{0,0,0},{.13f,.13f,.13f},Material{{.36f,.72f,1.f},1+awake*2,.18f,0});
}
void sentinel(Frame& f,M4 m,double t){
 ell(f,m,{0,0,0},{.58f,.45f,.50f},Material{{.065f,.045f,.08f},0,.18f,.82f,5});
 for(int i=0;i<3;i++){M4 a=m*rz(i*2*pi/3+float(t)*.05f);add(f,Monolith,a*translate({0,.66f,-.12f})*scale({.21f,.48f,.20f}),steel);rod(f,a,{0,.26f,.35f},{0,.96f,.11f},.036f,purple);}
 ell(f,m,{0,0,.46f},{.24f,.24f,.10f},eye);ring(f,m*rx(pi/2),{0,.55f,0},{.20f,.35f,.20f},Material{{1,.10f,.065f},3,.25f,.2f});
 ell(f,m,{0,0,.58f},{.10f,.10f,.03f},Material{{1,.23f,.07f},6,.2f,0});
}
void creature(Frame& f,M4 m,double t){
 add(f,Manta,m,Material{{.065f,.19f,.27f},0,.18f,.53f,6});
 ell(f,m,{0,.20f,0},{1.9f,.40f,.61f},Material{{.075f,.32f,.42f},0,.18f,.55f,5});
 for(int s:{-1,1}){ell(f,m,{1.22f,.31f,s*.41f},{.17f,.15f,.14f},eye);ell(f,m,{1.31f,.35f,s*.48f},{.067f,.065f,.057f},mint);}
 // Filament trails are coherent waves along each strand, not independent jiggle.
 for(int j=0;j<7;j++)for(int k=0;k<22;k++){
  auto p=[&](int n){float d=n*.28f;return V3{-1.4f-d,.24f+float(std::sin(t*1.2-d*.7+j*.65f))*(.05f+d*.075f),(j-3)*.13f+float(std::sin(t*.9-d*.9+j))*d*.055f};};
  rod(f,m,p(k),p(k+1),.016f*(1-k/27.f),j%2?blue:mint);
 }
 for(int j=0;j<9;j++)ell(f,m,{1.25f-j*.35f,.59f,0},{.031f,.018f,.075f},mint);
}
}
