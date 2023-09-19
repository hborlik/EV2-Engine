#include "vao.hpp"

#include "gl_shader.hpp"

namespace ev2::renderer {

void instance_buffer_setup(const GLBuffer* gl_inst) {
    if (gl_inst != nullptr) {
        // bind instance buffer
        gl_inst->bind();

        // mat4 instance info, note: max size for a vertex attribute is a vec4
        constexpr std::size_t vec4Size = sizeof(glm::vec4);
        const int binding = mat_spec::INSTANCE_BINDING_LOCATION;
        glEnableVertexAttribArray(binding);
        glVertexAttribPointer(binding, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

        glEnableVertexAttribArray(binding + 1);
        glVertexAttribPointer(binding + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

        glEnableVertexAttribArray(binding + 2);
        glVertexAttribPointer(binding + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

        glEnableVertexAttribArray(binding + 3);
        glVertexAttribPointer(binding + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

        glVertexAttribDivisor(binding,  1);
        glVertexAttribDivisor(binding+1, 1);
        glVertexAttribDivisor(binding+2, 1);
        glVertexAttribDivisor(binding+3, 1);

        gl_inst->unbind();
    }
}

VAO VAOFactory::make_vao(std::shared_ptr<VertexBuffer> vb, const std::unordered_map<AttributeLabel, int>& binding_map, std::shared_ptr<Buffer> instance_buffer) {
    auto gl_inst= std::dynamic_pointer_cast<const GLBuffer>(instance_buffer);

    EV_CORE_ASSERT(vb != nullptr, "const VertexBuffer* vb cannot be NULL!");
    EV_CORE_ASSERT(instance_buffer == nullptr || (instance_buffer != nullptr && gl_inst != nullptr), "const Buffer* instance_buffer must be of type const GLBuffer*!");

    VAO vao{vb, nullptr};
    vao.bind();

    instance_buffer_setup(gl_inst.get());

    vao.unbind();
    return vao;
}

VAO VAOFactory::make_vao(std::shared_ptr<VertexBuffer> vb, std::shared_ptr<Buffer> index_buffer, const std::unordered_map<AttributeLabel, int>& attributes, std::shared_ptr<Buffer> instance_buffer) {
    auto gl_ind = std::dynamic_pointer_cast<const GLBuffer>(index_buffer);
    auto gl_inst= std::dynamic_pointer_cast<const GLBuffer>(instance_buffer);

    EV_CORE_ASSERT(vb != nullptr, "const VertexBuffer* vb cannot be NULL!");
    EV_CORE_ASSERT(index_buffer == nullptr || (index_buffer != nullptr && gl_ind != nullptr), "const Buffer* index_buffer must be of type const GLBuffer*!");
    EV_CORE_ASSERT(instance_buffer == nullptr || (instance_buffer != nullptr && gl_inst != nullptr), "const Buffer* instance_buffer must be of type const GLBuffer*!");

    VAO vao{vb, index_buffer};
    vao.bind();

    if (gl_ind) {
        gl_ind->bind();
    }

    /* Every attribute assigns a vertex attribute (Vertex,Normal,Color,...) to a binding index in shader */
    for (const auto& l : attributes) {
        const int binding = l.second;
        const AttributeLabel accessor_id = l.first;
        EV_CORE_ASSERT(binding >= 0, "Cannot have binding index <0!");

        auto accessor_itr = vb->accessors.find(accessor_id);
        if(accessor_itr != vb->accessors.end()) {
            const VertexBufferAccessor& vba = accessor_itr->second;
            const Attribute& attr = vba.attribute;
        
            auto buffer = std::dynamic_pointer_cast<GLBuffer>(vb->get_buffer(vba.buffer_id));

            buffer->bind();
            glEnableVertexAttribArray(binding);
            const gl::VertexAttribPointer gl_vert_ptr = gl::getGLAttrPtrSize(attr.type);
            glVertexAttribPointer(binding, gl_vert_ptr.size, (GLenum)gl_vert_ptr.type, GL_FALSE, attr.stride, (void*)attr.byte_offset);
            buffer->unbind();
        }
            // std::cout << "could not find accessor for " << (int)accessor_id << std::endl;
    }

    instance_buffer_setup(gl_inst.get());

    // Unbind VAO
    vao.unbind();

    if (gl_ind) {
        gl_ind->unbind();
    }

    return std::move(vao);
}

} // namespace ev2::renderer