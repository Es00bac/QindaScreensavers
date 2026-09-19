// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#if defined(BRAWL_SYSTEM_SDL)
#include <SDL.h>
#else
#include <cstdint>
// Small Linux SDL2 ABI fallback, used only without development headers.
// These declarations do not implement or replace SDL. Link against the installed library.
using Uint8=std::uint8_t; using Uint16=std::uint16_t; using Uint32=std::uint32_t; using Sint32=std::int32_t; using Uint64=std::uint64_t;
struct SDL_Window; struct SDL_Renderer; struct SDL_Texture;
struct SDL_Rect {int x,y,w,h;};
struct SDL_FPoint{float x,y;};
struct SDL_Color{Uint8 r,g,b,a;};
struct SDL_Vertex{SDL_FPoint position;SDL_Color color;SDL_FPoint tex_coord;};
struct SDL_Keysym {Sint32 scancode,sym; Uint16 mod; Uint32 unused;};
struct SDL_KeyboardEvent {Uint32 type,timestamp,windowID; Uint8 state,repeat,padding2,padding3; SDL_Keysym keysym;};
struct SDL_WindowEvent {Uint32 type,timestamp,windowID; Uint8 event,padding1,padding2,padding3; Sint32 data1,data2;};
struct SDL_MouseMotionEvent {Uint32 type,timestamp,windowID,which,state; Sint32 x,y,xrel,yrel;};
union SDL_Event {Uint32 type; SDL_KeyboardEvent key; SDL_WindowEvent window; SDL_MouseMotionEvent motion; Uint8 padding[56]; std::uint64_t align;};
static_assert(sizeof(SDL_Event)==56);
extern "C" {
int SDL_Init(Uint32); void SDL_Quit(); const char* SDL_GetError(); int SDL_SetHint(const char*,const char*);
SDL_Window* SDL_CreateWindow(const char*,int,int,int,int,Uint32); void SDL_DestroyWindow(SDL_Window*);
SDL_Renderer* SDL_CreateRenderer(SDL_Window*,int,Uint32); void SDL_DestroyRenderer(SDL_Renderer*);
SDL_Texture* SDL_CreateTexture(SDL_Renderer*,Uint32,int,int,int); void SDL_DestroyTexture(SDL_Texture*);
int SDL_RenderGeometry(SDL_Renderer*,SDL_Texture*,const SDL_Vertex*,int,const int*,int);
int SDL_SetTextureBlendMode(SDL_Texture*,int);
int SDL_SetRenderDrawBlendMode(SDL_Renderer*,int);
int SDL_RenderSetScale(SDL_Renderer*,float,float);
int SDL_RenderReadPixels(SDL_Renderer*,const SDL_Rect*,Uint32,void*,int);
int SDL_UpdateTexture(SDL_Texture*,const SDL_Rect*,const void*,int);
int SDL_RenderClear(SDL_Renderer*); int SDL_RenderCopy(SDL_Renderer*,SDL_Texture*,const SDL_Rect*,const SDL_Rect*);
void SDL_RenderPresent(SDL_Renderer*); int SDL_SetRenderDrawColor(SDL_Renderer*,Uint8,Uint8,Uint8,Uint8);
int SDL_GetRendererOutputSize(SDL_Renderer*,int*,int*); int SDL_PollEvent(SDL_Event*);
int SDL_GetNumVideoDisplays(); int SDL_GetDisplayBounds(int,SDL_Rect*); Uint32 SDL_GetWindowID(SDL_Window*);
void SDL_GetWindowSize(SDL_Window*,int*,int*); int SDL_ShowCursor(int);
void SDL_EnableScreenSaver(); int SDL_IsScreenSaverEnabled(); const char* SDL_GetCurrentVideoDriver();
int SDL_SetWindowFullscreen(SDL_Window*,Uint32); Uint32 SDL_GetWindowFlags(SDL_Window*);
Uint64 SDL_GetPerformanceCounter(); Uint64 SDL_GetPerformanceFrequency(); void SDL_Delay(Uint32);
}
constexpr Uint32 SDL_INIT_VIDEO=0x20, SDL_INIT_TIMER=1;
constexpr Uint32 SDL_WINDOW_FULLSCREEN_DESKTOP=0x1001,SDL_WINDOW_RESIZABLE=0x20,SDL_WINDOW_ALLOW_HIGHDPI=0x2000;
constexpr Uint32 SDL_RENDERER_ACCELERATED=2,SDL_RENDERER_PRESENTVSYNC=4, SDL_RENDERER_SOFTWARE=1;
constexpr Uint32 SDL_PIXELFORMAT_ARGB8888=0x16362004;
constexpr int SDL_TEXTUREACCESS_STREAMING=1,SDL_DISABLE=0,SDL_ENABLE=1;
constexpr Uint32 SDL_QUIT=0x100,SDL_WINDOWEVENT=0x200,SDL_DISPLAYEVENT=0x150,SDL_KEYDOWN=0x300,SDL_MOUSEMOTION=0x400,SDL_MOUSEBUTTONDOWN=0x401,SDL_MOUSEWHEEL=0x403,SDL_FINGERDOWN=0x700;
constexpr Uint8 SDL_WINDOWEVENT_SHOWN=1,SDL_WINDOWEVENT_HIDDEN=2,SDL_WINDOWEVENT_MINIMIZED=7,SDL_WINDOWEVENT_RESTORED=9,SDL_WINDOWEVENT_FOCUS_LOST=13,SDL_WINDOWEVENT_CLOSE=14;
constexpr int SDLK_ESCAPE=27,SDLK_f='f',SDLK_m='m',SDLK_r='r';
constexpr const char* SDL_HINT_VIDEO_ALLOW_SCREENSAVER="SDL_VIDEO_ALLOW_SCREENSAVER";
constexpr const char* SDL_HINT_RENDER_SCALE_QUALITY="SDL_RENDER_SCALE_QUALITY";
#define SDL_WINDOWPOS_CENTERED_DISPLAY(X) (int(0x2FFF0000u | Uint32(X)))

constexpr int SDL_BLENDMODE_BLEND=1;
using SDL_GLContext=void*;
extern "C" {
int SDL_GL_SetAttribute(int,int);int SDL_GL_GetAttribute(int,int*);
SDL_GLContext SDL_GL_CreateContext(SDL_Window*);void SDL_GL_DeleteContext(SDL_GLContext);
int SDL_GL_MakeCurrent(SDL_Window*,SDL_GLContext);int SDL_GL_SetSwapInterval(int);
void SDL_GL_SwapWindow(SDL_Window*);void SDL_GL_GetDrawableSize(SDL_Window*,int*,int*);
const char* SDL_GetRevision();
}
constexpr int SDL_GL_DOUBLEBUFFER=5,SDL_GL_DEPTH_SIZE=6,SDL_GL_CONTEXT_MAJOR_VERSION=17,SDL_GL_CONTEXT_MINOR_VERSION=18,SDL_GL_CONTEXT_PROFILE_MASK=21,SDL_GL_CONTEXT_PROFILE_CORE=1;
constexpr Uint32 SDL_WINDOW_OPENGL=2,SDL_WINDOW_HIDDEN=8;

#endif
