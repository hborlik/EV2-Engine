/**
 * @file ssre.cpp
 * @author Hunter Borlik 
 * @brief definitions for EV2 library header
 * @version 0.1
 * @date 2019-09-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <ev.hpp>

#include <iostream>
#include <string>
#include <sstream>

#include <window.hpp>
#include <renderer/renderer.hpp>
#include <resource.hpp>
#include <physics.hpp>
#include <engine.hpp>

#include <ui/imgui.hpp>
#include <ui/imgui_impl_glfw.hpp>
#include <ui/imgui_impl_opengl3.hpp>

namespace ev2 {

Object::Object() : 
    uuid{util::get_unique_id()} {

}

engine_exception::engine_exception(std::string description) noexcept : description{std::move(description)} {

}

const char* engine_exception::what() const noexcept {
    return description.data();
}

shader_error::shader_error(std::string shaderName, std::string errorString) noexcept : 
    engine_exception{"Shader " + shaderName + " caused an error: " + errorString} {

}

void EV2_init(const Args& args, const std::filesystem::path& asset_path, const std::filesystem::path& log_file_dir) {
    Engine::init(asset_path, log_file_dir);

    #if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
#endif

    window::init(args);
    
    GLFWwindow* window = ev2::window::getContextWindow();
    if (window == nullptr)
        throw std::runtime_error{"window is null"};
    glfwMakeContextCurrent(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    glm::ivec2 screen_size = window::getWindowSize();
    renderer::Renderer::initialize(screen_size.x, screen_size.y);
    ResourceManager::initialize(asset_path);
    renderer::Renderer::get_singleton().init();
    Physics::initialize();

    Engine::get().log_file<Engine>("Initialized");
}

void EV2_shutdown() {
    Physics::shutdown();
    ResourceManager::shutdown();
    renderer::Renderer::shutdown();
    Engine::get().log_file<Engine>("Shutdown");
}

Args::Args(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {

    }
}

}