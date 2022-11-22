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

#include <ev.h>

#include <iostream>
#include <string>

#include <window.h>
#include <renderer/renderer.h>
#include <resource.h>
#include <physics.h>

static ev2::EngineConfig config;

namespace ev2 {

const EngineConfig& EngineConfig::get_config() {
    return config;
}

engine_exception::engine_exception(std::string description) noexcept : description{std::move(description)} {

}

const char* engine_exception::what() const noexcept {
    return description.data();
}

shader_error::shader_error(std::string shaderName, std::string errorString) noexcept : 
    engine_exception{"Shader " + shaderName + " caused an error: " + errorString} {

}

void EV2_init(const Args& args, const std::filesystem::path& asset_path) {
    config.asset_path = asset_path;
    config.shader_path = "shaders";
    window::init(args);
    glm::ivec2 screen_size = window::getWindowSize();
    renderer::Renderer::initialize(screen_size.x, screen_size.y);
    ResourceManager::initialize(asset_path);
    renderer::Renderer::get_singleton().init();
    Physics::initialize();
}

void EV2_shutdown() {
    Physics::shutdown();
    ResourceManager::shutdown();
    renderer::Renderer::shutdown();
}

Args::Args(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {

    }
}

}