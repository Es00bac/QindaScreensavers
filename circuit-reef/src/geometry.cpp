// SPDX-License-Identifier: GPL-3.0-or-later
#include "reef/geometry.hpp"
#include "reef/canvas.hpp"
#include <numeric>
#include <sstream>
namespace reef {
namespace {
double cross2(Vec2 a,Vec2 b){return a.x*b.y-a.y*b.x;}
double len(Vec2 a){return std::hypot(a.x,a.y);}
Vec2 unit(Vec2 a){return a*(1/std::max(1e-10,len(a)));}
bool inside(Vec2 p,Vec2 a,Vec2 b,Vec2 c,double winding){return cross2(b-a,p-a)*winding>=-1e-8&&cross2(c-b,p-b)*winding>=-1e-8&&cross2(a-c,p-c)*winding>=-1e-8;}
}
DrawList::DrawList(){vertices.reserve(150000);batches.reserve(32);points_.reserve(160);}
DrawList::~DrawList()=default;
void DrawList::begin(){vertices.clear();batches.clear();stack_.clear();transform_={};path();if(textCache_.size()>128)textCache_.clear();}
void DrawList::save(){stack_.push_back(transform_);}
void DrawList::restore(){if(stack_.empty())throw std::logic_error("Unbalanced painter restore");transform_=stack_.back();stack_.pop_back();}
void DrawList::translate(double x,double y){transform_.e+=transform_.a*x+transform_.c*y;transform_.f+=transform_.b*x+transform_.d*y;}
void DrawList::scale(double x,double y){transform_.a*=x;transform_.b*=x;transform_.c*=y;transform_.d*=y;}
void DrawList::rotate(double r){double s=std::sin(r),c=std::cos(r);auto t=transform_;transform_.a=t.a*c+t.c*s;transform_.b=t.b*c+t.d*s;transform_.c=-t.a*s+t.c*c;transform_.d=-t.b*s+t.d*c;}
void DrawList::path(){points_.clear();closed_=false;}
void DrawList::move(Vec2 p){points_.push_back(transform_.map(p));}
void DrawList::line(Vec2 p){auto v=transform_.map(p);if(points_.empty()||len(v-points_.back())>1e-6)points_.push_back(v);}
void DrawList::curve(Vec2 a,Vec2 b,Vec2 end){
 if(points_.empty()){move(end);return;}auto p=points_.back();a=transform_.map(a);b=transform_.map(b);end=transform_.map(end);
 int n=std::clamp(int((len(a-p)+len(b-a)+len(end-b))/12),6,32);
 for(int i=1;i<=n;i++){double t=i/double(n),v=1-t;points_.push_back(p*(v*v*v)+a*(3*v*v*t)+b*(3*v*t*t)+end*(t*t*t));}
}
void DrawList::close(){closed_=true;if(points_.size()>2&&len(points_.front()-points_.back())<1e-5)points_.pop_back();}
void DrawList::arc(double x,double y,double r,double begin,double end){
 double span=end-begin;int n=std::clamp(int(std::abs(span)*std::sqrt(r*std::hypot(transform_.a,transform_.b))*1.8),8,100);
 for(int i=0;i<=n;i++){double a=begin+span*i/n;line({x+r*std::cos(a),y+r*std::sin(a)});}
}
void DrawList::triangle(Vec2 a,Vec2 b,Vec2 c,Color ca,Color cb,Color cc,const Image* tex,Vec2 ua,Vec2 ub,Vec2 uc){
 if(batches.empty()||batches.back().texture!=tex)batches.push_back({vertices.size(),0,tex});
 vertices.push_back({a,ca,ua});vertices.push_back({b,cb,ub});vertices.push_back({c,cc,uc});batches.back().count+=3;
}
void DrawList::fillWith(const std::function<Color(Vec2)>& color){
 if(points_.size()<3){path();return;}
 if(len(points_.front()-points_.back())<1e-6)points_.pop_back();
 double area=0;for(std::size_t i=0;i<points_.size();i++)area+=cross2(points_[i],points_[(i+1)%points_.size()]);
 if(std::abs(area)<1e-9){path();return;}
 const double winding=area>0?1:-1;
 auto tri=[&](std::size_t a,std::size_t b,std::size_t c){triangle(points_[a],points_[b],points_[c],color(points_[a]),color(points_[b]),color(points_[c]));};
 bool convex=true;for(std::size_t i=0;i<points_.size();i++)if(cross2(points_[(i+1)%points_.size()]-points_[i],points_[(i+2)%points_.size()]-points_[(i+1)%points_.size()])*winding<-.00001){convex=false;break;}
 if(convex){for(std::size_t i=1;i+1<points_.size();i++)tri(0,i,i+1);}
 else{
  std::vector<std::size_t> remaining(points_.size());std::iota(remaining.begin(),remaining.end(),0);
  while(remaining.size()>3){
   bool found=false;
   for(std::size_t k=0;k<remaining.size();k++){
    auto a=remaining[(k+remaining.size()-1)%remaining.size()],b=remaining[k],c=remaining[(k+1)%remaining.size()];
    if(cross2(points_[b]-points_[a],points_[c]-points_[b])*winding<=1e-9)continue;
    bool occupied=false;for(auto j:remaining)if(j!=a&&j!=b&&j!=c&&inside(points_[j],points_[a],points_[b],points_[c],winding)){occupied=true;break;}
    if(occupied)continue;
    tri(a,b,c);remaining.erase(remaining.begin()+std::ptrdiff_t(k));found=true;break;
   }
   if(!found){ // Degenerate thin fin/coral paths: bounded fan fallback, never loop.
    for(std::size_t j=1;j+1<remaining.size();j++)tri(remaining[0],remaining[j],remaining[j+1]);
    remaining.clear();break;
   }
  }
  if(remaining.size()==3)tri(remaining[0],remaining[1],remaining[2]);
 }
 // Subpixel coverage fringe around fill boundaries.
 for(std::size_t i=0;i<points_.size();i++){
  auto a=points_[i],b=points_[(i+1)%points_.size()];auto edge=unit(b-a);Vec2 out={edge.y*winding*.60,-edge.x*winding*.60};
  auto ca=color(a),cb=color(b);triangle(a,b,b+out,ca,cb,cb.alpha(0));triangle(a,b+out,a+out,ca,cb.alpha(0),ca.alpha(0));
 }
 path();
}
void DrawList::fill(Color col){fillWith([col](Vec2){return col;});}
void DrawList::linear(Vec2 a,Vec2 b,Color c0,Color c1){
 a=transform_.map(a);b=transform_.map(b);auto d=b-a;double den=std::max(1e-8,d.x*d.x+d.y*d.y);
 fillWith([=](Vec2 p){double t=((p.x-a.x)*d.x+(p.y-a.y)*d.y)/den;return blend(c0,c1,clamp(t));});
}
void DrawList::stroke(Color col,double width){
 if(points_.size()<2){path();return;}
 double half=width*.5*std::sqrt(std::abs(transform_.a*transform_.d-transform_.b*transform_.c));
 std::vector<Vec2> normals;normals.reserve(points_.size());
 for(std::size_t i=0;i<points_.size();i++){
  auto before=(i==0?(closed_?points_.back():points_[0]-(points_[1]-points_[0])):points_[i-1]);
  auto after=(i+1==points_.size()?(closed_?points_[0]:points_[i]+(points_[i]-points_[i-1])):points_[i+1]);
  auto d1=unit(points_[i]-before),d2=unit(after-points_[i]);Vec2 n1={-d1.y,d1.x},n2={-d2.y,d2.x};auto n=unit(n1+n2);double dot=std::max(.3,n.x*n2.x+n.y*n2.y);normals.push_back(n*(1/dot));
 }
 auto quad=[&](Vec2 a,Vec2 b,Vec2 c,Vec2 d,Color ca,Color cb,Color cc,Color cd){triangle(a,b,c,ca,cb,cc);triangle(a,c,d,ca,cc,cd);};
 auto end=closed_?points_.size():points_.size()-1;
 for(std::size_t i=0;i<end;i++){
  auto j=(i+1)%points_.size();auto a=points_[i],b=points_[j],n=normals[i],m=normals[j];
  quad(a+n*half,b+m*half,b-m*half,a-n*half,col,col,col,col);
  for(int sign:{-1,1})quad(a+n*(half*sign),b+m*(half*sign),b+m*((half+.65)*sign),a+n*((half+.65)*sign),col,col,col.alpha(0),col.alpha(0));
 }
 path();
}
void DrawList::glow(double x,double y,double r,Color col){
 const int n=r<18?12:(r<55?20:32);
 std::array<double,4> radii={0,.28,.65,1};std::array<double,4> alpha={1,.46,.10,0};
 for(std::size_t k=0;k+1<radii.size();k++)for(int j=0;j<n;j++){
  double a=j*tau/n,b=(j+1)*tau/n;
  auto p0=transform_.map({x+std::cos(a)*r*radii[k],y+std::sin(a)*r*radii[k]});
  auto p1=transform_.map({x+std::cos(b)*r*radii[k],y+std::sin(b)*r*radii[k]});
  auto p2=transform_.map({x+std::cos(b)*r*radii[k+1],y+std::sin(b)*r*radii[k+1]});
  auto p3=transform_.map({x+std::cos(a)*r*radii[k+1],y+std::sin(a)*r*radii[k+1]});
  triangle(p0,p1,p2,col.alpha(alpha[k]),col.alpha(alpha[k]),col.alpha(alpha[k+1]));triangle(p0,p2,p3,col.alpha(alpha[k]),col.alpha(alpha[k+1]),col.alpha(alpha[k+1]));
 }
}
void DrawList::image(const Image& im,double x,double y,double w,double h,Color col){
 auto a=transform_.map({x,y}),b=transform_.map({x+w,y}),c=transform_.map({x+w,y+h}),d=transform_.map({x,y+h});
 triangle(a,b,c,col,col,col,&im,{0,0},{1,0},{1,1});triangle(a,c,d,col,col,col,&im,{0,0},{1,1},{0,1});
}
void DrawList::text(double x,double y,const std::string& text,double size,Color col,bool bold){
 std::ostringstream key;key<<text<<'|'<<size<<'|'<<bold;auto name=key.str();
 auto it=textCache_.find(name);
 if(it==textCache_.end()){
  Image scratch(1,1);Canvas c(scratch.surface);cairo_select_font_face(c.c,"sans-serif",CAIRO_FONT_SLANT_NORMAL,bold?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);cairo_set_font_size(c.c,size*2);
  cairo_text_extents_t ext{};cairo_text_extents(c.c,text.c_str(),&ext);
  auto img=std::make_unique<Image>(std::max(4,int(std::ceil(ext.x_advance))+6),std::max(4,int(std::ceil(size*2*1.6))));
  {Canvas glyph(img->surface);glyph.text(2,size*2+1,text,size*2,{1,1,1,1},bold);}
  it=textCache_.emplace(name,std::move(img)).first;
 }
 image(*it->second,x-1,y-size-.5,it->second->width*.5,it->second->height*.5,col);
}
}
