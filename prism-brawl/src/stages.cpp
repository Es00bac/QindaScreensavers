// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"

namespace sw {
namespace {
// The installed QindaQt wallpapers and the owner's Downloads are visual
// references. These environments are original geometry; no image paths are
// needed at runtime. Exact references are recorded in docs/STAGES.md.
struct Palette {V3 floor,frame,light,secondary;float surface;};
constexpr std::array<Palette,StageCount> palettes{{
 {{.035f,.065f,.078f},{.018f,.030f,.042f},{.22f,.86f,.68f},{.85f,.47f,.20f},20},
 {{.12f,.24f,.19f},{.036f,.080f,.065f},{.41f,.91f,.66f},{.69f,.48f,.89f},21},
 {{.057f,.082f,.12f},{.025f,.036f,.058f},{.24f,.74f,.94f},{.95f,.23f,.57f},22},
 {{.21f,.34f,.13f},{.12f,.17f,.075f},{.63f,.86f,.29f},{.95f,.66f,.28f},21},
 {{.14f,.16f,.14f},{.028f,.043f,.057f},{.25f,.84f,.62f},{.98f,.54f,.23f},21},
 {{.27f,.47f,.59f},{.048f,.12f,.21f},{.28f,.84f,.99f},{.68f,.48f,.97f},24},
 {{.68f,.77f,.86f},{.23f,.34f,.53f},{.32f,.76f,.96f},{.78f,.64f,.94f},23}
}};
const V3 metal{.12f,.17f,.22f},ink{.014f,.025f,.039f},copper{.51f,.27f,.12f};
void box(Frame& f,V3 p,V3 size,V3 color,float emission=0,float mode=0,float rough=.38f){
 add(f,Box,translate(p)*scale(size),{color,emission,rough,.45f,mode});
}
void beam(Frame& f,V3 a,V3 b,float radius,V3 color,float emission=0){
 add(f,Cylinder,segment(a,b,radius),{color,emission,.35f,.55f});
}
void orb(Frame& f,V3 p,V3 size,V3 color,float emission=0,float mode=0){
 add(f,Sphere,translate(p)*scale(size),{color,emission,.43f,.12f,mode});
}
void ring(Frame& f,M4 m,V3 size,V3 color,float emission=0){
 add(f,Torus,m*scale(size),{color,emission,.26f,.65f});
}
void arc(Frame& f,V3 center,float radius,float from,float to,V3 color,float width,float emission=0){
 for(int k=0;k<32;++k){float a=from+(to-from)*k/32,b=from+(to-from)*(k+1)/32;
  beam(f,center+V3{radius*std::cos(a),radius*std::sin(a),0},center+V3{radius*std::cos(b),radius*std::sin(b),0},width,color,emission);
 }
}
void emblem(Frame& f,M4 m,float radius,V3 color,float emission){
 ring(f,m*rx(pi/2),{radius,.60f,radius},color,emission);
 beam(f,point(m,{radius*.42f,-radius*.48f,0}),point(m,{radius*.91f,-radius*.98f,0}),radius*.095f,color,emission);
}
void foliage(Frame& f,V3 p,float size,V3 color,float time,int seed){
 for(int leaf=0;leaf<5;++leaf){float a=leaf*2*pi/5+seed*.7f;
  M4 m=translate(p)*ry(a)*rz(.30f*std::sin(time*.7f+seed+leaf))*rx(.48f+.06f*leaf);
  add(f,Leaf,m*scale({size,size*(.70f+.07f*leaf),size}),{color,0,.83f,.02f});
 }
}
void hologram(Frame& f,V3 p,V3 size,V3 color,float time,int seed){
 box(f,p,size,ink,.02f,0);
 for(int side:{-1,1}){
  beam(f,p+V3{side*size.x,-size.y,size.z+.02f},p+V3{side*size.x,size.y,size.z+.02f},.034f,color,.95f);
  beam(f,p+V3{-size.x,side*size.y,size.z+.02f},p+V3{size.x,side*size.y,size.z+.02f},.034f,color,.95f);
 }
 for(int row=0;row<6;++row){float y=(.68f-row*.25f)*size.y;
  float width=(.27f+.12f*((row+seed)%4))*size.x;
  beam(f,p+V3{-size.x*.74f,y,size.z+.04f},p+V3{-size.x*.74f+width,y,size.z+.04f},.018f,color,.75f);
 }
 float cursor=std::sin(time*1.2f+seed)*.1f+.7f;
 box(f,p+V3{size.x*.54f,-size.y*.63f,size.z+.05f},{.10f,.045f,.016f},color,cursor);
}

void platformArt(Frame& f,const Platform& p,int index,int stage,float time){
 const auto& theme=palettes[stage];bool main=index==0;
 float thickness=main?.54f:.27f,depth=p.depth;
 box(f,{p.x,p.y-thickness-.08f,0},{p.half,thickness,depth},theme.frame);
 // A full rectangular top shares exactly the simulation's one-way surface;
 // bevels, trim, roots, pipes and crystal caps all stay beneath or beside it.
 add(f,RoadSurface,translate({p.x,p.y,0})*scale({p.half,1,depth}),{theme.floor,0,.35f,.36f,theme.surface});
 box(f,{p.x,p.y-thickness*2-.15f,0},{p.half*.92f,.12f,depth*.92f},mix(theme.frame,theme.secondary,.22f));
 for(int side:{-1,1}){
  beam(f,{p.x-p.half+.16f,p.y-.065f,side*depth},{p.x+p.half-.16f,p.y-.065f,side*depth},.043f,theme.light,1.2f);
  beam(f,{p.x+side*p.half,p.y-.065f,-depth+.12f},{p.x+side*p.half,p.y-.065f,depth-.12f},.043f,theme.light,1.1f);
  int ribs=main?12:4;
  for(int k=0;k<ribs;++k){float x=p.x-p.half+.65f+(2*p.half-1.3f)*k/(ribs-1);
   box(f,{x,p.y-thickness,side*(depth+.015f)},{.18f,thickness*.62f,.10f},mix(theme.frame,metal,.6f));
   box(f,{x,p.y-thickness,side*(depth+.12f)},{.095f,.018f,.015f},k%3?theme.light:theme.secondary,.9f);
  }
 }
 // Underside silhouettes distinguish the decks even against a bright sky.
 if(stage==0||stage==6){
  add(f,Hull,translate({p.x,p.y-(main?1.45f:.8f),0})*scale({p.half*.24f,main?1.15f:.65f,depth*.63f}),{theme.frame,0,.24f,.7f});
  ring(f,translate({p.x,p.y-(main?1.6f:.82f),0}),{main?2.25f:.66f,.6f,main?1.48f:.60f},theme.light,1.25f);
  if(stage==6)for(int side:{-1,1})add(f,Fold,translate({p.x,p.y-(main?1.1f:.5f),side*depth*.68f})*ry(side*.2f)*scale({p.half*.98f,.55f,.40f}),{theme.floor,0,.34f,.20f,23});
 }else if(stage==1||stage==3){
  for(int k=0;k<(main?9:3);++k){float x=p.x+(k-(main?4:1))*(main?2.5f:1.5f);
   add(f,Primitive(Rock0+k%6),translate({x,p.y-(main?1.15f:.64f),0})*scale({main?1.6f:1.f,.65f,depth*.82f}),{theme.frame,0,.89f,.03f,2});
   if(main)for(int side:{-1,1}){
    foliage(f,{x,p.y+.015f,side*(depth-.33f)},.42f,stage==3?V3{.26f,.43f,.09f}:V3{.08f,.40f,.24f},time,k+side);
    beam(f,{x,p.y-.5f,side*depth*.82f},{x+.32f,p.y-1.9f,side*depth*.74f},.035f,theme.light,.18f);
   }
  }
 }else if(stage==2||stage==4){
  for(int side:{-1,1}){
   beam(f,{p.x-p.half*.8f,p.y-thickness*2-.32f,side*depth*.7f},{p.x+p.half*.8f,p.y-thickness*2-.32f,side*depth*.7f},.11f,copper);
   for(int k=0;k<(main?6:2);++k){float x=p.x+(k-(main?2.5f:.5f))*(main?3.6f:2.3f);
    box(f,{x,p.y-thickness*2-.31f,side*(depth-.2f)},{.24f,.36f,.32f},metal);
   }
  }
  if(stage==4){box(f,{p.x,p.y-.52f,depth+.16f},{p.half*.7f,.31f,.09f},ink,0,26);
   for(int k=0;k<(main?12:4);++k)box(f,{p.x+(k-(main?5.5f:1.5f))*.75f,p.y-.50f,depth+.27f},{.23f,.055f,.016f},k%4?theme.light:theme.secondary,.65f);
  }
 }else if(stage==5){
  for(int k=0;k<(main?11:4);++k){float x=p.x-p.half+.7f+(2*p.half-1.4f)*k/(main?10:3);
   add(f,Crystal,translate({x,p.y-1.15f,0})*rx(pi)*rz(.12f*std::sin(float(k)))*scale({.60f,.56f+.13f*(k%3),depth*.58f}),{mix(theme.floor,theme.light,.20f),.12f,.16f,.38f,24});
  }
 }
 if(!main){
  add(f,Cylinder,translate({p.x,p.y-1.02f,0})*scale({.40f,.15f,.40f}),{theme.frame,0,.28f,.7f});
  ring(f,translate({p.x,p.y-1.15f,0}),{.39f,.32f,.39f},theme.light,.85f);
 }else{
  // An inlaid Q mark and parallel guide lines leave the fighting lane quiet.
  emblem(f,translate({0,p.y+.013f,0})*rx(-pi/2),.98f,theme.light,.38f);
  for(int side:{-1,1})for(int k=0;k<4;++k){float x=side*(3.5f+k*1.75f);
   beam(f,{x,p.y+.01f,-depth+.45f},{x,p.y+.01f,-depth+1.15f},.018f,theme.secondary,.32f);
   beam(f,{x,p.y+.01f,depth-.45f},{x+side*.27f,p.y+.01f,depth-1.0f},.018f,theme.secondary,.32f);
  }
 }
}

void skyline(Frame& f,Random& rng,int stage,float time){
 const auto& theme=palettes[stage];
 bool day=stage==3;
 for(int layer=0;layer<3;++layer)for(int k=0;k<20;++k){
  float x=(k-9.5f)*5.4f+rng.range(-1.4f,1.4f),z=-35-layer*23-rng.range(0,9);
  float height=rng.range(5,18)*(layer==2?1.4f:1.f),base=-13.f-layer*2;
  if(day){z-=90;height*=.63f;base=-11.f;}
  V3 color=day?mix(V3{.21f,.40f,.42f},V3{.50f,.66f,.64f},layer*.22f):mix(ink,V3{.055f,.072f,.12f},layer*.20f);
  float width=rng.range(.85f,1.9f);
  box(f,{x,base+height*.5f,z},{width,height*.5f,width*.7f},color,0,day?0:14);
  box(f,{x,base+height+.16f,z},{width*.72f,.18f,width*.61f},mix(color,theme.light,.15f));
  if(k%3==0){
   beam(f,{x,base+height,z},{x,base+height+3.1f,z},.06f,theme.light,day?.12f:.60f);
   if(!day)beam(f,{x-width-.02f,base+1,z+.15f},{x-width-.02f,base+height-.7f,z+.15f},.075f,k%2?theme.light:theme.secondary,.95f);
  }
  if(k%4==0&&!day)hologram(f,{x,base+height*.55f,z+width*.72f},{width*.71f,height*.13f,.04f},k%2?theme.light:theme.secondary,time,k);
 }
}

void terminal(Frame& f,float time,Random& rng){
 const auto& theme=palettes[0];
 // A dimensional orbital gate, with dark mass and restrained mineral light.
 V3 center{0,2.5f,-27};
 arc(f,center,10.5f,-.15f,pi+.15f,metal,.27f);
 arc(f,center+V3{0,0,.12f},10.20f,.05f,pi-.05f,theme.light,.048f,1.3f);
 arc(f,center+V3{0,0,-.25f},11.05f,.14f,pi-.14f,theme.secondary,.08f,.30f);
 for(int k=0;k<18;++k){float angle=k*pi/17;
  V3 radial{std::cos(angle),std::sin(angle),0};
  beam(f,center+radial*10.36f,center+radial*11.28f,.11f,metal);
  box(f,center+radial*11.28f,{.18f,.18f,.31f},theme.frame);
 }
 for(int side:{-1,1}){
  box(f,{side*15.5f,-1,-17},{1.55f,6.8f,2.3f},theme.frame);
  for(int level=0;level<4;++level){float y=-4+level*2.1f;
   box(f,{side*15.5f,y,-14.65f},{1.2f,.46f,.09f},metal);
   beam(f,{side*15.5f-.85f,y,-14.5f},{side*15.5f+.85f,y,-14.5f},.032f,theme.light,.9f);
  }
  emblem(f,translate({side*15.5f,4.8f,-14.6f}),.73f,theme.light,.85f);
  for(int k=0;k<5;++k){V3 p{side*(21.f+k*5),-7+float(k%3)*2,-28.f-k*6};
   add(f,Crystal,translate(p)*rz(side*.17f)*scale({1.6f,3.f+rng.range(0,2),1.4f}),{theme.frame,0,.28f,.55f,5});
   beam(f,p+V3{0,-2,1.2f},p+V3{0,4,1.2f},.025f,theme.light,.6f);
  }
 }
 for(int k=0;k<14;++k){float a=k*2*pi/14+time*.025f;
  V3 p{std::cos(a)*18.f,5.5f+std::sin(a)*12.f,-30};
  add(f,Monolith,translate(p)*ry(time*.09f+k)*rz(a)*scale({.22f,.44f,.20f}),{theme.secondary,.08f,.27f,.65f});
 }
}

void garden(Frame& f,float time,Random& rng){
 const auto& theme=palettes[1];
 add(f,Terrain,translate({0,-13,-48})*scale({135,7,120}),{{.035f,.095f,.064f},0,.93f,.01f,27});
 for(int layer=1;layer<3;++layer){
  add(f,Terrain,translate({layer%2?8.f:-5.f,-9.f-layer*1.3f,-20.f-layer*20})*scale({52,5.f+layer*2,17}),{{.035f,.10f+.025f*layer,.087f},0,.85f,.02f,27});
 }
 for(int j=0;j<12;++j){float x=(j-5.5f)*4.8f,z=-14.f-std::abs(x)*.36f,top=rng.range(3.0f,9.0f);
  V3 root{x,-9,z};beam(f,root,{x,top,z},.17f,metal);
  beam(f,root+V3{.15f,0,.16f},{x+.15f,top,.16f+z},.022f,theme.light,.9f);
  for(int k=0;k<5;++k){float a=k*2*pi/5+j*.7f;
   V3 tip{x+std::cos(a)*2.0f,top+.3f+.20f*std::sin(time*.5f+j+k),z+std::sin(a)*1.5f};
   beam(f,{x,top-2,z},tip,.065f,metal);
   add(f,Leaf,translate(tip)*ry(a)*rz(.65f)*scale({3.2f,3.5f,1.8f}),{{.085f,.32f,.21f},.06f,.54f,.15f});
   orb(f,tip,{.16f,.28f,.16f},k%2?theme.light:theme.secondary,.85f);
  }
  ring(f,translate({x,top-2.1f,z}),{.36f,.55f,.36f},theme.light,.65f);
 }
 // A greenhouse arch and hydroponic channels connect the trees to the arena.
 for(int side:{-1,1}){
  arc(f,{side*10.f,-2,-19},6.5f,0,pi,metal,.10f);
  box(f,{side*15.f,-5,-11},{2.8f,.55f,1.4f},theme.frame);
  for(int k=0;k<5;++k)foliage(f,{side*15.f+(k-2)*.9f,-4.45f,-11},1.45f,{.12f,.42f,.23f},time,k);
 }
 for(int j=0;j<38;++j){V3 p{rng.range(-24,24),rng.range(-2,14)+.35f*std::sin(time*.6f+j),rng.range(-28,-10)};
  orb(f,p,{.037f,.037f,.037f},j%3?theme.light:theme.secondary,1.6f);
 }
}

void rooftop(Frame& f,float time,Random& rng){
 const auto& theme=palettes[2];skyline(f,rng,2,time);
 // Roof access machinery: the combat deck is a bridge between two towers.
 for(int side:{-1,1}){
  V3 base{side*18.f,-6,-10};box(f,base,{3.7f,5.7f,5.5f},theme.frame,0,14);
  box(f,base+V3{0,5.8f,0},{4.1f,.17f,5.8f},theme.floor,0,22,.15f);
  for(int k=0;k<3;++k){V3 p=base+V3{(k-1)*2.2f,6.5f,0};
   box(f,p,{.83f,.68f,1.3f},metal);
   for(int slot=0;slot<6;++slot)box(f,p+V3{0,(slot-2.5f)*.14f,1.32f},{.64f,.025f,.02f},ink);
   ring(f,translate(p+V3{0,.71f,0}),{.53f,.45f,.53f},theme.light,.40f);
  }
  beam(f,base+V3{-3.8f,5.9f,4.9f},base+V3{3.8f,5.9f,4.9f},.06f,metal);
  for(int k=0;k<5;++k)beam(f,base+V3{(k-2)*1.8f,5.9f,4.9f},base+V3{(k-2)*1.8f,7.1f,4.9f},.06f,metal);
  beam(f,base+V3{-3.8f,7.1f,4.9f},base+V3{3.8f,7.1f,4.9f},.06f,metal);
  hologram(f,{side*19.f,6.3f,-12},{2.f,3.3f,.12f},side<0?theme.light:theme.secondary,time,side+2);
  for(int k=0;k<3;++k){float y=-2.f+k*1.05f;
   beam(f,{side*12.f,y,-4},{side*18.f,y,-8},.08f,k%2?theme.light:metal,k%2?.5f:0);
  }
 }
 for(int j=0;j<12;++j){float x=std::fmod(time*(2.4f+j*.12f)+j*9,94.f)-47.f;
  V3 p{x,-1.f+j*.45f,-29.f-j*1.6f};
  add(f,Hull,translate(p)*scale({.33f,.33f,.43f}),{metal,0,.2f,.65f});
  beam(f,p+V3{-.8f,0,0},p+V3{-2.3f,0,0},.028f,j%2?theme.light:theme.secondary,1.0f);
 }
 // Sparse rain stays behind the fighter plane and falls continuously.
 for(int j=0;j<86;++j){float x=rng.range(-28,28),z=rng.range(-24,-7),speed=rng.range(5,9);
  float y=std::fmod(rng.range(0,28)+time*speed,28.f)-8;
  beam(f,{x,19-y,z},{x-.07f,18.55f-y,z},.009f,{.20f,.38f,.57f},.23f);
 }
}

void bliss(Frame& f,float time,Random& rng){
 add(f,Terrain,translate({0,-12,-45})*scale({140,8,125}),{{.16f,.34f,.065f},0,.93f,.01f,27});
 for(int layer=1;layer<4;++layer){float z=-35.f-layer*25;
  V3 color=mix(V3{.23f,.39f,.085f},V3{.33f,.48f,.32f},layer*.22f);
  add(f,Terrain,translate({layer%2?11.f:-7.f,-10.f-layer*.8f,z})*ry(layer%2?.20f:-.13f)*scale({95,7.f+layer*.8f,25}),{color,0,.92f,.01f,27});
 }
 skyline(f,rng,3,time);
 // Reclaimed circuit boards, retired machines and wildflowers from Qinda Bliss.
 for(int side:{-1,1}){
  for(int k=0;k<7;++k){V3 p{side*(14.f+k*2.8f),-5.4f-k*.5f,-10.f-k*2.4f};
   add(f,Box,translate(p)*rz(side*(.12f+.06f*(k%3)))*scale({1.5f,.15f,1.0f}),{{.07f,.20f,.10f},0,.77f,.12f,21});
   if(k%2==0){box(f,p+V3{0,.55f,0},{.65f,.55f,.40f},metal);box(f,p+V3{0,.61f,.42f},{.51f,.35f,.035f},{.14f,.28f,.20f},.03f);}
   foliage(f,p+V3{.7f,.18f,.7f},1.1f,{.28f,.43f,.085f},time,k+side);
  }
  for(int k=0;k<12;++k){V3 p{side*(13.f+rng.range(0,15)),-5.7f+rng.range(-1,1),rng.range(-22,-8)};
   float height=rng.range(.24f,.7f);beam(f,p,p+V3{0,height,0},.018f,{.21f,.38f,.10f});
   for(int petal=0;petal<4;++petal){float a=petal*pi/2;
    add(f,Leaf,translate(p+V3{0,height,0})*ry(a)*rx(pi/2)*scale({.28f,.26f,.24f}),{{.91f,.86f,.55f},0,.9f,0});
   }
   orb(f,p+V3{0,height+.025f,0},{.045f,.035f,.045f},{.96f,.55f,.10f});
  }
 }
 // The old communications dishes now watch over a green valley.
 for(int side:{-1,1}){
  V3 p{side*25.f,-3,-29};beam(f,p+V3{0,-6,0},p,.11f,metal);
  ring(f,translate(p)*rx(.76f),{2.4f,.55f,2.4f},palettes[3].secondary,.12f);
  for(int k=0;k<5;++k){float a=k*2*pi/5;beam(f,p,p+V3{std::cos(a)*2.3f,std::sin(a)*1.7f,-std::sin(a)*1.6f},.05f,metal);}
 }
}

void server(Frame& f,V3 p,float height,int seed,float time){
 box(f,p+V3{0,height*.5f,0},{1.25f,height*.5f,.9f},ink);
 for(int rack=0;rack<int(height/.65f);++rack){float y=.45f+rack*.65f;
  box(f,p+V3{0,y,.91f},{1.08f,.22f,.045f},metal);
  for(int vent=0;vent<5;++vent)box(f,p+V3{-.78f+vent*.26f,y,.967f},{.075f,.115f,.012f},ink);
  float glow=.48f+.22f*std::sin(time*.6f+rack+seed);
  box(f,p+V3{.80f,y,.97f},{.06f,.06f,.014f},(rack+seed)%4?V3{.17f,.82f,.56f}:V3{.92f,.40f,.17f},glow);
 }
}
void compileClub(Frame& f,float time,Random& rng){
 const auto& theme=palettes[4];
 box(f,{0,4,-26},{58,22,.8f},{.008f,.013f,.018f},0,0,.84f);
 box(f,{0,-8.3f,-13},{48,.17f,26},{.023f,.029f,.029f},0,22);
 box(f,{0,-1.7f,0},{10,.80f,3.35f},ink,0,26);
 for(int side:{-1,1})box(f,{side*9.f,-5.2f,0},{.28f,2.9f,2.9f},metal);
 box(f,{0,19,-16},{42,.55f,18},{.024f,.032f,.040f});
 for(int k=0;k<9;++k){float x=(k-4)*8.f;
  box(f,{x,5,-24.9f},{.15f,13,.13f},metal);
  beam(f,{x,-9,-24.6f},{x,16,-24.6f},.09f,copper);
 }
 for(int k=0;k<10;++k)server(f,{(k-4.5f)*4.8f,-7.5f,-20},rng.range(7,13),k,time);
 hologram(f,{0,7.4f,-24.6f},{5.2f,2.25f,.14f},theme.light,time,2);
 for(int side:{-1,1}){
  // Side workbenches frame the main server-case fighting deck.
  box(f,{side*18.f,-1.9f,-9},{5.2f,.38f,3.1f},{.22f,.12f,.065f},0,0,.7f);
  for(int leg:{-1,1})box(f,{side*18.f+leg*4.1f,-5.1f,-9},{.24f,2.9f,2.4f},metal);
  hologram(f,{side*17.f,1.6f,-9.8f},{2.55f,1.55f,.17f},theme.light,time,side+1);
  beam(f,{side*17.f,-1.5f,-9.8f},{side*17.f,.1f,-9.8f},.16f,metal);
  box(f,{side*17.f,-1.45f,-7},{2.3f,.11f,.61f},ink,0,26);
  // Oversized articulated lamps, the workshop's warm practical light source.
  beam(f,{side*21.f,-1.4f,-9},{side*22.f,3.9f,-9},.10f,copper);
  beam(f,{side*22.f,3.9f,-9},{side*19.f,5.7f,-9},.10f,copper);
  add(f,Cylinder,translate({side*19.f,5.55f,-9})*scale({1.05f,.26f,1.05f}),{copper,0,.35f,.6f});
  orb(f,{side*19.f,5.26f,-9},{.85f,.06f,.85f},theme.secondary,1.4f);
 }
 // A steaming jade-badged mug beside Ducke's very serious cooling fan.
 V3 mug{-14.f,-.80f,-6.7f};
 add(f,Cylinder,translate(mug)*scale({.70f,.83f,.70f}),{{.15f,.24f,.25f},0,.24f,.5f});
 ring(f,translate(mug+V3{0,.83f,0}),{.67f,.40f,.67f},copper);
 add(f,Cylinder,translate(mug+V3{0,.815f,0})*scale({.60f,.013f,.60f}),{{.035f,.012f,.006f},0,.16f,.1f});
 ring(f,translate(mug+V3{.86f,.04f,0})*rx(pi/2),{.48f,1.8f,.61f},metal);
 emblem(f,translate(mug+V3{0,0,.71f}),.25f,theme.light,.7f);
 for(int k=0;k<7;++k){float age=std::fmod(time*.35f+k/7.f,1.f),radius=(.10f+.18f*age)*(1-age);
  orb(f,mug+V3{.15f*std::sin(age*8+k),.98f+age*2.5f,.07f*std::cos(age*9)},{radius*.8f,radius*1.8f,radius*.8f},{.20f,.24f,.24f},.10f);
 }
 V3 fan{14.2f,.40f,-6.9f};
 beam(f,fan+V3{0,-1.8f,0},fan,.12f,metal);box(f,fan+V3{0,-1.83f,.12f},{.76f,.12f,.62f},metal);
 ring(f,translate(fan)*rx(pi/2),{1.10f,.9f,1.10f},copper);
 for(int k=0;k<3;++k){float angle=time*3.2f+k*2*pi/3;
  add(f,Leaf,translate(fan)*rz(angle)*scale({1.25f,.90f,.70f}),{metal,0,.45f,.5f});
 }
 for(int k=0;k<6;++k){float angle=k*pi/3;beam(f,fan+V3{0,0,.13f},fan+V3{std::cos(angle)*1.08f,std::sin(angle)*1.08f,.13f},.022f,metal);}
 orb(f,fan+V3{0,0,.16f},{.18f,.18f,.12f},theme.secondary,.25f);
 // Overhead cable runs form a readable arch, away from the combat plane.
 for(int cable=0;cable<4;++cable)for(int k=0;k<18;++k){float x=-27+k*3.f;
  auto pointOn=[&](float xx){return V3{xx,12.5f+xx*xx*.008f+cable*.24f,-16.f-cable*.20f};};
  beam(f,pointOn(x),pointOn(x+3),.032f,cable%2?copper:metal);
 }
}

void aurora(Frame& f,float time,Random& rng){
 const auto& theme=palettes[5];
 box(f,{0,-8.8f,-46},{110,.12f,83},{.045f,.16f,.23f},0,25,.11f);
 for(int layer=0;layer<3;++layer)for(int k=0;k<12;++k){
  float x=(k-5.5f)*11.7f+layer*2,z=-49.f-layer*25;
  float height=rng.range(3.5f,6.5f)*(1+layer*.12f);
  add(f,Crystal,translate({x,-9,z})*rz(rng.range(-.19f,.19f))*scale({rng.range(7,11),height,5.f}),{mix(V3{.075f,.17f,.25f},V3{.27f,.36f,.49f},layer*.26f),0,.65f,.20f,24});
 }
 for(int side:{-1,1})for(int k=0;k<6;++k){V3 p{side*(15.f+k*3.5f),-5.4f,-13.f-k*4};
  float height=2.4f+(k%3)*1.5f;
  add(f,Crystal,translate(p)*rz(side*.18f)*scale({.9f,height,1.1f}),{theme.floor,.08f,.20f,.4f,24});
  beam(f,p+V3{0,-height,.8f},p+V3{0,height*.75f,.6f},.045f,k%2?theme.light:theme.secondary,.8f);
  ring(f,translate(p+V3{0,-height,0}),{1.35f,.27f,1.35f},theme.light,.30f);
 }
 for(int k=0;k<30;++k){float x=rng.range(-32,32),z=rng.range(-26,-10),y=std::fmod(rng.range(0,20)+time*.23f,20.f)-4;
  orb(f,{x+.2f*std::sin(time*.4f+k),y,z},{.029f,.029f,.029f},theme.light,.95f);
 }
}

void azure(Frame& f,float time){
 // Broad, sculptural folds carry the blue/cyan wallpaper's silhouette in 3D.
 for(int layer=0;layer<6;++layer){
  V3 color=layer%3==0?V3{.026f,.12f,.66f}:layer%3==1?V3{.018f,.38f,.73f}:V3{.16f,.15f,.66f};
  float sway=.035f*std::sin(time*.17f+layer);
  M4 m=translate({-3.f+layer*2,-10.f+layer*1.7f,-19.f-layer*8})*ry(layer%2?-.28f:.32f)*rz(-.10f+.045f*layer+sway);
  M4 ribbon=m*scale({33.f+layer*2,14.f+layer*2,7.f});
  add(f,Fold,ribbon,{color,0,.25f,.35f,28});
  // A separate fine highlight follows the actual crest rather than a painted line.
  for(int k=0;k<40;++k){float u=k/40.f,v=(k+1)/40.f;
   beam(f,point(ribbon,mineralFold(u,1)),point(ribbon,mineralFold(v,1)),.026f,{.41f,.82f,.97f},.40f);
  }
 }
 for(int side:{-1,1})for(int k=0;k<5;++k){
  V3 p{side*(16.f+k*5),-4.1f+.30f*std::sin(time*.3f+k),-10.f-k*6};
  orb(f,p,{2.2f,.42f,1.45f},k%2?V3{.69f,.76f,.87f}:V3{.19f,.43f,.73f},0,23);
  ring(f,translate(p),{2.3f,.3f,1.51f},{.50f,.79f,.94f},.45f);
 }
}
}

void stageModel(Frame& f,int stage,double time,std::uint64_t seed,bool environment){
 if(stage<0||stage>=StageCount)throw std::runtime_error("Stage art out of range");
 float t=float(std::fmod(time,4096.0));auto surfaces=platforms(stage,time);
 for(int index=0;index<int(surfaces.size());++index)platformArt(f,surfaces[index],index,stage,t);
 if(!environment)return;
 Random rng(seed+713+stage*173);
 switch(stage){
 case 0:terminal(f,t,rng);break;
 case 1:garden(f,t,rng);break;
 case 2:rooftop(f,t,rng);break;
 case 3:bliss(f,t,rng);break;
 case 4:compileClub(f,t,rng);break;
 case 5:aurora(f,t,rng);break;
 case 6:azure(f,t);break;
 }
}
}
