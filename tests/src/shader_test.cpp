#include <iostream>
#include <filesystem>
#include <renderer/shader.hpp>
#include <ev.h>

using namespace ev2;
namespace fs = std::filesystem;

int main() {
    // ev2::EV2_init(ev2::Args{}, fs::path{"asset"}, fs::path{});

    ev2::renderer::ShaderPreprocessor prep{fs::path{"asset"} / "shader"};


    ev2::renderer::Program p{"test"};
    p.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "forward_lighting.glsl.vert", prep);
    p.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "forward_lighting.glsl.frag", prep);
    p.link();

    std::cout << p << std::endl;

    // ev2::EV2_shutdown();
}