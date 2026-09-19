// SPDX-License-Identifier: GPL-3.0-or-later
#include "reef/renderer.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <numeric>
#include <sstream>
namespace reef {
Renderer::Renderer(std::uint64_t seed,std::string scheme,int density):world_(seed,density),p_(palette(scheme)){}
static std::string value(std::optional<double> v,int digits=0){if(!v||!std::isfinite(*v))return "--";std::ostringstream s;s<<std::fixed<<std::setprecision(digits)<<*v;return s.str();}
static Vec3 rotateFish(Vec3 p,double yaw,double pitch){double x=p.x,y=p.y+pitch*p.x;return{x*std::cos(yaw)-p.z*std::sin(yaw),y,x*std::sin(yaw)+p.z*std::cos(yaw)};}
static Vec2 project(Vec3 p){return{p.x,p.y-p.z*.10};}
void Renderer::background(Canvas& c){
 const double W=1600*layout_;
 c.path();c.move(0,0);c.line(W,0);c.line(W,900);c.line(0,900);c.close();c.linear(0,0,W*.7,900,p_.water.light(.92),p_.deep);
 c.glow(W*.28,300,700,p_.glow.alpha(.052));c.glow(W*.83,210,480,p_.pink.alpha(.065));
 // Diffuse shafts through the aquarium glass.
 for(int i=0;i<7;i++){
  double x=(120+i*231)*layout_;double sw=45+(i%3)*24;
  c.path();c.move(x,-10);c.line(x+sw,-10);c.line(x+340,845);c.line(x+140,845);c.close();
  c.linear(x,0,x+220,800,p_.glow.alpha(.027),p_.glow.alpha(0));
 }
 // The broken, plant-covered circuit city sits behind the living reef.
 Rng r(world_.seed^0xb14e18ULL);
 for(int i=0;i<37;i++){
  double x=r.between(-30,1630)*layout_,y=r.between(610,785),w=r.between(16,64),h=840-y;
  c.roundRect(x,y,w,h,4,blend(p_.deep,p_.water,.4));
  c.path();c.move(x+3,y+h);c.line(x+3,y+3);c.line(x+w-5,y+3);c.stroke(p_.glow.alpha(.075),1.1);
  for(int j=0;j<int(h/16);j++){
   c.rect(x+8,y+10+j*16,w-16,3,p_.glow.alpha(r.between(.015,.06)));
   if(j%4==0)c.ellipse(x+w-7,y+12+j*16,1.4,1.4,p_.gold.alpha(.3));
  }
 }
 // Faint glass arch. No bright fixed borders or opaque dashboard.
 c.save();c.translate(W*.5,520);c.scale(W/1640,1);
 c.path();c.move(-808,500);c.curve(-865,-490,865,-490,808,500);c.stroke(p_.glow.alpha(.048),3);
 c.path();c.move(-780,500);c.curve(-831,-454,831,-454,780,500);c.stroke(p_.glow.alpha(.033),1);c.restore();
 // Seafloor silhouette and organic layered shelves.
 for(int i=0;i<4;i++){
  double y=773+i*32;c.path();c.move(-80,y);
  c.curve(W*.21,y-80+i*12,W*.27,y+67,W*.50,y+12);
  c.curve(W*.71,y-65,W*.87,y+56,W+80,y-32);c.line(W+80,960);c.line(-80,960);c.close();
  c.fill(blend(p_.water,p_.deep,.53+i*.12));
 }
 // Dark faceted basalt and embedded copper traces.
 for(int i=0;i<35;i++){
  double x=r.between(-30,1630)*layout_,y=r.between(815,928),w=r.between(24,107),h=r.between(14,62);
  c.polygon({{x-w,y},{x-w*.7,y-h*.6},{x-w*.15,y-h},{x+w*.75,y-h*.7},{x+w,y},{x,y+h*.2}},blend(p_.ink,p_.deep,r.between(.1,.7)),p_.glow.alpha(.05),.7);
  if(i%3==0){c.path();c.move(x-w*.6,y-h*.48);c.line(x,y-h*.75);c.line(x+w*.25,y-h*.3);c.stroke(p_.glow.alpha(.09),1);}
 }
 // Memory reservoir pedestal, mostly submerged in shadow.
 c.ellipse(W*.52,827,170,25,p_.deep.alpha(.9));
 c.save();c.translate(W*.52,815);c.scale(1,.17);c.ring(0,0,157,p_.glow.alpha(.10),3);c.ring(0,0,137,p_.glow.alpha(.06),2);c.restore();
 for(const auto& plantSpec:world_.plants)if(plantSpec.type==3)plant(c,plantSpec,0,false);
}
void Renderer::plant(Canvas& c,const Plant& p,double t,bool front)const{
 const double x=p.x*layout_,y=p.y;const double opacity=front?.70:.22;
 const auto col=(p.type==1?p_.pink:(p.type==2?p_.gold:p_.glow));
 c.save();c.translate(x,y);
 if(p.type==3){
  // Circuit coral is a branching fan with glowing terminal polyps.
  for(int j=0;j<11;j++){
   double a=-pi*.5+(j-5)*.14, l=p.height*(.65+.22*std::sin(j*1.7+p.phase));
   double ex=std::cos(a)*l,ey=std::sin(a)*l;
   c.path();c.move(0,0);c.curve(ex*.04,ey*.35,ex*.75,ey*.4,ex,ey);c.stroke(col.alpha(opacity*.5),2.0);
   for(int k=1;k<4;k++){
    double s=.25+k*.16;double bx=ex*s*s,by=ey*s;
    double tx=bx+(j<5?-1:1)*(13+k*5),ty=by-8-k*4;
    c.path();c.move(bx,by);c.line(tx,ty);c.stroke(col.alpha(opacity*.32),1.2);
    c.ellipse(tx,ty,1.6,1.6,col.alpha(opacity*.75));
   }
   c.glow(ex,ey,7,col.alpha(opacity*.16));c.ellipse(ex,ey,2,2,col.alpha(opacity));
  }
 }else if(p.type==2){
  // Fiber-optic anemone, each filament lags the current.
  for(int j=0;j<8;j++){
   double phase=t*.45+p.phase+j*.6;
   double ex=(j-3.5)*p.width*.3+std::sin(phase)*11;
   double ey=-p.height*(.65+.28*std::sin(j*.65+1));
   c.path();c.move(0,0);c.curve(ex*.06,-p.height*.28,ex-15*std::sin(phase-.5),ey*.75,ex,ey);c.stroke(col.alpha(opacity*.34),1.8);
   c.glow(ex,ey,11,col.alpha(opacity*.16));c.ellipse(ex,ey,2.7,2.7,col.alpha(opacity*.88));
  }
 }else{
  // Ribbon kelp. The root stays fixed while curvature travels toward the tip.
  for(int j=0;j<4;j++){
   double ph=t*.36+p.phase+j*.9;double ex=(j-1.5)*p.width*.47+15*std::sin(ph);
   double ey=-p.height*(.62+j*.13);double mid=ex*.45+17*std::sin(ph-.7);
   c.path();c.move((j-1.5)*3,0);c.curve(mid-11,ey*.33,ex-15,ey*.73,ex,ey);
   c.curve(ex+5,ey*.75,mid+8,ey*.36,(j-1.5)*3,0);c.close();
   c.linear(0,0,ex,ey,col.alpha(opacity*.055),col.alpha(opacity*.38));
   c.path();c.move((j-1.5)*3,0);c.curve(mid,ey*.33,ex-4,ey*.75,ex,ey);c.stroke(col.alpha(opacity*.5),.9);
  }
 }
 c.restore();
}
void Renderer::water(Canvas& c,double t,bool front)const{
 Rng r(world_.seed^(front?0xc73ULL:0xbb273ULL));const double W=1600*layout_;
 // Infinite particles wrap only while invisible outside the viewport.
 const int count=front?43:95;
 for(int i=0;i<count;i++){
  double x0=r.between(-20,W+20),y0=r.between(0,1020),speed=r.between(3,13),phase=r.between(0,tau);
  double y=960-std::fmod(y0+t*speed,1040.0);double x=x0+12*std::sin(t*.18+phase)+7*std::sin(y*.009+phase);
  double a=smooth((y+40)/60)*smooth((925-y)/80)*(front?.34:.19);
  double radius=r.between(.6,front?2.1:1.3);auto col=(i%5==0?p_.gold:p_.glow).alpha(a);
  if(front&&i%4==0)c.glow(x,y,8,col.alpha(.4));
  c.ellipse(x,y,radius,radius,col);
 }
 // Air escapes from vents in loose columns, with refraction rims and specular glints.
 for(int i=0;i<(front?22:28);++i){
  double phase=i*73.91,velocity=18+(i%7)*3.4;
  double y=1000-std::fmod(t*velocity+phase,1160.0);
  double x=((i%5)*329+80)*layout_+22*std::sin(t*.42+i)+9*std::sin(y*.028+i);
  double radius=2.5+(i%5)*1.35;double a=smooth((y+100)/80)*smooth((980-y)/80)*(front?.46:.23);
  c.ellipse(x,y,radius,radius,p_.water.alpha(a*.2));c.ring(x,y,radius,p_.glow.alpha(a),.85);
  c.path();c.arc(x-1,y-1,radius*.67,pi*1.06,pi*1.55);c.stroke(rgb(0xe9ffff,a*.95),1.2);
 }
 if(!front){
  // Slowly moving surface caustics, not a rapidly flashing overlay.
  for(int i=0;i<9;i++){
   double yy=22+i*8;c.path();c.move(-30,yy);
   for(int k=0;k<19;k++){double x=k*W/17;c.line(x,yy+9*std::sin(k*.53+t*.15+i));}
   c.stroke(p_.glow.alpha(.017),1.1);
  }
  // Three schools of little glass minnows.
  for(int s=0;s<3;s++)for(int j=0;j<8;j++){
   double a=t*(.075+s*.013)+s*2.3;
   double x=(800+560*std::sin(a))*layout_+(j%4)*23;
   double y=165+s*145+(j/4)*13+7*std::sin(t*.6+j);
   double yaw=std::atan2(-.45*std::sin(a),std::cos(a));double sx=std::cos(yaw);
   c.save();c.translate(x,y);c.scale(sx,1);
   c.path();c.move(8,0);c.curve(1,-4,-6,-4,-9,0);c.curve(-6,4,1,4,8,0);c.close();c.fill(p_.glow.alpha(.12));
   c.polygon({{-8,0},{-15,-4},{-13,0},{-15,4}},p_.glow.alpha(.09));c.restore();
  }
 }
}
void Renderer::fishArt(Canvas& c,const FishPose& f,int style,double opacity)const{
 c.save();c.translate(f.p.x,f.p.y);c.scale(f.scale,f.scale);
 const auto neon=(style==2?p_.pink:(style==0?p_.gold:p_.glow));
 c.glow(0,0,99,neon.alpha(.035*opacity));
 struct Face{std::vector<Vec2> points;double depth;Color col,edge;double stroke;};std::vector<Face> faces;faces.reserve(370);
 auto transform=[&](Vec3 p){return rotateFish(p,f.yaw,f.pitch);};
 auto add=[&](const std::vector<Vec3>& verts,Color col,Color edge={},double stroke=0){Face item{{},0,col.alpha(opacity),edge.alpha(opacity),stroke};for(auto v:verts){auto p=transform(v);item.points.push_back(project(p));item.depth+=p.z;}item.depth/=double(verts.size());faces.push_back(std::move(item));};
 // Smooth, closed Catmull-Rom fin contours; their control points still
 // deform in three dimensions with the same continuous swimming phase.
 auto silk=[&](const std::vector<Vec3>& controls,Color col,Color edge,double stroke){
  std::vector<Vec3> outline;outline.reserve(controls.size()*5);
  for(std::size_t j=0;j<controls.size();j++){
   auto p0=controls[(j+controls.size()-1)%controls.size()],p1=controls[j];
   auto p2=controls[(j+1)%controls.size()],p3=controls[(j+2)%controls.size()];
   for(int k=0;k<5;k++){
    double u=k/5.0;
    outline.push_back((p1*2+(p2-p0)*u+(p0*2-p1*5+p2*4-p3)*(u*u)+(p0*(-1)+p1*3-p2*3+p3)*(u*u*u))*.5);
   }
  }
  add(outline,col,edge,stroke);
 };
 // Caudal fin: a connected pair of flexible lobes, never a whole-sprite flip.
 const double bend=std::sin(f.phase-5.8)*17;
 for(int side:{-1,1}){
  std::vector<Vec3> tail;
  tail.push_back({-78,side*3.0,bend});
  for(int k=0;k<=8;k++){
   double s=k/8.0;double x=-80-64*s;double y=side*(4+34*std::sin(s*pi*.5));
   double z=bend+std::sin(f.phase-5.8-s*2.8)*(7+18*s);
   tail.push_back({x,y,z});
  }
  tail.push_back({-118,side*3.0,bend+std::sin(f.phase-7.1)*18});
  silk(tail,neon.alpha(.25),neon.alpha(.51),.9);
  for(int j=0;j<4;j++){
   double a=.35+j*.2;std::vector<Vec3> ray;
   ray.push_back({-79,side*2.0,bend});
   ray.push_back({-108,side*(10+j*4.0),bend+std::sin(f.phase-6.5)*16});
   ray.push_back({-80-64*a,side*(4+34*std::sin(a*pi*.5)),bend+std::sin(f.phase-5.8-a*2.8)*(7+18*a)});
   add(ray,neon.alpha(.025),neon.alpha(.22),.6);
  }
 }
 // Dorsal and paired pectoral fins move independently with a phase lag.
 silk({{40,-18,0},{10,-43,4*std::sin(f.phase)},{-38,-35,8*std::sin(f.phase-.8)},{-50,-9,5}},neon.alpha(.23),neon.alpha(.46),.75);
 for(int side:{-1,1}){
  double sway=std::sin(f.phase*.63+.6)*8;
  silk({{30,10,side*12.0},{18,34,side*31.0},{-24,48+sway,side*(37+sway)},{-14,21,side*17.0},{7,12,side*13.0}},neon.alpha(.24),neon.alpha(.52),.8);
  silk({{-27,13,side*9.0},{-52,34,side*27.0},{-57,22,side*13.0}},neon.alpha(.22),neon.alpha(.4),.7);
 }
 const int along=20,around=18;
 for(int i=0;i<along;i++)for(int j=0;j<around;j++){
  double u0=i/double(along),u1=(i+1)/double(along),u=(u0+u1)*.5,a0=j*tau/around,a1=(j+1)*tau/around,a=(a0+a1)*.5;
  auto normalLocal=normal(Vec3{.5*std::cos(pi*u),std::cos(a),std::sin(a)});
  auto n=transform(normalLocal);if(n.z<-.22)continue;
  Color body;
  if(style==0){body=rgb(0xe9e6d3);if((u<.3&&std::cos(a)<.25)||(u>.45&&u<.7&&std::sin(a)>.05))body=rgb(0xe18d56);if(u>.33&&u<.47&&std::cos(a)<-.65)body=rgb(0x193a43);}
  else if(style==1){body=rgb(0x235168);if(u<.35)body=rgb(0x416779);if(u>.46&&u<.6)body=rgb(0xbbcebd);}
  else{body=rgb(0xd6d7e2);if(u<.2||(u>.45&&u<.63))body=rgb(0xb684bd);}
  const double light=.46+.56*clamp(n.x*.15-n.y*.68+n.z*.75);
  body=body.light(light);if(i%5==0)body=blend(body,neon,.13);
  std::vector<Vec3> vertices={world_.fishLocal(u0,a0,f.phase),world_.fishLocal(u0,a1,f.phase),world_.fishLocal(u1,a1,f.phase),world_.fishLocal(u1,a0,f.phase)};
  // Thin panel joints are intentional robot armor, not transparent mesh cracks.
  add(vertices,body,body,.42);
 }
 std::stable_sort(faces.begin(),faces.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});
 for(const auto& face:faces)c.polygon(face.points,face.col,face.edge,face.stroke);
 // Etched circuitry on the visible flank, warped with the same body surface.
 for(int side:{-1,1}){
  double facing=side*std::cos(f.yaw);if(facing<=0)continue;
  double a=side*pi*.5;
  for(int band:{5,10,14}){
   c.path();for(int k=0;k<13;k++){
    double aa=a-.58+k*.096;auto p=project(transform(world_.fishLocal(band/20.0,aa,f.phase)));if(k==0)c.move(p);else c.line(p);
   }c.stroke(p_.deep.alpha(.50*facing*opacity),1.3);
  }
  c.path();for(int k=0;k<=13;k++){
   auto pp=project(transform(world_.fishLocal(.28+k*.033,a-.15+std::sin(k*.8)*.06,f.phase)));
   if(k==0)c.move(pp);else c.line(pp);
  }c.stroke(neon.alpha(.76*facing*opacity),1.1);
  for(double u:{.30,.50,.70}){auto pp=project(transform(world_.fishLocal(u,a-.15,f.phase)));c.ellipse(pp.x,pp.y,1.7,1.7,neon.alpha(opacity*facing));}
  auto eye=world_.fishLocal(.115,a,f.phase);eye.y-=5;auto pp=project(transform(eye));
  const double eyeW=std::max(.35,5*std::sqrt(facing));
  c.ellipse(pp.x,pp.y,eyeW+1,6.3,p_.deep.alpha(opacity));c.ellipse(pp.x,pp.y,eyeW,5.1,neon.alpha(.91*opacity));c.ellipse(pp.x+.6,pp.y-.6,eyeW*.4,2.5,p_.deep.alpha(opacity));c.ellipse(pp.x+eyeW*.28,pp.y-2.3,1.1,1.1,rgb(0xffffff,.8*opacity));
 }
 auto nose=project(transform({63,0,std::sin(f.phase)*.2}));c.ellipse(nose.x,nose.y,1.5,1.2,neon.alpha(.75*opacity));
 c.restore();
}

void Renderer::creatureArt(Canvas& c,const FishPose& pose,Species species,double opacity)const{
 c.save();c.translate(pose.p.x,pose.p.y);c.scale(pose.scale,pose.scale);c.rotate(pose.pitch);
 double facing=std::cos(pose.yaw),t=pose.phase;
 c.scale(facing,1);const Color mint=p_.glow.alpha(opacity),gold=p_.gold.alpha(opacity),pink=p_.pink.alpha(opacity);
 if(species==Species::Shark){
  double tail=std::sin(t)*19;
  // A muscular silhouette with a forked caudal fin and separate paired fins.
  c.path();c.move(-104,2);c.curve(-136,-11,-155,-55+tail,-159,-65+tail);c.curve(-130,-53,-117,-20,-101,-4);c.curve(-134,30,-139,39+tail,-152,41+tail);c.curve(-132,10,-120,3,-104,2);c.close();c.fill(rgb(0x377184,opacity));
  c.path();c.move(-34,-24);c.curve(-22,-46,-15,-72,0,-76);c.curve(5,-45,24,-25,34,-20);c.close();c.linear(0,-76,0,-20,rgb(0x528da0,opacity),rgb(0x204854,opacity));
  c.path();c.move(36,12);c.curve(5,40,-14,64,-47,71);c.curve(-35,41,-20,20,-7,10);c.close();c.fill(rgb(0x225260,opacity));
  c.path();c.move(123,-4);c.curve(104,-26,60,-33,7,-32);c.curve(-38,-32,-80,-12,-109,1);c.curve(-60,12,-39,25,3,28);c.curve(61,34,106,22,123,-4);c.close();c.linear(0,-36,0,34,rgb(0x689da9,opacity),rgb(0x1e414f,opacity));
  c.path();c.move(121,1);c.curve(81,11,54,20,-29,15);c.curve(-8,31,78,38,112,14);c.close();c.fill(rgb(0xc3d6cc,opacity*.85));
  c.path();c.move(21,14);c.curve(4,41,-25,59,-61,65);c.curve(-33,33,-22,9,0,2);c.close();c.linear(0,6,-40,63,rgb(0x548c9a,opacity),rgb(0x1c4456,opacity));
  for(int k=0;k<5;k++){double x=46-k*8;c.path();c.move(x,-15);c.curve(x-6,-8,x-6,0,x-3,6);c.stroke(p_.deep.alpha(opacity*.8),1.6);}
  c.ellipse(90,-10,6.2,5.2,p_.deep.alpha(opacity));c.ellipse(91,-10,3.4,3.3,gold);c.ellipse(92,-11,1.3,1.3,rgb(0xffffff,opacity));
  c.path();c.move(97,11);c.curve(105,10,113,5,116,2);c.stroke(p_.deep.alpha(opacity),1.5);
  c.path();c.move(-84,-3);c.curve(-29,-21,36,-25,77,-18);c.stroke(mint.alpha(.30),1.1);
 }else if(species==Species::Octopus){
  // Eight independently phased arms with decreasing thickness and suction cups.
  for(int j=0;j<8;j++){
   double side=(j-3.5)/3.5,root=side*26,prevX=root,prevY=20;
   Color arm=blend(p_.pink,rgb(0x874d86),.45+.04*j).alpha(opacity);
   for(int k=1;k<=26;k++){
    double u=k/26.,curl=t-u*5.8+j*.63;
    double x=root+side*73*u+std::sin(curl)*u*28,y=20+u*(85-23*std::abs(side))+std::cos(curl)*u*18;
    c.path();c.move(prevX,prevY);c.line(x,y);c.stroke(arm,12*(1-u)+1.7);
    if(k%3==0)c.ellipse(x-1,y-1,2.3*(1-u)+.6,1.6*(1-u)+.5,gold.alpha(.65));prevX=x;prevY=y;
   }
  }
  double pulse=1+.025*std::sin(t*2);c.save();c.scale(pulse,1/pulse);
  c.path();c.move(-31,30);c.curve(-69,-14,-47,-79,0,-82);c.curve(49,-81,67,-11,30,31);c.curve(11,44,-12,42,-31,30);c.close();c.linear(-30,-65,26,40,rgb(0xd48cac,opacity),rgb(0x703d74,opacity));
  for(int j=0;j<12;j++){double a=j*2.4,r=15+j*1.6;c.ellipse(std::cos(a)*r,-32+std::sin(a)*r,3,2,pink.alpha(.28));}
  for(int side:{-1,1}){double xx=side*19;c.ellipse(xx,3,12,15,rgb(0xf5dbbe,opacity));c.ellipse(xx+3,5,6,9,p_.deep.alpha(opacity));c.ellipse(xx+4,1,2.4,3,rgb(0xffffff,opacity));}
  c.path();c.move(-5,23);c.curve(-2,27,2,27,6,22);c.stroke(p_.deep.alpha(opacity*.6),1.5);c.restore();
 }else if(species==Species::Ray){
  double flap=std::sin(t)*27;
  c.path();c.move(49,-6);c.curve(19,-19,-15,-59,-59,-91+flap);c.curve(-50,-41,-119,-38,-62,2);c.curve(-28,24,-34,80-flap,6,73-flap);c.curve(14,35,19,13,49,-6);c.close();c.linear(-70,-65,25,51,rgb(0x50669a,opacity),rgb(0x233951,opacity));
  c.path();c.move(-46,0);c.curve(-79,10,-121,16,-151,9+std::sin(t-2)*18);c.stroke(p_.pink.alpha(opacity*.65),3);
  c.ellipse(12,-2,33,14,rgb(0x58798d,opacity));for(int side:{-1,1}){c.ellipse(32,side*6,3,3,gold);}
  for(int j=0;j<19;j++){double a=j*2.4;c.ellipse(-20+std::cos(a)*28,std::sin(a)*40,2,1.7,mint.alpha(.35));}
 }else if(species==Species::Turtle){
  for(int side:{-1,1})for(int k=0;k<2;k++){double x=k?32:-36,yy=side*22,fin=std::sin(t*1.3+k)*12;c.path();c.move(x,yy);c.curve(x-15,yy+side*40,x-45,yy+side*(40+fin),x-30,yy+side*12);c.close();c.fill(rgb(0x6b9b83,opacity));}
  c.ellipse(62,0,25,17,rgb(0x9abb85,opacity));c.ellipse(72,-5,4,4,p_.deep.alpha(opacity));c.ellipse(73,-6,1.5,1.5,rgb(0xffffff,opacity));
  c.ellipse(-1,0,61,39,rgb(0x284e51,opacity));c.ellipse(0,-3,54,32,rgb(0x659076,opacity));
  for(int j=0;j<7;j++){double a=j*tau/6,x=j==6?0:32*std::cos(a),y=j==6?-3:19*std::sin(a)-3;c.path();for(int k=0;k<6;k++){double aa=k*tau/6;if(k==0)c.move(x+15*std::cos(aa),y+11*std::sin(aa));else c.line(x+15*std::cos(aa),y+11*std::sin(aa));}c.close();c.fill(rgb(j%2?0x3b6e5e:0x4c7c63,opacity));}
 }else{
  c.path();c.move(-4,35);for(int k=0;k<45;k++){double a=k*.12,r=21*(1-k/50.);c.line(-8+std::sin(a)*r,51-std::cos(a)*r);}c.stroke(gold,7);
  c.path();c.move(-5,37);c.curve(38,20,13,-12,7,-34);c.curve(32,-44,33,-65,12,-71);c.curve(-15,-76,-20,-55,-11,-45);c.curve(-25,-14,-24,5,-5,37);c.close();c.linear(-19,-65,19,31,gold,p_.pink.alpha(opacity));
  c.roundRect(13,-60,28,8,4,gold);c.ellipse(9,-59,4,4,p_.deep.alpha(opacity));c.ellipse(10,-60,1.6,1.6,rgb(0xffffff,opacity));
  for(int k=0;k<8;k++){double yy=-27+k*8;c.path();c.move(-16,yy);c.line(3+(k<4?k:8-k)*2,yy+3);c.stroke(p_.deep.alpha(opacity*.2),1.2);}
  c.path();c.move(-17,-20);c.line(-40,-5+std::sin(t*4)*5);c.line(-19,16);c.close();c.fill(mint.alpha(.32));
 }
 c.restore();
}

void Renderer::jellyArt(Canvas& c,double x,double y,double size,double t,int style)const{
 c.save();c.translate(x,y);c.scale(size,size);
 const Color col=style?p_.pink:p_.glow;double phase=t*1.65;double pulse=std::sin(phase);double R=49*(1-.10*pulse),H=44*(1+.11*pulse);
 c.glow(0,-12,120,col.alpha(.095));
 // Flow trails are attached to the contracting bell and lag along their length.
 for(int j=0;j<11;j++){
  double xx=(j-5)*6.1;double len=92+(j%4)*16;double ax=xx*(R/49);
  c.path();c.move(ax,10);
  for(int k=1;k<=22;k++){
   double u=k/22.0;double px=ax+std::sin(t*.95-u*5.6+j*.5)*u*24+std::sin(t*.45-u*3+j)*u*7;
   c.line(px,10+u*len);
  }
  c.stroke(col.alpha(.10+(j%3)*.06),j%3==0?2.5:1.0);
 }
 for(int j=0;j<4;j++){
  double xx=(j-1.5)*15;double sway=std::sin(t*.85+j)*13;
  c.path();c.move(xx,5);c.curve(xx-18,45,xx+23+sway,74,xx+sway,108);
  c.stroke(col.alpha(.12),7.5);c.path();c.move(xx,5);c.curve(xx-18,45,xx+23+sway,74,xx+sway,108);c.stroke(col.alpha(.34),1.1);
 }
 c.path();c.move(-R,10);c.curve(-R-1,-H*.42,-R*.65,-H,0,-H);c.curve(R*.65,-H,R+1,-H*.42,R,10);c.curve(R*.45,25,-R*.45,25,-R,10);c.close();
 c.linear(0,-H,0,25,col.alpha(.06),col.alpha(.29));
 c.path();c.move(-R,10);c.curve(-R-1,-H*.42,-R*.65,-H,0,-H);c.curve(R*.65,-H,R+1,-H*.42,R,10);c.stroke(col.alpha(.66),1.45);
 for(int j=-2;j<=2;j++){
  c.path();c.move(0,-H+1);c.curve(j*R*.15,-H*.6,j*R*.40,-H*.1,j*R*.45,12);c.stroke(col.alpha(.19),.8);
 }
 c.ellipse(0,10,R,12,col.alpha(.05));
 for(int j=0;j<17;j++){
  double a=j*tau/17;double xx=std::cos(a)*R,yy=10+std::sin(a)*11;
  c.glow(xx,yy,6,col.alpha(.21));c.ellipse(xx,yy,1.4,1.4,col.alpha(.75));
 }
 // A tiny, quiet face under the dome keeps the machines kind of cute.
 c.ellipse(-8,-6,1.5,2.5,p_.deep.alpha(.68));c.ellipse(8,-6,1.5,2.5,p_.deep.alpha(.68));
 c.path();c.move(-3,-1);c.curve(-1,1,1,1,3,-1);c.stroke(p_.deep.alpha(.5),.75);
 c.restore();
}
void Renderer::pufferArt(Canvas& c,double x,double y,double t,double size)const{
 c.save();c.translate(x,y);c.scale(size,size);c.rotate(std::sin(t*.38)*.055);
 c.glow(0,0,95,p_.gold.alpha(.09));
 for(int side:{-1,1}){
  double wave=std::sin(t*4.2+side)*7;
  c.path();c.move(side*31,-1);c.curve(side*69,-24,side*56,22+wave,side*31,13);c.close();c.fill(p_.glow.alpha(.30));
  c.path();c.move(side*33,4);c.line(side*54,7+wave*.7);c.stroke(p_.glow.alpha(.64),1.3);
 }
 c.ellipse(0,4,42,37,p_.deep);c.path();c.arc(0,0,38,0,tau);c.linear(-20,-34,22,39,rgb(0xe2d2a3),rgb(0x857c5f));
 c.ellipse(0,14,27,19,rgb(0xf4e8c3,.36));
 c.roundRect(-33,-18,66,33,14,p_.deep);
 // QQ eyes look around independently of the camera and close smoothly to blink.
 double b=std::fmod(t+1.7,6.8);double opening=1-.96*std::exp(-std::pow((b-3.2)/.10,2));
 for(int side:{-1,1}){
  double ex=side*14.0,look=std::sin(t*.29)*2.4;
  c.ellipse(ex,-3,9,10*opening,p_.glow.alpha(.81));c.ellipse(ex+look,-3+std::sin(t*.21),4.3,6.3*opening,p_.deep);
  c.path();c.move(ex+4,2);c.line(ex+10,10);c.stroke(p_.glow.alpha(.68),2.4);
 }
 c.path();c.move(0,-36);c.curve(0,-44,12,-44,12,-53);c.stroke(p_.gold.alpha(.85),2.2);c.glow(12,-54,15,p_.gold.alpha(.30));c.ellipse(12,-54,3,3,p_.gold);
 for(int i=0;i<5;i++)c.ellipse(-19+i*9.5,26,1.6,1.6,p_.deep.alpha(.38));
 c.path();c.move(-5,19);c.curve(-1,22,1,22,5,19);c.stroke(p_.deep.alpha(.50),1.5);
 c.restore();
}
void Renderer::instruments(Canvas& c,double t,const Metrics& m)const{
 if(m.privateMode)return;
 // These drifting buoys are part of the reef, with periodically fading text.
 const double visibility=.66+.13*std::sin(t*.095);
 for(int k=0;k<2;k++){
  double x=(k?1242:434)*layout_+22*std::sin(t*.065+k*2);
  double y=(k?628:697)+14*std::sin(t*.14+k);
  const auto col=k?p_.pink:p_.glow;
  c.save();c.translate(x,y);c.rotate(std::sin(t*.06+k)*.014);
  c.path();c.move(0,20);c.curve(8,61,-9,83,4,118);c.stroke(col.alpha(.10),.9);
  c.glow(0,0,93,col.alpha(.038));
  c.roundRect(-104,-28,208,62,16,p_.deep.alpha(.72));
  c.path();c.move(-85,-28);c.line(76,-28);c.curve(95,-28,105,-15,105,0);c.stroke(col.alpha(.32),1.1);
  c.ellipse(-86,-6,3,3,col.alpha(visibility));
  std::string top,bottom;
  const bool alt=std::fmod(t+8,38)>26;
  if(!alt){
   top=k?"MEMORY RESERVOIR":"PROCESSOR CURRENT";
   if(k)bottom=value(m.ramUsed,1)+" / "+value(m.ramTotal,0)+" GiB";
   else bottom=value(m.cpu,0)+" %   CPU";
  }else{
   top=k?"UPTIME":"PACKET CURRENT";
   if(k){if(m.uptime)bottom=std::to_string(int(*m.uptime/3600))+" h  "+std::to_string(int(*m.uptime/60)%60)+" min";else bottom="--";}
   else bottom=value(m.rx?std::optional<double>(*m.rx/1024):std::nullopt,0)+" / "+value(m.tx?std::optional<double>(*m.tx/1024):std::nullopt,0)+" KiB/s";
  }
  if(m.demo)top="DEMO / "+top;
  // Crossfade around mode boundaries so label changes do not pop visibly.
  double cycle=std::fmod(t+8,38);double transition=std::min({std::abs(cycle-26),cycle,38-cycle});double ta=smooth(transition/.8);
  c.text(-75,-5,top,8.8,col.alpha(.65*visibility*ta),true);
  c.text(-75,18,bottom,16.5,rgb(0xe7f6ed,visibility*ta),true);
  c.restore();
 }
}
void Renderer::title(Canvas& c,double t,const Metrics& m)const{
 double alpha=.56*(1-smooth((t-7)/6));if(alpha<.001)return;
 c.text(54,58,"QINDAQT  /  KIND OF QUIET",11,p_.glow.alpha(alpha),true);
 c.text(54,88,"Circuit Reef",26,rgb(0xd5eee7,alpha));
 c.text(55,111,m.demo?"PROCEDURAL AQUARIUM  /  DEMO TELEMETRY":"PROCEDURAL AQUARIUM",9,p_.glow.alpha(alpha*.6));
}
const Image& Renderer::render(int width,int height,double time,const Metrics& metrics,const RenderOptions& opt){
 if(width<32||height<32||width>8192||height>8192)throw std::invalid_argument("Render size outside 32..8192");
 if(!std::isfinite(time)||time<0)throw std::invalid_argument("Time must be finite and non-negative");
 if(frame_.width!=width||frame_.height!=height||background_.width!=width||background_.height!=height){
  frame_=Image(width,height);background_=Image(width,height);layout_=double(width)/height/(16.0/9);
  Canvas bg(background_.surface);bg.scale(height/900.,height/900.);background(bg);
 }
 Canvas c(frame_.surface);cairo_set_source_surface(c.c,background_.surface,0,0);cairo_paint(c.c);
 c.scale(height/900.,height/900.);
 drawScene(c,time,metrics,opt);
 if(cairo_status(c.c))throw std::runtime_error(cairo_status_to_string(cairo_status(c.c)));
 cairo_surface_flush(frame_.surface);return frame_;
}
void Renderer::drawScene(Canvas& c,double time,const Metrics& metrics,const RenderOptions& opt){
 // Analytic poses use continuous time; reduced motion slows the entire ecosystem.
 double t=time*(opt.reducedMotion?.40:1.0);
 water(c,t,false);
 for(std::size_t i=0;i<world_.plants.size();i++)if(i%3==0&&world_.plants[i].type!=3)plant(c,world_.plants[i],t,false);
 struct Actor{double depth;int type;std::size_t index;};std::vector<Actor> actors;actors.reserve(world_.fish.size()+world_.jellies.size()+world_.creatures.size());
 for(std::size_t i=0;i<world_.fish.size();i++)actors.push_back({world_.fishPose(i,t).depth,0,i});
 for(std::size_t i=0;i<world_.jellies.size();i++)actors.push_back({-60+std::sin(t*.05+world_.jellies[i].phase)*140,1,i});
 for(std::size_t i=0;i<world_.creatures.size();++i)actors.push_back({world_.creaturePose(i,t).depth,2,i});
 std::stable_sort(actors.begin(),actors.end(),[](const Actor&a,const Actor&b){return a.depth<b.depth;});
 for(auto a:actors){
  if(a.type==0){auto pose=world_.fishPose(a.index,t);pose.p.x*=layout_;fishArt(c,pose,world_.fish[a.index].style,.84+clamp((pose.depth+350)/700)*.16);}
  else if(a.type==1){const auto& j=world_.jellies[a.index];auto pos=world_.jellyPose(a.index,t);jellyArt(c,pos.x*layout_,pos.y,j.size,t+j.phase*3,j.style);}
  else{auto pose=world_.creaturePose(a.index,t);pose.p.x*=layout_;creatureArt(c,pose,world_.creatures[a.index].species,.68+clamp((pose.depth+300)/600)*.3);}
 }
 pufferArt(c,(230+130*std::sin(t*.065))*layout_,655+35*std::sin(t*.16),t,.78);
 if(opt.metrics)instruments(c,time,metrics);
 for(std::size_t i=0;i<world_.plants.size();i++)if(i%3!=0)plant(c,world_.plants[i],t,true);
 water(c,t,true);
 // Moving reservoir pulses; activity changes opacity, never frantic swim speed.
 double cpu=metrics.cpu.value_or(18)/100;
 for(int j=0;j<9;j++){
  double a=t*.18+j*tau/9;double x=1600*layout_*.52+153*std::cos(a),y=815+25*std::sin(a);
  c.glow(x,y,9,p_.glow.alpha(.065+cpu*.12));c.ellipse(x,y,1.5,1.1,p_.glow.alpha(.24+cpu*.20));
 }
 if(opt.branding)title(c,time,metrics);
 // Soft vignette and user brightness, not an OLED burn-in guarantee.
 c.rect(0,0,1600*layout_,900,rgb(0x000000,1-clamp(opt.brightness,.05,1)));
}
void Renderer::drawGpu(DrawList& list,int width,int height,double time,const Metrics& metrics,const RenderOptions& opt){
 if(width<32||height<32||width>8192||height>8192||!std::isfinite(time)||time<0)throw std::invalid_argument("Invalid GPU scene dimensions or time");
 if(background_.width!=width||background_.height!=height){
  background_=Image(width,height);layout_=double(width)/height/(16.0/9);
  Canvas bg(background_.surface);bg.scale(height/900.,height/900.);background(bg);
 }
 list.begin();list.image(background_,0,0,width,height);
 Canvas c(list);c.scale(height/900.,height/900.);drawScene(c,time,metrics,opt);
}
void Renderer::exportAssets(const std::string& directory){
 namespace fs=std::filesystem;fs::create_directories(directory);const double savedLayout=layout_;layout_=1;
 auto asset=[&](const std::string& name,int w,int h,const std::function<void(Canvas&)>& draw){
  Image im(w,h);{Canvas c(im.surface);draw(c);}im.png((fs::path(directory)/(name+".png")).string());
  auto* svg=cairo_svg_surface_create((fs::path(directory)/(name+".svg")).c_str(),w,h);
  {Canvas c(svg);draw(c);}cairo_surface_finish(svg);auto status=cairo_surface_status(svg);cairo_surface_destroy(svg);if(status)throw std::runtime_error("SVG export failed");
 };
 for(int style=0;style<3;style++){
  asset("koi-"+std::to_string(style),360,220,[&](Canvas& c){fishArt(c,{{215,108},0,0,1.5,.8,0},style);});
  Image sheet(320*8,192*4);Canvas c(sheet.surface);
  for(int i=0;i<32;i++){c.save();c.translate((i%8)*320,(i/8)*192);fishArt(c,{{198,94},0,0,1.35,i*tau/32,0},style);c.restore();}
  sheet.png((fs::path(directory)/("koi-"+std::to_string(style)+"-swim-32.png")).string());
 }
 for(int style=0;style<2;style++){
  asset("jelly-"+std::to_string(style),256,320,[&](Canvas& c){jellyArt(c,128,98,1.3,.5,style);});
  Image sheet(192*8,256*3);Canvas c(sheet.surface);
  for(int i=0;i<24;i++){c.save();c.translate((i%8)*192,(i/8)*256);jellyArt(c,96,65,1.0,i*(tau/1.65)/24,style);c.restore();}
  sheet.png((fs::path(directory)/("jelly-"+std::to_string(style)+"-pulse-24.png")).string());
 }
 for(int kind=0;kind<5;++kind)asset("creature-"+std::to_string(kind),420,300,[&](Canvas& c){creatureArt(c,{{240,145},0,0,1.15,.8,0},Species(kind));});
 asset("qq-puffer",220,200,[&](Canvas&c){pufferArt(c,110,112,.3,1.25);});
 for(int type=0;type<4;type++)asset("coral-"+std::to_string(type),256,300,[&](Canvas&c){plant(c,{128,295,225,70,.3,type},.3,true);});
 asset("aquascape-"+p_.name,1600,900,[&](Canvas&c){background(c);});
 asset("icon",256,256,[&](Canvas&c){c.roundRect(0,0,256,256,56,p_.deep);pufferArt(c,128,146,.3,1.72);});
 std::ofstream manifest(fs::path(directory)/"manifest.json");
 manifest<<R"({
  "generator": "circuit-reef --export-assets",
  "license": "GPL-3.0-or-later",
  "master": "src/renderer.cpp and src/world.cpp; continuous procedural vector rigs",
  "runtime_requires_atlases": false,
  "koi": {"styles": 3, "frames": 32, "columns": 8, "cell": [320,192], "pivot": [198,94], "cycle_seconds": 2.45},
  "jelly": {"styles": 2, "frames": 24, "columns": 8, "cell": [192,256], "pivot": [96,65], "cycle_seconds": 3.807991},
  "note": "Atlases are inspection and reuse assets. Runtime animation is continuous, not frame-sheet playback. Jelly bell pulse loops; its independently moving tentacles need not exactly loop at this atlas boundary."
})";
 layout_=savedLayout;
}
}
