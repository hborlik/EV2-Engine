/**
 * @file gl_material.hpp
 * @brief
 * @date 2023-08-10
 *
 */
#ifndef EV2_GL_MATERIAL_HPP
#define EV2_GL_MATERIAL_HPP

#include "core/assert.hpp"

#include "renderer/material.hpp"

namespace ev2::renderer {

struct GLMaterial : public Material {

    GLMaterial() = default;
    GLMaterial(std::string name, class GLRenderer* owner) : Material{ std::move(name) }, m_owner{owner} {
        EV_CORE_ASSERT(owner, "GLRenderer* owner cannot be null for material");
    }

    GLMaterial(const GLMaterial&) = delete;
    GLMaterial& operator=(const GLMaterial&) = delete;

    GLMaterial(GLMaterial&&) = delete;
    GLMaterial& operator=(GLMaterial&&) = delete;

    void set_diffuse(const glm::vec3& d) override;
    void set_emissive(const glm::vec3& e) override;
    void set_metallic(float m) override;
    void set_subsurface(float s) override;
    void set_specular(float s) override;
    void set_roughness(float r) override;
    void set_specularTint(float st) override;
    void set_clearcoat(float c) override;
    void set_clearcoatGloss(float cg) override;
    void set_anisotropic(float a) override;
    void set_sheen(float s) override;
    void set_sheenTint(float st) override;

    void set_ambient_tex(std::shared_ptr<Texture> tex) override { ambient_tex = tex; }
    void set_diffuse_tex(std::shared_ptr<Texture> tex) override { diffuse_tex = tex; }
    void set_specular_tex(std::shared_ptr<Texture> tex) override { specular_tex = tex; }
    void set_specular_highlight_tex(std::shared_ptr<Texture> tex) override { specular_highlight_tex = tex; }
    void set_bump_tex(std::shared_ptr<Texture> tex) override { bump_tex = tex; }
    void set_displacement_tex(std::shared_ptr<Texture> tex) override { displacement_tex = tex; }
    void set_alpha_tex(std::shared_ptr<Texture> tex) override { alpha_tex = tex; }
    void set_reflection_tex(std::shared_ptr<Texture> tex) override { reflection_tex = tex; }


    int32_t get_material_id() const noexcept { return material_id; }
    int32_t get_material_slot() const noexcept { return material_slot; }
    bool is_registered() noexcept { return material_id != -1 && material_slot != -1; }

public:
    std::shared_ptr<Texture> ambient_tex;             // map_Ka
    std::shared_ptr<Texture> diffuse_tex;             // map_Kd
    std::shared_ptr<Texture> specular_tex;            // map_Ks
    std::shared_ptr<Texture> specular_highlight_tex;  // map_Ns
    std::shared_ptr<Texture> bump_tex;                // map_bump, map_Bump, bump
    std::shared_ptr<Texture> displacement_tex;        // disp
    std::shared_ptr<Texture> alpha_tex;               // map_d
    std::shared_ptr<Texture> reflection_tex;          // refl

private:
    friend class GLRenderer;
    class GLRenderer* m_owner = nullptr;

    int32_t material_id = -1;
    int32_t material_slot = -1;

    struct MaterialData* m_material_data = nullptr;
};

}

#endif // EV2_GL_MATERIAL_HPP