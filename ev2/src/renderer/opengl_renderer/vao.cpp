#include "vao.hpp"

#include "gl_shader.hpp"

namespace ev2::renderer {

VAO VAOFactory::gen_vao_for_attributes(const VertexBuffer* vb, const Buffer* index_buffer, const std::unordered_map<AttributeLabel, int>& attributes, const Buffer* instance_buffer) {
    const GLBuffer* gl_ind = dynamic_cast<const GLBuffer*>(index_buffer);
    const GLBuffer* gl_inst= dynamic_cast<const GLBuffer*>(instance_buffer);

    EV_CORE_ASSERT(vb != nullptr, "const VertexBuffer* vb cannot be NULL!");
    EV_CORE_ASSERT(index_buffer == nullptr || (index_buffer != nullptr && gl_ind != nullptr), "const Buffer* index_buffer must be of type const GLBuffer*!")
    EV_CORE_ASSERT(instance_buffer == nullptr || (instance_buffer != nullptr && gl_inst != nullptr), "const Buffer* instance_buffer must be of type const GLBuffer*!")

    VAO vao{};
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
        
            auto buffer = std::dynamic_pointer_cast<GLBuffer>(vb->get_buffer(vba.buffer_id));

            buffer->bind();
            glEnableVertexAttribArray(binding);
            glVertexAttribPointer(binding, vba.count, (GLenum)vba.type, vba.normalized ? GL_TRUE : GL_FALSE, vba.stride, (void*)vba.byte_offset);
            buffer->unbind();
        }
            // std::cout << "could not find accessor for " << (int)accessor_id << std::endl;
    }

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

    // Unbind VAO
    vao.unbind();

    if (gl_ind) {
        gl_ind->unbind();
    }

    return std::move(vao);
}

} // namespace ev2::renderer