/**
 * @file gl_material.hpp
 * @brief 
 * @date 2023-08-10
 * 
 */
#ifndef EV2_GL_MATERIAL_HPP
#define EV2_GL_MATERIAL_HPP

#include "renderer/material.hpp"

namespace ev2::renderer {

struct GLMaterial : public Material {

    GLMaterial() = default;
    GLMaterial(std::string name) : Material{std::move(name)} {}

    GLMaterial(const GLMaterial&) = delete;
    GLMaterial& operator=(const GLMaterial&) = delete;
    
    GLMaterial(GLMaterial&&) = delete;
    GLMaterial& operator=(GLMaterial&&) = delete;


    int32_t get_material_id() const noexcept {return material_id;}
    int32_t get_material_slot() const noexcept {return material_slot;}
    bool is_registered() noexcept {return material_id != -1 && material_slot != -1;}

private:
    friend class GLRenderer;

    int32_t material_id = -1;
    int32_t material_slot = -1;
};

}

#endif // EV2_GL_MATERIAL_HPP