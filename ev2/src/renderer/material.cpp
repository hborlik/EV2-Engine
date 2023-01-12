#include <renderer/material.h>
#include <renderer/renderer.h>

namespace ev2::renderer {

Material::~Material() {
    renderer::Renderer::get_singleton().destroy_material(this);
}

}