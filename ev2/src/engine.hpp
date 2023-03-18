/**
 * @file engine.hpp
 * @brief 
 * @date 2023-03-17
 * 
 */
#ifndef EV2_ENGINE_HPP
#define EV2_ENGINE_HPP

#include <fstream>
#include <filesystem>
#include <memory>

#include <ev.hpp>
#include <util.hpp>

namespace ev2 {

namespace fs = std::filesystem;
class Engine {
public:

    static Engine& get() {
        return *(s_instance.get());
    }

    template<typename T>
    static void log_file(const std::string& message);

    static void init(const fs::path& asset_path, const fs::path& log_file_dir, const fs::path& shader_path = "shaders") {
        Engine(asset_path, log_file_dir, shader_path);
    }

private:
    Engine(const fs::path& asset_path, const fs::path& log_file_dir, const fs::path& shader_path):
        asset_path{ asset_path }, shader_path{ shader_path } {
        // assert(s_instance == nullptr);
        s_instance = std::unique_ptr<Engine>(this);
        log_file_stream.open(log_file_dir / ("ev2_log-" + util::formatted_current_time() + ".txt"));
        if (!log_file_stream.is_open())
            throw engine_exception{"log file not open"};
    }

public:
    fs::path asset_path;
    fs::path shader_path;

private:
    std::ofstream log_file_stream;

    static std::unique_ptr<Engine> s_instance;
};

template<typename T>
void Engine::log_file(const std::string& message) {
    std::string mstr = std::string("[") + util::type_name<T>() + "]:" + message + "\n";
    s_instance->log_file_stream << mstr;
}

} // namespace

#endif // EV2_ENGINE_HPP