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

class Material {
public:
    Material() = default;
    explicit Material(std::string name) : name{std::move(name)} {}

    virtual ~Material() = default;

    virtual void set_diffuse(const glm::vec3& d) = 0;
    virtual void set_emissive(const glm::vec3& e) = 0;

    virtual void set_metallic(float m) = 0;
    virtual void set_subsurface(float s) = 0;
    virtual void set_specular(float s) = 0;
    virtual void set_roughness(float r) = 0;
    virtual void set_specularTint(float st) = 0;
    virtual void set_clearcoat(float c) = 0;
    virtual void set_clearcoatGloss(float cg) = 0;
    virtual void set_anisotropic(float a) = 0;
    virtual void set_sheen(float s) = 0;
    virtual void set_sheenTint(float st) = 0;

    virtual void set_ambient_tex(std::shared_ptr<Texture> tex) = 0;             // map_Ka
    virtual void set_diffuse_tex(std::shared_ptr<Texture> tex) = 0;             // map_Kd
    virtual void set_specular_tex(std::shared_ptr<Texture> tex) = 0;            // map_Ks
    virtual void set_specular_highlight_tex(std::shared_ptr<Texture> tex) = 0;  // map_Ns
    virtual void set_bump_tex(std::shared_ptr<Texture> tex) = 0;                // map_bump, map_Bump, bump
    virtual void set_displacement_tex(std::shared_ptr<Texture> tex) = 0;        // disp
    virtual void set_alpha_tex(std::shared_ptr<Texture> tex) = 0;               // map_d
    virtual void set_reflection_tex(std::shared_ptr<Texture> tex) = 0;          // refl

    std::string get_name() const noexcept {return name;}

protected:
    std::string name = "default";
};

} // namespace ev2::renderer

#endif // EV2_RENDERER_MATERIAL_H