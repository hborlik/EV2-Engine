#ifndef EV_LOG_HPP
#define EV_LOG_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "util.hpp"

namespace ev2 {

class Log {
public:
    static void Init();

    static std::shared_ptr<spdlog::logger> get_core_logger() {return s_core_logger;}
    static std::shared_ptr<spdlog::logger> get_client_logger() {return s_client_logger;}

    template<typename... Args>
    static void log_core(spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
        get_core_logger()->log(lvl, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename T>
    static void log_core(spdlog::level::level_enum lvl, const T& msg)
    {
        get_core_logger()->log(lvl, std::move(msg));
    }

    template<typename... Args>
    static void log(spdlog::level_t lvl, spdlog::format_string_t<Args...> fmt, Args &&... args)
    {
        get_client_logger()->log(lvl, std::move(fmt), std::forward<Args>(args)...);
    }

    // core logging functions

    template<typename CLS, typename... Args>
    static void trace_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::trace, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void trace_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::trace, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename CLS, typename _T>
    static void trace_core(const _T& msg) {
        log_core(spdlog::level::trace, std::move(msg));
    }

    template<typename _T>
    static void trace_core(const _T& msg) {
        log_core(spdlog::level::trace, std::move(msg));
    }


    template<typename CLS, typename... Args>
    static void info_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::info, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::info, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename CLS, typename _T>
    static void info_core(const _T& msg) {
        log_core(spdlog::level::info, std::move(msg));
    }

    template<typename _T>
    static void info_core(const _T& msg) {
        log_core(spdlog::level::info, std::move(msg));
    }


    template<typename CLS, typename... Args>
    static void warn_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::warn, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::warn, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename CLS, typename _T>
    static void warn_core(const _T& msg) {
        log_core(spdlog::level::warn, std::move(msg));
    }

    template<typename _T>
    static void warn_core(const _T& msg) {
        log_core(spdlog::level::warn, std::move(msg));
    }


    template<typename CLS, typename... Args>
    static void error_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::err, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::err, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename CLS, typename _T>
    static void error_core(const _T& msg) {
        log_core(spdlog::level::err, std::move(msg));
    }

    template<typename _T>
    static void error_core(const _T& msg) {
        log_core(spdlog::level::err, std::move(msg));
    }


    template<typename CLS, typename... Args>
    static void critical_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::critical, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical_core(spdlog::format_string_t<Args...> fmt, Args &&... args) {
        log_core(spdlog::level::critical, std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename CLS, typename _T>
    static void critical_core(const _T& msg) {
        log_core(spdlog::level::critical, std::move(msg));
    }

    template<typename _T>
    static void critical_core(const _T& msg) {
        log_core(spdlog::level::critical, std::move(msg));
    }

private:
    static std::shared_ptr<spdlog::logger> s_core_logger;
    static std::shared_ptr<spdlog::logger> s_client_logger;
};

}

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector) {
    return os << glm::to_string(vector);
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T,
          glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix) {
    return os << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion) {
    return os << glm::to_string(quaternion);
}

// Core log macros
#define EV_CORE_TRACE(...)    ::ev2::Log::get_core_logger()->trace(__VA_ARGS__)
#define EV_CORE_INFO(...)     ::ev2::Log::get_core_logger()->info(__VA_ARGS__)
#define EV_CORE_WARN(...)     ::ev2::Log::get_core_logger()->warn(__VA_ARGS__)
#define EV_CORE_ERROR(...)    ::ev2::Log::get_core_logger()->error(__VA_ARGS__)
#define EV_CORE_CRITICAL(...) ::ev2::Log::get_core_logger()->critical(__VA_ARGS__)

// Client log macros
#define EV_TRACE(...)         ::ev2::Log::get_client_logger()->trace(__VA_ARGS__)
#define EV_INFO(...)          ::ev2::Log::get_client_logger()->info(__VA_ARGS__)
#define EV_WARN(...)          ::ev2::Log::get_client_logger()->warn(__VA_ARGS__)
#define EV_ERROR(...)         ::ev2::Log::get_client_logger()->error(__VA_ARGS__)
#define EV_CRITICAL(...)      ::ev2::Log::get_client_logger()->critical(__VA_ARGS__)

#endif // EV_LOG_HPP