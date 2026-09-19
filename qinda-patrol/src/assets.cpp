#include "assets.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
namespace patrol {
namespace {
constexpr Color Ink=0xff08101du,Armor=0xff25344au,Steel=0xff53657cu,White=0xffe8f2f0u,Amber=0xffffb64du,Orange=0xffd67530u,Teal=0xff65f8c9u,Blue=0xff65cbffu;
void outlineEllipse(Canvas& c,int x,int y,int w,int h,Color color,int line=2){c.ellipse(x,y,w,h,Ink);c.ellipse(x+line,y+line,w-2*line,h-2*line,color);}
Canvas penguinFrame(int state,int frame){
    Canvas c(80,88);int bob=(state==0?(frame%4==0?1:0):0),stride=state==0?int(std::sin(frame*6.2831853/8)*4):0;
    int jump=state==2?2:0;
    // Wind-blown black cape with the reference's burnt-orange lining.
    c.polygon({{26,41},{9,44},{3,68+frame%3},{17,73},{31,60}},Ink);
    c.polygon({{24,44},{12,47},{7,66+frame%3},{18,69},{27,57}},Orange);
    c.polygon({{24,44},{11,44},{8,65},{20,62},{27,56}},Armor);
    // Orange armored boots; the legs actually alternate, not a sliding sprite.
    outlineEllipse(c,23+stride,68-jump,19,14,Orange);outlineEllipse(c,45-stride,68+jump,21,14,Amber);
    c.rect(25+stride,78-jump,16,5,Ink);c.rect(46-stride,78+jump,19,5,Ink);
    c.rect(29+stride,70-jump,8,3,Amber);c.rect(48-stride,71+jump,11,3,0xffffd68bu);
    outlineEllipse(c,20,36+bob,45,41,Armor);outlineEllipse(c,29,47+bob,29,26,0xffa9b9c6u,1);
    c.ellipse(31,48+bob,24,19,White);c.ellipse(47,53+bob,9,18,0xffb9c8d2u);
    c.polygon({{24,42},{44,43},{43,60},{28,63},{23,54}},Steel);
    c.polygon({{26,44},{40,46},{39,57},{29,59},{26,52}},Armor);
    c.text(29,49,"Q",Teal);c.rect(41,63+bob,17,4,Ink);c.rect(47,63+bob,5,4,Amber);
    // Oversized expressive head and two white cheek/face patches.
    outlineEllipse(c,15,5+bob,50,45,0xff364153u);
    c.ellipse(18,7+bob,43,37,0xff202b3cu);c.ellipse(20,8+bob,31,9,0xff556171u);
    c.ellipse(24,17+bob,22,29,White);c.ellipse(41,16+bob,20,28,White);
    c.polygon({{31,17+bob},{39,24+bob},{43,17+bob},{45,13+bob}},0xff202b3cu);
    bool blink=state==1 && frame>=6;
    if(blink)c.line(29,30+bob,35,30+bob,Ink,2);
    else {c.ellipse(30,26+bob,6,9,Ink);c.rect(32,26+bob,2,3,White);c.line(27,24+bob,35,26+bob,0xff465569u,2);}
    c.rect(27,37+bob,5,2,0xffd3a5a7u);
    // Mint cybernetic monocle and metallic temple/headset.
    c.polygon({{44,21+bob},{63,18+bob},{64,35+bob},{44,37+bob},{41,30+bob}},Ink);
    c.polygon({{46,23+bob},{61,21+bob},{61,33+bob},{46,35+bob},{44,30+bob}},0xff259a86u);
    c.ellipse(48,24+bob,10,10,Teal);c.rect(54,26+bob,3,5,0xff116357u);c.rect(47,23+bob,11,2,0xffc2ffeeu);
    c.rect(62,23+bob,4,11,Steel);outlineEllipse(c,61,25+bob,10,14,Armor,1);c.ellipse(64,29+bob,4,6,Teal);
    c.line(65,8+bob,67,25+bob,Ink,3);c.line(65,8+bob,66,19+bob,Steel);c.dot(65,7+bob,Amber);
    c.polygon({{44,34+bob},{53,36+bob},{45,40+bob},{40,38+bob}},Orange);c.polygon({{45,34+bob},{51,36+bob},{41,37+bob}},Amber);
    // Raised collar, piping, shoulder hardware.
    c.polygon({{20,43+bob},{62,43+bob},{59,53+bob},{26,54+bob}},Ink);
    c.line(21,44+bob,61,44+bob,Amber,2);c.line(27,51+bob,58,49+bob,Steel);
    outlineEllipse(c,16,46+bob,16,17,Armor);c.line(20,48+bob,27,49+bob,Steel,2);c.dot(22,53+bob,Teal);
    outlineEllipse(c,55,47+bob,17,17,Armor);c.ellipse(58,49+bob,9,9,Steel);c.ellipse(60,51+bob,5,5,Teal);
    if(state==3){ // Coffee break; the mug is part of the original reference.
        c.line(29,58+bob,36,53+bob,Ink,8);c.rect(33,48+bob,13,13,Ink);c.rect(35,50+bob,9,9,Armor);
        c.ellipse(42,49+bob,7,9,Ink);c.ellipse(44,51+bob,3,5,Teal);c.text(36,51+bob,"Q",Teal);
        int s=frame%4;c.line(38,45-s,36,41-s,0xffb9e2d4u);c.dot(37,38-s,0xffb9e2d4u);
    }else{
        c.rect(62,58+bob,13,8,Ink);c.rect(63,59+bob,11,5,Steel);c.rect(72,59+bob,5,4,Teal);c.rect(65,61+bob,4,2,Amber);
    }
    return c;
}
Canvas duckFrame(int state,int frame){
    Canvas c(64,64);int bob=frame%4==0?1:0,flame=4+frame%3*2;
    // Paired blue thrusters with bright cores, not ordinary duck feet.
    c.polygon({{20,46},{31,46},{27,51+flame},{24,60},{21,51+flame}},0xff2987cbu);
    c.polygon({{38,46},{49,46},{45,53+flame},{41,60},{39,52}},0xff2987cbu);
    c.polygon({{23,47},{29,47},{26,54+flame}},Blue);c.polygon({{41,47},{47,47},{43,54+flame}},Blue);
    c.rect(24,48,3,5,White);c.rect(42,48,3,5,White);
    c.polygon({{18,32},{10,24},{7,30},{12,41},{24,42}},Ink);c.polygon({{18,33},{10,28},{11,35},{17,39}},Amber);
    outlineEllipse(c,16,24+bob,36,26,Armor);c.rect(23,44+bob,9,7,Ink);c.rect(39,44+bob,9,7,Ink);
    c.rect(24,46+bob,7,3,Steel);c.rect(40,46+bob,7,3,Steel);
    outlineEllipse(c,21,3+bob,29,29,Orange);c.ellipse(23,5+bob,24,24,Amber);c.ellipse(27,5+bob,15,7,0xffffdc88u);
    c.polygon({{31,5},{32,0},{35,1},{35,5}},Amber);c.polygon({{35,5},{39,1},{42,3},{38,7}},Amber);
    // Wide dark sunglasses and reflected city highlights.
    c.line(21,14+bob,49,13+bob,Ink,3);c.rect(27,13+bob,10,10,Ink);c.rect(39,12+bob,11,10,Ink);
    c.rect(28,14+bob,7,5,0xff18263cu);c.rect(40,13+bob,7,5,0xff18263cu);
    c.line(30,14+bob,28,17+bob,Steel);c.line(44,13+bob,41,17+bob,Steel);
    c.ellipse(43,22+bob,15,8,Ink);c.ellipse(44,23+bob,13,5,Orange);c.rect(45,23+bob,9,2,Amber);
    c.line(20,28+bob,48,29+bob,Ink,3);c.line(22,29+bob,46,30+bob,Steel);
    c.rect(24,32+bob,21,10,Ink);c.rect(26,34+bob,16,6,0xff394b5eu);c.rect(29,36+bob,6,2,Amber);
    outlineEllipse(c,12,29+bob,15,15,Orange);c.ellipse(16,32+bob,8,8,Steel);c.ellipse(18,34+bob,4,4,Ink);
    outlineEllipse(c,42,30+bob,13,14,Amber);c.ellipse(45,33+bob,7,8,Steel);c.rect(48,35+bob,7,4,Ink);
    if(state==1){c.rect(54,35+bob,6,2,Blue);c.dot(60,35+bob,White);}
    return c;
}
Canvas enemyFrame(EnemyKind kind,int frame){
    Canvas c(56,56);int bob=frame%4==0?1:0;
    if(kind==EnemyKind::Bug){
        for(int i=0;i<3;++i){int y=25+i*7;int d=(frame+i)%2?3:-1;c.line(18,y,8,y+d,Ink,4);c.line(38,y,48,y-d,Ink,4);c.line(9,y+d,6,y+d+5,0xffb586dcu,2);c.line(47,y-d,50,y-d+5,0xffb586dcu,2);}
        outlineEllipse(c,14,17+bob,28,29,0xff7e559fu);c.ellipse(18,21+bob,19,20,0xffb183d0u);c.line(28,21,28,44,0xff523e7du,2);
        outlineEllipse(c,15,8+bob,25,19,0xff58496du);c.rect(21,15+bob,5,6,White);c.rect(30,15+bob,5,6,White);c.rect(24,17+bob,2,4,Ink);c.rect(33,17+bob,2,4,Ink);
        c.line(20,10,15,3,Ink,2);c.line(34,10,39,3,Ink,2);c.dot(14,2,0xffffa6cdu);c.dot(40,2,0xffffa6cdu);
    }else if(kind==EnemyKind::Leak){
        c.polygon({{24,7},{17,19},{11,28},{10,42},{45,44},{45,29},{35,18},{31,4}},Ink);
        c.ellipse(8,24+bob,41,24,Ink);c.ellipse(11,23+bob,35,22,0xffb35591u);
        c.polygon({{27,8},{21,21},{15,31},{40,31},{34,21},{30,8}},0xffe58bb1u);c.ellipse(15,22+bob,27,18,0xffe58bb1u);
        c.ellipse(18,20+bob,8,5,0xffffc1d2u);c.rect(19,31+bob,5,6,Ink);c.rect(33,31+bob,5,6,Ink);c.rect(26,38+bob,5,2,Ink);
        c.rect(13,42+bob,8,4,0xffbc659du);c.rect(33,42+bob,8,4,0xffbc659du);c.rect(26,23,5,4,0xfff8adcdu);
    }else if(kind==EnemyKind::Zombie){
        c.rect(15,39,10,10,Ink);c.rect(31,39,11,10,Ink);c.rect(15+frame%2*2,45,13,5,Steel);c.rect(33-frame%2*2,45,13,5,Steel);
        c.rect(12,23,34,22,Ink);c.rect(15,25,28,16,0xff617b74u);c.rect(9,27,8,14,Ink);c.rect(42,26,8,14,Ink);
        c.rect(13,6+bob,31,24,Ink);c.rect(16,8+bob,25,20,0xffb7c6a3u);c.rect(17,11+bob,10,9,Ink);c.rect(30,12+bob,9,8,Ink);
        c.rect(20,14+bob,5,3,0xffa8ff82u);c.line(32,13+bob,38,19+bob,0xffffbe79u,2);c.line(38,13+bob,32,19+bob,0xffffbe79u,2);
        c.rect(23,23+bob,10,2,Ink);c.line(22,9,26,13,Steel,2);c.rect(21,32,15,5,Ink);c.rect(23,33,3,3,Teal);c.rect(29,33,4,3,Orange);
    }else{
        c.line(7,12,49,12,Ink,3);c.rect(4+frame%2*2,10,18,2,Steel);c.rect(35-frame%2*2,10,18,2,Steel);
        c.line(15,13,20,28,Ink,3);c.line(42,13,37,28,Ink,3);
        outlineEllipse(c,10,22+bob,38,23,0xffb45366u);c.ellipse(15,24+bob,25,16,0xffe57d86u);
        c.rect(17,28+bob,23,9,Ink);c.rect(21,31+bob,14,3,0xffffd395u);c.rect(32,42+bob,5,7,Ink);c.dot(34,49+bob,Orange);
        c.rect(7,29+bob,7,9,Steel);c.rect(44,29+bob,7,9,Steel);c.ellipse(25,25+bob,5,2,0xffffb1a3u);
    }
    return c;
}
Canvas tileFrame(Biome b,int kind){
    const auto& p=palette(b);Canvas c(32,32,p.wall);
    c.rect(0,0,32,1,mix(p.wall,p.edge,0.5));c.rect(31,1,1,31,mix(p.wall,Ink,0.35));
    if(kind<3){c.rect(0,0,32,5,Ink);c.rect(0,2,32,2,p.accent);c.rect(1,5,30,6,p.edge);c.rect(0,11,32,3,Ink);c.rect(2,15,28,14,mix(p.wall,Steel,0.15));c.dot(4,18,p.edge);c.dot(27,27,p.edge);if(kind==0)c.rect(0,5,3,27,Ink);if(kind==2)c.rect(29,5,3,27,Ink);}
    else if(kind==3){c.rect(4,5,24,23,Ink);for(int y=7;y<27;y+=4)c.rect(6,y,20,2,p.edge);}
    else if(kind==4){ // Panel slot differs per district: lit window, mossy stone, grille, container wall.
        if(b==Biome::Rooftops){c.rect(5,4,22,24,Ink);c.rect(7,6,8,9,0xff2f3d55u);c.rect(17,6,8,9,mix(p.accent,Ink,0.55));c.rect(7,17,8,9,mix(Amber,Ink,0.5));c.rect(17,17,8,9,0xff2f3d55u);c.rect(15,4,2,24,Ink);c.rect(5,15,22,2,Ink);}
        else if(b==Biome::Cooling){c.rect(3,3,26,26,Ink);c.rect(5,5,22,22,mix(p.wall,p.edge,0.25));for(int y=7;y<26;y+=4)c.rect(6,y,20,2,Ink);c.dot(4,4,p.edge);c.dot(27,27,p.edge);}
        else if(b==Biome::Network){for(int x=0;x<32;x+=8){c.rect(x,2,4,28,mix(p.wall,Ink,0.22));c.rect(x+4,2,4,28,mix(p.wall,p.edge,0.14));}c.rect(0,2,32,1,Ink);c.rect(0,29,32,1,Ink);c.rect(10,12,12,7,Ink);c.rect(11,13,10,5,p.secondary);}
        else{c.rect(3,3,26,26,mix(p.wall,p.edge,0.17));c.rect(5,6,22,1,p.edge);c.ellipse(4,20,12,8,0xff4e7a64u);c.ellipse(19,4,10,7,0xff76a47cu);c.dot(25,25,p.edge);}}
    else if(kind==5){c.rect(0,9,32,14,Ink);for(int x=-10;x<32;x+=14)c.polygon({{x,21},{x+9,11},{x+14,11},{x+5,21}},Orange);}
    else if(kind==6){c.rect(0,12,32,11,Ink);c.rect(0,14,32,7,p.edge);c.rect(0,14,32,2,mix(p.edge,White,0.25));c.rect(4,11,4,14,Steel);}
    else if(kind==7){
        if(b==Biome::Memory){c.line(4,32,10,20,0xff517c60u,2);c.line(10,20,8,8,0xff517c60u,2);c.line(10,20,20,14,0xff517c60u,2);c.ellipse(4,6,8,6,0xff76a47cu);c.ellipse(18,10,9,6,0xff95bb79u);c.ellipse(9,22,7,5,0xffeea8c8u);}
        else{c.line(0,17,11,17,p.edge);c.line(11,17,18,10,p.edge);c.line(18,10,31,10,p.edge);c.ellipse(22,7,5,5,p.accent);c.line(3,32,3,25,p.edge);c.line(3,25,12,25,p.edge);}}
    else {c.rect(3,4,26,23,Ink);c.rect(5,6,22,19,0xff0d2831u);for(int x=8;x<25;x+=5)c.rect(x,9,2,11,p.accent);c.rect(7,23,17,2,p.edge);}
    if(b==Biome::Memory){c.dot(5,29,0xff76a47cu);c.dot(22,27,0xff4e7a64u);if(kind<3){for(int x=2;x<32;x+=7)c.line(x,2,x+2,-2,0xff95bb79u,2);c.rect(0,4,32,2,0xff517c60u);}}
    return c;
}
Canvas propFrame(int kind){
    Canvas c(80,112);
    if(kind==0){ // rack
        c.rect(9,22,57,86,Ink);c.rect(12,24,49,79,Armor);c.rect(13,24,3,79,Steel);
        for(int y=29;y<98;y+=17){c.rect(19,y,36,13,Ink);c.rect(22,y+3,24,1,Steel);c.rect(23,y+6,20,1,Steel);c.rect(49,y+4,3,3,Teal);c.dot(50,y+9,Amber);}
        c.rect(15,108,9,4,Ink);c.rect(52,108,9,4,Ink);
    }else if(kind==1){ // fan housing, animated blades drawn in world
        c.rect(8,48,64,60,Ink);c.rect(11,50,58,55,Armor);outlineEllipse(c,16,55,48,48,Steel);c.ellipse(20,59,40,40,Ink);c.ellipse(37,76,7,7,Teal);
        for(int x=13;x<69;x+=13){c.dot(x,53,White);}
        c.rect(15,108,10,4,Ink);c.rect(55,108,10,4,Ink);
    }else if(kind==2){ // tech planter
        c.polygon({{15,85},{68,85},{59,109},{23,109}},Ink);c.polygon({{20,88},{64,88},{57,106},{26,106}},Armor);c.rect(24,91,36,3,Amber);
        for(int i=0;i<5;++i){int x=29+i*6;c.line(41,88,x,42+i%2*10,0xff568c78u,3);c.ellipse(x-12,49+i%2*10,16,8,0xff80b781u);c.ellipse(x,39+i%2*11,16,9,0xffaccc85u);}
        c.ellipse(35,63,11,9,0xffeea8c8u);c.ellipse(39,65,4,4,Amber);
    }else if(kind==3){ // terminal
        c.rect(17,51,51,38,Ink);c.rect(20,54,45,29,Steel);c.rect(23,57,38,22,0xff0d302fu);c.text(27,62,"> QQ",Teal);c.rect(27,73,26,1,0xff299875u);
        c.rect(37,87,10,12,Ink);c.rect(23,99,42,5,Steel);c.rect(10,104,65,7,Ink);c.line(40,106,40,112,Ink,3);
    }else if(kind==4){ // antenna
        c.rect(32,100,27,12,Ink);c.line(45,102,45,11,Steel,3);c.line(45,17,22,34,Steel,2);c.line(45,17,66,33,Steel,2);
        c.line(28,44,61,44,Steel,2);c.line(32,52,57,52,Steel,2);c.ellipse(41,6,9,9,Teal);c.dot(44,5,White);
    }else if(kind==5){ // coffee vending machine
        c.rect(12,29,56,83,Ink);c.rect(15,32,50,73,Armor);c.rect(18,35,43,13,Amber);c.text(22,38,"COFFEE",Ink);c.rect(19,54,31,28,Ink);c.text(22,59,"QQ",Teal,2);
        c.rect(55,57,5,5,Teal);c.rect(55,67,5,5,Orange);c.rect(23,87,34,12,Ink);c.rect(30,90,9,7,White);c.line(37,92,41,92,White,2);
    }else if(kind==6){ // abandoned monitor and motherboard
        c.rect(6,76,47,28,Ink);c.rect(9,78,41,22,0xff667471u);c.rect(13,81,31,15,0xff142b32u);c.line(17,91,25,84,0xff2e605cu);c.rect(23,104,16,5,Steel);
        c.polygon({{39,102},{67,83},{77,95},{56,112}},0xff466354u);c.rect(57,96,8,7,Ink);c.line(62,94,69,92,Amber);
    }else if(kind==7){ // small lamp with a QQ insignia
        c.rect(35,60,5,52,Ink);c.rect(28,54,21,20,Ink);c.rect(30,56,17,14,Teal);c.text(32,59,"QQ",Ink);c.rect(25,108,26,4,Steel);
    }else if(kind==8){ // rooftop water tower on four legs
        c.line(18,60,10,112,Ink,4);c.line(62,60,70,112,Ink,4);c.line(30,64,26,112,Steel,2);c.line(50,64,54,112,Steel,2);
        c.line(12,90,68,90,Ink,2);c.line(14,102,66,102,Steel,1);
        c.rect(16,24,48,44,Ink);c.rect(19,27,42,38,Armor);c.rect(19,27,6,38,Steel);
        for(int y=32;y<62;y+=10)c.rect(17,y,46,2,Ink);
        c.polygon({{12,26},{40,8},{68,26}},Ink);c.polygon({{18,25},{40,12},{62,25}},Steel);c.rect(38,2,4,8,Ink);c.dot(40,1,Amber);
        c.rect(27,42,26,10,Ink);c.text(30,44,"H2O",Teal);
    }else if(kind==9){ // paired air-handling units
        c.rect(4,66,72,44,Ink);c.rect(7,69,30,38,Armor);c.rect(43,69,30,38,Armor);
        for(int i=0;i<2;++i){int bx=7+i*36;outlineEllipse(c,bx+4,72,22,22,Steel,1);c.ellipse(bx+11,79,8,8,Ink);c.dot(bx+14,82,Teal);for(int y=97;y<105;y+=3)c.rect(bx+3,y,24,1,Ink);c.rect(bx+2,71,26,2,Steel);}
        c.rect(10,108,60,4,Ink);c.rect(36,58,8,10,Steel);c.rect(30,56,20,4,Ink);
    }else if(kind==10){ // stacked cargo crates
        auto crate=[&](int x,int y,Color face){c.rect(x,y,34,34,Ink);c.rect(x+2,y+2,30,30,face);c.rect(x+2,y+2,30,2,mix(face,White,0.25));c.rect(x+2,y+30,30,2,mix(face,Ink,0.4));c.line(x+4,y+28,x+28,y+4,mix(face,Ink,0.25),2);c.rect(x+11,y+13,12,8,Ink);c.text(x+13,y+14,"QQ",Amber);};
        crate(6,76,Armor);crate(41,76,0xff5a4a3cu);crate(23,42,0xff7a4f2fu);c.rect(3,110,74,2,Ink);
    }else if(kind==11){ // garden gate with paper lanterns
        c.rect(20,44,6,68,Ink);c.rect(54,44,6,68,Ink);c.rect(21,45,2,66,0xff7a4f2fu);c.rect(55,45,2,66,0xff7a4f2fu);
        c.rect(12,38,56,7,Ink);c.rect(13,39,54,3,0xff9c5d33u);c.rect(16,49,48,3,Ink);
        for(int i=0;i<2;++i){int lx=27+i*20;c.line(lx+6,52,lx+6,58,Ink,2);outlineEllipse(c,lx,58,13,20,Amber,1);c.rect(lx+1,64,11,2,Orange);c.rect(lx+1,70,11,2,Orange);c.rect(lx+4,78,5,3,Ink);}
        c.ellipse(8,98,24,12,0xff5f8f63u);c.ellipse(50,96,26,14,0xff6f9d6au);c.ellipse(14,92,10,9,0xff9dc98au);c.ellipse(58,88,12,10,0xff9dc98au);c.dot(20,95,0xffeea8c8u);c.dot(62,91,0xffeea8c8u);
    }else{ // twin chimney stack with a valve wheel; steam is drawn in world
        c.rect(18,14,16,98,Ink);c.rect(21,17,10,92,Steel);c.rect(21,17,3,92,0xff7b8ea3u);
        c.rect(44,34,16,78,Ink);c.rect(47,37,10,72,Steel);c.rect(47,37,3,72,0xff7b8ea3u);
        for(int y=30;y<108;y+=18){c.rect(19,y,14,3,Ink);if(y>=37)c.rect(45,y,14,3,Ink);}
        c.rect(15,10,22,6,Ink);c.rect(41,30,22,6,Ink);c.rect(17,11,18,2,0xff7b8ea3u);
        c.rect(33,60,12,4,Ink);outlineEllipse(c,29,66,20,20,Steel,2);c.line(39,69,39,83,Ink,2);c.line(32,76,46,76,Ink,2);c.dot(39,76,Amber);
        c.rect(8,100,66,12,Ink);c.rect(11,103,60,6,Armor);c.rect(60,104,6,4,Teal);
    }
    return c;
}
}
const Palette& palette(Biome b){
 static const std::array<Palette,4> p{{
 {0xff090f21u,0xff293348u,0xff182439u,0xff202e44u,0xff263647u,0xff4d6275u,Teal,0xffba84d7u},
 {0xff111e2bu,0xff334c4du,0xff223b42u,0xff2e4850u,0xff304444u,0xff627d70u,0xffbbdd89u,0xffefa9b6u},
 {0xff10192bu,0xff30465bu,0xff1b3146u,0xff263e55u,0xff293e51u,0xff647c96u,0xff79d9f5u,0xffffc27au},
 {0xff161a30u,0xff4b3d51u,0xff2c2d49u,0xff383853u,0xff373b52u,0xff73788cu,0xffffc783u,0xffb199ffu}
 }};return p[int(b)];
}
Palette blendPalette(const Palette& a,const Palette& b,double t){
 return {mix(a.skyTop,b.skyTop,t),mix(a.skyBottom,b.skyBottom,t),mix(a.far,b.far,t),mix(a.near,b.near,t),mix(a.wall,b.wall,t),mix(a.edge,b.edge,t),mix(a.accent,b.accent,t),mix(a.secondary,b.secondary,t)};
}
Assets::Assets(){
 for(int s=0;s<4;++s)for(int f=0;f<8;++f)penguin.push_back(penguinFrame(s,f));
 for(int s=0;s<2;++s)for(int f=0;f<8;++f)duck.push_back(duckFrame(s,f));
 for(int k=0;k<4;++k)for(int f=0;f<8;++f)enemies.push_back(enemyFrame(EnemyKind(k),f));
 for(int b=0;b<4;++b)for(int k=0;k<9;++k)tiles[b].push_back(tileFrame(Biome(b),k));
 for(int k=0;k<13;++k)props.push_back(propFrame(k));
}
bool Assets::exportTo(const std::string& folder)const {
 std::error_code ec;std::filesystem::create_directories(folder,ec);if(ec)return false;
 auto sheet=[&](const std::vector<Canvas>& frames,int cols,const std::string& name){if(frames.empty())return false;Canvas s(cols*frames[0].width,int((frames.size()+cols-1)/cols)*frames[0].height);for(std::size_t i=0;i<frames.size();++i)s.blit(frames[i],int(i%cols)*frames[0].width,int(i/cols)*frames[0].height);return s.png(folder+"/"+name+".png");};
 bool ok=sheet(penguin,8,"penguin")&&sheet(duck,8,"duck")&&sheet(enemies,8,"enemies")&&sheet(props,4,"props");
 for(int b=0;b<4;++b)ok=sheet(tiles[b],9,"tiles-"+std::to_string(b))&&ok;
 std::ofstream manifest(folder+"/atlas.json");
 manifest<<R"JSON({
  "version": 1,
  "format": "straight-alpha RGBA PNG; nearest-neighbor sampling; no padding",
  "source": "src/assets.cpp; deterministic editable C++ pixel-art masters",
  "penguin": {"file":"penguin.png","cell":[80,88],"columns":8,"rows":4,"pivot":[40,83],"fps":10,"states":{"walk":[0,7],"idle":[8,15],"jump":[16,23],"coffee":[24,31]}},
  "duck": {"file":"duck.png","cell":[64,64],"columns":8,"rows":2,"pivot":[32,32],"fps":10,"states":{"hover":[0,7],"fire":[8,15]}},
  "enemies": {"file":"enemies.png","cell":[56,56],"columns":8,"rows":4,"pivot":[28,28],"fps":8,"states":{"bug":[0,7],"memory_leak":[8,15],"zombie":[16,23],"runaway_drone":[24,31]}},
  "props": {"file":"props.png","cell":[80,112],"columns":4,"rows":4,"pivot":[40,112],"names":["server_rack","cooling_fan","planter","terminal","antenna","coffee_machine","e_waste","qq_lamp","water_tower","ac_units","crates","lantern_gate","chimney_stack"]},
  "tiles": {"files":["tiles-0.png","tiles-1.png","tiles-2.png","tiles-3.png"],"biomes":["neon_rooftops","memory_gardens","cooling_district","packet_docks"],"cell":[32,32],"columns":9,"names":["roof_left","roof_mid","roof_right","vent","panel","hazard_band","pipe","circuit","data_bank"],"note":"panel and circuit slots carry district-specific art: window / mossy stone / grille / container wall, and a vine for memory_gardens"},
  "effects": "Beams, sparks, steam, leaves, rain, fan blades, reflections, sign graphs, gateway arches, billboards, cables, catwalk trusses and the street layer are generated by src/renderer.cpp; skyline-*.png strips are samples of the world-anchored procedural skyline chunks; no external assets required.",
  "font": "Original 5x7 ASCII bitmap glyph definitions embedded in src/canvas.cpp. No external font file."
})JSON";
 return ok&&bool(manifest);
}
}
