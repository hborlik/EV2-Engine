/**
 * @file shader.h
 * @brief GPU shader
 * @version 0.2
 * @date 2019-09-21
 *
 *
 */
#ifndef EV2_GL_SHADER_HPP
#define EV2_GL_SHADER_HPP

#include "evpch.hpp"

#include "core/ev.hpp"
#include "ev_gl.hpp"
#include "renderer/opengl_renderer/gl_buffer.hpp"
#include "renderer/shader.hpp"
#include "renderer/shader_builder.hpp"
#include "renderer/vertex_buffer_layout.hpp"

namespace ev2::renderer
{

namespace mat_spec
{

/**
 * @brief standard shader inputs
 */
const std::string       VertexAttributeName     = "VertPos";
constexpr gl::DataType  VertexAttributeType     = gl::DataType::VEC3F;
const std::string       TextureAttributeName    = "TexPos";
constexpr gl::DataType  TextureAttributeType    = gl::DataType::VEC2F;
const std::string       NormalAttributeName     = "Normal";
constexpr gl::DataType  NormalAttributeType     = gl::DataType::VEC3F;
const std::string       VertexColorAttributeName= "VertCol";
constexpr gl::DataType  VertexColorAttributeType= gl::DataType::VEC3F;
const std::string       BiTangentAttributeName  = "BiTangent";
constexpr gl::DataType  BiTangentAttributeType  = gl::DataType::VEC3F;
const std::string       TangentAttributeName    = "Tangent";
constexpr gl::DataType  TangentAttributeType    = gl::DataType::VEC3F;

constexpr int32_t VERTEX_BINDING_LOCATION      = 0;
constexpr int32_t NORMAL_BINDING_LOCATION      = 1;
constexpr int32_t COLOR_BINDING_LOCATION       = 2;
constexpr int32_t TEXCOORD_BINDING_LOCATION    = 3;
constexpr int32_t TANGENT_BINDING_LOCATION     = 5;
constexpr int32_t BITANGENT_BINDING_LOCATION   = 6;
constexpr int32_t INSTANCE_BINDING_LOCATION    = 7;

const std::unordered_map<std::string, uint32_t> AttributeBindings {
    std::make_pair("POSITION",  VERTEX_BINDING_LOCATION),
    std::make_pair("NORMAL",    NORMAL_BINDING_LOCATION),
    std::make_pair("COLOR",     COLOR_BINDING_LOCATION),
    std::make_pair("TEXCOORD_0",TEXCOORD_BINDING_LOCATION),
    std::make_pair("TANGENT",   TANGENT_BINDING_LOCATION),
    std::make_pair("BITANGENT", BITANGENT_BINDING_LOCATION)
};

inline uint32_t glBindingLocation(const std::string& attribute_name) {return AttributeBindings.at(attribute_name);}

const std::unordered_map<AttributeLabel, int32_t> DefaultBindings {
    std::make_pair(AttributeLabel::Vertex,    VERTEX_BINDING_LOCATION),
    std::make_pair(AttributeLabel::Normal,    NORMAL_BINDING_LOCATION),
    std::make_pair(AttributeLabel::Color,     COLOR_BINDING_LOCATION),
    std::make_pair(AttributeLabel::Texcoord,  TEXCOORD_BINDING_LOCATION),
    std::make_pair(AttributeLabel::Tangent,   TANGENT_BINDING_LOCATION),
    std::make_pair(AttributeLabel::Bitangent, BITANGENT_BINDING_LOCATION)
};

const std::unordered_map<std::string, std::tuple<uint32_t, gl::DataType, AttributeLabel>> ShaderVertexAttributes {
    std::make_pair(VertexAttributeName,     std::make_tuple(VERTEX_BINDING_LOCATION,    VertexAttributeType,        AttributeLabel::Vertex)),
    std::make_pair(NormalAttributeName,     std::make_tuple(NORMAL_BINDING_LOCATION,    NormalAttributeType,        AttributeLabel::Normal)),
    std::make_pair(VertexColorAttributeName,std::make_tuple(COLOR_BINDING_LOCATION,     VertexColorAttributeType,   AttributeLabel::Color)),
    std::make_pair(TextureAttributeName,    std::make_tuple(TEXCOORD_BINDING_LOCATION,  TextureAttributeType,       AttributeLabel::Texcoord)),
    std::make_pair(TangentAttributeName,    std::make_tuple(TANGENT_BINDING_LOCATION,   TangentAttributeType,       AttributeLabel::Tangent)),
    std::make_pair(BiTangentAttributeName,  std::make_tuple(BITANGENT_BINDING_LOCATION, BiTangentAttributeType,     AttributeLabel::Bitangent))
};

const std::string ModelMatrixUniformName = "M";
const std::string NormalMatrixUniformName = "G";

constexpr uint32_t GUBBindingLocation = 0; // Binding location for global block
const std::string GUBName = "Globals";

// shader specific inputs
const std::string ShaderUniformBlockName = "ShaderData";

} // mat_spec

inline gl::GLSLShaderType glShaderType(ShaderType type) {
    switch (type) {
        case ShaderType::VERTEX_SHADER:
            return gl::GLSLShaderType::VERTEX_SHADER;
        case ShaderType::FRAGMENT_SHADER:
            return gl::GLSLShaderType::FRAGMENT_SHADER;
        case ShaderType::GEOMETRY_SHADER:
            return gl::GLSLShaderType::GEOMETRY_SHADER;
        case ShaderType::TESS_EVALUATION_SHADER:
            return gl::GLSLShaderType::TESS_EVALUATION_SHADER;
        case ShaderType::TESS_CONTROL_SHADER:
            return gl::GLSLShaderType::TESS_CONTROL_SHADER;
        case ShaderType::COMPUTE_SHADER:
            return gl::GLSLShaderType::COMPUTE_SHADER;
    }
}

/**
 * @brief Container for GPU Shader
 */
class GLShader : public Shader
{
public:
    explicit GLShader(gl::GLSLShaderType type);
    virtual ~GLShader();

    GLShader(const GLShader &) = delete;
    GLShader &operator=(const GLShader &) = delete;

    GLShader(GLShader &&o) = delete;

    GLShader& operator=(GLShader &&o) = delete;

    /**
     * @brief Compile or recompile all current source and return true on success
     * 
     * @param source    shader_source
     * @return true     after successful compilation, delete the source string
     * @return false    do not delete the source
     */
    bool compile(const ShaderSource& source);

    /**
     * @brief Get the shader type
     *
     * @return ShaderType
     */
    gl::GLSLShaderType Type() const noexcept { return m_type; }

    bool IsCompiled() const noexcept { 
        GLint compiled;
        glGetShaderiv(gl_reference, GL_COMPILE_STATUS, &compiled);

        return compiled == GL_TRUE;
    }

    /**
     * @brief return opengl reference, for this shader program
     *
     * @return GLuint
     */
    GLuint getHandle() const noexcept { return gl_reference; }

private:
    friend void swap(GLShader& left, GLShader& right) noexcept {
        using std::swap;
        
        swap(left.gl_reference, right.gl_reference);
        swap(left.m_type, right.m_type);
    }

private:
    GLuint gl_reference;
    gl::GLSLShaderType m_type;
};

class Buffer;

struct ProgramUniformDescription
{
    GLint Location = -1;
    GLenum Type = 0;
    GLint Length = 0;
};

struct ProgramInputDescription
{
    GLint Location = -1;
    GLenum Type = 0;
};

struct BlockLayoutUtil
{
    /**
     * @brief bind a given buffer as a uniform buffer
     * @deprecated use buffer.bind_range
     * 
     * @param buffer 
     */
    static void bind_buffer(const ProgramBlockLayout& layout, const GLBuffer &buffer) {
        GL_CHECKED_CALL(
            glBindBufferRange(GL_UNIFORM_BUFFER, layout.location_index, buffer.handle(), 0, buffer.get_capacity())
        );
    }
};

// GPU program specific data
class GLProgram : public Program
{
public:
    GLProgram();
    explicit GLProgram(std::string name);
    virtual ~GLProgram();

    GLProgram(GLProgram &&) = delete;
    GLProgram(const GLProgram &) = delete;

    GLProgram &operator=(const GLProgram &) = delete;

    /**
     * @brief Set the shader attached to a specific stage. The shader should already be built
     * 
     * @param 
     * @param shader 
     */
    void attach_shader(std::shared_ptr<Shader> shader) override;

    /**
     * @brief  Set path for shader stage in this program
     * 
     * @param type 
     * @param path 
     * @param preprocessor 
     * @param version 
     */
    void loadShader(ShaderType type, const std::filesystem::path &path, const PreprocessorSettings& settings, int version = 450);

    /**
     * @brief Link shader programs
     */
    void link() override;

    /**
     * @brief Set this program to active
     */
    void use() const;

    void unbind()
    {
        GL_CHECKED_CALL(glUseProgram(0));
    }

    /**
     * @brief get linking status from OpenGL
     *
     * @return true
     * @return false
     */
    bool is_linked() const override;

    /**
     * @brief Get the Modified Count
     *
     * @return uint32_t
     */
    uint32_t getModifiedCount() const noexcept { return modifiedCount; }

    /**
     * @brief Get the Uniform description for given name
     *
     * @param uniformName
     * @return ProgramUniformDescription zero initialized if it does not exist
     */
    ProgramUniformDescription getUniformInfo(const std::string& uniformName) const {
        auto itr = uniforms.find(uniformName);
        if (itr != uniforms.end())
            return itr->second;
        return { -1, 0 };
    }

    /**
     * @brief Get the Input description for given name
     *
     * @param inputName
     * @return ProgramInputDescription default initialized if it does not exist
     */
    ProgramInputDescription getInputInfo(const std::string& inputName) const {
        auto itr = inputs.find(inputName);
        if (itr != inputs.end())
            return itr->second;
        return {};
    }

    /**
     * @brief Get the Attribute Map. Mapping AttributeLabel to binding location in the shader.
     * 
     * @return std::unordered_map<AttributeLabel, int> mapping attribute identifier to binding location.
     */
    std::unordered_map<AttributeLabel, int> getAttributeMap() const {
        std::unordered_map<AttributeLabel, int> map;
        // convert map of name->ProgramInputDescription to AttributeLabel->binding location map
        for (auto& mapping : inputs) {
            // if the input has a name in ShaderVertexAttributes
            auto default_binding = mat_spec::ShaderVertexAttributes.find(mapping.first);
            if (default_binding != mat_spec::ShaderVertexAttributes.end()) {
                auto label = std::get<2>(default_binding->second);
                map.insert(std::make_pair(label, mapping.second.Location));
            }
        }
        return map;
    }

    /**
     * @brief Get the Uniform Block Description for shader object
     *
     * @param uboName
     * @return ProgramUniformBlockDescription zero initialized if it does not exist
     */
    ProgramBlockLayout get_uniform_block_layout(const std::string& uboName) const override {
        auto itr = uniformBlocks.find(uboName);
        if (itr != uniformBlocks.end())
            return itr->second;
        return {};
    }

    GLuint getHandle() const noexcept { return gl_reference; }

    /**
     * @brief point uniform block with name to binding location
     *
     * @return true
     * @return false
     */
    bool setUniformBlockBinding(const std::string &uniformName, uint32_t location);

    /**
     * @brief Query the Program Resource Index for a resource of type 
     * 
     * @param resource resource type
     * @param name resource name
     * @return GLuint 
     */
    GLuint getProgramResourceLocation(GLenum resource, const std::string& name);

protected:


    // get info on program inputs
    void updateProgramInputInfo();

    // get info about program uniforms
    void updateProgramUniformInfo();

    void updateProgramUniformBlockInfo();

    /**
     * @brief Allows base classes to configure additional shader options after program builds
     *
     */
    virtual void onBuilt(){};

private:
    bool gl_attach_shader(GLuint shader_name);

public:
    /**
     * @brief Program name
     */
    std::string ProgramName;

protected:
    std::unordered_map<std::string, ProgramUniformDescription> uniforms;
    std::unordered_map<std::string, ProgramInputDescription> inputs;
    std::unordered_map<std::string, ProgramBlockLayout> uniformBlocks;

    // counter for dependent objects. incremented when shader has been reloaded.
    uint32_t modifiedCount = 0;
    GLuint gl_reference;

private:
    // helper function to print program information
    friend std::ostream &operator<<(std::ostream &os, const GLProgram &input);
};

} // namespace ev2

#endif // EV2_GL_SHADER_HPP