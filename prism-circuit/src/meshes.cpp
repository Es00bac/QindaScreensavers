// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
#include <functional>
namespace sw {
static Mesh parametric(std::string name,int nu,int nv,std::function<V3(float,float)> fn,bool reverse=false){
 Mesh m;m.name=name;
 for(int j=0;j<=nv;j++)for(int i=0;i<=nu;i++){
  float u=float(i)/nu,v=float(j)/nv;V3 p=fn(u,v),a=fn(u+.00015f,v)-fn(u-.00015f,v),b=fn(u,v+.00015f)-fn(u,v-.00015f);V3 n=unit(cross(a,b));if(reverse)n=-n;
  m.v.push_back({p,n,{u,v}});
 }
 for(int j=0;j<nv;j++)for(int i=0;i<nu;i++){unsigned a=j*(nu+1)+i,b=a+1,c=a+nu+1,d=c+1;
  if(reverse)m.ix.insert(m.ix.end(),{a,c,b,b,c,d});else m.ix.insert(m.ix.end(),{a,b,c,b,d,c});
 }return m;
}
static void append(Mesh& out,const Mesh& a){unsigned start=out.v.size();out.v.insert(out.v.end(),a.v.begin(),a.v.end());for(auto i:a.ix)out.ix.push_back(start+i);}
static float rockNoise(V3 p){return std::sin(p.x*4.1f+std::sin(p.y*5.3f))*std::cos(p.z*3.7f-p.x*.9f)*.07f+std::sin(p.x*11.4f+p.z*6.3f)*std::cos(p.y*12.3f)*.026f+std::sin(p.x*27+p.z*19)*std::cos(p.y*31)*.012f;}
void initialize(Frame& f){
 f.batches[Sphere].mesh=parametric("sphere",40,28,[](float u,float v){float a=u*2*pi,b=v*pi;return V3{std::cos(a)*std::sin(b),std::cos(b),std::sin(a)*std::sin(b)};});
 Mesh box;box.name="beveled-plate";
 for(int axis=0;axis<3;axis++)for(int side:{-1,1})append(box,parametric("",6,6,[=](float u,float v){V3 p;float *q=&p.x;q[axis]=float(side);q[(axis+1)%3]=u*2-1;q[(axis+2)%3]=v*2-1;V3 c{clamp(p.x,-.85,.85),clamp(p.y,-.85,.85),clamp(p.z,-.85,.85)};return c+unit(p-c)*.15f;},side<0));
 f.batches[Box].mesh=box;
 f.batches[Torus].mesh=parametric("machined-collar",48,10,[](float u,float v){float a=u*2*pi,b=v*2*pi;return V3{(1+.055f*std::cos(b))*std::cos(a),.055f*std::sin(b),(1+.055f*std::cos(b))*std::sin(a)};},true);
 Mesh cyl=parametric("cylinder",24,1,[](float u,float v){float a=u*2*pi;return V3{std::cos(a),v*2-1,std::sin(a)};},true);
 for(int sign:{-1,1})append(cyl,parametric("",24,1,[=](float u,float v){float a=u*2*pi;return V3{v*std::cos(a),float(sign),v*std::sin(a)};},sign<0));
 f.batches[Cylinder].mesh=cyl;
 f.batches[Hull].mesh=parametric("sculpted-fuselage",48,28,[](float u,float v){
  const float knot[]={0,.12f,.24f,.47f,.70f,.88f,1};
  const float widths[]={.30f,.86f,1.10f,.89f,.49f,.19f,.007f};
  const float heights[]={.19f,.46f,.54f,.40f,.26f,.11f,.007f};
  const float centers[]={0,.02f,.04f,-.02f,-.15f,-.28f,-.38f};
  auto sample=[&](const float* a){int i=0;while(i<5&&u>knot[i+1])i++;float t=clamp((u-knot[i])/(knot[i+1]-knot[i]));t=t*t*(3-2*t);return a[i]*(1-t)+a[i+1]*t;};
  float a=v*2*pi;float y=std::copysign(std::pow(std::abs(std::sin(a)),.47f),std::sin(a));float z=std::copysign(std::pow(std::abs(std::cos(a)),.47f),std::cos(a));
  return V3{-2.7f+u*5.8f,sample(centers)+y*sample(heights),z*sample(widths)};
 });
 f.batches[Wing].mesh=parametric("swept-fin",30,12,[](float u,float v){float t=v*2*pi;float chord=.07f+1.1f*std::pow(1-clamp(u),.72f);return V3{-1.8f*u+std::cos(t)*chord,std::sin(t)*.105f*(1-u*.75f),u*2.3f};},true);
 for(int k=0;k<6;k++){Random rng(709+k*441);std::vector<V3> centers;std::vector<float> rad;for(int q=0;q<9;q++){centers.push_back(unit({rng.range(-1,1),rng.range(-1,1),rng.range(-1,1)}));rad.push_back(rng.range(.08,.27));}
 f.batches[Rock0+k].mesh=parametric("asteroid-"+std::to_string(k),48,30,[=](float u,float v){float a=u*2*pi,b=v*pi;V3 p{std::cos(a)*std::sin(b),std::cos(b),std::sin(a)*std::sin(b)};float r=1+rockNoise(p+V3{float(k),0,0});for(size_t c=0;c<centers.size();c++){float d=length(p-centers[c])/rad[c];r-=std::exp(-d*d*4)*rad[c]*.43f;r+=std::exp(-(d-.88f)*(d-.88f)*80)*rad[c]*.11f;}return V3{p.x*(r+std::sin(a*3+b*4)*.06f),p.y*r*(.83f+.04f*k),p.z*r};});
 }
 f.batches[Monolith].mesh=parametric("alien-shard",1,8,[](float u,float v){float a=v*2*pi;float r=u>.5f?.48f:.80f;return V3{std::cos(a)*r,(u*2-1)*2,std::sin(a)*r};});
 f.batches[Manta].mesh=parametric("sail-organism",52,40,[](float u,float v){float z=(v*2-1)*4;float x=(u*2-1)*2.1f*(1-.55f*std::pow(std::abs(v*2-1),.8f))-.85f*std::abs(v*2-1);float y=.23f*std::sin(pi*u)+.27f*std::pow(std::abs(z),1.15f);return V3{x,y,z};},true);
 // Beveled directional side armor. Its entire outline is intentional, not a scaled capsule.
 Mesh plate;plate.name="directional-cheek-armor";
 std::array<V2,7> outline{{{-1.66f,-.15f},{-.92f,-.40f},{.81f,-.43f},{2.20f,-.19f},{1.47f,.17f},{.18f,.37f},{-1.38f,.28f}}};
 for(int face=0;face<2;face++){
  float zoff=face?.07f:-.03f;V3 center{0,0,1.0f+zoff};unsigned first=plate.v.size();plate.v.push_back({center,{.1676f,0,.9859f},{.45f,.08f}});
  for(auto p:outline){V3 pos{p.x,p.y,1.0f-.17f*p.x+zoff};plate.v.push_back({pos,{.1676f,0,.9859f},{(p.x+2.7f)/5.8f,.08f+p.y*.06f}});}
  for(unsigned j=0;j<7;j++)plate.ix.insert(plate.ix.end(),{first,first+1+j,first+1+(j+1)%7});
 }
 for(int j=0;j<7;j++){
  V2 a=outline[j],b=outline[(j+1)%7];V3 pa{a.x,a.y,1-.17f*a.x-.03f},pb{b.x,b.y,1-.17f*b.x-.03f};V3 n=unit(cross(pb-pa,{0,0,1}));unsigned k=plate.v.size();
  plate.v.push_back({pa,n,{0,0}});plate.v.push_back({pb,n,{1,0}});plate.v.push_back({pb+V3{0,0,.10f},n,{1,1}});plate.v.push_back({pa+V3{0,0,.10f},n,{0,1}});plate.ix.insert(plate.ix.end(),{k,k+1,k+2,k,k+2,k+3});
 }
 f.batches[Sideplate].mesh=plate;
 f.batches[Ribbon].mesh=parametric("flight-scarf",28,6,[](float u,float v){return V3{-u*2.0f,-.10f*u,std::sin(v*pi)*.08f+(v-.5f)*.40f*(1-u*.28f)};});

 f.batches[Ear].mesh=parametric("sculpted-ear",16,16,[](float u,float v){float a=v*2*pi;float r=(1-u)*(.52f+.12f*std::sin(u*pi));return V3{std::cos(a)*r,u*1.6f,std::sin(a)*r*.48f};});
}
void add(Frame& f,Primitive p,M4 model,Material mat){f.batches[p].instances.push_back({model,{mat.color.x,mat.color.y,mat.color.z,mat.emission},{mat.rough,mat.metal,mat.kind,0}});}
}
