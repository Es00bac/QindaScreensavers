// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#if defined(STARWARD_SYSTEM_CAIRO)
#include <cairo.h>
#include <cairo-svg.h>
#else
// Runtime-only build fallback for Linux. Normal builds use the installed Cairo headers.
extern "C" {
struct _cairo; using cairo_t = _cairo;
struct _cairo_surface; using cairo_surface_t = _cairo_surface;
struct _cairo_pattern; using cairo_pattern_t = _cairo_pattern;
using cairo_status_t = int;
struct cairo_text_extents_t { double x_bearing,y_bearing,width,height,x_advance,y_advance; };
cairo_t* cairo_create(cairo_surface_t*); void cairo_destroy(cairo_t*);
cairo_status_t cairo_status(cairo_t*); const char* cairo_status_to_string(cairo_status_t);
cairo_surface_t* cairo_image_surface_create(int,int,int);
unsigned char* cairo_image_surface_get_data(cairo_surface_t*);
int cairo_image_surface_get_stride(cairo_surface_t*);
cairo_surface_t* cairo_svg_surface_create(const char*,double,double);
void cairo_surface_destroy(cairo_surface_t*); void cairo_surface_flush(cairo_surface_t*);
void cairo_surface_mark_dirty(cairo_surface_t*);
void cairo_surface_finish(cairo_surface_t*); cairo_status_t cairo_surface_status(cairo_surface_t*);
cairo_status_t cairo_surface_write_to_png(cairo_surface_t*,const char*);
void cairo_save(cairo_t*); void cairo_restore(cairo_t*); void cairo_translate(cairo_t*,double,double);
void cairo_scale(cairo_t*,double,double); void cairo_rotate(cairo_t*,double);
void cairo_set_source_rgba(cairo_t*,double,double,double,double);
void cairo_set_source(cairo_t*,cairo_pattern_t*); void cairo_set_source_surface(cairo_t*,cairo_surface_t*,double,double);
void cairo_set_operator(cairo_t*,int); void cairo_set_line_width(cairo_t*,double);
void cairo_set_line_cap(cairo_t*,int); void cairo_set_line_join(cairo_t*,int);
void cairo_set_antialias(cairo_t*,int);
void cairo_new_path(cairo_t*); void cairo_move_to(cairo_t*,double,double); void cairo_line_to(cairo_t*,double,double);
void cairo_curve_to(cairo_t*,double,double,double,double,double,double);
void cairo_close_path(cairo_t*); void cairo_rectangle(cairo_t*,double,double,double,double);
void cairo_arc(cairo_t*,double,double,double,double,double);
void cairo_fill(cairo_t*); void cairo_fill_preserve(cairo_t*); void cairo_stroke(cairo_t*);
void cairo_paint(cairo_t*); void cairo_paint_with_alpha(cairo_t*,double); void cairo_clip(cairo_t*);
cairo_pattern_t* cairo_pattern_create_linear(double,double,double,double);
cairo_pattern_t* cairo_pattern_create_radial(double,double,double,double,double,double);
void cairo_pattern_add_color_stop_rgba(cairo_pattern_t*,double,double,double,double,double);
void cairo_pattern_destroy(cairo_pattern_t*);
void cairo_select_font_face(cairo_t*,const char*,int,int); void cairo_set_font_size(cairo_t*,double);
void cairo_show_text(cairo_t*,const char*); void cairo_text_extents(cairo_t*,const char*,cairo_text_extents_t*);
}
constexpr int CAIRO_FORMAT_ARGB32=0, CAIRO_OPERATOR_CLEAR=0, CAIRO_OPERATOR_SOURCE=1, CAIRO_OPERATOR_OVER=2;
constexpr int CAIRO_LINE_CAP_ROUND=1, CAIRO_LINE_JOIN_ROUND=1, CAIRO_ANTIALIAS_NONE=1, CAIRO_ANTIALIAS_DEFAULT=0;
constexpr int CAIRO_FONT_SLANT_NORMAL=0, CAIRO_FONT_WEIGHT_NORMAL=0, CAIRO_FONT_WEIGHT_BOLD=1;

#endif
