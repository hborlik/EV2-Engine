#include "gl_material.hpp"

#include "gl_renderer.hpp"

namespace ev2::renderer {

GLMaterial::GLMaterial(std::string name,
    const MaterialData* material_data,
    int32_t id,
    int32_t slot,
    class GLRenderer* owner
    ) :
    Material{ std::move(name) },
    m_owner{ owner },
    material_id{id},
    material_slot{slot} {
    EV_CORE_ASSERT(owner, "GLRenderer* owner cannot be null for material");

    if (material_data) {
        set_diffuse(material_data->diffuse);
        set_emissive(material_data->emissive);
        set_metallic(material_data->metallic);
        set_subsurface(material_data->subsurface);
        set_specular(material_data->specular);
        set_roughness(material_data->roughness);
        set_specularTint(material_data->specularTint);
        set_clearcoat(material_data->clearcoat);
        set_clearcoatGloss(material_data->clearcoatGloss);
        set_anisotropic(material_data->anisotropic);
        set_sheen(material_data->sheen);
        set_sheenTint(material_data->sheenTint);

        if (material_data->ambient_map) 
            set_ambient_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->ambient_map.get()));
        if (material_data->diffuse_map) 
            set_diffuse_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->diffuse_map.get()));
        if (material_data->specular_map) 
            set_specular_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->specular_map.get()));
        if (material_data->specular_highlight_map) 
            set_specular_highlight_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->specular_highlight_map.get()));
        if (material_data->bump_map) 
            set_bump_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->bump_map.get()));
        if (material_data->displacement_map) 
            set_displacement_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->displacement_map.get()));
        if (material_data->alpha_map) 
            set_alpha_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->alpha_map.get()));
        if (material_data->reflection_map) 
            set_reflection_tex(m_owner->make_texture(TextureType::Tex_2D, material_data->reflection_map.get()));
    }
}

void GLMaterial::set_diffuse(const glm::vec3& d) {
    m_owner->material_data_buffer[material_slot].diffuse = d;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_emissive(const glm::vec3& e) {
    m_owner->material_data_buffer[material_slot].emissive = e;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_metallic(float m) {
    m_owner->material_data_buffer[material_slot].metallic = m;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_subsurface(float s) {
    m_owner->material_data_buffer[material_slot].subsurface = s;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_specular(float s) {
    m_owner->material_data_buffer[material_slot].specular = s;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_roughness(float r) {
    m_owner->material_data_buffer[material_slot].roughness = r;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_specularTint(float st) {
    m_owner->material_data_buffer[material_slot].specularTint = st;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_clearcoat(float c) {
    m_owner->material_data_buffer[material_slot].clearcoat = c;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_clearcoatGloss(float cg) {
    m_owner->material_data_buffer[material_slot].clearcoatGloss = cg;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_anisotropic(float a) {
    m_owner->material_data_buffer[material_slot].anisotropic = a;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_sheen(float s) {
    m_owner->material_data_buffer[material_slot].sheen = s;
    m_owner->material_data_buffer[material_slot].changed = true;
}

void GLMaterial::set_sheenTint(float st) {
    m_owner->material_data_buffer[material_slot].sheenTint = st;
    m_owner->material_data_buffer[material_slot].changed = true;
}


}