#include "gl_mesh.hpp"

namespace ev2::renderer {

void GLMesh::set_vertex_buffer(std::shared_ptr<VertexBuffer> vBuffer) {
    m_vertex_buffer = vBuffer;
}

void GLMesh::set_index_buffer(std::shared_ptr<Buffer> buffer) {
    m_index_buffer = std::dynamic_pointer_cast<GLBuffer>(buffer);
}

void GLMesh::set_primitives(const std::vector<Primitive>& primitives) {
    m_primitives = primitives;
}

void GLMesh::set_materials(const std::vector<std::shared_ptr<Material>>& materials) {
    m_materials.clear();
    m_materials.reserve(materials.size());
    for (auto& m : materials) {
        auto mat = std::dynamic_pointer_cast<GLMaterial>(m);
        EV_CORE_ASSERT(mat, "Material is not a GLMaterial");
        if (mat)
            m_materials.push_back(mat);
    }
}


std::shared_ptr<VertexBuffer> GLMesh::get_vertex_buffer() const {
    return m_vertex_buffer;
}

std::shared_ptr<Buffer> GLMesh::get_index_buffer() const {
    return m_index_buffer;
}

} // namespace ev2::renderer