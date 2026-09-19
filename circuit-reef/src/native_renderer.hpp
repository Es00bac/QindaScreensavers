// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "reef/renderer.hpp"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <cstring>
#include <map>
namespace reef {
// The vector triangle stream is the same one used in the SDL preview; native
// output windows keep it on the GPU instead of rasterizing a full-screen image.
class NativeRenderer {
 QOpenGLExtraFunctions* gl_;Renderer reef_;DrawList list_;GLuint shader_=0,vao_=0,vbo_=0,white_=0;
 std::map<std::uint64_t,GLuint> textures_;int w_,h_,cap_;
 struct Vertex {float x,y,r,g,b,a,u,v;};std::vector<Vertex> vertices_;
 GLuint texture(const Image* im){
  GLuint id;gl_->glGenTextures(1,&id);gl_->glBindTexture(GL_TEXTURE_2D,id);
  gl_->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);gl_->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  gl_->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);gl_->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  const unsigned char white[]{255,255,255,255};
  gl_->glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  gl_->glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,im?im->width:1,im?im->height:1,0,im?GL_BGRA:GL_RGBA,GL_UNSIGNED_BYTE,im?im->data():white);return id;
 }
public:
 NativeRenderer(int w,int h,std::uint64_t seed,const std::string& palette,int density,int cap):gl_(QOpenGLContext::currentContext()->extraFunctions()),reef_(seed,palette,density),w_(w),h_(h),cap_(cap){
  gl_->initializeOpenGLFunctions();
  auto compile=[&](GLenum kind,const char* source){GLuint s=gl_->glCreateShader(kind);gl_->glShaderSource(s,1,&source,nullptr);gl_->glCompileShader(s);GLint ok=0;gl_->glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok)throw std::runtime_error("Reef native shader compilation failed");return s;};
  auto v=compile(GL_VERTEX_SHADER,"#version 330 core\nlayout(location=0)in vec2 pos;layout(location=1)in vec4 color;layout(location=2)in vec2 uv;uniform vec2 resolution;out vec4 tint;out vec2 tex;void main(){gl_Position=vec4(pos.x/resolution.x*2-1,1-pos.y/resolution.y*2,0,1);tint=color;tex=uv;}");
  auto f=compile(GL_FRAGMENT_SHADER,"#version 330 core\nin vec4 tint;in vec2 tex;uniform sampler2D image;out vec4 frag;void main(){vec4 p=texture(image,tex);frag=vec4(p.rgb*tint.rgb*tint.a,p.a*tint.a);}");
  shader_=gl_->glCreateProgram();gl_->glAttachShader(shader_,v);gl_->glAttachShader(shader_,f);gl_->glLinkProgram(shader_);gl_->glDeleteShader(v);gl_->glDeleteShader(f);
  GLint ok=0;gl_->glGetProgramiv(shader_,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("Reef native shader link failed");
  gl_->glGenVertexArrays(1,&vao_);gl_->glBindVertexArray(vao_);gl_->glGenBuffers(1,&vbo_);gl_->glBindBuffer(GL_ARRAY_BUFFER,vbo_);
  for(int i=0;i<3;++i)gl_->glEnableVertexAttribArray(i);
  gl_->glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),nullptr);gl_->glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(2*sizeof(float)));gl_->glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
  white_=texture(nullptr);
 }
 ~NativeRenderer(){for(auto [serial,id]:textures_){(void)serial;gl_->glDeleteTextures(1,&id);}gl_->glDeleteTextures(1,&white_);gl_->glDeleteProgram(shader_);gl_->glDeleteBuffers(1,&vbo_);gl_->glDeleteVertexArrays(1,&vao_);}
 int width()const{return w_;}int height()const{return h_;}void resize(int w,int h){w_=w;h_=h;}
 void draw(double time,const Metrics& metrics,const RenderOptions& options){
  double ratio=std::min({1.,double(cap_)/h_,8192./w_});int w=std::max(32,int(w_*ratio)),h=std::max(32,int(h_*ratio));
  reef_.drawGpu(list_,w,h,time,metrics,options);
  if(textures_.size()>160){for(auto [serial,id]:textures_){(void)serial;gl_->glDeleteTextures(1,&id);}textures_.clear();}
  vertices_.clear();vertices_.reserve(list_.vertices.size());for(const auto& v:list_.vertices)vertices_.push_back({float(v.position.x),float(v.position.y),float(v.color.r),float(v.color.g),float(v.color.b),float(v.color.a),float(v.uv.x),float(v.uv.y)});
  gl_->glViewport(0,0,w_,h_);gl_->glDisable(GL_DEPTH_TEST);gl_->glDisable(GL_CULL_FACE);gl_->glEnable(GL_BLEND);gl_->glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);gl_->glClearColor(.008,.025,.06,1);gl_->glClear(GL_COLOR_BUFFER_BIT);
  gl_->glUseProgram(shader_);gl_->glUniform2f(gl_->glGetUniformLocation(shader_,"resolution"),w,h);gl_->glUniform1i(gl_->glGetUniformLocation(shader_,"image"),0);
  gl_->glBindVertexArray(vao_);gl_->glBindBuffer(GL_ARRAY_BUFFER,vbo_);gl_->glBufferData(GL_ARRAY_BUFFER,vertices_.size()*sizeof(Vertex),vertices_.data(),GL_STREAM_DRAW);gl_->glActiveTexture(GL_TEXTURE0);
  for(const auto& batch:list_.batches){GLuint id=white_;if(batch.texture){auto serial=batch.texture->serial;auto it=textures_.find(serial);if(it==textures_.end())it=textures_.emplace(serial,texture(batch.texture)).first;id=it->second;}gl_->glBindTexture(GL_TEXTURE_2D,id);gl_->glDrawArrays(GL_TRIANGLES,int(batch.first),int(batch.count));}
  if(gl_->glGetError()!=GL_NO_ERROR)throw std::runtime_error("Reef native OpenGL draw failed");
 }
};
}
