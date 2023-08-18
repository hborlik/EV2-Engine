#include "gl_material.hpp"

#include "gl_renderer.hpp"

namespace ev2::renderer {

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