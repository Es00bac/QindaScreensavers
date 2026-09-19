// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
namespace sw {
namespace {
const V3 dark{.019f,.029f,.048f},steel{.17f,.24f,.32f};
V3 prism(float h){return {.50f+.46f*std::cos(h*2*pi),.51f+.45f*std::cos((h-.333f)*2*pi),.54f+.44f*std::cos((h+.333f)*2*pi)};}
void line(Frame& f,V3 a,V3 b,float r,V3 color,float emission=0){add(f,Cylinder,segment(a,b,r),{color,emission,.29f,.60f});}
void panel(Frame& f,V3 p,V3 s,V3 color,float emission=0,float mode=0){add(f,Box,translate(p)*scale(s),{color,emission,.30f,.55f,mode});}
void collar(Frame& f,M4 m,V3 s,V3 color,float e=0){add(f,Torus,m*scale(s),{color,e,.25f,.65f});}
void curve(Frame& f,V3 center,float r,float a0,float a1,V3 co,float emission=1.8f,float thickness=.045f){
 const int n=25;for(int i=0;i<n;i++){float a=a0+(a1-a0)*i/n,b=a0+(a1-a0)*(i+1)/n;
  line(f,center+V3{std::cos(a)*r,std::sin(a)*r,0},center+V3{std::cos(b)*r,std::sin(b)*r,0},thickness,co,emission);}
}
} // namespace
void stageModel(Frame& f,int stage,double time,std::uint64_t seed,bool environment){
 float t=std::fmod(time,4096.);auto ps=platforms(stage,time);
 V3 accent=stage==0?V3{.23f,.84f,.91f}:stage==1?V3{.32f,.90f,.55f}:V3{.96f,.27f,.54f};
 // The top of every visible slab is shared with its one-way collision surface.
 for(int i=0;i<4;i++){
  const auto& p=ps[i];float d=p.depth,th=i==0?.64f:.30f;
  panel(f,{p.x,p.y-th-.09f,0},{p.half,th,d},dark);
  panel(f,{p.x,p.y-.075f,0},{p.half-.16f,.075f,d-.14f},i?steel:V3{.16f,.23f,.32f},0,i?0:19);
  panel(f,{p.x,p.y-th*1.7f,0},{p.half*.82f,.19f,d*.86f},steel);
  for(int side:{-1,1}){
   line(f,{p.x-p.half+.3f,p.y-.12f,side*(d-.02f)},{p.x+p.half-.3f,p.y-.12f,side*(d-.02f)},i?.045f:.064f,accent,1.7f);
   for(int k=0;k<(i?5:13);k++){
    float x=p.x-p.half+.6f+(2*p.half-1.2f)*k/(i?4:12);
    panel(f,{x,p.y-th*.9f,side*(d+.01f)},{.20f,.12f,.055f},steel);
    panel(f,{x,p.y-th*.9f,side*(d+.074f)},{.13f,.027f,.012f},prism(k*.095f+i*.2f),1.2f);
   }
  }
  for(int side:{-1,1}){
   line(f,{p.x+side*(p.half-.03f),p.y-.10f,-d+.25f},{p.x+side*(p.half-.03f),p.y-.10f,d-.25f},.055f,accent,1.4f);
   panel(f,{p.x+side*(p.half-.20f),p.y-.42f,0},{.18f,.2f,d*.9f},steel);
  }
  if(i){
   add(f,Cylinder,translate({p.x,p.y-.77f,0})*scale({.55f,.31f,.55f}),{dark,0,.35f,.65f});
   collar(f,translate({p.x,p.y-.97f,0}),{.49f,.32f,.49f},accent,2.0f);
   add(f,Sphere,translate({p.x,p.y-1.07f,0})*scale({.31f,.075f,.31f}),{accent,2,.3f,.2f});
  }
 }
 // A quiet central prism insignia and etched circuits are physical stage details.
 for(int j=0;j<3;j++){
  float z=-2.6f+j*2.1f;
  line(f,{-1.45f,.016f,z},{0,.016f,z+1.13f},.028f,accent,.65f);
  line(f,{0,.016f,z+1.13f},{1.45f,.016f,z},.028f,accent,.65f);
 }
 for(int side:{-1,1})for(int j=0;j<6;j++){
  float x=side*(3.0f+j*1.2f);
  line(f,{x,.019f,-3},{x,.019f,-2.1f},.018f,steel,.3f);
  line(f,{x,.019f,2.1f},{x+.4f*side,.019f,2.6f},.016f,accent,.4f);
 }
 if(!environment)return;
 // A mechanical undercarriage below the fighting plane, never an extra invisible wall.
 add(f,Hull,translate({0,-2.0f,0})*scale({3.1f,2.2f,2.5f}),{dark,0,.31f,.7f});
 for(int side:{-1,1}){
  line(f,{side*9.4f,-.8f,2.0f},{side*5.9f,-2.4f,1.3f},.19f,steel);
  collar(f,translate({side*7.f,-2.0f,0}),{1.25f,.40f,1.25f},accent,1.2f);
 }
 Random rng(seed+713+stage*173);
 // A layered city with small architectural lights, not a wall of noisy props.
 for(int i=0;i<30;i++){
  float x=(i-14.5f)*6.5f+rng.range(-1,1),z=-49-rng.range(0,34),height=rng.range(6,20);
  V3 pos{x,-20+height*.5f,z};
  panel(f,pos,{rng.range(1.4f,2.8f),height*.5f,rng.range(1.3f,2.5f)},{.024f,.033f,.076f},0,14);
  panel(f,pos+V3{0,height*.5f+.08f,0},{1.2f,.1f,1.2f},prism(i*.18f)*.3f,.8f);
  if(i%3==0)line(f,pos+V3{0,height*.5f,0},pos+V3{0,height*.5f+2.5f,0},.055f,accent,.8f);
 }
 if(stage==0){
  // Architectural halo and drifting prism shards, spatially behind the platforms.
  M4 m=translate({0,4.1f,-17.5f})*rx(pi/2);
  collar(f,m,{11.8f,1.1f,11.8f},steel);
  collar(f,m*translate({0,-.12f,0}),{11.62f,.23f,11.62f},{.51f,.25f,.94f},1.35f);
  collar(f,m*ry(t*.022f),{12.25f,.22f,12.25f},accent,.7f);
  for(int k=0;k<12;k++){
   float a=k*pi/6+t*.024f;
   V3 pos{std::cos(a)*13.2f,4.1f+std::sin(a)*13.2f,-17.5f};
   add(f,Monolith,translate(pos)*rz(a)*ry(t*.07f)*scale({.48f,.69f,.34f}),{prism(k/12.f)*.27f,0,.22f,.8f,5});
   add(f,Sphere,translate(pos+V3{0,0,.37f})*scale({.095f,.095f,.06f}),{prism(k/12.f),1.6f,.3f,0});
  }
 }else if(stage==1){
  // The garden is a ring of bioluminescent, circuit-veined glass trees.
  for(int j=0;j<8;j++){
   float x=(j-3.5f)*5.0f,z=-13.f-std::abs(x)*.30f,top=3.5f+rng.range(0,7);
   line(f,{x,-8,z},{x,top,z},.16f,steel);
   for(int k=0;k<4;k++){
    float a=k*pi*.5f+j*.8f;
    V3 tip{x+std::cos(a)*2.0f,top+.5f+std::sin(t*.2f+j)*.20f,z+std::sin(a)*1.6f};
    line(f,{x,top-2,z},tip,.055f,accent,.8f);
    add(f,Monolith,translate(tip)*rz(.2f*std::sin(t*.2f+j))*scale({.48f,.67f,.35f}),{{.045f,.19f,.17f},.12f,.12f,.6f,5});
    collar(f,translate(tip)*rx(.4f),{.67f,.10f,.67f},{.30f,.78f,.51f},1.15f);
   }
  }
  for(int j=0;j<32;j++){
   V3 p{rng.range(-22,22),rng.range(-3,14)+.5f*std::sin(t*.29f+j),rng.range(-24,-8)};
   add(f,Sphere,translate(p)*scale({.035f,.035f,.035f}),{accent,2,.4f,0});
  }
 }else{
  // Rooftop air-conditioning, aerial pylons and traffic lanes; no cosmic halo here.
  for(int side:{-1,1}){
   V3 p{side*18.f,-1,-15};panel(f,p,{2.8f,7,2.8f},dark,0,14);
   for(int j=0;j<3;j++){
    collar(f,translate(p+V3{0,5.f+j*1.3f,0}),{3.2f,.7f,3.2f},j%2?accent:steel,j%2?.8f:0);
   }
   line(f,p+V3{0,8,0},p+V3{0,13,0},.11f,steel);
   add(f,Sphere,translate(p+V3{0,13,0})*scale({.15f,.15f,.15f}),{accent,1.5f,.4f,0});
  }
  for(int j=0;j<9;j++){
   float x=std::fmod(t*(2.4f+j*.15f)+j*8,75.f)-37.5f;
   V3 p{x,-2.f+j*.52f,-25.f-j*1.6f};
   add(f,Hull,translate(p)*scale({.27f,.4f,.4f}),{steel,0,.25f,.8f});
   line(f,p+V3{-.8f,0,0},p+V3{-2.7f,0,0},.027f,accent,1.2f);
  }
 }
}
void compose(Frame& f,const BattleState& s,const SceneOptions& opt){
 for(auto& batch:f.batches)batch.instances.clear();
 f.time=s.time;f.motionTime=s.roundTime;f.chapter=s.stage;f.voyage=s.round;f.courseName=stageName(s.stage);
 f.specials.fill("");for(int i=0;i<s.active;++i)if(s.fighters[i].action==Action::Special)f.specials[i]=specialName(s.fighters[i].id);
 f.active=s.active;f.round=s.round+1;f.winner=s.winner>=0?s.fighters[s.winner].id:-1;
 f.secondsLeft=std::max(0,int(std::ceil(92.6-s.roundTime)));f.countdown=s.roundTime<2.6?int(std::ceil(2.6-s.roundTime)):0;
 f.fade=ease(0,.7f,s.roundTime);if(s.finishedAt>=0)f.fade*=1-ease(4.3f,5.45f,s.roundTime-s.finishedAt);
 f.showcase=opt.gallery;
 if(opt.gallery){
  int id=opt.galleryId;f.focus=id;f.fade=1;f.pengu={0,1,0};f.eye={4.6f,3.2f,7.5f};f.target={0,1.2f,0};f.fov=.53f;f.up={0,1,0};
  add(f,Cylinder,translate({0,-.25f,0})*scale({2.1f,.25f,2.1f}),{dark,0,.3f,.7f});
  collar(f,translate({0,-.01f,0}),{2.04f,.22f,2.04f},racerColors[id],1.1f);
  Anim a;a.yaw=.06f;fighterModel(f,translate({0,.72f,0})*ry(.12f*std::sin(s.time*.3)),id,s.time,a);return;
 }
 f.pengu={0,2,0};f.target=opt.camera==Camera::Fixed||opt.reduced?V3{0,3.1f,0}:s.cameraTarget;
 float z=opt.camera==Camera::Fixed||opt.reduced?37:s.cameraDistance;
 if(opt.camera==Camera::Close)z*=.87f;
 f.eye=f.target+V3{opt.reduced?0.f:1.2f*float(std::sin(s.time*.028)),7.8f,z};f.fov=.63f;f.up={0,1,0};
 stageModel(f,s.stage,s.roundTime,777+s.round*79);
 for(int i=0;i<s.active;i++){
  const auto& c=s.fighters[i];f.roster[i]=c.id;f.damage[i]=c.damage;f.stocks[i]=c.stocks;f.kos[i]=c.kos;f.shield[i]=c.shield;
  if(c.stocks<=0||c.respawn>0)continue;
  V3 pos{c.x,c.y+.72f,.11f*(i%2?-1:1)};
  float enter=c.invulnerable>2.4f?ease(2.8f,2.4f,c.invulnerable):1.f;
  enter=std::max(.04f,enter);
  M4 m=translate(pos)*ry(c.anim.yaw)*scale({enter,enter,enter});
  fighterModel(f,m,c.id,s.roundTime,c.anim);
  V3 co=racerColors[c.id];
  // Light-footprint ring makes the combat plane and fighter color immediately legible.
  if(c.floor>=0)collar(f,translate({c.x,c.y+.025f,0}),{.65f,.17f,.38f},co,.85f);
  if(c.anim.shield>.04f){
   float r=1.40f*c.anim.shield;
   add(f,Sphere,translate({c.x,c.y+1.25f,.05f})*scale({r,r*1.1f,r*.88f}),{co,.7f,.2f,.1f,18});
   collar(f,translate({c.x,c.y+1.25f,.05f})*rx(pi/2),{r,.15f,r*1.1f},co,1.0f);
  }
  if(c.invulnerable>.15f){
   collar(f,translate({c.x,c.y+.1f,0}),{.86f,.16f,.65f},co,.65f);
   if(c.invulnerable>1.8f){for(int j=0;j<3;j++)collar(f,translate({c.x,c.y+.25f+j*.9f,0})*rx(.08f*std::sin(s.time+j)),{.72f,.06f,.72f},co,.65f);}
  }
  float atk=std::max({c.anim.punch,c.anim.heavy,c.anim.aerial});
  if(atk>.2f&&c.action!=Action::Hurt){
   float a0=c.face>0?-.75f:pi-.75f;float a1=a0+1.7f*atk;
   curve(f,{c.x,c.y+1.18f,.5f},1.45f+(.45f*c.anim.heavy),a0,a1,co,1.2f*atk,.026f+.022f*c.anim.heavy);
  }
  if(c.action==Action::Recovery||c.anim.recover>.1f){
   for(int side:{-1,1}){
    V3 p=point(m,{side*.28f,-.58f,0});
    line(f,p,p+V3{-c.vx*.04f,-.9f*c.anim.recover,0},.06f,co,2);
   }
  }
  if(c.stun>0&&std::abs(c.vx)>8){
   V3 trail{clamp(-c.vx*.09f,-2,2),-c.vy*.06f,0};
   for(int j=0;j<4;j++){float k=(j+1)/4.f;add(f,Sphere,translate({c.x+trail.x*k,c.y+1.2f+trail.y*k,-.15f})*scale({.09f*(1-k*.7f),.09f*(1-k*.7f),.09f}),{co,1.5f*(1-k*.6f),.4f,0});}
  }
  if(c.action==Action::Special&&c.anim.cast>.05f){float power=c.anim.cast;curve(f,{c.x,c.y+1.2f,.6f},.7f+power*.8f,0,2*pi,co,1.3f,.018f);
   if(c.id==2||c.id==4){for(int k=0;k<4;++k)line(f,{c.x-c.vx*.018f*k,c.y+.3f+k*.35f,-.2f},{c.x-c.vx*.04f*(k+1),c.y+.3f+k*.35f,-.2f},.033f,co,1.1f);}
  }
  if(c.overclock>0){collar(f,translate(pos)*rx(pi/2)*rz(s.time),{1.07f,.06f,1.25f},co,1.2f);}
 }
 for(const auto& p:s.shots){
  if(p.id==0||p.id==6){V3 co=racerColors[p.id];for(int k=0;k<3;++k){float r=.42f+k*.24f;collar(f,translate(p.p+V3{-p.v.x*.014f*k,0,0})*rz(pi/2),{r,.4f,r},co,1.4f);}
   if(p.id==6)for(int k=0;k<5;++k)add(f,Monolith,translate(p.p+V3{-.18f*k,.14f*float(k%2),0})*rz(s.time*7+k)*scale({.13f,.24f,.13f}),{co,1.2f,.4f,.3f});continue;}
  if(p.id==3){M4 m=translate(p.p)*rz(s.time*14);for(int k=0;k<3;++k){float a=k*2*pi/3;line(f,point(m,{0,0,0}),point(m,{std::cos(a)*.65f,std::sin(a)*.65f,0}),.07f,racerColors[p.id],1.3f);}continue;}
  if(p.id==5){add(f,Sphere,translate(p.p)*scale({.42f,.42f,.42f}),{{.06f,.008f,.12f},.2f,.12f,.7f});for(int k=0;k<3;++k)collar(f,translate(p.p)*rx(k*.8f)*rz(s.time*3+k),{.67f,.12f,.67f},racerColors[p.id],1.5f);continue;}
  if(p.id==7){add(f,Sphere,translate(p.p)*scale({.6f,.6f,.6f}),{racerColors[7],.6f,.12f,.2f,18});collar(f,translate(p.p)*rx(pi/2),{.6f,.15f,.6f},racerColors[7],1);continue;}
  V3 co=racerColors[p.id];V3 tail=p.p-unit(p.v)*1.1f;
  line(f,tail,p.p,.06f,co,2.3f);
  add(f,Sphere,translate(p.p)*scale({p.radius,p.radius*.7f,p.radius*.7f}),{co,2.7f,.2f,.1f});
  collar(f,translate(p.p)*rz(pi/2)*rx(s.time*3),{p.radius*1.32f,.16f,p.radius*1.32f},co,1.6f);
 }
 for(const auto& item:s.pickups){
  V3 co=item.kind==0?V3{.35f,1,.61f}:item.kind==1?V3{1,.63f,.17f}:V3{.32f,.6f,1};
  V3 pos{item.x,item.y+.16f*float(std::sin(s.time*2)),0};
  add(f,Box,translate(pos)*ry(s.time*.9)*scale({.32f,.32f,.32f}),{{.035f,.065f,.10f},0,.22f,.6f});
  if(item.kind==0){line(f,pos+V3{-.21f,0,.36f},pos+V3{.21f,0,.36f},.07f,co,1.6f);line(f,pos+V3{0,-.21f,.36f},pos+V3{0,.21f,.36f},.07f,co,1.6f);}
  else if(item.kind==1){line(f,pos+V3{-.14f,-.22f,.36f},pos+V3{.12f,.02f,.36f},.065f,co,1.7f);line(f,pos+V3{.12f,.02f,.36f},pos+V3{0,.23f,.36f},.065f,co,1.7f);}
  else collar(f,translate(pos+V3{0,0,.36f})*rx(pi/2),{.23f,.3f,.26f},co,1.6f);
  collar(f,translate(pos)*rx(.4f),{.48f,.08f,.48f},co,1.5f);
 }
 for(const auto& e:s.effects){
  float k=e.age/e.life,alpha=std::sin(pi*clamp(k));V3 co=racerColors[e.id];
  if(e.kind==2){
   // A bounded ring-out burst, not a white flash over the whole display.
   curve(f,e.p,.5f+k*2.8f,0,2*pi,co,2*(1-k),.1f*(1-k)+.005f);
   for(int j=0;j<16;j++){float a=j*pi/8;V3 p=e.p+V3{std::cos(a),std::sin(a),0}*(.2f+k*3.4f);add(f,Monolith,translate(p)*rz(a)*scale({.08f,.17f*(1-k)+.005f,.07f}),{co,1.5f*(1-k),.3f,0});}
  }else if(e.kind==4){
   collar(f,translate(e.p)*rx(pi/2),{.2f+k*.8f,.13f,.2f+k*.8f},co,1.3f*(1-k));
  }else{
   for(int j=0;j<9;j++){float a=j*2*pi/9+e.kind*.28f;V3 delta{std::cos(a),std::sin(a),.04f};
    line(f,e.p+delta*(.15f+k*.4f),e.p+delta*(.35f+k*1.2f),.024f*(1-k)+.004f,co,2.0f*(1-k));}
   add(f,Sphere,translate(e.p)*scale({.12f*alpha+.005f,.12f*alpha+.005f,.07f*alpha+.005f}),{{.7f,.9f,1},2*(1-k),.3f,0});
  }
 }
}
}
