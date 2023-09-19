/**
 * @file model.hpp
 * @brief 
 * @date 2023-08-18
 * 
 * 
 */
#ifndef EV2_IO_MODEL_HPP
#define EV2_IO_MODEL_HPP

#include "evpch.hpp"

#include "io/material.hpp"
#include "renderer/vertex_buffer_layout.hpp"

namespace ev2 {

struct DrawObject {
    size_t start;
    size_t numTriangles;
    size_t material_id;
};

class Model {
public:
    Model(const std::string &name,
          std::vector<DrawObject> draw_objects,
          std::vector<MaterialData> materials,
          glm::vec3 bmin,
          glm::vec3 bmax,
          renderer::VertexBufferLayout layout,
          std::vector<float> vb) : name{name},
                                   draw_objects{std::move(draw_objects)},
                                   materials{std::move(materials)},
                                   buffer_layout{std::move(layout)},
                                   buffer{std::move(vb)},
                                   bmin{bmin},
                                   bmax{bmax}
                                   {}

    std::string             name;
    std::vector<DrawObject> draw_objects;
    std::vector<MaterialData>materials;
    renderer::VertexBufferLayout buffer_layout;
    std::vector<float>      buffer;

    glm::vec3 bmin, bmax;
};

std::unique_ptr<Model> load_model(const std::filesystem::path& filename, const std::filesystem::path& base_dir);

}

#endif // EV2_IO_MODEL_HPP