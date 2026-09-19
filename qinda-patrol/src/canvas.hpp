#pragma once
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>
namespace patrol {
using Color=std::uint32_t; // straight-alpha 0xAARRGGBB, compatible with QImage::Format_ARGB32
constexpr Color rgb(int r,int g,int b,int a=255) {return (Color(a)<<24)|(Color(r)<<16)|(Color(g)<<8)|Color(b);}
Color alpha(Color color,int a);
Color mix(Color a,Color b,double t);
struct Point {int x,y;};
class Canvas {
public:
    Canvas(int width=1,int height=1,Color color=0);
    int width,height;
    std::vector<Color> pixels;
    void clear(Color color);
    void dot(int x,int y,Color color);
    void rect(int x,int y,int w,int h,Color color);
    void line(int x1,int y1,int x2,int y2,Color color,int thickness=1);
    void ellipse(int x,int y,int w,int h,Color color);
    void polygon(std::initializer_list<Point> vertices,Color color);
    void blit(const Canvas& other,int x,int y,int scale=1,bool flip=false);
    void crossfade(const Canvas& a,const Canvas& b,int k); // k in 0..256, same-size sources
    void text(int x,int y,const std::string& text,Color color,int scale=1);
    static int textWidth(const std::string& text,int scale=1);
    bool png(const std::string& path) const;
};
}
