/**
 * @file ev.h
 * @author Hunter Borlik
 * @brief standard EV definitions. 
 * @version 0.1
 * @date 2019-09-04
 * 
 * 
 */
#ifndef EV2_HPP
#define EV2_HPP

#include <string>
#include <exception>
#include <map>
#include <filesystem>

#include <reference_counted.hpp>

#define EV2_CHECK_THROW(expr, message) if(!(expr)) throw ev2::engine_exception{"[" + std::string{__FILE__} + ":" + std::to_string(__LINE__) + "]:" + message}

namespace ev2 {

class Object : public ReferenceCounted<Object> {
protected:
    Object();
public:
    virtual ~Object();

    const std::string uuid;
    const std::size_t uuid_hash;
};

template<typename T>
class ObjectT : public Object {};

class engine_exception : public std::exception {
public:
    engine_exception(std::string description) noexcept;
    virtual ~engine_exception() = default;

    const char* what() const noexcept override;

protected:
    std::string description;
};

class shader_error : public engine_exception {
public:
    shader_error(std::string shaderName, std::string errorString) noexcept;
    virtual ~shader_error() = default;
};

struct Args {
    Args() = default;
    Args(int argc, char* argv[]);

    int height = 300, width = 600;

    std::map<std::string, std::string> args;
};

void EV2_init(const Args& args, const std::filesystem::path& asset_path, const std::filesystem::path& log_file);

void EV2_shutdown();

} // ev2

#endif // EV2_ENGINE_H