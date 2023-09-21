/**
 * @file mesh.hpp
 * @brief 
 * @date 2023-08-11
 * 
 */
#ifndef EV2_MESH_HPP
#define EV2_MESH_HPP

#include "evpch.hpp"

#include "geometry.hpp"
#include "renderer/vertex_buffer.hpp"
#include "renderer/buffer.hpp"
#include "renderer/material.hpp"


namespace ev2::renderer {

/**
 * @brief Frustum Culling shape used by mesh
 * 
 */
enum class FrustumCull {
    None,
    Sphere,
    AABB
};

/**
 * @brief Face culling mode
 * 
 */
enum class CullMode {
    None = 0,
    Back,
    Front
};

enum class FrontFacing {
    CCW = 0,
    CW
};

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

    /**
     * @brief 
     * 
     * @param vBuffer required vertex buffer
     * @param index_buffer optional 
     * @param primitives 
     * @param face_cull 
     * @param front 
     * @return std::unique_ptr<Mesh> 
     */
    static std::unique_ptr<Mesh> make_mesh(
        
    std::shared_ptr<VertexBuffer> vBuffer,
        std::shared_ptr<Buffer> index_buffer,
        const std::vector<Primitive>& primitives,
        const std::vector<std::shared_ptr<Material>>& materials,
        const AABB& aabb,
        const Sphere& sphere,
        FrustumCull mode,
        CullMode face_cull = CullMode::Back,
        FrontFacing front = FrontFacing::CCW);
};

} // namespace ev2::renderer

#endif // EV2_MESH_HPP