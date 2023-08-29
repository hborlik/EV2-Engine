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

#include "evpch.hpp"

#include "core/base.hpp"

#include "core/reference_counted.hpp"

namespace ev2 {

class Object : public ReferenceCountInherit<Object> {
protected:
    Object();
public:
    virtual ~Object();

    const std::string uuid;
    const std::size_t uuid_hash;
};

template<typename T>
class ObjectT : public Object {};

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