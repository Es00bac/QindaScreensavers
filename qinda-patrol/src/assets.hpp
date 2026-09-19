#pragma once
#include "canvas.hpp"
#include "world.hpp"
#include <array>
#include <string>
#include <vector>
namespace patrol {
struct Palette {Color skyTop,skyBottom,far,near,wall,edge,accent,secondary;};
const Palette& palette(Biome b);
Palette blendPalette(const Palette& a,const Palette& b,double t);
class Assets {
public:
    Assets();
    std::vector<Canvas> penguin,duck,enemies,props;
    std::array<std::vector<Canvas>,4> tiles;
    bool exportTo(const std::string& folder)const;
};
}
