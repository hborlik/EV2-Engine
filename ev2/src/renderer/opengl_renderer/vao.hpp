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
#include "mesh.hpp"

namespace ev2::renderer {

class VAOFactory {
public:
    VAOFactory() = delete;

    static GLuint make_vao(const std::map<VertexAttributeLabel, int>& binding_map, const VertexBuffer& vb);
};

}

#endif // EV2_VAO_H