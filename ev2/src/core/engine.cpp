#include "core/engine.hpp"

namespace ev2 {

std::string Engine::formatted_log_elapsed_time() {
    std::ostringstream oss;
    oss << std::setw(12) << std::setprecision(3) << std::fixed << elapsed_time();
    return oss.str();
}

double Engine::elapsed_time() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = now - m_engine_init;
    return elapsed_seconds.count();
}

}