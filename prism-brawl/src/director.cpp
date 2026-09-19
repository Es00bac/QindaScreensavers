// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
namespace sw {
namespace {
const V3 dark{.019f,.029f,.048f};
void line(Frame& f,V3 a,V3 b,float r,V3 color,float emission=0){add(f,Cylinder,segment(a,b,r),{color,emission,.29f,.60f});}
void collar(Frame& f,M4 m,V3 s,V3 color,float e=0){add(f,Torus,m*scale(s),{color,e,.25f,.65f});}
void curve(Frame& f,V3 center,float r,float a0,float a1,V3 co,float emission=1.8f,float thickness=.045f){
 const int n=25;for(int i=0;i<n;i++){float a=a0+(a1-a0)*i/n,b=a0+(a1-a0)*(i+1)/n;
  line(f,center+V3{std::cos(a)*r,std::sin(a)*r,0},center+V3{std::cos(b)*r,std::sin(b)*r,0},thickness,co,emission);}
}
} // namespace
void compose(Frame& f,const BattleState& s,const SceneOptions& opt){
 for(auto& batch:f.batches)batch.instances.clear();
 f.time=s.time;f.motionTime=s.roundTime;f.chapter=s.stage;f.voyage=s.round;f.courseName=stageName(s.stage);
 f.specials.fill("");for(int i=0;i<s.active;++i)if(s.fighters[i].action==Action::Special)f.specials[i]=specialName(s.fighters[i].id);
 f.active=s.active;f.round=s.round+1;f.winner=s.winner>=0?s.fighters[s.winner].id:-1;
 f.secondsLeft=std::max(0,int(std::ceil(92.6-s.roundTime)));f.countdown=s.roundTime<2.6?int(std::ceil(2.6-s.roundTime)):0;
 f.fade=ease(0,.7f,s.roundTime);if(s.finishedAt>=0)f.fade*=1-ease(4.3f,5.45f,s.roundTime-s.finishedAt);
 f.showcase=opt.gallery;
 if(opt.gallery){
  f.chapter=0;
  int id=opt.galleryId;f.focus=id;f.fade=1;f.pengu={0,1,0};f.eye={4.6f,3.2f,7.5f};f.target={0,1.2f,0};f.fov=.53f;f.up={0,1,0};
  if(opt.animationDemo){f.target.y=1.55f;f.fov=.64f;}
  add(f,Cylinder,translate({0,-.25f,0})*scale({2.1f,.25f,2.1f}),{dark,0,.3f,.7f});
  collar(f,translate({0,-.01f,0}),{2.04f,.22f,2.04f},racerColors[id],1.1f);
  Fighter demo;
  if(opt.animationDemo)demo=animationDemo(id,s.time);
  else {demo.id=id;animateFighter(demo,.5f,s.time);}
  M4 rig=translate({0,.72f+demo.y,0})*translate({0,.45f,0})*rz(demo.anim.tumble)*translate({0,-.45f,0})*ry(.12f*std::sin(s.time*.3));
  fighterModel(f,rig,id,s.time,demo.anim);return;
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
  M4 m=translate(pos)*translate({0,.45f,0})*rz(c.anim.tumble)*translate({0,-.45f,0})*ry(c.anim.yaw)*scale({enter,enter,enter});
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
  if(c.action==Action::Jab||c.action==Action::Heavy||c.action==Action::Aerial){
   auto attack=move(c.action,c.id);float t=c.actionTime;
   float stroke=ease(attack.start-.025f,attack.start+.04f,t)*(1-ease(attack.end,attack.end+.13f,t));
   if(stroke>.02f){
    float sweep=clamp((t-attack.start)/(attack.end-attack.start));
    float a0=(c.face>0?-.95f:pi-.95f)+sweep*.7f;
    float radius=c.action==Action::Heavy?1.94f:c.action==Action::Aerial?1.66f:1.50f;
    float height=c.action==Action::Aerial?.90f:1.26f;
    curve(f,{c.x,c.y+height,.55f},radius,a0,a0+1.5f*stroke,co,1.5f*stroke,.018f+.04f*stroke);
    curve(f,{c.x,c.y+height,.54f},radius-.14f,a0-.13f,a0+1.20f*stroke,co,.8f*stroke,.014f);
   }
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
  if(c.reaction==Reaction::Dizzy&&c.reactionTime>0){
   for(int star=0;star<3;++star){
    float orbit=float(s.time)*5+star*2*pi/3;
    V3 p{c.x+.63f*std::cos(orbit),c.y+2.85f+.10f*std::sin(orbit*2),.40f*std::sin(orbit)};
    for(int ray=0;ray<4;++ray){float theta=ray*pi/2+float(s.time)*2;
     line(f,p,p+V3{std::cos(theta)*.14f,std::sin(theta)*.14f,0},.035f,{1,.72f,.22f},1.3f);
    }
   }
  }
  if(c.anim.impact>.1f&&c.launchPower>.4f){
   float r=.25f+(1-c.anim.impact)*.65f;
   curve(f,{c.x-c.hitDirection*.35f,c.y+1.18f,.68f},r,0,2*pi,co,c.anim.impact,.027f*c.anim.impact);
  }
  if(c.action==Action::Special&&c.anim.cast>.05f){float power=c.anim.cast;curve(f,{c.x,c.y+1.2f,.6f},.7f+power*.8f,0,2*pi,co,1.3f,.018f);
   if(c.id==2||c.id==4){for(int k=0;k<4;++k)line(f,{c.x-c.vx*.018f*k,c.y+.3f+k*.35f,-.2f},{c.x-c.vx*.04f*(k+1),c.y+.3f+k*.35f,-.2f},.033f,co,1.1f);}
  }
  if(c.overclock>0){collar(f,translate(pos)*rx(pi/2)*rz(s.time),{1.07f,.06f,1.25f},co,1.2f);}
 }
 for(const auto& p:s.shots){
  if(p.id==0||p.id==6){V3 co=racerColors[p.id];for(int k=0;k<3;++k){float r=.42f+k*.24f;collar(f,translate(p.p+V3{-p.v.x*.014f*k,0,0})*rz(pi/2),{r,.4f,r},co,1.4f);}
   if(p.id==6){for(int k=0;k<5;++k)add(f,Monolith,translate(p.p+V3{-.18f*k,.14f*float(k%2),0})*rz(s.time*7+k)*scale({.13f,.24f,.13f}),{co,1.2f,.4f,.3f});}
   continue;}
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
  }else if(e.kind==7){
   // Low, spreading dust settles at the actual platform contact point.
   for(int j=0;j<6;++j){float side=j%2?1.f:-1.f,spread=.20f+k*(.65f+j*.12f),r=(.10f+.17f*k)*(1-k);
    add(f,Sphere,translate(e.p+V3{side*spread,.05f+std::sin(k*pi)*.16f,(j/2-1)*.17f})*scale({r*1.7f,r*.55f,r}),{mix({.35f,.40f,.46f},co,.20f),.05f,.95f,0});
   }
  }else if(e.kind==8){
   // A world-space trail survives the tumbling actor moving away from it.
   float r=(.13f+.28f*k)*(1-k);
   add(f,Sphere,translate(e.p)*scale({r,r*.82f,r*.8f}),{mix({.29f,.33f,.41f},co,.23f),.06f*(1-k),.95f,0});
   if(k<.45f)line(f,e.p,e.p+e.v*.6f,.019f*(1-k),co,.75f*(1-k));
  }else if(e.kind==6){
   for(int j=0;j<8;++j){float angle=j*pi/4+.13f,inner=.10f+k*.45f,outer=(j%2?.66f:1.05f)*(1+k*.6f);
    V3 dir{std::cos(angle),std::sin(angle),0};
    line(f,e.p+dir*inner,e.p+dir*outer,.05f*(1-k)+.005f,j%2?co:V3{1,.81f,.43f},1.7f*(1-k));
   }
   curve(f,e.p,.18f+k*1.25f,0,2*pi,co,1.1f*(1-k),.035f*(1-k)+.002f);
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
