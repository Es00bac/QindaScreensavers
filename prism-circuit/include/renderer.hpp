// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "gl_api.hpp"
#include "scene.hpp"
#include <string>
#include <vector>
namespace sw {
struct RenderOptions {
    float brightness = 1, bloom = .24f;
    bool titles = true, metrics = true, demo = false;
    int samples = 4;
};
class Renderer {
    struct GpuMesh {
        GLuint vao = 0, vbo = 0, ibo = 0, instances = 0;
        int count = 0;
    };
    std::array<GpuMesh, Count> gpu_{};
    GLuint mesh_ = 0, depth_ = 0, sky_ = 0, blur_ = 0, post_ = 0, quad_ = 0, noise_ = 0,
           overlay_ = 0, hull_ = 0;
    GLuint msFbo_ = 0, msColor_ = 0, msDepth_ = 0, hdrFbo_ = 0, hdr_ = 0, z_ = 0, shadowFbo_ = 0,
           shadow_ = 0, blurFbo_[2]{}, blurTex_[2]{}, materials_ = 0, facades_ = 0;
    int w_ = 0, h_ = 0;
    long overlayStamp_ = -1;
    int lastChapter_ = -1;
    float lastAlpha_ = -1;
    RenderOptions opt_;
    void uploadOverlay(const Frame &, const std::string &metrics);

  public:
    Renderer(const Frame &, int width, int height, RenderOptions);
    ~Renderer();
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    void resize(int, int);
    void draw(const Frame &, const std::string &metrics = "");
    std::vector<unsigned char> pixels();
    int width() const { return w_; }
    int height() const { return h_; }
};
void png(const std::string &, const std::vector<unsigned char> &rgb, int width, int height);
} // namespace sw
