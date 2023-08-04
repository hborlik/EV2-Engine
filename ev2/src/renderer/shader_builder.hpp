/**
 * @file shader_builder.hpp
 * @brief 
 * @date 2023-07-13
 * 
 */
#ifndef EV2_SHADER_BUILDER_HPP
#define EV2_SHADER_BUILDER_HPP

#include "evpch.hpp"

#include "renderer/shader.hpp"

namespace ev2 {

class PreprocessorSettings {
public:
    explicit PreprocessorSettings(const std::filesystem::path& shader_include_dir = "shaders") : shader_include_dir{shader_include_dir} {}

    std::filesystem::path get_shader_dir() const noexcept {return shader_include_dir;}

private:
    std::filesystem::path shader_include_dir;
};

/**
 * @brief Shader source that contains multiple stages
 * 
 */
class ShaderBuilder {
public:
    /**
     * @brief Read shader source and append it to the source content.
     *
     * @param path path to shader code file
     */
    void push_source_file(const std::filesystem::path &path);

    /**
     * @brief append source string to shader content
     * 
     * @param source_string 
     */
    void push_source_string(const std::string& source_string);


    /**
     * @brief Simple text replacement for loading include files. 
     *  Note: this will ignore all other preprocessor directives
     * 
     * @param settings 
     */
    void preprocess(const PreprocessorSettings& settings);

    ShaderSource make_shader_stage_source(ShaderType type, int version);

    /**
     * @brief Make all available shader stage sources for this shader.
     * 
     * @param version the version for this shader source ex: will put "#version 450" in shader for 450
     * @return std::vector<ShaderSource>
     */
    std::vector<ShaderSource> make_shader_stages_source(int version);


    static ShaderBuilder make_default_shader_builder(const std::filesystem::path& shader_asset_path = {"shaders"});

private:
    std::string m_source{};
    std::size_t last_shader_begin{};
    std::filesystem::path shader_asset_path{};
};


}

#endif