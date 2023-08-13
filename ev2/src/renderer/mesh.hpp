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

namespace ev2::renderer {

class Mesh {
public:
    virtual void set_vertex_buffer(std::shared_ptr<VertexBuffer> vBuffer) = 0;
    virtual void set_index_buffer(std::shared_ptr<Buffer> buffer) = 0;
};

} // namespace ev2::renderer

#endif // EV2_MESH_HPP