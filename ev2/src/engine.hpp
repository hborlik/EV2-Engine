/**
 * @file engine.hpp
 * @brief 
 * @date 2023-03-17
 * 
 */
#ifndef EV2_ENGINE_HPP
#define EV2_ENGINE_HPP

#include "evpch.hpp"

#include "core/singleton.hpp"
#include "ev.hpp"
#include "util.hpp"

namespace ev2 {

namespace fs = std::filesystem;

class Engine : public Singleton<Engine> {
public:

    Engine(const fs::path& asset_path, const fs::path& log_file_dir):
        asset_path{ asset_path }, m_engine_init{std::chrono::steady_clock::now()} {}

    std::string formatted_log_elapsed_time();
    double elapsed_time();

public:
    fs::path asset_path;

private:
    
    std::chrono::time_point<std::chrono::steady_clock> m_engine_init;
};

} // namespace

#endif // EV2_ENGINE_HPP