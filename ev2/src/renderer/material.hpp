/**
 * @file material.h
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
#include "ev.hpp"

namespace ev2::renderer {

struct Material : public Object {
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
    Material(std::string name) : name{std::move(name)} {}

    virtual ~Material();

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    
    Material(Material&&) = delete;
    Material& operator=(Material&&) = delete;

    int32_t get_material_id() const noexcept {return material_id;}
    bool is_registered() noexcept {return material_id != -1 && material_slot != -1;}

private:
    friend class Renderer;

    int32_t material_id = -1;
    int32_t material_slot = -1;
};

} // namespace ev2::renderer

#endif // EV2_RENDERER_MATERIAL_H