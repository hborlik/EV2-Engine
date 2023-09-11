/**
 * @file vao.h
 * @brief 
 * @date 2022-10-17
 * 
 * 
 */
#ifndef EV2_VAO_H
#define EV2_VAO_H

#include "evpch.hpp"

#include "renderer/vertex_buffer.hpp"
#include "util.hpp"

#include "ev_gl.hpp"
#include "gl_buffer.hpp"

namespace ev2::renderer {

class VAO {
public:
    VAO() {
        glGenVertexArrays(1, &m_handle.v);
    }

    explicit VAO(GLuint handle) : m_handle{handle} {}

    VAO(VAO&&) = default;
    VAO& operator=(VAO&&) = default;

    ~VAO() {
        if (glIsVertexArray(m_handle.v))
            glDeleteVertexArrays(1, &m_handle.v);
    }

    void bind() const { glBindVertexArray(m_handle.v); }
    void unbind() const { glBindVertexArray(0); }
    GLuint get_handle() const noexcept {return m_handle.v;}

private:
    util::non_copyable<GLuint> m_handle;
};

class VAOFactory {
public:
    VAOFactory() = delete;

    static VAO make_vao(const std::map<AttributeLabel, int>& binding_map, const VertexBuffer& vb);

    /**
     * @brief Create a vertex array object using stored accessors and binding locations specified in attributes. This
     * function requires memory layout information contained in the VertexBuffer.
     * 
     * @param vb vertex buffer, required
     * @param index_buffer optional index buffer
     * @param attributes map where key is the attribute identifier and value is the binding location in the shader 
     *  vertex_buffer
     * @param instance_buffer optional instance buffer for use in instanced rendering
     * @return VAO 
     */
    static VAO gen_vao_for_attributes(const VertexBuffer* vb, const Buffer* index_buffer, const std::unordered_map<AttributeLabel, int>& attributes, const Buffer* instance_buffer = nullptr);
};

}

#endif // EV2_VAO_H