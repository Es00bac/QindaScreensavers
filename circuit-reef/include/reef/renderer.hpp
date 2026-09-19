// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "reef/canvas.hpp"
#include "reef/metrics.hpp"
namespace reef {
struct RenderOptions {bool metrics=true,reducedMotion=false,branding=true;double brightness=.86;};
class Renderer {
public:
 explicit Renderer(std::uint64_t seed=2026,std::string scheme="lagoon",int density=10);
 const Image& render(int width,int height,double time,const Metrics& metrics,const RenderOptions& options={});
 void drawGpu(DrawList&,int width,int height,double time,const Metrics&,const RenderOptions& ={});
 void exportAssets(const std::string& directory);
 void fishArt(Canvas& c,const FishPose& pose,int style,double alpha=1)const;
 void jellyArt(Canvas& c,double x,double y,double size,double time,int style)const;
 void creatureArt(Canvas&,const FishPose&,Species,double alpha=1)const;
 void pufferArt(Canvas& c,double x,double y,double time,double size=1)const;
 const World& world()const{return world_;}
private:
 World world_; Palette p_;Image frame_,background_;double layout_=1;
 void drawScene(Canvas&,double,const Metrics&,const RenderOptions&);
 void background(Canvas& c);
 void plant(Canvas& c,const Plant& plant,double time,bool front)const;
 void water(Canvas& c,double t,bool foreground)const;
 void instruments(Canvas& c,double t,const Metrics& m)const;
 void title(Canvas& c,double t,const Metrics& m)const;
};
}
