// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#ifdef REEF_CAIRO_SYSTEM_HEADERS
#include <cairo.h>
#include <cairo-svg.h>
#else
#include "cairo_abi.hpp"
#endif
#include "reef/types.hpp"
#include "reef/geometry.hpp"
#include <atomic>
#include <stdexcept>
#include <utility>
namespace reef {
class Image {
public:
 inline static std::atomic<std::uint64_t> nextSerial{0};
 std::uint64_t serial=++nextSerial;
 cairo_surface_t* surface{};int width{},height{};
 Image()=default;
 Image(int w,int h):surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,w,h)),width(w),height(h){if(cairo_surface_status(surface)){cairo_surface_destroy(surface);surface=nullptr;throw std::runtime_error("Could not allocate image");}}
 ~Image(){if(surface)cairo_surface_destroy(surface);}
 Image(const Image&)=delete;Image&operator=(const Image&)=delete;
 Image(Image&& o)noexcept:serial(o.serial),surface(std::exchange(o.surface,nullptr)),width(o.width),height(o.height){}
 Image&operator=(Image&&o)noexcept{if(this!=&o){if(surface)cairo_surface_destroy(surface);serial=o.serial;surface=std::exchange(o.surface,nullptr);width=o.width;height=o.height;}return *this;}
 unsigned char* data()const{cairo_surface_flush(surface);return cairo_image_surface_get_data(surface);}
 int stride()const{return cairo_image_surface_get_stride(surface);}
 void png(const std::string& path)const{auto code=cairo_surface_write_to_png(surface,path.c_str());if(code)throw std::runtime_error(cairo_status_to_string(code));}
};
class Canvas {
public:
 cairo_t* c{}; DrawList* gpu{};
 explicit Canvas(cairo_surface_t* s):c(cairo_create(s)){cairo_set_line_cap(c,CAIRO_LINE_CAP_ROUND);cairo_set_line_join(c,CAIRO_LINE_JOIN_ROUND);}
 explicit Canvas(DrawList& list):gpu(&list){}
 ~Canvas(){if(c)cairo_destroy(c);} Canvas(const Canvas&)=delete;
 void save(){if(gpu)gpu->save();else cairo_save(c);}
 void restore(){if(gpu)gpu->restore();else cairo_restore(c);}
 void translate(double x,double y){if(gpu)gpu->translate(x,y);else cairo_translate(c,x,y);}
 void scale(double x,double y){if(gpu)gpu->scale(x,y);else cairo_scale(c,x,y);}
 void rotate(double a){if(gpu)gpu->rotate(a);else cairo_rotate(c,a);}
 void color(Color a){if(c)cairo_set_source_rgba(c,a.r,a.g,a.b,a.a);}
 void path(){if(gpu)gpu->path();else cairo_new_path(c);}
 void move(double x,double y){if(gpu)gpu->move({x,y});else cairo_move_to(c,x,y);}
 void move(Vec2 p){move(p.x,p.y);}
 void line(double x,double y){if(gpu)gpu->line({x,y});else cairo_line_to(c,x,y);}
 void line(Vec2 p){line(p.x,p.y);}
 void curve(double x1,double y1,double x2,double y2,double x,double y){if(gpu)gpu->curve({x1,y1},{x2,y2},{x,y});else cairo_curve_to(c,x1,y1,x2,y2,x,y);}
 void curve(Vec2 a,Vec2 b,Vec2 p){curve(a.x,a.y,b.x,b.y,p.x,p.y);}
 void close(){if(gpu)gpu->close();else cairo_close_path(c);}
 void arc(double x,double y,double r,double a,double b){if(gpu)gpu->arc(x,y,r,a,b);else cairo_arc(c,x,y,r,a,b);}
 void fill(Color a){if(gpu)gpu->fill(a);else{color(a);cairo_fill(c);}}
 void stroke(Color a,double w){if(gpu)gpu->stroke(a,w);else{color(a);cairo_set_line_width(c,w);cairo_stroke(c);}}
 void rect(double x,double y,double w,double h,Color col){path();move(x,y);line(x+w,y);line(x+w,y+h);line(x,y+h);close();fill(col);}
 void ellipse(double x,double y,double rx,double ry,Color col){save();translate(x,y);scale(std::max(.001,rx),std::max(.001,ry));path();arc(0,0,1,0,tau);close();fill(col);restore();}
 void ring(double x,double y,double r,Color col,double w=1){path();arc(x,y,r,0,tau);close();stroke(col,w);}
 void roundRect(double x,double y,double w,double h,double r,Color col){r=std::min({r,w*.5,h*.5});path();move(x+r,y);line(x+w-r,y);arc(x+w-r,y+r,r,-pi/2,0);line(x+w,y+h-r);arc(x+w-r,y+h-r,r,0,pi/2);line(x+r,y+h);arc(x+r,y+h-r,r,pi/2,pi);line(x,y+r);arc(x+r,y+r,r,pi,1.5*pi);close();fill(col);}
 void text(double x,double y,const std::string& s,double size,Color col,bool bold=false){if(gpu){gpu->text(x,y,s,size,col,bold);return;}cairo_select_font_face(c,"sans-serif",CAIRO_FONT_SLANT_NORMAL,bold?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);cairo_set_font_size(c,size);color(col);move(x,y);cairo_show_text(c,s.c_str());}
 void linear(double x0,double y0,double x1,double y1,Color a,Color b){if(gpu){gpu->linear({x0,y0},{x1,y1},a,b);return;}auto* p=cairo_pattern_create_linear(x0,y0,x1,y1);cairo_pattern_add_color_stop_rgba(p,0,a.r,a.g,a.b,a.a);cairo_pattern_add_color_stop_rgba(p,1,b.r,b.g,b.b,b.a);cairo_set_source(c,p);cairo_fill(c);cairo_pattern_destroy(p);}
 void glow(double x,double y,double r,Color col){if(gpu){gpu->glow(x,y,r,col);return;}auto* p=cairo_pattern_create_radial(x,y,0,x,y,r);cairo_pattern_add_color_stop_rgba(p,0,col.r,col.g,col.b,col.a);cairo_pattern_add_color_stop_rgba(p,.32,col.r,col.g,col.b,col.a*.4);cairo_pattern_add_color_stop_rgba(p,1,col.r,col.g,col.b,0);path();arc(x,y,r,0,tau);cairo_set_source(c,p);cairo_fill(c);cairo_pattern_destroy(p);}
 void polygon(const std::vector<Vec2>& points,Color fillCol,Color edge={},double width=0){if(points.empty())return;auto build=[&](){path();move(points.front());for(std::size_t i=1;i<points.size();i++)line(points[i]);close();};build();if(width>0){if(gpu){fill(fillCol);if(width>=.5||fillCol.r!=edge.r||fillCol.g!=edge.g||fillCol.b!=edge.b){build();stroke(edge,width);}}else{color(fillCol);cairo_fill_preserve(c);stroke(edge,width);}}else fill(fillCol);}
};
}
