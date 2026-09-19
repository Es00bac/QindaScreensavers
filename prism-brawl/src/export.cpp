// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace sw {
namespace {
// Inverse-transpose normal transform, including nonuniform instance scaling.
V3 transformedNormal(const M4& matrix, V3 normal) {
    const auto& a = matrix.a;
    const V3 x{a[0], a[1], a[2]};
    const V3 y{a[4], a[5], a[6]};
    const V3 z{a[8], a[9], a[10]};
    const float determinant = dot(x, cross(y, z));
    V3 result = cross(y, z) * normal.x + cross(z, x) * normal.y
              + cross(x, y) * normal.z;
    if (determinant < 0) result = -result;
    return unit(result);
}
}

void exportModels(const std::string& directory,std::uint64_t seed,int course) {
    namespace fs = std::filesystem;
    fs::create_directories(directory);
    Frame frame;
    initialize(frame);

    (void)seed; (void)course;
    for (int asset = 0; asset < 11; ++asset) {
        for (auto& batch : frame.batches) batch.instances.clear();
        std::string name;
        if(asset<8){
            name=std::string(racerSlugs[asset])+"-fighter";
            Anim anim;fighterModel(frame,translate({0,.72f,0}),asset,0,anim);
        }else{
            name=asset==8?"prism-terminal":asset==9?"reactor-garden":"afterglow-rooftop";
            stageModel(frame,asset-8,0,seed,false);
        }

        std::ofstream obj(fs::path(directory) / (name + ".obj"));
        std::ofstream mtl(fs::path(directory) / (name + ".mtl"));
        if (!obj || !mtl) throw std::runtime_error("Cannot create OBJ/MTL files");
        obj << std::setprecision(7)
            << "# Prism Brawl / GPL-3.0-or-later\nmtllib "
            << name << ".mtl\ns 1\n";
        std::uint64_t offset = 1;
        int part = 0;
        for (const auto& batch : frame.batches) {
            for (const auto& instance : batch.instances) {
                const std::string material = "part_" + std::to_string(part++);
                obj << "o " << batch.mesh.name << '_' << material
                    << "\nusemtl " << material << '\n';
                const auto& c = instance.color;
                mtl << "newmtl " << material
                    << "\nKd " << c.x << ' ' << c.y << ' ' << c.z
                    << "\nKe " << c.x*c.w << ' ' << c.y*c.w << ' ' << c.z*c.w
                    << "\nKs " << instance.surface.y << ' ' << instance.surface.y
                    << ' ' << instance.surface.y
                    << "\nNs " << (1-instance.surface.x)*200 << "\n\n";
                for (const auto& vertex : batch.mesh.v) {
                    const V3 p = point(instance.model, vertex.p);
                    obj << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n';
                }
                for (const auto& vertex : batch.mesh.v)
                    obj << "vt " << vertex.uv.x << ' ' << vertex.uv.y << '\n';
                for (const auto& vertex : batch.mesh.v) {
                    const V3 n = transformedNormal(instance.model, vertex.n);
                    obj << "vn " << n.x << ' ' << n.y << ' ' << n.z << '\n';
                }
                for (std::size_t k = 0; k < batch.mesh.ix.size(); k += 3) {
                    obj << 'f';
                    for (int corner = 0; corner < 3; ++corner) {
                        const auto index = batch.mesh.ix[k+corner] + offset;
                        obj << ' ' << index << '/' << index << '/' << index;
                    }
                    obj << '\n';
                }
                offset += batch.mesh.v.size();
            }
        }
        if (!obj || !mtl) throw std::runtime_error("Model export write failed");
    }
}
}
