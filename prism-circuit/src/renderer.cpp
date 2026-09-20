// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer.hpp"
#include "cairo_abi.hpp"
#include "shader_strings.hpp"
#include <QImage>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
namespace sw {
namespace {
GLuint program(const char *vert, const char *frag) {
    auto shader = [](GLenum kind, const char *source) {
        GLuint s = glCreateShader(kind);
        glShaderSource(s, 1, &source, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint l = 0;
            glGetShaderiv(s, GL_INFO_LOG_LENGTH, &l);
            std::string log(std::max(1, l), ' ');
            glGetShaderInfoLog(s, l, nullptr, log.data());
            glDeleteShader(s);
            throw std::runtime_error("GLSL compile: " + log);
        }
        return s;
    };
    GLuint vs = shader(GL_VERTEX_SHADER, vert), fs = shader(GL_FRAGMENT_SHADER, frag),
           p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint l;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &l);
        std::string log(std::max(1, l), ' ');
        glGetProgramInfoLog(p, l, nullptr, log.data());
        glDeleteProgram(p);
        throw std::runtime_error("GLSL link: " + log);
    }
    return p;
}
void uniform(GLuint p, const char *n, int v) {
    glUniform1i(glGetUniformLocation(p, n), v);
}
void uniform(GLuint p, const char *n, float v) {
    glUniform1f(glGetUniformLocation(p, n), v);
}
void uniform(GLuint p, const char *n, V3 v) {
    glUniform3f(glGetUniformLocation(p, n), v.x, v.y, v.z);
}
void uniform(GLuint p, const char *n, M4 m) {
    glUniformMatrix4fv(glGetUniformLocation(p, n), 1, GL_FALSE, m.data());
}
void tex(GLuint t, int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, t);
}
GLuint texture(int w, int h, GLint format, GLenum input = GL_RGBA, GLenum type = GL_FLOAT,
               const void *data = nullptr) {
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, input, type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}
void checkFbo() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Incomplete OpenGL framebuffer");
}
void text(cairo_t *c, double x, double y, double size, const std::string &s, bool bold = false) {
    cairo_select_font_face(c, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                           bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(c, size);
    cairo_move_to(c, x, y);
    cairo_show_text(c, s.c_str());
}
} // namespace
Renderer::Renderer(const Frame &f, int w, int h, RenderOptions options) : opt_(options) {
    mesh_ = program(shaders::mesh_vert, shaders::mesh_frag);
    depth_ = program(shaders::mesh_vert, shaders::shadow_frag);
    sky_ = program(shaders::full_vert, shaders::sky_frag);
    blur_ = program(shaders::full_vert, shaders::blur_frag);
    post_ = program(shaders::full_vert, shaders::post_frag);
    glGenVertexArrays(1, &quad_);
    for (size_t k = 0; k < gpu_.size(); k++) {
        auto &g = gpu_[k];
        const auto &mesh = f.batches[k].mesh;
        g.count = mesh.ix.size();
        glGenVertexArrays(1, &g.vao);
        glBindVertexArray(g.vao);
        glGenBuffers(1, &g.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.v.size() * sizeof(Vertex), mesh.v.data(),
                     GL_STATIC_DRAW);
        glGenBuffers(1, &g.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.ix.size() * sizeof(std::uint32_t),
                     mesh.ix.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, n));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, 0, sizeof(Vertex), (void *)offsetof(Vertex, uv));
        glGenBuffers(1, &g.instances);
        glBindBuffer(GL_ARRAY_BUFFER, g.instances);
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, 0, sizeof(Instance),
                                  (void *)(sizeof(float) * i * 4));
            glVertexAttribDivisor(3 + i, 1);
        }
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, 0, sizeof(Instance),
                              (void *)offsetof(Instance, color));
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, 0, sizeof(Instance),
                              (void *)offsetof(Instance, surface));
        glVertexAttribDivisor(8, 1);
    }

    // Eight original race-number decals. Fonts are provided by the host, never bundled.
    {
        int tw = 512, th = 1024;
        auto *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, tw, th);
        auto *paint = cairo_create(surf);
        cairo_set_operator(paint, CAIRO_OPERATOR_CLEAR);
        cairo_paint(paint);
        cairo_set_operator(paint, CAIRO_OPERATOR_OVER);
        for (int i = 0; i < 8; i++) {
            cairo_set_source_rgba(paint, .008, .018, .027, .99);
            cairo_rectangle(paint, 0, i * 128, 512, 128);
            cairo_fill(paint);
            cairo_set_source_rgba(paint, .92, .96, .98, .98);
            text(paint, 116, i * 128 + 101, 116, "0" + std::to_string(i + 1), true);
        }
        cairo_surface_flush(surf);
        auto *a = cairo_image_surface_get_data(surf);
        int stride = cairo_image_surface_get_stride(surf);
        std::vector<unsigned char> rgba(tw * th * 4);
        for (int y = 0; y < th; y++)
            for (int x = 0; x < tw; x++) {
                auto *p = a + y * stride + x * 4;
                auto *o = &rgba[(y * tw + x) * 4];
                int alpha = p[3];
                o[0] = alpha ? std::min(255, p[2] * 255 / alpha) : 0;
                o[1] = alpha ? std::min(255, p[1] * 255 / alpha) : 0;
                o[2] = alpha ? std::min(255, p[0] * 255 / alpha) : 0;
                o[3] = alpha;
            }
        hull_ = texture(tw, th, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        cairo_destroy(paint);
        cairo_surface_destroy(surf);
    }
    Random rng(99183);
    std::vector<unsigned char> noise(256 * 256);
    for (auto &n : noise)
        n = rng.next() & 255;
    QImage atlas(":/prism/assets/textures/material-atlas.png");
    if (atlas.isNull())
        throw std::runtime_error("Embedded material atlas could not be loaded");
    atlas = atlas.convertToFormat(QImage::Format_RGBA8888);
    materials_ = texture(atlas.width(), atlas.height(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
                         atlas.constBits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    QImage facade(":/prism/assets/textures/facade-atlas.png");
    if (facade.isNull())
        throw std::runtime_error("Embedded facade atlas could not be loaded");
    facade = facade.convertToFormat(QImage::Format_RGBA8888);
    facades_ = texture(facade.width(), facade.height(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
                       facade.constBits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    noise_ = texture(256, 256, GL_R8, GL_RED, GL_UNSIGNED_BYTE, noise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    shadow_ = texture(2048, 2048, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenFramebuffers(1, &shadowFbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    checkFbo();
    glGenFramebuffers(1, &msFbo_);
    glGenRenderbuffers(1, &msColor_);
    glGenRenderbuffers(1, &msDepth_);
    glGenFramebuffers(1, &hdrFbo_);
    glGenFramebuffers(2, blurFbo_);
    resize(w, h);
    glDisable(GL_FRAMEBUFFER_SRGB);
}
Renderer::~Renderer() {
    for (auto g : gpu_) {
        glDeleteBuffers(1, &g.vbo);
        glDeleteBuffers(1, &g.ibo);
        glDeleteBuffers(1, &g.instances);
        glDeleteVertexArrays(1, &g.vao);
    }
    for (GLuint p : {mesh_, depth_, sky_, blur_, post_})
        glDeleteProgram(p);
    glDeleteVertexArrays(1, &quad_);
    for (GLuint t : {hdr_, z_, shadow_, overlay_, noise_, hull_, materials_, facades_, blurTex_[0],
                     blurTex_[1]})
        glDeleteTextures(1, &t);
    for (GLuint b : {hdrFbo_, msFbo_, shadowFbo_, blurFbo_[0], blurFbo_[1]})
        glDeleteFramebuffers(1, &b);
    glDeleteRenderbuffers(1, &msColor_);
    glDeleteRenderbuffers(1, &msDepth_);
}
void Renderer::resize(int w, int h) {
    if (w <= 0 || h <= 0 || w > 8192 || h > 8192)
        throw std::runtime_error("Framebuffer dimensions outside 1..8192");
    GLint limit = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
    if (w > limit || h > limit)
        throw std::runtime_error("Output exceeds GPU texture limit");
    w_ = w;
    h_ = h;
    if (opt_.samples > 1) {
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        opt_.samples = std::min(opt_.samples, maxSamples);
        glBindFramebuffer(GL_FRAMEBUFFER, msFbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, msColor_);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, opt_.samples, GL_RGBA16F, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msColor_);
        glBindRenderbuffer(GL_RENDERBUFFER, msDepth_);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, opt_.samples, GL_DEPTH_COMPONENT24, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msDepth_);
        checkFbo();
    }

    for (GLuint t : {hdr_, z_, overlay_, blurTex_[0], blurTex_[1]})
        if (t)
            glDeleteTextures(1, &t);
    hdr_ = texture(w, h, GL_RGBA16F);
    z_ = texture(w, h, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
    overlay_ = texture(w, h, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, z_, 0);
    checkFbo();
    for (int i = 0; i < 2; i++) {
        blurTex_[i] = texture(std::max(1, w / 3), std::max(1, h / 3), GL_RGBA16F);
        glBindFramebuffer(GL_FRAMEBUFFER, blurFbo_[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex_[i], 0);
        checkFbo();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    lastChapter_ = -1;
    overlayStamp_ = -1;
}
void Renderer::uploadOverlay(const Frame &f, const std::string &) {
    long stamp = opt_.titles ? long(f.time * 6) : -2;
    if (overlayStamp_ == stamp && lastChapter_ == f.focus)
        return;
    overlayStamp_ = stamp;
    lastChapter_ = f.focus;
    auto *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w_, h_);
    auto *c = cairo_create(s);
    cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);
    cairo_paint(c);
    cairo_set_operator(c, CAIRO_OPERATOR_OVER);
    if (opt_.titles) {
        float k = h_ / 1080.f;
        double x = 46 * k, y = 48 * k;
        // Sparse broadcast graphics leave the characters and circuit in charge.
        cairo_set_source_rgba(c, .005, .012, .031, .57);
        cairo_rectangle(c, x - 15 * k, y - 25 * k, 370 * k, 105 * k);
        cairo_fill(c);
        cairo_set_source_rgba(c, .36, .95, .81, .96);
        text(c, x, y - 2 * k, 11 * k, "QINDAQT   /   AFTERGLOW GRAND PRIX", true);
        cairo_set_source_rgba(c, .94, .97, 1, .96);
        text(c, x, y + 30 * k, 27 * k, "PRISM CIRCUIT", true);
        cairo_set_source_rgba(c, .66, .76, .88, .86);
        text(c, x, y + 49 * k, 10 * k, f.courseName + "   /   AUTONOMOUS RACE");
        V3 col = racerColors[f.focus];
        double bottom = h_ - 95 * k;
        cairo_set_source_rgba(c, .005, .012, .03, .65);
        cairo_rectangle(c, x - 15 * k, bottom - 35 * k, 290 * k, 80 * k);
        cairo_fill(c);
        cairo_set_source_rgba(c, col.x, col.y, col.z, .95);
        cairo_rectangle(c, x - 15 * k, bottom - 35 * k, 3 * k, 80 * k);
        cairo_fill(c);
        std::ostringstream name;
        name << f.position << "  /  " << racerNames[f.focus];
        cairo_set_source_rgba(c, .96, .97, 1, .95);
        text(c, x, bottom - 7 * k, 22 * k, name.str(), true);
        std::ostringstream status;
        status << "LAP " << f.lap << " / 3     " << int(std::round(f.speed)) << " km/h";
        cairo_set_source_rgba(c, .64, .78, .87, .93);
        text(c, x, bottom + 19 * k, 12 * k, status.str());
        if (f.boost > .05f) {
            cairo_set_source_rgba(c, .26, .81, 1, .95);
            text(c, x + 214 * k, bottom + 19 * k, 10 * k, "BOOST", true);
        }
        cairo_set_source_rgba(c, .56, .71, .8, .9);
        text(c, x, y + 69 * k, 10 * k, f.shotLabel);
        if (f.damage > 0) {
            cairo_set_source_rgba(c, 1, .43, .18, .9);
            text(c, x, bottom + 37 * k, 10 * k,
                 "DAMAGE " + std::to_string(int(f.damage)) + "%  /  KO " +
                     std::to_string(f.takedowns));
        }
        if (!f.eventLabel.empty()) {
            cairo_set_source_rgba(c, .97, .73, .30, .95);
            text(c, w_ * .5f - 200 * k, 65 * k, 14 * k, f.eventLabel, true);
        }
        if (f.countdown > 0) {
            cairo_set_source_rgba(c, .9, .99, 1, .94);
            text(c, w_ / 2.f - 20 * k, h_ * .23f, 58 * k, std::to_string(f.countdown), true);
        }
        if (f.winner >= 0) {
            std::string title = std::string(racerNames[f.winner]) + "  /  RACE WINNER";
            cairo_set_source_rgba(c, .92, .8, .42, .97);
            text(c, w_ * .5f - 185 * k, 78 * k, 23 * k, title, true);
        }
        if (!f.map.empty()) {
            double mx = w_ - 131 * k, my = h_ - 124 * k;
            float sz = 73 * k;
            cairo_set_line_width(c, 2 * k);
            cairo_set_source_rgba(c, .23, .36, .51, .72);
            for (size_t i = 0; i < f.map.size(); i++) {
                auto p = f.map[i];
                if (i)
                    cairo_line_to(c, mx + p.x * sz, my + p.y * sz);
                else
                    cairo_move_to(c, mx + p.x * sz, my + p.y * sz);
            }
            cairo_stroke(c);
            for (int i = 7; i >= 0; i--) {
                V3 co = racerColors[i];
                cairo_set_source_rgba(c, co.x, co.y, co.z, .95);
                cairo_arc(c, mx + f.mapX[i] * sz, my + f.mapY[i] * sz,
                          (i == f.focus ? 4.2 : 2.7) * k, 0, 2 * pi);
                cairo_fill(c);
            }
        }
    }
    if (opt_.titles && !f.itemLabel.empty()) {
        float k = std::min(h_ / 900.f, w_ / 1440.f);
        cairo_set_source_rgba(c, .32, .95, .8, .95);
        text(c, w_ * .5 - 64 * k, h_ - 36 * k, 13 * k, f.itemLabel, true);
    }
    cairo_surface_flush(s);
    auto *bytes = cairo_image_surface_get_data(s);
    int stride = cairo_image_surface_get_stride(s);
    std::vector<unsigned char> rgba(size_t(w_) * h_ * 4);
    for (int y = 0; y < h_; y++)
        for (int x = 0; x < w_; x++) {
            auto *src = bytes + y * stride + x * 4;
            auto *dst = &rgba[(size_t(y) * w_ + x) * 4];
            int a = src[3];
            dst[0] = a ? std::min(255, src[2] * 255 / a) : 0;
            dst[1] = a ? std::min(255, src[1] * 255 / a) : 0;
            dst[2] = a ? std::min(255, src[0] * 255 / a) : 0;
            dst[3] = src[3];
        }
    tex(overlay_, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    cairo_destroy(c);
    cairo_surface_destroy(s);
}
void Renderer::draw(const Frame &f, const std::string &metrics) {
    const bool profile = std::getenv("PRISM_PROFILE");
    auto now = std::chrono::steady_clock::now();
    auto tick = [&](const char *phase) {
        if (profile) {
            glFinish();
            auto p = std::chrono::steady_clock::now();
            std::cerr << phase << ": " << std::chrono::duration<double>(p - now).count() << "\n";
            now = p;
        }
    };
    uploadOverlay(f, metrics);
    tick("overlay");
    for (size_t k = 0; k < gpu_.size(); k++) {
        auto &g = gpu_[k];
        const auto &a = f.batches[k].instances;
        glBindBuffer(GL_ARRAY_BUFFER, g.instances);
        glBufferData(GL_ARRAY_BUFFER, a.size() * sizeof(Instance), a.data(), GL_STREAM_DRAW);
    }
    M4 lv = ortho(-42, 42, -42, 42, 1, 180) * lookAt(f.pengu + V3{-43, 68, 52}, f.pengu);
    M4 vp = perspective(f.fov, float(w_) / h_, .15f, 5000) * lookAt(f.eye, f.target, f.up);
    auto geometry = [&](GLuint p, M4 matrix) {
        glUseProgram(p);
        uniform(p, "vp", matrix);
        uniform(p, "lightVP", lv);
        uniform(p, "time", float(f.motionTime));
        uniform(p, "eye", f.eye);
        uniform(p, "hero", f.pengu);
        uniform(p, "chapter", f.chapter);
        uniform(p, "noiseTex", 0);
        uniform(p, "shadowTex", 1);
        uniform(p, "hullTex", 3);
        uniform(p, "materialAtlas", 4);
        uniform(p, "facadeAtlas", 5);
        for (size_t k = 0; k < gpu_.size(); k++) {
            if (k == Smoke)
                continue;
            auto &g = gpu_[k];
            auto count = f.batches[k].instances.size();
            if (!count)
                continue;
            glBindVertexArray(g.vao);
            glDrawElementsInstanced(GL_TRIANGLES, g.count, GL_UNSIGNED_INT, nullptr, int(count));
        }
    };
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo_);
    glViewport(0, 0, 2048, 2048);
    glClear(GL_DEPTH_BUFFER_BIT);
    geometry(depth_, lv);
    tick("shadow");
    glBindFramebuffer(GL_FRAMEBUFFER, opt_.samples > 1 ? msFbo_ : hdrFbo_);
    glViewport(0, 0, w_, h_);
    glClearColor(.001, .001, .002, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(sky_);
    tex(noise_, 0);
    uniform(sky_, "noiseTex", 0);
    uniform(sky_, "time", float(f.motionTime));
    uniform(sky_, "seed", float(f.voyage) + 3.f);
    uniform(sky_, "chapter", f.chapter);
    uniform(sky_, "eye", f.eye);
    uniform(sky_, "target", f.target);
    uniform(sky_, "up", f.up);
    uniform(sky_, "fov", f.fov);
    glUniform2f(glGetUniformLocation(sky_, "resolution"), w_, h_);
    glBindVertexArray(quad_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    tick("sky");
    glEnable(GL_DEPTH_TEST);
    tex(noise_, 0);
    tex(shadow_, 1);
    tex(hull_, 3);
    tex(materials_, 4);
    tex(facades_, 5);
    geometry(mesh_, vp);
    if (!f.batches[Smoke].instances.empty()) {
        auto particles = f.batches[Smoke].instances;
        std::sort(particles.begin(), particles.end(), [&](const Instance &a, const Instance &b) {
            return length(point(a.model, {0, 0, 0}) - f.eye) >
                   length(point(b.model, {0, 0, 0}) - f.eye);
        });
        auto &g = gpu_[Smoke];
        glBindBuffer(GL_ARRAY_BUFFER, g.instances);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Instance), particles.data(),
                     GL_STREAM_DRAW);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBindVertexArray(g.vao);
        glDrawElementsInstanced(GL_TRIANGLES, g.count, GL_UNSIGNED_INT, nullptr,
                                int(particles.size()));
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    glDisable(GL_DEPTH_TEST);
    tick("meshes");
    if (opt_.samples > 1) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msFbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFbo_);
        glBlitFramebuffer(0, 0, w_, h_, 0, 0, w_, h_, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
    }
    glBindVertexArray(quad_);
    glViewport(0, 0, std::max(1, w_ / 3), std::max(1, h_ / 3));
    glUseProgram(blur_);
    uniform(blur_, "image", 0);
    for (int pass = 0; pass < 6; pass++) {
        int dst = pass % 2;
        glBindFramebuffer(GL_FRAMEBUFFER, blurFbo_[dst]);
        tex(pass == 0 ? hdr_ : blurTex_[1 - dst], 0);
        uniform(blur_, "extract", pass == 0 ? 1 : 0);
        glUniform2f(glGetUniformLocation(blur_, "direction"), (pass % 2 == 0) ? 3.8f / w_ : 0,
                    (pass % 2) ? 3.8f / h_ : 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    tick("bloom");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w_, h_);
    glUseProgram(post_);
    tex(hdr_, 0);
    tex(blurTex_[1], 1);
    tex(overlay_, 2);
    uniform(post_, "image", 0);
    uniform(post_, "bloom", 1);
    uniform(post_, "overlay", 2);
    uniform(post_, "fade", f.fade);
    uniform(post_, "brightness", opt_.brightness);
    uniform(post_, "bloomAmount", opt_.bloom);
    glUniform2f(glGetUniformLocation(post_, "resolution"), w_, h_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    tick("post");
    GLenum err = glGetError();
    if (err)
        throw std::runtime_error("OpenGL error: " + std::to_string(err));
}
std::vector<unsigned char> Renderer::pixels() {
    std::vector<unsigned char> out(size_t(w_) * h_ * 3), bottom(out.size());
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w_, h_, GL_RGB, GL_UNSIGNED_BYTE, bottom.data());
    for (int y = 0; y < h_; y++)
        std::memcpy(out.data() + size_t(y) * w_ * 3, bottom.data() + size_t(h_ - 1 - y) * w_ * 3,
                    size_t(w_) * 3);
    return out;
}
void png(const std::string &file, const std::vector<unsigned char> &rgb, int w, int h) {
    auto *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    auto *p = cairo_image_surface_get_data(s);
    int stride = cairo_image_surface_get_stride(s);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            const auto *a = &rgb[(size_t(y) * w + x) * 3];
            auto *b = p + y * stride + x * 4;
            b[0] = a[2];
            b[1] = a[1];
            b[2] = a[0];
            b[3] = 255;
        }
    cairo_surface_mark_dirty(s);
    auto status = cairo_surface_write_to_png(s, file.c_str());
    cairo_surface_destroy(s);
    if (status)
        throw std::runtime_error("PNG write: " + std::string(cairo_status_to_string(status)));
}
} // namespace sw
