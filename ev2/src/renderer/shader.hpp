/**
 * @file shader.hpp
 * @brief 
 * @date 2023-07-13
 * 
 */
#ifndef EV2_SHADER_HPP
#define EV2_SHADER_HPP

#include "evpch.hpp"

namespace ev2::renderer {

struct ShaderTypeFlag {
    enum : uint8_t {
        VERTEX_SHADER           = 1 << 0,
        FRAGMENT_SHADER         = 1 << 1,
        GEOMETRY_SHADER         = 1 << 2,
        TESS_EVALUATION_SHADER  = 1 << 3,
        TESS_CONTROL_SHADER     = 1 << 4,
        COMPUTE_SHADER          = 1 << 5
    };
    uint8_t v;
};

enum class ShaderType {
    VERTEX_SHADER           = 0,
    FRAGMENT_SHADER         = 1,
    GEOMETRY_SHADER         = 2,
    TESS_EVALUATION_SHADER  = 3,
    TESS_CONTROL_SHADER     = 4,
    COMPUTE_SHADER          = 5
};

inline std::string ShaderTypeName(ShaderType type) {
    switch (type) {
        case ShaderType::VERTEX_SHADER:
            return "VERTEX_SHADER";
        case ShaderType::FRAGMENT_SHADER:
            return "FRAGMENT_SHADER";
        case ShaderType::GEOMETRY_SHADER:
            return "GEOMETRY_SHADER";
        case ShaderType::TESS_EVALUATION_SHADER:
            return "TESS_EVALUATION_SHADER";
        case ShaderType::TESS_CONTROL_SHADER:
            return "TESS_CONTROL_SHADER";
        case ShaderType::COMPUTE_SHADER:
            return "COMPUTER_SHADER";
        default:
            return "";
    }
}

struct ShaderSource {
    std::string source{};
    ShaderType type{};
};


class Shader {
public:
    explicit Shader(ShaderType type);
    virtual ~Shader() = default;

    static std::unique_ptr<Shader> make_shader(const ShaderSource& source);
};

class Program {
public:

    /**
     * @brief Set the shader attached to a specific stage. The shader should already be built
     * 
     * @param 
     * @param shader 
     */
    virtual void attach_shader(std::shared_ptr<Shader> shader) = 0;

    /**
     * @brief Link shader programs
     */
    virtual void link() = 0;

    /**
     * @brief get linking status
     *
     * @return true
     * @return false
     */
    virtual bool is_linked() const = 0;

    static std::unique_ptr<Program> make_program();
};

} // namespace ev2

#endif 