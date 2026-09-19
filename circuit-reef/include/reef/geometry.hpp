// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "reef/types.hpp"
#include <functional>
#include <map>
#include <memory>
namespace reef {
class Image;
struct Vertex {Vec2 position;Color color;Vec2 uv;};
struct Batch {std::size_t first{},count{};const Image* texture{};};
struct Transform {
 double a=1,b=0,c=0,d=1,e=0,f=0;
 Vec2 map(Vec2 p)const{return{a*p.x+c*p.y+e,b*p.x+d*p.y+f};}
};
// Backend-independent, antialiased triangle stream. SDL consumes this directly
// on the GPU; Cairo remains the reference/export backend for the same artwork.
class DrawList {
public:
 DrawList();~DrawList();DrawList(const DrawList&)=delete;
 void begin();void save();void restore();void translate(double,double);void scale(double,double);void rotate(double);
 void path();void move(Vec2);void line(Vec2);void curve(Vec2,Vec2,Vec2);void close();void arc(double,double,double,double,double);
 void fill(Color);void linear(Vec2,Vec2,Color,Color);void stroke(Color,double);
 void glow(double,double,double,Color);void image(const Image&,double,double,double,double,Color tint={1,1,1,1});
 void text(double,double,const std::string&,double,Color,bool);
 std::vector<Vertex> vertices;std::vector<Batch> batches;
 std::size_t textCacheSize()const{return textCache_.size();}
private:
 Transform transform_;std::vector<Transform> stack_;std::vector<Vec2> points_;bool closed_=false;
 std::map<std::string,std::unique_ptr<Image>> textCache_;
 void triangle(Vec2,Vec2,Vec2,Color,Color,Color,const Image* texture=nullptr,Vec2 uv0={},Vec2 uv1={},Vec2 uv2={});
 void fillWith(const std::function<Color(Vec2)>&);
};
}
