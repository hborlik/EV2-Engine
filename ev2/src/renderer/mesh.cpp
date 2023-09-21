#include "renderer/mesh.hpp"

#include "renderer/render_api.hpp"

#include "renderer/opengl_renderer/gl_mesh.hpp"

namespace ev2::renderer {

std::unique_ptr<Mesh> Mesh::make_mesh(
    std::shared_ptr<VertexBuffer> vBuffer,
    std::shared_ptr<Buffer> index_buffer,
    const std::vector<Primitive>& primitives,
    const std::vector<std::shared_ptr<Material>>& materials,
    const AABB& aabb,
    const Sphere& sphere,
    FrustumCull mode,
    CullMode face_cull,
    FrontFacing front) {
        
    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL: {

            auto gl_ib = std::dynamic_pointer_cast<GLBuffer>(index_buffer);
            EV_CORE_ASSERT(gl_ib, "Buffer must be a GL buffer!");

            std::vector<std::shared_ptr<GLMaterial>> gl_mat;
            gl_mat.reserve(materials.size());
            for (auto& mat : materials) {
                auto m = std::dynamic_pointer_cast<GLMaterial>(mat);
                EV_CORE_ASSERT(m, "All materials must be GLMaterials!");
                gl_mat.push_back(m);
            }

            auto mesh = std::make_unique<GLMesh>(
                vBuffer,
                gl_ib, // index_buffer
                primitives,
                gl_mat,
                aabb,
                sphere,
                mode,
                gl::CullMode::BACK,
                gl::FrontFacing::CCW
            );

            return mesh;
        }
        default: 
        EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
        break;
    }

    return {};
}

}