/**
 * @file mesh.hpp
 * @brief 
 * @date 2023-08-11
 * 
 */
#ifndef EV2_MESH_HPP
#define EV2_MESH_HPP

#include "evpch.hpp"

#include "renderer/vertex_buffer.hpp"
#include "renderer/buffer.hpp"
#include "renderer/material.hpp"


namespace ev2::renderer {

struct Primitive {
    size_t      start_index = 0;
    size_t      num_elements = 0;
    int32_t     material_ind = 0;
};

class Mesh {
public:
    virtual ~Mesh() = default;

    virtual void set_vertex_buffer(std::shared_ptr<VertexBuffer> vBuffer) = 0;

    /**
     * @brief Set the index buffer object. If an index buffer is set, the indices in Primitives 
     * will refer to index of the index buffer, rather than the vertex buffers 
     * 
     * @param buffer 
     */
    virtual void set_index_buffer(std::shared_ptr<Buffer> buffer) = 0;
    virtual void set_primitives(const std::vector<Primitive>& primitives) = 0;
    virtual void set_materials(const std::vector<std::shared_ptr<Material>>& materials) = 0;

    virtual std::shared_ptr<VertexBuffer> get_vertex_buffer() const = 0;
    virtual std::shared_ptr<Buffer> get_index_buffer() const = 0;
    virtual std::vector<Primitive> get_primitives() const = 0;
};

} // namespace ev2::renderer

#endif // EV2_MESH_HPP