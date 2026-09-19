// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include <algorithm>
namespace sw {
V3 Track::curve(double u)const{
 double a=u*2.0*3.14159265358979323846;
 if(kind==1){ // A broad figure eight. Crossings are separated vertically.
  return {float(151*std::sin(a)),float(27*std::cos(a)+5*std::sin(3*a)),float(92*std::sin(2*a))};
 }
 if(kind==2){
  return {float(132*std::cos(a)+21*std::cos(3*a)),float(17*std::sin(2*a)+5*std::sin(3*a)),float(115*std::sin(a)+13*std::sin(3*a))};
 }
 double r=48*(2+(.88+variation*.04)*std::cos(3*a));
 return {float(r*std::cos(2*a)),float(30*std::sin(3*a)),float(r*std::sin(2*a))};
}
TrackPose Track::exact(double u)const{
 constexpr double e=.000035;
 V3 p=curve(u),d=curve(u+e)-curve(u-e),forward=unit(d);
 V3 right=unit(cross({0,1,0},forward)),up=unit(cross(forward,right));
 V3 ta=unit(curve(u)-curve(u-e*2)),tb=unit(curve(u+e*2)-curve(u));
 float curvature=dot((tb-ta)/std::max(.001f,sw::length(d)),right);
 float bank=clamp(-curvature*16.f,-.46f,.46f);
 V3 br=right*std::cos(bank)+up*std::sin(bank),bu=up*std::cos(bank)-right*std::sin(bank);
 return {p,br,bu,forward,bank,curvature};
}
Track::Track(std::uint64_t s,int k):seed(s),kind(k){
 if(kind<0||kind>2)throw std::invalid_argument("Unknown course");
 Random r(seed);variation=r.range(-1,1);
 for(int i=0;i<=Samples;i++){
  poses_[i]=exact(double(i)/Samples);
  if(i)lengths_[i]=lengths_[i-1]+sw::length(poses_[i].position-poses_[i-1].position);
 }
 length=lengths_.back();
 for(int i=0;i<6;i++)pads[i]={length*(.085+i*.153),float((i%3)-1)*4.1f};
}
TrackPose Track::at(double distance)const{
 double s=std::fmod(distance,length);if(s<0)s+=length;
 auto it=std::upper_bound(lengths_.begin(),lengths_.end(),s);
 int i=std::clamp(int(it-lengths_.begin())-1,0,Samples-1);
 float a=float((s-lengths_[i])/(lengths_[i+1]-lengths_[i]));
 const auto& p=poses_[i];const auto& q=poses_[i+1];
 V3 f=unit(mix(p.forward,q.forward,a));V3 right=unit(mix(p.right,q.right,a));V3 up=unit(cross(f,right));right=unit(cross(up,f));
 return {mix(p.position,q.position,a),right,up,f,p.bank+(q.bank-p.bank)*a,p.curvature+(q.curvature-p.curvature)*a};
}
double Track::signedGap(double a,double b)const{
 double d=std::fmod(a-b+length*.5,length);if(d<0)d+=length;return d-length*.5;
}
const char* Track::name()const{return kind==0?"PRISM KNOT":kind==1?"CHROMATIC EIGHT":"AURORA LOOP";}
void buildTrackMeshes(Frame& f,const Track& tr){
 Mesh top;top.name="prismatic-road";Mesh shell;shell.name="carbon-road-chassis";
 constexpr int N=1300,W=12;
 for(int i=0;i<=N;i++){
  double s=tr.length*i/N;auto p=tr.at(s);
  for(int j=0;j<=W;j++){
   float v=float(j)/W,x=(v*2-1)*tr.halfWidth;
   top.v.push_back({p.position+p.right*x,p.up,{float(s),v}});
  }
  // Full structural underside and both sides, not a paper-thin floating plane.
  for(auto v:std::array<V2,4>{{{-tr.halfWidth,0},{-tr.halfWidth,-.75f},{tr.halfWidth,-.75f},{tr.halfWidth,0}}}){
   V3 n=v.y<0?-p.up:(v.x<0?-p.right:p.right);
   shell.v.push_back({p.position+p.right*v.x+p.up*v.y,n,{float(s),v.x}});
  }
 }
 for(int i=0;i<N;i++){
  for(int j=0;j<W;j++){
   unsigned a=i*(W+1)+j,b=a+W+1;
   top.ix.insert(top.ix.end(),{a,b,a+1,a+1,b,b+1});
  }
  for(int j=0;j<3;j++){
   unsigned a=i*4+j,b=a+4;shell.ix.insert(shell.ix.end(),{a,a+1,b,a+1,b+1,b});
  }
 }
 f.batches[RoadSurface].mesh=std::move(top);f.batches[RoadShell].mesh=std::move(shell);
 f.map.clear();for(int i=0;i<=160;i++){auto p=tr.at(tr.length*i/160).position;f.map.push_back({p.x/190.f,p.z/190.f});}
}
}
