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
#include <iostream>
#include <chrono>

#include <singleton.hpp>
#include <ev.hpp>
#include <util.hpp>

namespace ev2 {

namespace fs = std::filesystem;
class Engine : public Singleton<Engine> {
public:

    Engine(const fs::path& asset_path, const fs::path& log_file_dir, const fs::path& shader_path = "shaders"):
        asset_path{ asset_path }, shader_path{ shader_path }, log_file_stream{}, m_engine_init{std::chrono::steady_clock::now()} {
        // assert(s_instance == nullptr);
        log_file_stream.open(log_file_dir / ("ev2_log-" + util::formatted_current_time() + ".txt"));
        if (!log_file_stream.is_open())
            throw engine_exception{"log file not open"};
    }

    template<typename T>
    static void log_file(std::string_view message);

    template<typename T>
    static void log_t(std::string_view message);

    static void log(std::string_view message);

    std::string formatted_log_elapsed_time();
    double elapsed_time();

public:
    fs::path asset_path;
    fs::path shader_path;

private:
    std::ofstream log_file_stream;
    std::chrono::time_point<std::chrono::steady_clock> m_engine_init;
};

template<typename T>
void Engine::log_file(std::string_view message) {
    log_t<T>(message);
    std::string mstr = std::string("[") + get_singleton().formatted_log_elapsed_time() + " " + util::type_name<T>() + "]:" + message.data() + "\n";
    get_singleton().log_file_stream << mstr;
}

template<typename T>
void Engine::log_t(std::string_view message) {
    std::string mstr = std::string("[") + get_singleton().formatted_log_elapsed_time() + " " + util::type_name<T>() + "]:" + message.data() + "\n";
    std::cout << mstr;
}

} // namespace

#endif // EV2_ENGINE_HPP