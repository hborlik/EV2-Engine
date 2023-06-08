#include "core/log.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace ev2 {

std::shared_ptr<spdlog::logger> Log::s_core_logger;
std::shared_ptr<spdlog::logger> Log::s_client_logger;

void Log::Init() {
    try {
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("meltdown.log", true));

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        s_core_logger = std::make_shared<spdlog::logger>("MELTDOWN ENGINE", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_core_logger);
        s_core_logger->set_level(spdlog::level::trace);
        s_core_logger->flush_on(spdlog::level::trace);

        s_client_logger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_client_logger);
        s_client_logger->set_level(spdlog::level::trace);
        s_client_logger->flush_on(spdlog::level::trace);

    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}

}