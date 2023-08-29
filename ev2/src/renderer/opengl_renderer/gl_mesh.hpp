/**
 * @file gl_mesh.hpp
 * @author Hunter Borlik
 * @brief 
 * @date 2023-08-17
 * 
 */
#ifndef EV2_GL_MESH_HPP
#define EV2_GL_MESH_HPP

#include "geometry.hpp"
#include "ev_gl.hpp"
#include "renderer/mesh.hpp"
#include "renderer/renderer.hpp"

#include "gl_buffer.hpp"
#include "gl_material.hpp"

namespace ev2::renderer {

// struct GLMesh : public Mesh {

//     GLMesh(VertexBuffer &&vb,
//              std::vector<Primitive> primitives,
//              std::vector<std::shared_ptr<Material>> materials,
//              AABB bounding_box,
//              Sphere bounding_sphere,
//              FrustumCull frustum_cull,
//              gl::CullMode cull,
//              gl::FrontFacing ff) : vertex_buffer{std::move(vb)},
//                                    primitives{std::move(primitives)},
//                                    materials{std::move(materials)},
//                                    bounding_box{bounding_box},
//                                    bounding_sphere{bounding_sphere},
//                                    frustum_cull{frustum_cull},
//                                    cull_mode{cull},
//                                    front_facing{ff}
//     {
//     }

//     VertexBuffer                vertex_buffer;
// };

class GLMesh : public Mesh {
public:
    void set_vertex_buffer(std::shared_ptr<VertexBuffer> vBuffer) override;
    void set_index_buffer(std::shared_ptr<Buffer> buffer) override;
    void set_primitives(const std::vector<Primitive>& primitives) override;
    void set_materials(const std::vector<std::shared_ptr<Material>>& materials) override;

    std::shared_ptr<VertexBuffer> get_vertex_buffer() const override;
    std::shared_ptr<Buffer> get_index_buffer() const override;

public:
    AABB            bounding_box{};
    Sphere          bounding_sphere{};
    FrustumCull     frustum_cull = FrustumCull::None;
    gl::CullMode    cull_mode = gl::CullMode::BACK;
    gl::FrontFacing front_facing = gl::FrontFacing::CCW;

    float vertex_color_weight = 0.f;

    std::shared_ptr<VertexBuffer>   m_vertex_buffer{};
    std::shared_ptr<GLBuffer>       m_index_buffer{};

    std::vector<Primitive>          m_primitives;
    std::vector<std::shared_ptr<GLMaterial>>  m_materials;
};

}

#endif // EV2_GL_MESH_HPP