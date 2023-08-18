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

#include "ev_gl.hpp"
#include "renderer/mesh.hpp"

namespace ev2::renderer {

class VAOFactory {
public:
    VAOFactory() = delete;

    static GLuint make_vao(const std::map<AttributeLabel, int>& binding_map, const VertexBuffer& vb);

    /**
     * @brief create a vertex array object using stored accessors and locations specified in map
     * 
     * @param attributes map where key is the attribute identifier and value is the binding location in the shader 
     *  vertex_buffer
     * @return GLuint 
     */
    static GLuint gen_vao_for_attributes(const VertexBuffer& vb, const std::unordered_map<AttributeLabel, uint32_t>& attributes, const Buffer* instance_buffer=nullptr);
};

}

#endif // EV2_VAO_H