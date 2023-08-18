/**
 * @file material.hpp
 * @brief 
 * @date 2023-08-12
 * 
 */
#ifndef EV2_IO_MATERIAL_HPP
#define EV2_IO_MATERIAL_HPP

#include "evpch.hpp"
#include "image.hpp"

#include "glm/glm.hpp"

namespace ev2 {

struct MaterialData {
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

    std::shared_ptr<Image> ambient_map;             // map_Ka
    std::shared_ptr<Image> diffuse_map;             // map_Kd
    std::shared_ptr<Image> specular_map;            // map_Ks
    std::shared_ptr<Image> specular_highlight_map;  // map_Ns
    std::shared_ptr<Image> bump_map;                // map_bump, map_Bump, bump
    std::shared_ptr<Image> displacement_map;        // disp
    std::shared_ptr<Image> alpha_map;               // map_d
    std::shared_ptr<Image> reflection_map;          // refl
};

}

#endif // EV2_IO_MATERIAL_HPP
