#pragma once
#include "assets.hpp"
#include "metrics.hpp"
#include "world.hpp"
#include <array>
#include <cstdint>
#include <vector>
namespace patrol {
class Renderer {
public:
    Renderer();
    const Canvas& render(const World& world,const Metrics& metrics,bool overlay=true);
    bool exportAssets(const std::string& folder) const;
    const Assets& assets() const{return assets_;}
private:
    // Skylines are anchored to absolute world x and built lazily in 480 px chunks
    // keyed by (layer, chunk index, seed). A small LRU keeps what is on or near screen.
    struct Chunk { int layer=0; std::int64_t index=0; std::uint64_t seed=0; unsigned used=0; Canvas image; };
    Assets assets_;
    Canvas frame_{ViewW,ViewH};
    std::array<Canvas,4> sky_;
    std::vector<Chunk> chunks_;
    unsigned tick_=0;
    void buildSkies();
    static void buildChunk(Canvas& target,int layer,std::int64_t index,std::uint64_t seed,int forcedBiome=-1);
    const Canvas& chunk(int layer,std::int64_t index,std::uint64_t seed);
    void street(const World& w,const Palette& p);
    void sign(const Platform& platform,int x,int y,const World& w,const Metrics& m);
    void prop(int kind,int x,int floor,const World& w,const Metrics& m);
    void gate(const Platform& platform,int x,int y,const World& w);
    void billboard(const Platform& platform,int x,int y,const World& w);
    void cable(const Platform& from,const Platform& to,const World& w);
    void weather(Biome biome,int strength,const World& w);
};
}
