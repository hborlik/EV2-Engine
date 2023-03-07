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
#include <ctime>
#include <sstream>

#include <window.hpp>
#include <renderer/renderer.hpp>
#include <resource.hpp>
#include <physics.hpp>

static ev2::Engine engine;

namespace ev2 {

std::string formatted_current_time() {
    // from https://stackoverflow.com/questions/16357999/current-date-and-time-as-string
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    auto str = oss.str();
    return str;
}

Object::Object() : 
    uuid{util::get_unique_id()} {

}

const Engine& Engine::get() {
    return engine;
}

Engine& Engine::get_internal() {
    return engine;
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
    engine.asset_path = asset_path;
    engine.shader_path = "shaders";
    engine.log_file_stream.open(log_file_dir / ("ev2_log-" + formatted_current_time() + ".txt"));
    window::init(args);
    glm::ivec2 screen_size = window::getWindowSize();
    renderer::Renderer::initialize(screen_size.x, screen_size.y);
    ResourceManager::initialize(asset_path);
    renderer::Renderer::get_singleton().init();
    Physics::initialize();
    engine.log_file<Engine>("Initialized");
}

void EV2_shutdown() {
    Physics::shutdown();
    ResourceManager::shutdown();
    renderer::Renderer::shutdown();
    engine.log_file<Engine>("Shutdown");
}

Args::Args(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {

    }
}

}