#include "renderer/shader.hpp"

namespace ev2 {

std::unique_ptr<Shader> make_shader(const ShaderSource& source) {
    auto shader = std::make_unique<Shader>(source.type);

    return shader;
}

} // namespace ev2
