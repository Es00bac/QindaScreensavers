// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Linux OpenGL ABI declarations. No renderer implementation is bundled here.
// Functions are exported by libGL (GLVND/Mesa). Public OpenGL 3.3 core APIs only.
#include <cstdint>
#include <cstddef>
using GLenum=unsigned;using GLuint=unsigned;using GLint=int;using GLsizei=int;using GLboolean=unsigned char;using GLbitfield=unsigned;using GLfloat=float;using GLchar=char;using GLsizeiptr=std::ptrdiff_t;using GLubyte=unsigned char;
constexpr GLenum GL_FALSE=0,GL_TRUE=1,GL_FLOAT=0x1406,GL_UNSIGNED_INT=0x1405,GL_UNSIGNED_BYTE=0x1401,GL_ARRAY_BUFFER=0x8892,GL_ELEMENT_ARRAY_BUFFER=0x8893,GL_STATIC_DRAW=0x88E4,GL_STREAM_DRAW=0x88E0,GL_TRIANGLES=4,GL_VERTEX_SHADER=0x8B31,GL_FRAGMENT_SHADER=0x8B30,GL_COMPILE_STATUS=0x8B81,GL_LINK_STATUS=0x8B82,GL_INFO_LOG_LENGTH=0x8B84,GL_FRAMEBUFFER=0x8D40,GL_COLOR_ATTACHMENT0=0x8CE0,GL_DEPTH_ATTACHMENT=0x8D00,GL_FRAMEBUFFER_COMPLETE=0x8CD5,GL_TEXTURE_2D=0x0DE1,GL_TEXTURE0=0x84C0,GL_TEXTURE_MIN_FILTER=0x2801,GL_TEXTURE_MAG_FILTER=0x2800,GL_LINEAR=0x2601,GL_NEAREST=0x2600,GL_TEXTURE_WRAP_S=0x2802,GL_TEXTURE_WRAP_T=0x2803,GL_CLAMP_TO_EDGE=0x812F,GL_REPEAT=0x2901,GL_RGBA=0x1908,GL_RGB=0x1907,GL_RED=0x1903,GL_RGBA16F=0x881A,GL_RGBA8=0x8058,GL_R8=0x8229,GL_DEPTH_COMPONENT=0x1902,GL_DEPTH_COMPONENT24=0x81A6,GL_DEPTH_TEST=0x0B71,GL_CULL_FACE=0x0B44,GL_BACK=0x0405,GL_BLEND=0x0BE2,GL_SRC_ALPHA=0x0302,GL_ONE_MINUS_SRC_ALPHA=0x0303,GL_ONE=1,GL_COLOR_BUFFER_BIT=0x4000,GL_DEPTH_BUFFER_BIT=0x0100,GL_LEQUAL=0x0203,GL_LESS=0x0201,GL_RENDERER=0x1F01,GL_VERSION=0x1F02,GL_MAX_TEXTURE_SIZE=0x0D33,GL_NONE=0,GL_PACK_ALIGNMENT=0x0D05,GL_UNPACK_ALIGNMENT=0x0CF5,GL_FRAMEBUFFER_SRGB=0x8DB9;
extern "C"{
const GLubyte* glGetString(GLenum);void glGetIntegerv(GLenum,GLint*);GLenum glGetError();
void glGenVertexArrays(GLsizei,GLuint*);void glBindVertexArray(GLuint);void glDeleteVertexArrays(GLsizei,const GLuint*);
void glGenBuffers(GLsizei,GLuint*);void glBindBuffer(GLenum,GLuint);void glBufferData(GLenum,GLsizeiptr,const void*,GLenum);void glDeleteBuffers(GLsizei,const GLuint*);
void glEnableVertexAttribArray(GLuint);void glVertexAttribPointer(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);void glVertexAttribDivisor(GLuint,GLuint);
GLuint glCreateShader(GLenum);void glShaderSource(GLuint,GLsizei,const GLchar* const*,const GLint*);void glCompileShader(GLuint);void glGetShaderiv(GLuint,GLenum,GLint*);void glGetShaderInfoLog(GLuint,GLsizei,GLsizei*,GLchar*);void glDeleteShader(GLuint);
GLuint glCreateProgram();void glAttachShader(GLuint,GLuint);void glLinkProgram(GLuint);void glGetProgramiv(GLuint,GLenum,GLint*);void glGetProgramInfoLog(GLuint,GLsizei,GLsizei*,GLchar*);void glDeleteProgram(GLuint);void glUseProgram(GLuint);GLint glGetUniformLocation(GLuint,const GLchar*);
void glUniform1i(GLint,GLint);void glUniform1f(GLint,GLfloat);void glUniform2f(GLint,GLfloat,GLfloat);void glUniform3f(GLint,GLfloat,GLfloat,GLfloat);void glUniformMatrix4fv(GLint,GLsizei,GLboolean,const GLfloat*);
void glGenTextures(GLsizei,GLuint*);void glBindTexture(GLenum,GLuint);void glDeleteTextures(GLsizei,const GLuint*);void glActiveTexture(GLenum);void glTexParameteri(GLenum,GLenum,GLint);void glTexImage2D(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*);void glTexSubImage2D(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void*);void glGenerateMipmap(GLenum);
void glGenFramebuffers(GLsizei,GLuint*);void glBindFramebuffer(GLenum,GLuint);void glDeleteFramebuffers(GLsizei,const GLuint*);void glFramebufferTexture2D(GLenum,GLenum,GLenum,GLuint,GLint);GLenum glCheckFramebufferStatus(GLenum);void glDrawBuffer(GLenum);void glReadBuffer(GLenum);
void glViewport(GLint,GLint,GLsizei,GLsizei);void glClearColor(GLfloat,GLfloat,GLfloat,GLfloat);void glClear(GLbitfield);void glEnable(GLenum);void glDisable(GLenum);void glDepthFunc(GLenum);void glDepthMask(GLboolean);void glCullFace(GLenum);void glBlendFunc(GLenum,GLenum);void glDrawElementsInstanced(GLenum,GLsizei,GLenum,const void*,GLsizei);void glDrawArrays(GLenum,GLint,GLsizei);void glReadPixels(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void*);void glPixelStorei(GLenum,GLint);void glFinish();
}
constexpr GLenum GL_RENDERBUFFER=0x8D41,GL_READ_FRAMEBUFFER=0x8CA8,GL_DRAW_FRAMEBUFFER=0x8CA9,GL_MAX_SAMPLES=0x8D57,GL_LINEAR_MIPMAP_LINEAR=0x2703;
extern "C" {
void glGenRenderbuffers(GLsizei,GLuint*);void glDeleteRenderbuffers(GLsizei,const GLuint*);void glBindRenderbuffer(GLenum,GLuint);void glRenderbufferStorageMultisample(GLenum,GLsizei,GLenum,GLsizei,GLsizei);void glFramebufferRenderbuffer(GLenum,GLenum,GLenum,GLuint);void glBlitFramebuffer(GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLbitfield,GLenum);
}
