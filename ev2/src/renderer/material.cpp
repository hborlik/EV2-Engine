#include <renderer/material.hpp>
#include <renderer/renderer.hpp>

namespace ev2::renderer {

Material::~Material() {
    renderer::Renderer::get_singleton().destroy_material(this);
}

}