#include "renderer.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>
namespace patrol {
namespace {
constexpr Color Ink=0xff080f1du,White=0xffe6f1edu,Muted=0xff8cabaeu,Teal=0xff65f8c9u,Amber=0xffffbc66u,Post=0xff354958u;
constexpr int ChunkW=480,ChunkH=400;
constexpr double FarParallax=0.14,NearParallax=0.32,StreetParallax=0.5;
std::string number(double value,int precision=1){if(value<0||!std::isfinite(value))return "--";std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<value;return s.str();}
void border(Canvas& c,int x,int y,int w,int h,Color edge,Color inside){c.rect(x,y,w,h,edge);c.rect(x+2,y+2,w-4,h-4,inside);}
void centerText(Canvas& c,int x,int y,int width,const std::string& s,Color color,int scale=1){c.text(x+(width-Canvas::textWidth(s,scale))/2,y,s,color,scale);}
int positiveMod(int a,int b){return (a%b+b)%b;}
double wrap(double a,double b){a=std::fmod(a,b);return a<0?a+b:a;}
Palette paletteAt(const DistrictMix& d){if(d.t<=0)return palette(d.from);if(d.t>=1)return palette(d.to);return blendPalette(palette(d.from),palette(d.to),d.t);}
Biome dominant(const DistrictMix& d){return d.t<0.5?d.from:d.to;}
const char* billboardText(Biome b,int i){static constexpr const char* words[4][3]{{"RAMEN","NEON QQ","NIGHT"},{"BLOOM","GARDEN","ROOTS"},{"COOLANT","ICE 9","CHILL"},{"PORT 443","CARGO","SHIP IT"}};return words[int(b)][positiveMod(i,3)];}
const char* billboardCaption(int i){static constexpr const char* words[]{"OPEN ALL NIGHT","KIND OF CUTE","NO BUGS ALLOWED","PATCH TUESDAY"};return words[positiveMod(i,4)];}
// Right-hand roof prop, drawn from a per-district pool; catwalks only carry light fixtures.
int propKind(const Platform& p){
    if(p.coffee)return 5;
    if(p.style==RoofStyle::Catwalk){static constexpr int light[]{7,3,4,6,7,3};return light[p.decoration%6];}
    static constexpr int pools[4][6]{{0,4,3,8,7,0},{2,11,6,2,3,11},{1,9,12,0,1,9},{10,4,6,10,1,10}};
    return pools[int(p.biome)][p.decoration%6];
}

void machinery(Canvas& c,const Platform& p,int x,int y,const World& w){
    const auto& pal=palette(p.biome);Color glow=p.repaired?Teal:Amber;
    int cx=x+int(p.width*.47),top=y-124;
    if(p.arena){
        // Reinforced turbine hall, a real destination connected to the roof services.
        for(int side:{0,1}){int px=x+24+side*(int(p.width)-64);border(c,px,y-206,18,208,pal.edge,Ink);
            for(int k=0;k<7;++k)c.rect(px+3,y-198+k*28,12,9,alpha(glow,150));}
        c.rect(x+24,y-211,int(p.width)-46,12,Ink);c.rect(x+26,y-210,int(p.width)-50,2,pal.edge);
        for(int k=0;k<8;++k){int px=x+65+k*85;c.line(px,y-199,px+40,y-165,pal.edge,2);c.line(px+40,y-165,px+80,y-199,pal.edge,2);}
        cx=x+int(p.width*.60);top=y-162;
        border(c,cx-52,top,104,137,pal.edge,0xff132636u);
        c.ellipse(cx-42,top+12,84,84,Ink);c.ellipse(cx-37,top+17,74,74,alpha(pal.accent,55));
        for(int k=0;k<8;++k){double a=k*.7854+w.time*(p.repaired?1.8:.3);int ex=cx+int(std::cos(a)*30),ey=top+54+int(std::sin(a)*30);c.line(cx,top+54,ex,ey,pal.edge,6);}
        c.ellipse(cx-9,top+45,18,18,glow);centerText(c,cx-52,top+110,104,p.repaired?"SYSTEM ONLINE":"REACTOR SEALED",glow);
        for(int edge:{x+40,x+int(p.width)-48})if(!p.repaired){c.rect(edge,y-130,5,130,alpha(pal.secondary,130));for(int yy=y-130;yy<y;yy+=12)c.rect(edge-4,yy,13,2,pal.secondary);}
        return;
    }
    if(!p.service)return;
    if(p.facility==Facility::Generator){
        border(c,cx-45,top+34,90,87,Post,0xff183443u);
        for(int k=0;k<3;++k){int xx=cx-32+k*27;c.rect(xx,top+42,17,55,Ink);for(int n=0;n<7;++n)c.rect(xx+3,top+89-n*6,11,3,n<(p.repaired?7:2)?glow:Post);}
        for(int side:{-1,1})c.line(cx+side*45,top+70,cx+side*80,top+70,pal.edge,5);
    }else if(p.facility==Facility::Greenhouse){
        c.polygon({{cx-58,y-4},{cx-58,top+25},{cx,top-3},{cx+58,top+25},{cx+58,y-4}},alpha(Teal,27));
        c.line(cx-60,top+25,cx,top-5,pal.edge,3);c.line(cx,top-5,cx+60,top+25,pal.edge,3);
        for(int xx=cx-55;xx<=cx+55;xx+=22){c.line(xx,top+28,xx,y-5,pal.edge);int grow=p.repaired?32:17;c.line(xx,y-12,xx,y-12-grow,Teal,2);c.ellipse(xx-9,y-15-grow,12,6,Teal);c.ellipse(xx,y-11-grow,12,7,0xff98cf78u);}
        c.rect(cx-61,y-7,123,7,Post);if(p.repaired)for(int k=0;k<8;++k)c.dot(cx-49+k*14,top+32+int(std::fmod(w.time*25+k*14,64)),0xff83dfffu);
    }else if(p.facility==Facility::Pump){
        for(int side:{-1,1}){int xx=cx+side*28;border(c,xx-19,top+10,38,108,Post,Ink);c.rect(xx-13,top+22,26,87,alpha(pal.accent,80));int fill=p.repaired?62:18;c.rect(xx-12,y-14-fill,24,fill,alpha(0xff77dfffu,150));for(int k=0;k<4;++k)c.ellipse(xx-6+(k%2)*7,y-20-int(std::fmod(w.time*16+k*13,fill)),3,3,White);}
        c.line(cx-70,y-13,cx+70,y-13,pal.edge,6);c.ellipse(cx-12,top+49,24,24,Post);c.line(cx-8,top+61,cx+8,top+61,glow,3);
    }else{
        c.line(cx,top+25,cx,y-5,Post,7);c.polygon({{cx-45,top+8},{cx+43,top+8},{cx+26,top+38},{cx-20,top+42}},pal.edge);c.line(cx,top+31,cx+17,top-10,glow,3);
        for(int k=0;k<3;++k){int r=12+k*14+int(std::fmod(w.time*9,14));c.ellipse(cx+17-r/2,top-10-r/2,r,r,alpha(glow,p.repaired?60-k*15:16));}
        border(c,cx-40,y-45,80,38,Post,Ink);centerText(c,cx-40,y-31,80,p.repaired?"LINK / OK":"NO SIGNAL",glow);
    }
    border(c,cx-76,top-26,152,19,pal.edge,Ink);centerText(c,cx-76,top-20,152,p.repaired?"SERVICE RESTORED":facilityName(p.facility),glow);
    c.line(cx+57,y-20,x+int(p.width)-12,y-20,pal.edge,2);
    for(int k=0;k<4;++k){int xx=cx+58+int(std::fmod(w.time*(p.repaired?38:0)+k*25,std::max(1,int(p.width*.5)-78)));c.rect(xx,y-21,5,3,glow);}
}
void bossArt(Canvas& c,const Enemy& e,int x,int y,double t){
    const Color col=e.bossKind==0?Amber:e.bossKind==1?Teal:e.bossKind==2?0xffaaadffu:0xffff80b4u;
    double a=t*2;int pulse=int(3*std::sin(a));
    c.ellipse(x-61,y-64,122,128,alpha(col,e.stage==0?18:8));
    if(e.bossKind==0){
        for(int side:{-1,1}){int xx=x+side*39,arm=int(e.stage==0?std::sin(std::fmod(e.attackClock,3.8)*1.57)*35:e.stage==1?-16:0);
            c.line(x+side*23,y-17,xx,y-14-arm,Post,13);border(c,xx-13,y-21-arm,26,31,col,0xff304353u);
            c.line(x+side*17,y+22,x+side*24,y+49,Post,14);border(c,x+side*24-14,y+41,30,14,Post,Ink);}
        border(c,x-29,y-33,58,68,Post,0xff2d4454u);c.rect(x-24,y-30,48,4,col);
        border(c,x-20,y-59,40,32,col,Ink);c.rect(x-14,y-48,28,6,col);c.rect(x-4,y-4,8,23,col);
        for(int k=0;k<3;++k)c.rect(x-17+k*13,y+24,8,3,Amber);
    }else if(e.bossKind==1){
        for(int k=-1;k<=1;++k){int nx=x+k*34+int(std::sin(a+k)*9),ny=y-40-std::abs(k)*10+pulse;c.line(x,y+23,nx,ny+12,0xff2c766au,14);c.line(x,y+23,nx,ny+12,Teal,3);c.ellipse(nx-20,ny-16,40,32,0xff164b48u);c.ellipse(nx-15,ny-12,30,18,Teal);c.rect(nx-10,ny-6,6,5,Ink);c.rect(nx+5,ny-6,6,5,Ink);}
        c.ellipse(x-42,y+9,84,45,0xff21554fu);c.ellipse(x-29,y+18,58,24,alpha(Teal,130));
    }else if(e.bossKind==2){
        int flap=int(std::sin(a)*20);c.polygon({{x,y-16},{x-77,y-30+flap},{x-48,y+22},{x,y+12},{x+48,y+22},{x+77,y-30+flap}},0xff485381u);
        c.line(x-74,y-27+flap,x,y+9,col,3);c.line(x,y+9,x+74,y-27+flap,col,3);
        c.ellipse(x-23,y-25,46,50,Ink);c.ellipse(x-16,y-19,32,34,Post);c.rect(x-11,y-7,22,6,col);
        c.line(x,y+22,x+int(std::sin(a)*21),y+58,col,3);
    }else{
        c.ellipse(x-32,y-35,64,70,Post);c.ellipse(x-25,y-28,50,56,Ink);c.ellipse(x-16,y-19,32,38,col);c.rect(x-8,y-6,16,10,White);
        for(int k=0;k<6;++k){double aa=a*.65+k*1.047;int xx=x+int(std::cos(aa)*55),yy=y+int(std::sin(aa)*48);c.line(x,y,xx,yy,alpha(col,90),2);border(c,xx-10,yy-10,20,20,Post,Ink);c.rect(xx-5,yy-5,10,10,col);}
    }
    if(e.hit>0)c.ellipse(x-17,y-13,34,26,alpha(White,100));
    centerText(c,x-100,y-91,200,bossName(e.bossKind),col);
    border(c,x-70,y-77,140,7,Post,Ink);c.rect(x-68,y-75,136*e.hp/e.maxHp,3,col);
    centerText(c,x-100,y-105,200,e.stage==0?"ATTACK CHARGING":e.stage==1?"DODGE!":"CORE EXPOSED",e.stage==2?Teal:Amber);
}
}
Renderer::Renderer(){buildSkies();}
void Renderer::buildSkies(){
 for(int b=0;b<4;++b){
    // One shared star/cloud layout for every district so a sky crossfade is a pure recolor, never a ghosted double image.
    const auto& p=palette(Biome(b));Rng random{4551};
    sky_[b]=Canvas(ViewW,ViewH);auto& sky=sky_[b];
    for(int y=0;y<ViewH;++y)sky.rect(0,y,ViewW,1,mix(p.skyTop,p.skyBottom,double(y)/ViewH));
    for(int i=0;i<85;++i){int x=random.range(0,959),y=random.range(20,230);sky.dot(x,y,mix(p.skyBottom,White,0.3));if(i%12==0){sky.dot(x-1,y,alpha(White,40));sky.dot(x+1,y,alpha(White,40));}}
    sky.ellipse(722,46,155,155,alpha(p.accent,8));sky.ellipse(736,60,127,127,alpha(p.accent,12));
    sky.ellipse(754,79,89,89,mix(p.skyBottom,p.accent,0.19));sky.ellipse(757,81,80,83,mix(p.skyBottom,p.accent,0.24));
    sky.ellipse(774,92,29,19,mix(p.skyBottom,p.accent,0.17));sky.ellipse(788,130,33,22,mix(p.skyBottom,p.accent,0.15));
    for(int i=0;i<9;++i){int x=random.range(-100,960),y=random.range(75,240),w=random.range(60,230);sky.rect(x,y,w,4,alpha(p.skyBottom,55));sky.rect(x+20,y+4,w+15,3,alpha(p.skyBottom,30));}
 }
}
void Renderer::buildChunk(Canvas& c,int layer,std::int64_t index,std::uint64_t seed,int forced){
    c=Canvas(ChunkW,ChunkH);
    Rng random{seed*0x9e3779b97f4a7c15ULL^std::uint64_t(index)*0xbf58476d1ce4e5b9ULL^std::uint64_t(layer+1)*0x94d049bb133111ebULL};
    double parallax=layer?NearParallax:FarParallax;int x=0;
    while(x<ChunkW){
        int width=random.range(34,91);if(ChunkW-x-width<34)width=ChunkW-x;
        int height=random.range(layer?60:85,layer?230:295),top=ChunkH-height;
        // Every building takes its colors from the district at its own world position, so a
        // district line appears on the horizon as a left-to-right gradient, well before the gate.
        DistrictMix d=forced>=0?DistrictMix{Biome(forced),Biome(forced),0}:districtAt((double(index)*ChunkW+x+width*0.5)/parallax);
        Palette p=paletteAt(d);Biome flavor=dominant(d);
        Color face=layer?p.near:p.far,side=mix(face,Ink,0.28);
        c.rect(x,top,width-6,height,face);c.rect(x+width-15,top,9,height,side);c.rect(x+4,top-4,width-19,4,side);
        if(random.range(0,2)==0){c.line(x+width/2,top-25,x+width/2,top,side,2);c.dot(x+width/2,top-26,p.secondary);}
        for(int wy=top+12;wy<ChunkH-20;wy+=random.range(10,14))for(int wx=x+8;wx<x+width-17;wx+=10)if(random.range(0,4)<2){Color light=random.range(0,3)==0?p.secondary:p.accent;c.rect(wx,wy,3,4,alpha(light,layer?60:30));}
        if(layer&&width>65){int sy=top+29;c.rect(x+13,sy,width-37,19,Ink);c.rect(x+14,sy+1,width-39,17,alpha(p.secondary,20));centerText(c,x+12,sy+6,width-35,flavor==Biome::Memory?"GROW":flavor==Biome::Network?"PORT":flavor==Biome::Cooling?"COOL":"QQ",alpha(p.secondary,140));}
        if(layer&&random.range(0,3)==0){c.rect(x+width-12,top+18,3,height-37,alpha(p.accent,100));c.rect(x+width-11,top+20,1,height-40,alpha(White,65));}
        if(layer){ // District-flavored rooftop silhouettes on the near skyline.
            int cx=x+(width-6)/2;
            switch(flavor){
            case Biome::Rooftops:if(width>50){c.rect(x+10,top-12,width-30,8,alpha(p.accent,90));c.rect(x+12,top-10,width-34,2,alpha(White,60));}break;
            case Biome::Memory:for(int k=0;k<3;++k){int tx=x+10+k*((width-6)/3);c.ellipse(tx-8,top-14,17,15,mix(p.near,0xff6f9d6au,0.55));c.ellipse(tx-5,top-17,10,9,mix(p.near,0xff9dc98au,0.5));}c.rect(x+6,top-3,width-18,3,mix(p.near,0xff517c60u,0.6));break;
            case Biome::Cooling:c.rect(cx-9,top-30,7,30,side);c.rect(cx+2,top-22,7,22,side);c.rect(cx-10,top-33,9,3,Ink);c.rect(cx+1,top-25,9,3,Ink);c.ellipse(cx-14,top-44,18,10,alpha(White,22));break;
            case Biome::Network:c.rect(cx-2,top-46,4,46,side);c.rect(cx-2,top-46,36,3,side);c.line(cx+30,top-43,cx+30,top-18,alpha(p.edge,180));c.rect(cx+26,top-18,9,6,p.accent);c.dot(cx,top-48,p.secondary);break;
            }
        }
        x+=width;
    }
}
const Canvas& Renderer::chunk(int layer,std::int64_t index,std::uint64_t seed){
    for(auto& c:chunks_)if(c.layer==layer&&c.index==index&&c.seed==seed){c.used=tick_;return c.image;}
    if(chunks_.size()<12)chunks_.emplace_back();
    auto& slot=*std::min_element(chunks_.begin(),chunks_.end(),[](const Chunk& a,const Chunk& b){return a.used<b.used;});
    slot.layer=layer;slot.index=index;slot.seed=seed;slot.used=tick_;buildChunk(slot.image,layer,index,seed);return slot.image;
}
void Renderer::street(const World& w,const Palette& p){
    // Street level far below the roofs: fog, distant lights and slow traffic. It shows through
    // every gap and under catwalks instead of a flat backdrop.
    double cam=w.absoluteCamera();Color top=mix(p.skyBottom,Ink,0.58);
    for(int y=378;y<ViewH;++y)frame_.rect(0,y,ViewW,1,mix(top,Ink,double(y-378)/(ViewH-378)));
    frame_.rect(0,384,ViewW,3,alpha(p.accent,30));
    for(int i=0;i<72;++i){int lx=int(wrap(i*173.7+((i*i)%97)*5-cam*StreetParallax,1920.0));if(lx>=ViewW)continue;int ly=396+(i*37)%118;frame_.rect(lx,ly,2,i%4==0?2:1,alpha(i%3?p.secondary:p.accent,36+(i*29)%50));}
    for(int i=0;i<6;++i){double speed=(i%2?1:-1)*(38+i*9);int x=int(wrap(i*257.0+w.time*speed-cam*StreetParallax,1240.0))-140,y=468+i*11;frame_.rect(x,y,14,1,alpha(i%2?Amber:White,70));frame_.dot(i%2?x+14:x-1,y,i%2?0xffff8a6bu:White);}
}
void Renderer::prop(int kind,int x,int floor,const World& w,const Metrics& m){
    frame_.blit(assets_.props[kind],x,floor-112);
    if(kind==1){double speed=1.8+(m.cpu>=0?std::min(m.cpu,100.0)*0.035:1.0);double a=w.time*speed;
        for(int i=0;i<4;++i){double t=a+i*1.5707963;int xx=int(15*std::cos(t)),yy=int(15*std::sin(t));frame_.line(x+40,floor-33,x+40+xx,floor-33+yy,0xff6b98a7u,5);}
        frame_.ellipse(x+37,floor-36,7,7,Teal);frame_.line(x+19,floor-33,x+61,floor-33,alpha(Ink,110));
    }
    if(kind==5){double t=std::fmod(w.time,2.5);int sy=int(t*11);frame_.dot(x+35+int(std::sin(t*4)*3),floor-38-sy,alpha(White,int(140*(1-t/2.5))));}
    if(kind==11){int g=int(26+14*std::sin(w.time*1.7));for(int k=0;k<2;++k)frame_.ellipse(x+21+k*20,floor-60,25,32,alpha(Amber,g));}
    if(kind==12){for(int k=0;k<2;++k){double t=std::fmod(w.time*0.5+k*0.5,1.0);int px=x+(k?52:26)+int(std::sin(t*6+k)*3),py=floor-112+(k?34:14)-6-int(t*34);frame_.ellipse(px-4,py,8+int(t*6),4+int(t*2),alpha(White,int(70*(1-t))));}}
}
void Renderer::gate(const Platform& p,int x,int y,const World& w){
    // A lit checkpoint arch on the first roof of each district: the line itself, made visible.
    const auto& pal=palette(p.biome);constexpr int span=216;int gx=x+8,top=y-214;
    double pulse=0.5+0.5*std::sin(w.time*2.2+double(p.district));
    for(int px:{gx,gx+span-12}){frame_.rect(px,top,12,y-top,Ink);frame_.rect(px+2,top+4,3,y-top-8,alpha(pal.edge,150));frame_.rect(px+8,top+4,2,y-top-8,alpha(pal.accent,90));
        frame_.ellipse(px-4,top-18,20,20,alpha(pal.accent,int(28+42*pulse)));frame_.ellipse(px+2,top-12,8,8,Ink);frame_.ellipse(px+3,top-11,6,6,mix(pal.accent,White,0.35));}
    frame_.rect(gx,top,span,14,Ink);frame_.rect(gx+2,top+2,span-4,3,pal.edge);frame_.rect(gx+2,top+9,span-4,2,alpha(pal.accent,120));
    for(int k=gx+6;k<gx+span-6;k+=12)frame_.dot(k,top+6,alpha(pal.accent,int(120+100*pulse)));
    int bx=gx+8,by=top+22,bw=span-16,bh=34;
    frame_.rect(bx-6,by-6,bw+12,bh+12,alpha(pal.accent,22));frame_.rect(bx,by,bw,bh,Ink);border(frame_,bx+2,by+2,bw-4,bh-4,pal.edge,Ink);
    centerText(frame_,bx,by+10,bw,biomeName(p.biome),pal.accent,2);
    frame_.rect(bx+bw/2-42,by+bh+3,84,11,Ink);centerText(frame_,bx+bw/2-42,by+bh+5,84,"DISTRICT LINE",Muted);
    frame_.rect(gx-8,y-3,span+16,3,alpha(pal.accent,int(120+80*pulse)));
}
void Renderer::billboard(const Platform& p,int x,int y,const World& w){
    const auto& pal=palette(p.biome);int bw=144,bh=54,bx=x+120,by=y-206;
    double glow=0.7+0.3*std::sin(w.time*0.8+double(p.id));
    frame_.rect(bx+14,by+bh,5,y-by-bh,Post);frame_.rect(bx+bw-19,by+bh,5,y-by-bh,Post);frame_.line(bx+16,by+bh+30,bx+bw-17,by+bh+30,Post,2);
    frame_.rect(bx-8,by-8,bw+16,bh+16,alpha(pal.secondary,int(18*glow)));
    frame_.rect(bx,by,bw,bh,Ink);border(frame_,bx+2,by+2,bw-4,bh-4,alpha(pal.secondary,int(200*glow)),0xff0c1a24u);
    centerText(frame_,bx,by+11,bw,billboardText(p.biome,p.decoration),alpha(pal.secondary,int(255*glow)),2);
    centerText(frame_,bx,by+33,bw,billboardCaption(p.structure+int(p.id)),Muted);
}
void Renderer::cable(const Platform& a,const Platform& b,const World& w){
    // Sagging service line between neighboring roofs, with a lamp at the low point.
    int x1=int(std::round(a.x+a.width-w.camera))-8,y1=int(a.y)-62,x2=int(std::round(b.x-w.camera))+8,y2=int(b.y)-62;
    if(x2-x1<=0||x2<-20||x1>ViewW+20)return;
    frame_.rect(x1-1,y1,3,int(a.y)-y1,Post);frame_.rect(x2-1,y2,3,int(b.y)-y2,Post);frame_.dot(x1,y1-1,Amber);frame_.dot(x2,y2-1,Amber);
    double sag=(x2-x1)*0.22;int px=x1,py=y1;
    for(int s=1;s<=8;++s){double u=s/8.0;int nx=x1+int((x2-x1)*u),ny=int(y1+(y2-y1)*u+sag*4*u*(1-u));frame_.line(px,py,nx,ny,Ink,2);frame_.line(px,py,nx,ny,alpha(Muted,90),1);px=nx;py=ny;}
    int mx=x1+(x2-x1)/2,my=int(y1+(y2-y1)*0.5+sag);frame_.rect(mx-1,my+1,3,4,Ink);frame_.dot(mx,my+3,Amber);
}
void Renderer::weather(Biome biome,int strength,const World& w){
    // Sparse slow weather and dust. No flashing/strobing or screen shake.
    if(strength<=0)return;
    const auto& p=palette(biome);double cam=w.absoluteCamera();
    for(int i=0;i<24;++i){int x=int(wrap(i*107.31-w.time*9-cam*.07,ViewW)),y=int(wrap(i*61.3+w.time*12,ViewH));
        if(biome==Biome::Memory)frame_.line(x,y,x+3,y-1,alpha(p.accent,65*strength/255));else frame_.line(x,y,x-2,y+5,alpha(p.accent,24*strength/255));}
}
void Renderer::sign(const Platform& p,int x,int y,const World& w,const Metrics& m){
    constexpr int sw=224,sh=116;const auto& colors=palette(p.biome);
    // Bolted, hooded street display. Metric text is a world object, not a HUD.
    frame_.rect(x+22,y+sh,5,int(p.y)-y-sh,Post);frame_.rect(x+sw-30,y+sh,5,int(p.y)-y-sh,Post);
    frame_.rect(x-5,y-5,sw+10,sh+10,Ink);border(frame_,x-2,y-2,sw+4,sh+4,colors.edge,Ink);
    frame_.rect(x+4,y+4,sw-8,sh-8,0xff0c232au);frame_.rect(x+4,y+4,sw-8,18,0xff193b40u);
    for(int sx:{0,sw-2})for(int sy:{0,sh-2})frame_.dot(x+sx,y+sy,White);
    frame_.text(x+12,y+10,p.sign==0?"CPU REACTOR":p.sign==1?"MEMORY BANK":p.sign==2?"PACKET PORT":"UPTIME TOWER",colors.accent);
    std::string tag=m.mode==MetricMode::Demo?"DEMO":m.mode==MetricMode::Hidden?"ART":"LIVE";
    frame_.text(x+sw-12-Canvas::textWidth(tag),y+10,tag,m.mode==MetricMode::Demo?Amber:colors.accent);
    if(m.mode==MetricMode::Hidden){centerText(frame_,x,y+34,sw,"STAY CURIOUS",colors.accent,2);centerText(frame_,x,y+62,sw,"KIND OF CUTE",White,2);centerText(frame_,x,y+94,sw,"NO SYSTEM DATA DISPLAYED",Muted);return;}
    bool stale=m.mode==MetricMode::Live && (m.sampled.time_since_epoch().count()==0||std::chrono::steady_clock::now()-m.sampled>std::chrono::seconds(4));
    double value=p.sign==0?m.cpu:m.memory;
    if(p.sign==0||p.sign==1){
        std::string big=stale?"--":number(value)+"%";
        frame_.text(x+13,y+33,big,White,3);
        frame_.text(x+sw-46,y+44,p.sign==0?"BUSY":"USED",colors.accent);
        std::string sub=p.sign==0?"LOAD1 "+number(stale?-1:m.load1,2):number(stale?-1:m.memoryUsedGiB)+" / "+number(stale?-1:m.memoryTotalGiB)+" GIB";
        frame_.text(x+14,y+63,sub,Muted);
        border(frame_,x+13,y+82,sw-26,13,0xff2d5559u,Ink);
        if(value>=0&&!stale){int fill=int((sw-32)*std::clamp(value/100.0,0.0,1.0));frame_.rect(x+16,y+85,fill,7,colors.accent);for(int i=20;i<sw-22;i+=12)frame_.rect(x+i,y+85,1,7,alpha(Ink,75));}
        frame_.text(x+14,y+102,stale?"WAITING FOR FRESH SAMPLE":"READ ONLY / 1 HZ",stale?Amber:Muted);
    }else if(p.sign==2){
        frame_.text(x+14,y+32,"RX "+number(stale?-1:m.rxKiB,0),White,2);
        frame_.text(x+14,y+53,"TX "+number(stale?-1:m.txKiB,0),colors.accent,2);
        frame_.text(x+14,y+78,"KIB/S / ONE INTERFACE",Muted);
        std::string iface=m.interface.empty()?"NO IPV4 DEFAULT ROUTE":m.interface.substr(0,29);
        frame_.text(x+14,y+101,stale?"WAITING FOR FRESH SAMPLE":iface,Muted);
    }else{
        double up=stale?-1:m.uptime;std::string valueText="--";
        if(up>=0){auto seconds=std::uint64_t(up);valueText=std::to_string(seconds/86400)+"D "+std::to_string(seconds/3600%24)+"H";}
        frame_.text(x+14,y+34,valueText,White,3);
        frame_.text(x+14,y+65,"ON PATROL / "+std::to_string(w.cleared)+" PATCHED",colors.accent);
        frame_.text(x+14,y+84,"FICTIONAL ENEMIES ONLY",Muted);
        frame_.text(x+14,y+102,stale?"WAITING FOR FRESH SAMPLE":"REAL UPTIME / NO OS CHANGES",Muted);
    }
}
const Canvas& Renderer::render(const World& w,const Metrics& m,bool overlay){
    ++tick_;
    double cam=w.absoluteCamera();
    DistrictMix ambient=districtAt(cam+ViewW*0.5);Palette p=paletteAt(ambient);
    Biome biome=w.currentBiome();
    // Sky: a per-pixel crossfade while the screen center is inside a district blend.
    if(ambient.t<=0||ambient.from==ambient.to)frame_.pixels=sky_[int(ambient.from)].pixels;
    else if(ambient.t>=1)frame_.pixels=sky_[int(ambient.to)].pixels;
    else frame_.crossfade(sky_[int(ambient.from)],sky_[int(ambient.to)],int(std::lround(ambient.t*256)));
    // World-anchored parallax skylines from cached chunks; district color is baked per building.
    auto layer=[&](int l,double parallax,int y){double lx=cam*parallax;auto first=std::int64_t(std::floor(lx/ChunkW));int offset=int(lx-double(first)*ChunkW);for(int i=0;i*ChunkW-offset<ViewW;++i)frame_.blit(chunk(l,first+i,w.seed()),i*ChunkW-offset,y);};
    layer(0,FarParallax,-3);layer(1,NearParallax,62);
    street(w,p);
    // Distant transit line: packet dots travel independently of the heroes.
    frame_.line(0,354,ViewW,354,alpha(p.edge,160),2);frame_.line(0,359,ViewW,359,Ink,2);
    for(int i=0;i<14;++i){int x=int(wrap(i*99+w.time*22-cam*.45,1100))-50;frame_.rect(x,349,9,3,alpha(p.accent,130));}
    const auto& plats=w.platforms;
    for(std::size_t i=0;i<plats.size();++i){
        const auto& platform=plats[i];const Platform* next=i+1<plats.size()?&plats[i+1]:nullptr;
        int x=int(std::round(platform.x-w.camera)),y=int(platform.y),width=int(platform.width),cols=width/Tile;
        if(x>ViewW+80||x+width<-80)continue;
        const auto& pal=palette(platform.biome);const auto& tiles=assets_.tiles[int(platform.biome)];
        // Individual 32 px modular tiles, not one repeated background image.
        int first=std::max(0,(-x)/Tile-1),last=std::min(cols,((ViewW-x)/Tile)+2);
        for(int col=first;col<last;++col)frame_.blit(tiles[col==0?0:col==cols-1?2:1],x+col*Tile,y);
        if(platform.style==RoofStyle::Building){
            int pipeRow=2+platform.pattern%2;
            for(int col=first;col<last;++col)for(int row=1;y+row*Tile<ViewH;++row){int kind=row==pipeRow?6:(int(platform.id)+col*(platform.pattern+1)+row*3)%5+3;frame_.blit(tiles[kind],x+col*Tile,y+row*Tile);}
            // Recessed machinery, foreground facade conduits and roof studs.
            frame_.rect(x,y+29,width,2,Ink);frame_.rect(x+8,y+63,width-16,1,alpha(pal.accent,50));
            for(int k=0;k<3;++k){int sx=x+24+k*83;if(sx+20<x+width)frame_.rect(sx,y+10,20,1,alpha(White,70));}
        }else{
            // Catwalk: deck, lattice truss and pillars, with the street visible beneath.
            frame_.rect(x,y+32,width,6,Ink);frame_.rect(x+1,y+33,width-2,2,pal.edge);
            for(int k=0;k<width;k+=32){frame_.line(x+k,y+38,x+k+32,y+58,Ink,2);frame_.line(x+k+32,y+38,x+k,y+58,Ink,2);}
            frame_.rect(x,y+56,width,5,Ink);frame_.rect(x+1,y+57,width-2,1,alpha(pal.edge,120));
            for(int px=x+14;px<x+width-10;px+=96){int h=ViewH-(y+61);frame_.rect(px,y+61,10,h,Ink);frame_.rect(px+2,y+61,3,h,alpha(pal.edge,70));frame_.rect(px+7,y+61,1,h,alpha(pal.accent,45));
                if(px+96<x+width-10){frame_.line(px+10,y+120,px+96,y+150,Ink,2);frame_.line(px+10,y+150,px+96,y+120,Ink,2);}}
        }
        machinery(frame_,platform,x,y,w);
        if(platform.gate)gate(platform,x,y,w);
        else if(!platform.arena&&!platform.service){
            prop(propKind(platform),x+width-102,y,w,m);
            if(platform.sign>=0)sign(platform,x+52,y-192,w,m);
            else{
                static constexpr int lefts[]{6,7,3,4};prop(lefts[platform.pattern],x+30,y,w,m);
                if(width>=12*Tile&&platform.structure<3)billboard(platform,x,y,w);
                else{
                    if(width>=352&&platform.structure%2)prop(platform.biome==Biome::Memory?2:0,x+117,y,w,m);
                    // Tiny illuminated street tag.
                    frame_.rect(x+110,y-178,138,21,Ink);frame_.text(x+119,y-171,biomeName(platform.biome),pal.accent);
                    frame_.line(x+130,y-157,x+130,y-113,pal.edge,2);
                }
            }
            if(platform.structure>=5&&platform.style==RoofStyle::Building){ // Back railing along the roof.
                frame_.rect(x+8,y-16,width-16,2,Ink);frame_.rect(x+8,y-15,width-16,1,alpha(pal.edge,160));
                for(int px=x+12;px<x+width-12;px+=32)frame_.rect(px,y-16,2,16,Ink);}
        }
        // District machinery connects the traversable roofs to the city below.
        if(platform.biome==Biome::Memory){
            for(int k=18;k<width;k+=79){int stem=x+k;frame_.line(stem,y+28,stem+6,y+104,0xff3b6856u,2);for(int j=0;j<5;++j){int yy=y+34+j*13;frame_.ellipse(stem-5+(j%2)*7,yy,11,5,0xff578768u);}}
            if(!platform.gate&&!platform.service&&!platform.arena&&platform.sign<0&&width>=384){int cx=x+width/2;frame_.line(cx,y-8,cx,y-91,Post,4);for(int j=0;j<4;++j){int ly=y-64-j*12;frame_.ellipse(cx-51+j*6,ly,102-j*12,20,alpha(0xff80c4a3u,80));frame_.line(cx-42+j*5,ly+12,cx+42-j*5,ly+12,Teal);}}
        }
        if(platform.biome==Biome::Cooling){
            for(int k=24;k<width;k+=96){frame_.rect(x+k,y+34,15,ViewH-y,0xff304e65u);frame_.rect(x+k+3,y+37,4,ViewH-y,alpha(0xffb8e8ffu,120));for(int j=0;j<4;++j){int yy=y+36+int(std::fmod(w.time*31+j*30,150.));frame_.rect(x+k+5,yy,3,7,0xff74e1ffu);}}
            for(int k=0;k<width;k+=39)frame_.polygon({{x+k,y+2},{x+k+9,y+2},{x+k+4,y+15+(k%3)*4}},0xff8badc4u);
        }
        if(platform.biome==Biome::Network&&!platform.gate&&!platform.service&&!platform.arena&&platform.sign<0&&width>=384){
            int cx=x+width-145;frame_.rect(cx,y-161,7,160,Post);frame_.line(cx-22,y-150,cx+94,y-150,Post,5);frame_.line(cx+3,y-190,cx+88,y-151,pal.edge,2);frame_.line(cx+3,y-190,cx-22,y-151,pal.edge,2);
            int hook=cx+54+int(std::sin(w.time*.35+platform.id)*24),hy=y-88+int(std::sin(w.time*.22)*12);frame_.line(hook,y-149,hook,hy,pal.edge);border(frame_,hook-24,hy,48,29,0xffb68553u,0xff775331u);for(int k=0;k<5;++k)frame_.rect(hook-20+k*9,hy+4,2,21,0xffc28c59u);centerText(frame_,hook-24,hy+10,48,"CARGO",White);
        }
        if(platform.biome==Biome::Memory){for(int k=0;k<width;k+=57){frame_.line(x+k+7,y+2,x+k+10,y-9,0xff87b986u);frame_.line(x+k+10,y-4,x+k+17,y-8,0xff87b986u);}}
        if(platform.biome==Biome::Cooling&&platform.style==RoofStyle::Building&&platform.decoration%2==0){ // Roof vent steam.
            int vx=x+int(width*0.35);frame_.rect(vx-6,y-6,12,6,Ink);frame_.rect(vx-4,y-4,8,2,pal.edge);
            for(int k=0;k<3;++k){double t=std::fmod(w.time*0.55+k*0.33+double(platform.id)*0.1,1.0);frame_.ellipse(vx-4+int(std::sin((t+k)*5)*4),y-10-int(t*46),8+int(t*8),5+int(t*3),alpha(White,int(80*(1-t))));}}
        // A real gap separates roofs; animated packet water runs beneath it between buildings.
        int end=x+width,gap=next?std::max(Tile,int(std::round(next->x-platform.x-platform.width))):64;
        if(platform.style==RoofStyle::Building&&(!next||next->style==RoofStyle::Building)){
            frame_.rect(end,y+87,gap,40,0xff142731u);
            for(int r=0;r<3;++r){int rx=end+positiveMod(int(w.time*13+r*19),gap);frame_.rect(rx,y+94+r*11,7,1,alpha(pal.accent,95));}}
        if(platform.cable&&next)cable(platform,*next,w);
    }
    // Harmless enemies, with distinct silhouettes and occasional larger elites.
    for(const auto& e:w.enemies){int x=int(std::round(e.pos.x-w.camera)),y=int(e.pos.y);if(x<-80||x>ViewW+80)continue;
        int scale=e.boss?2:1;frame_.ellipse(x-21,y+20,43,8,alpha(Ink,100));
        int f=int(w.time*9+e.phase)%8;
        if(e.boss){bossArt(frame_,e,x,y,w.time);continue;}
        frame_.blit(assets_.enemies[int(e.kind)*8+f],x-28*scale,y-28*scale,scale);
        if(e.hit>0)frame_.ellipse(x-6,y-4,12,8,alpha(White,int(e.hit/0.12*150)));
        std::string label=e.boss?"BIG "+std::string(enemyName(e.kind)):enemyName(e.kind);
        centerText(frame_,x-70,y-34*scale,140,label,0xffc4a4bdu);
        if(e.boss){frame_.rect(x-29,y-34*scale-8,58,3,Ink);frame_.rect(x-28,y-34*scale-7,e.hp*6,1,Amber);}
    }
    for(const auto& item:w.pickups){int xx=int(item.pos.x-w.camera),yy=int(item.pos.y+std::sin(w.time*4)*5);frame_.ellipse(xx-19,yy-19,38,38,alpha(Teal,25));border(frame_,xx-11,yy-11,22,22,Teal,Ink);frame_.text(xx-2,yy-3,item.kind==Power::Rapid?"R":item.kind==Power::Shield?"S":"A",White);}
    for(const auto& h:w.hazards){int xx=int(h.pos.x-w.camera),yy=int(h.pos.y);Color co=h.kind==1?Teal:h.kind==2?0xffbfa9ffu:Amber;frame_.ellipse(xx-14,yy-14,28,28,alpha(co,35));frame_.ellipse(xx-6,yy-6,12,12,co);frame_.dot(xx-2,yy-2,White);}
    // Soft contact shadows and the reference-matched duo.
    int hx=int(std::round(w.hero.pos.x-w.camera)),hy=int(std::round(w.hero.pos.y));
    if(const auto* floor=w.platform(w.hero.platform)){double height=std::max(0.0,floor->y-w.hero.pos.y);int sw=int(std::clamp(50-height*.15,20.0,50.0));frame_.ellipse(hx-sw/2,int(floor->y)-2,sw,8,alpha(Ink,110));}
    int frame=int(w.time*12)%8,state=w.hero.coffee>0?3:w.hero.airborne?2:0;
    if(w.hero.powerTime>0){Color co=w.hero.power==Power::Shield?0xff8cbdffu:Teal;frame_.ellipse(hx-34,hy-87,68,86,alpha(co,22));for(int k=0;k<8;++k){double a=w.time*2+k*.7854;frame_.rect(hx+int(std::cos(a)*34),hy-43+int(std::sin(a)*42),3,3,co);}}
    frame_.blit(assets_.penguin[state*8+frame],hx-40,hy-83);
    if(w.hero.serviceTime>0){frame_.line(hx+20,hy-38,hx+43,hy-25,Teal,2);frame_.text(hx-30,hy-103,"REPAIRING...",Teal);}
    if(w.victory>0){centerText(frame_,220,93,520,"GUARDIAN CLEARED / DISTRICT RESTORED",Teal,2);}
    if(w.hero.powerTime>0){frame_.text(28,77,std::string(powerName(w.hero.power))+" / "+std::to_string(int(w.hero.powerTime))+"S",Teal);}
    int dx=int(std::round(w.duck.x-w.camera)),dy=int(std::round(w.duck.y));
    frame_.blit(assets_.duck[(w.duckFire>0.45?8:0)+frame],dx-32,dy-32);
    if(w.hero.coffee>0){frame_.rect(hx-21,hy-115,118,21,Ink);frame_.text(hx-13,hy-108,"COFFEE = UPTIME",Amber);frame_.polygon({{hx+10,hy-94},{hx+17,hy-94},{hx+10,hy-89}},Ink);}
    for(const auto& bolt:w.bolts){int x=int(bolt.pos.x-w.camera),y=int(bolt.pos.y);Color color=bolt.duck?0xff80d8ffu:Teal;double length=std::hypot(bolt.velocity.x,bolt.velocity.y);int tx=x-int(bolt.velocity.x/length*12),ty=y-int(bolt.velocity.y/length*12);frame_.line(tx,ty,x,y,alpha(color,35),7);frame_.line(tx,ty,x,y,color,3);frame_.dot(x,y,White);}
    for(const auto& v:w.particles){int x=int(v.pos.x-w.camera),y=int(v.pos.y);Color color=v.color==0?Teal:v.color==1?0xff8bd8ffu:0xffe6a4d4u;frame_.rect(x,y,2,2,alpha(color,int(std::clamp(v.life/v.initialLife,0.0,1.0)*255)));}
    // Weather belongs to the district; both kinds overlap at fading strengths across a line.
    weather(ambient.from,int(std::lround((1-ambient.t)*255)),w);if(ambient.t>0)weather(ambient.to,int(std::lround(ambient.t*255)),w);
    if(overlay){ // The subtle title drifts to avoid leaving a static logo.
        int ox=24+int(std::sin(w.time*.03)*8),oy=23+int(std::sin(w.time*.023)*5);
        frame_.text(ox+1,oy+1,"QINDA PATROL",Ink,2);frame_.text(ox,oy,"QINDA PATROL",0xffbfd4d4u,2);
        frame_.text(ox,oy+21,"KIND OF CUTE / NIGHT SHIFT",Muted);
        std::string zone=biomeName(biome);DistrictMix ahead=districtAt(w.absoluteHero());
        if(ahead.t>0&&ahead.from==biome&&ahead.to!=biome)zone="NEXT: "+std::string(biomeName(ahead.to));
        frame_.text(ViewW-26-Canvas::textWidth(zone),oy+2,zone,Muted);
        std::string count=std::to_string(w.cleared)+" PATCHED";frame_.text(ViewW-26-Canvas::textWidth(count),oy+19,count,p.accent);
    }
    // Always mark synthetic telemetry; never let a demo frame masquerade as live.
    if(m.mode==MetricMode::Demo){int x=24+int(std::sin(w.time*.04)*8);frame_.rect(x-4,ViewH-25,180,16,alpha(Ink,195));frame_.text(x,ViewH-21,"DEMO METRICS / AUTOPILOT",Amber);}
    return frame_;
}
bool Renderer::exportAssets(const std::string& path)const{
    if(!assets_.exportTo(path))return false;
    bool ok=true;
    for(int b=0;b<4;++b){ok=sky_[b].png(path+"/sky-"+std::to_string(b)+".png")&&ok;
        // Skyline strips: four chunks of each layer forced to one district, as a sample of the procedural set.
        for(int layer=0;layer<2;++layer){Canvas strip(1920,ChunkH);for(int i=0;i<4;++i){Canvas c;buildChunk(c,layer,i,4551,b);strip.blit(c,i*ChunkW,0);}
            ok=strip.png(path+"/skyline-"+std::string(layer?"near-":"far-")+std::to_string(b)+".png")&&ok;}}
    return ok;
}
}
