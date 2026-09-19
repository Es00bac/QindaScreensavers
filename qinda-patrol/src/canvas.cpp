#include "canvas.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
namespace patrol {
Color alpha(Color c,int a){return (c&0xffffffu)|(Color(std::clamp(a,0,255))<<24);}
Color mix(Color a,Color b,double t){t=std::clamp(t,0.0,1.0);auto c=[&](int s){return int(std::lround(((a>>s)&255)*(1-t)+((b>>s)&255)*t));};return rgb(c(16),c(8),c(0),c(24));}
Canvas::Canvas(int w,int h,Color c):width(w),height(h){if(w<=0||h<=0||w>8192||h>8192)throw std::invalid_argument("invalid canvas size");pixels.assign(std::size_t(w)*h,c);}
void Canvas::clear(Color c){std::fill(pixels.begin(),pixels.end(),c);}
void Canvas::dot(int x,int y,Color c){
    if(x<0||y<0||x>=width||y>=height)return;
    unsigned a=c>>24;if(a==0)return;
    auto& d=pixels[std::size_t(y)*width+x];
    if(a==255 || (d>>24)==0){d=c;return;}
    unsigned da=d>>24,oa=a+(da*(255-a)+127)/255;
    auto ch=[&](int s){return ((((c>>s)&255)*a+(((d>>s)&255)*da*(255-a)+127)/255)+oa/2)/oa;};
    d=rgb(ch(16),ch(8),ch(0),oa);
}
void Canvas::rect(int x,int y,int w,int h,Color c){
    if(w<=0||h<=0)return;
    int x0=std::clamp(x,0,width),x1=std::clamp(x+w,0,width),y0=std::clamp(y,0,height),y1=std::clamp(y+h,0,height);
    if((c>>24)==255)for(int yy=y0;yy<y1;++yy)std::fill(pixels.begin()+std::size_t(yy)*width+x0,pixels.begin()+std::size_t(yy)*width+x1,c);
    else for(int yy=y0;yy<y1;++yy)for(int xx=x0;xx<x1;++xx)dot(xx,yy,c);
}
void Canvas::line(int x0,int y0,int x1,int y1,Color c,int thickness){
    int dx=std::abs(x1-x0),sx=x0<x1?1:-1,dy=-std::abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;
    for(;;){rect(x0-thickness/2,y0-thickness/2,thickness,thickness,c);if(x0==x1&&y0==y1)break;int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}
}
void Canvas::ellipse(int x,int y,int w,int h,Color c){
    if(w<=0||h<=0)return;
    double rx=w*0.5,ry=h*0.5;
    for(int yy=std::max(0,y);yy<std::min(height,y+h);++yy){double dy=(yy+0.5-y-ry)/ry;double r=rx*std::sqrt(std::max(0.0,1-dy*dy));int a=int(std::ceil(x+rx-r-0.5)),b=int(std::floor(x+rx+r-0.5));rect(a,yy,b-a+1,1,c);}
}
void Canvas::polygon(std::initializer_list<Point> points,Color c){
    std::vector<Point> v(points);if(v.size()<3)return;
    int lo=height,hi=0;for(auto p:v){lo=std::min(lo,p.y);hi=std::max(hi,p.y);}lo=std::max(lo,0);hi=std::min(hi,height-1);
    std::vector<double> crossings;crossings.reserve(v.size());
    for(int y=lo;y<=hi;++y){crossings.clear();for(std::size_t i=0,j=v.size()-1;i<v.size();j=i++){
        auto a=v[i],b=v[j];double yy=y+0.5;
        if((a.y<=yy&&b.y>yy)||(b.y<=yy&&a.y>yy))crossings.push_back(a.x+(yy-a.y)*double(b.x-a.x)/(b.y-a.y));
    }std::sort(crossings.begin(),crossings.end());for(std::size_t i=0;i+1<crossings.size();i+=2){int a=int(std::ceil(crossings[i]-0.5)),b=int(std::floor(crossings[i+1]-0.5));rect(a,y,b-a+1,1,c);}}
}
void Canvas::blit(const Canvas& s,int x,int y,int scale,bool flip){
    if(scale<1)return;
    for(int yy=0;yy<s.height;++yy)for(int xx=0;xx<s.width;++xx){Color c=s.pixels[std::size_t(yy)*s.width+xx];if(!(c>>24))continue;int dx=x+(flip?s.width-1-xx:xx)*scale;if(scale==1)dot(dx,y+yy,c);else rect(dx,y+yy*scale,scale,scale,c);}
}
void Canvas::crossfade(const Canvas& a,const Canvas& b,int k){
    // Integer lerp of all four channels at once, cheap enough to run per frame during a district blend.
    if(a.pixels.size()!=pixels.size()||b.pixels.size()!=pixels.size())return;
    unsigned kb=unsigned(std::clamp(k,0,256)),ka=256-kb;
    for(std::size_t i=0;i<pixels.size();++i){Color x=a.pixels[i],y=b.pixels[i];
        Color rb=(((x&0x00ff00ffu)*ka+(y&0x00ff00ffu)*kb)>>8)&0x00ff00ffu;
        Color ag=(((x>>8)&0x00ff00ffu)*ka+((y>>8)&0x00ff00ffu)*kb)&0xff00ff00u;
        pixels[i]=rb|ag;}
}
namespace {
// Original 5x7 bitmap glyph definitions. No bundled font files or font runtime.
const std::map<char,std::array<unsigned,7>>& glyphs(){
 static const std::map<char,std::array<unsigned,7>> g={
 {'A',{14,17,17,31,17,17,17}},{'B',{30,17,17,30,17,17,30}},{'C',{14,17,16,16,16,17,14}},
 {'D',{30,17,17,17,17,17,30}},{'E',{31,16,16,30,16,16,31}},{'F',{31,16,16,30,16,16,16}},
 {'G',{14,17,16,23,17,17,15}},{'H',{17,17,17,31,17,17,17}},{'I',{31,4,4,4,4,4,31}},
 {'J',{7,2,2,2,18,18,12}},{'K',{17,18,20,24,20,18,17}},{'L',{16,16,16,16,16,16,31}},
 {'M',{17,27,21,21,17,17,17}},{'N',{17,25,21,19,17,17,17}},{'O',{14,17,17,17,17,17,14}},
 {'P',{30,17,17,30,16,16,16}},{'Q',{14,17,17,17,21,18,13}},{'R',{30,17,17,30,20,18,17}},
 {'S',{15,16,16,14,1,1,30}},{'T',{31,4,4,4,4,4,4}},{'U',{17,17,17,17,17,17,14}},
 {'V',{17,17,17,17,17,10,4}},{'W',{17,17,17,21,21,21,10}},{'X',{17,17,10,4,10,17,17}},
 {'Y',{17,17,10,4,4,4,4}},{'Z',{31,1,2,4,8,16,31}},
 {'0',{14,17,19,21,25,17,14}},{'1',{4,12,4,4,4,4,14}},{'2',{14,17,1,2,4,8,31}},
 {'3',{30,1,1,14,1,1,30}},{'4',{2,6,10,18,31,2,2}},{'5',{31,16,16,30,1,1,30}},
 {'6',{14,16,16,30,17,17,14}},{'7',{31,1,2,4,8,8,8}},{'8',{14,17,17,14,17,17,14}},
 {'9',{14,17,17,15,1,1,14}},{':',{0,4,4,0,4,4,0}},{'.',{0,0,0,0,0,6,6}},
 {'-',{0,0,0,31,0,0,0}},{'/',{1,2,2,4,8,8,16}},{'%',{25,25,2,4,8,19,19}},
 {'+',{0,4,4,31,4,4,0}},{'!',{4,4,4,4,4,0,4}},{'?',{14,17,1,2,4,0,4}},
 {'>',{16,8,4,2,4,8,16}},{'<',{1,2,4,8,4,2,1}},{'=',{0,0,31,0,31,0,0}},
 {'[',{14,8,8,8,8,8,14}},{']',{14,2,2,2,2,2,14}},{'_', {0,0,0,0,0,0,31}},
 {' ',{0,0,0,0,0,0,0}}
 };return g;
}
void be(std::vector<unsigned char>& o,std::uint32_t n){for(int s=24;s>=0;s-=8)o.push_back((n>>s)&255);}
std::uint32_t crc(const unsigned char* p,std::size_t n){std::uint32_t c=~0u;for(std::size_t i=0;i<n;++i){c^=p[i];for(int j=0;j<8;++j)c=(c>>1)^((c&1)?0xedb88320u:0u);}return ~c;}
void chunk(std::vector<unsigned char>& o,const char* type,const std::vector<unsigned char>& data){be(o,std::uint32_t(data.size()));std::size_t start=o.size();o.insert(o.end(),type,type+4);o.insert(o.end(),data.begin(),data.end());be(o,crc(o.data()+start,data.size()+4));}
}
int Canvas::textWidth(const std::string& t,int s){return t.empty()?0:int(t.size()*6-1)*s;}
void Canvas::text(int x,int y,const std::string& t,Color c,int s){for(unsigned char ch:t){if(ch>='a'&&ch<='z')ch-=32;auto it=glyphs().find(char(ch));if(it==glyphs().end())it=glyphs().find('?');for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(it->second[row]&(1<<(4-col)))rect(x+col*s,y+row*s,s,s,c);x+=6*s;}}
bool Canvas::png(const std::string& path)const {
    std::vector<unsigned char> raw;raw.reserve(std::size_t(width*4+1)*height);
    for(int y=0;y<height;++y){raw.push_back(0);for(int x=0;x<width;++x){auto c=pixels[std::size_t(y)*width+x];raw.push_back((c>>16)&255);raw.push_back((c>>8)&255);raw.push_back(c&255);raw.push_back(c>>24);}}
    // Valid zlib stream using stored DEFLATE blocks. Asset PNGs may be recompressed
    // losslessly with any optimizer; runtime has no PNG/codec dependency.
    std::vector<unsigned char> z{0x78,0x01};std::uint32_t a=1,b=0;
    for(auto v:raw){a=(a+v)%65521;b=(b+a)%65521;}
    for(std::size_t i=0;i<raw.size();){unsigned n=unsigned(std::min<std::size_t>(65535,raw.size()-i));z.push_back(i+n==raw.size()?1:0);z.push_back(n&255);z.push_back(n>>8);unsigned inv=(~n)&65535;z.push_back(inv&255);z.push_back(inv>>8);z.insert(z.end(),raw.begin()+i,raw.begin()+i+n);i+=n;}
    be(z,(b<<16)|a);std::vector<unsigned char> out{137,80,78,71,13,10,26,10},ihdr;
    be(ihdr,width);be(ihdr,height);ihdr.insert(ihdr.end(),{8,6,0,0,0});chunk(out,"IHDR",ihdr);chunk(out,"IDAT",z);chunk(out,"IEND",{});
    std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(out.data()),std::streamsize(out.size()));return bool(f);
}
}
