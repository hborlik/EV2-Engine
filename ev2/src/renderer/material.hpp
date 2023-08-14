/**
 * @file material.hpp
 * @author Hunter Borlik
 * @brief 
 * @date 2022-10-27
 * 
 * 
 */
#ifndef EV2_RENDERER_MATERIAL_H
#define EV2_RENDERER_MATERIAL_H

#include "evpch.hpp"

#include "renderer/texture.hpp"
#include "core/ev.hpp"

namespace ev2::renderer {

struct Material {
    std::string name = "default";

    glm::vec3 diffuse   = {1.00f,0.10f,0.85f};
    glm::vec3 emissive  = {};
    float metallic       = 0;
    float subsurface     = 0;
    float specular       = .5f;
    float roughness      = .5f;
    float specularTint   = 0;
    float clearcoat      = 0;
    float clearcoatGloss = 1.f;
    float anisotropic    = 0;
    float sheen          = 0;
    float sheenTint      = .5f;

    std::shared_ptr<Texture> ambient_tex;             // map_Ka
    std::shared_ptr<Texture> diffuse_tex;             // map_Kd
    std::shared_ptr<Texture> specular_tex;            // map_Ks
    std::shared_ptr<Texture> specular_highlight_tex;  // map_Ns
    std::shared_ptr<Texture> bump_tex;                // map_bump, map_Bump, bump
    std::shared_ptr<Texture> displacement_tex;        // disp
    std::shared_ptr<Texture> alpha_tex;               // map_d
    std::shared_ptr<Texture> reflection_tex;          // refl

    Material() = default;
    explicit Material(std::string name) : name{std::move(name)} {}

    virtual ~Material() = default;

};

} // namespace ev2::renderer

#endif // EV2_RENDERER_MATERIAL_H