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
#include "io/material.hpp"
#include "gl_texture.hpp"

namespace ev2::renderer {

struct GLMaterial : public Material {

    /**
     * @brief Construct a new GLMaterial object
     * 
     * @param name 
     * @param id 
     * @param slot 
     * @param owner required owner pointer
     * @param material_data optional material data pointer used to initialize internal material data
     */
    GLMaterial(std::string name,
        const MaterialData* material_data,
        int32_t id,
        int32_t slot,
        class GLRenderer* owner
        );

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

    void set_ambient_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        ambient_tex = gl_tex;
    }

    void set_diffuse_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        diffuse_tex = gl_tex;
    }

    void set_specular_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        specular_tex = gl_tex;
    }

    void set_specular_highlight_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        specular_highlight_tex = gl_tex;
    }

    void set_bump_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        bump_tex = gl_tex;
    }

    void set_displacement_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        displacement_tex = gl_tex;
    }

    void set_alpha_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        alpha_tex = gl_tex;
    }

    void set_reflection_tex(std::shared_ptr<Texture> tex) override {
        auto gl_tex = std::dynamic_pointer_cast<GLTexture>(tex);
        reflection_tex = gl_tex;
    }



    int32_t get_material_id() const noexcept { return material_id; }
    int32_t get_material_slot() const noexcept { return material_slot; }
    bool is_registered() noexcept { return material_id != -1 && material_slot != -1; }

public:
    std::shared_ptr<GLTexture> ambient_tex;             // map_Ka
    std::shared_ptr<GLTexture> diffuse_tex;             // map_Kd
    std::shared_ptr<GLTexture> specular_tex;            // map_Ks
    std::shared_ptr<GLTexture> specular_highlight_tex;  // map_Ns
    std::shared_ptr<GLTexture> bump_tex;                // map_bump, map_Bump, bump
    std::shared_ptr<GLTexture> displacement_tex;        // disp
    std::shared_ptr<GLTexture> alpha_tex;               // map_d
    std::shared_ptr<GLTexture> reflection_tex;          // refl

private:
    friend class GLRenderer;
    class GLRenderer* m_owner = nullptr;

    int32_t material_id = -1;
    int32_t material_slot = -1;
};

}

#endif // EV2_GL_MATERIAL_HPP