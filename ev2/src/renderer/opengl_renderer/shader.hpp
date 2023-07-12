/**
 * @file shader.h
 * @brief GPU shader
 * @version 0.2
 * @date 2019-09-21
 *
 *
 */
#ifndef EV2_SHADER_H
#define EV2_SHADER_H

#include "evpch.hpp"

#include "core/ev.hpp"
#include "ev_gl.hpp"
#include "buffer.hpp"

namespace ev2::renderer
{

enum class VertexAttributeLabel : int {
    Vertex = 0,
    Normal,
    Color,
    Texcoord,
    Tangent,
    Bitangent
};

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

constexpr uint32_t VERTEX_BINDING_LOCATION      = 0;
constexpr uint32_t NORMAL_BINDING_LOCATION      = 1;
constexpr uint32_t COLOR_BINDING_LOCATION       = 2;
constexpr uint32_t TEXCOORD_BINDING_LOCATION    = 3;
constexpr uint32_t TANGENT_BINDING_LOCATION     = 5;
constexpr uint32_t BITANGENT_BINDING_LOCATION   = 6;
constexpr uint32_t INSTANCE_BINDING_LOCATION    = 7;

const std::unordered_map<std::string, uint32_t> AttributeBindings {
    std::make_pair("POSITION",  VERTEX_BINDING_LOCATION),
    std::make_pair("NORMAL",    NORMAL_BINDING_LOCATION),
    std::make_pair("COLOR",     COLOR_BINDING_LOCATION),
    std::make_pair("TEXCOORD_0",TEXCOORD_BINDING_LOCATION),
    std::make_pair("TANGENT",   TANGENT_BINDING_LOCATION),
    std::make_pair("BITANGENT", BITANGENT_BINDING_LOCATION)
};

inline uint32_t glBindingLocation(const std::string& attribute_name) {return AttributeBindings.at(attribute_name);}

const std::unordered_map<VertexAttributeLabel, uint32_t> DefaultBindings {
    std::make_pair(VertexAttributeLabel::Vertex,    VERTEX_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Normal,    NORMAL_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Color,     COLOR_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Texcoord,  TEXCOORD_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Tangent,   TANGENT_BINDING_LOCATION),
    std::make_pair(VertexAttributeLabel::Bitangent, BITANGENT_BINDING_LOCATION)
};

const std::unordered_map<std::string, std::tuple<uint32_t, gl::DataType, VertexAttributeLabel>> ShaderVertexAttributes {
    std::make_pair(VertexAttributeName,     std::make_tuple(VERTEX_BINDING_LOCATION,    VertexAttributeType,        VertexAttributeLabel::Vertex)),
    std::make_pair(NormalAttributeName,     std::make_tuple(NORMAL_BINDING_LOCATION,    NormalAttributeType,        VertexAttributeLabel::Normal)),
    std::make_pair(VertexColorAttributeName,std::make_tuple(COLOR_BINDING_LOCATION,     VertexColorAttributeType,   VertexAttributeLabel::Color)),
    std::make_pair(TextureAttributeName,    std::make_tuple(TEXCOORD_BINDING_LOCATION,  TextureAttributeType,       VertexAttributeLabel::Texcoord)),
    std::make_pair(TangentAttributeName,    std::make_tuple(TANGENT_BINDING_LOCATION,   TangentAttributeType,       VertexAttributeLabel::Tangent)),
    std::make_pair(BiTangentAttributeName,  std::make_tuple(BITANGENT_BINDING_LOCATION, BiTangentAttributeType,     VertexAttributeLabel::Bitangent))
};

const std::string ModelMatrixUniformName = "M";
const std::string NormalMatrixUniformName = "G";

constexpr uint32_t GUBBindingLocation = 0; // Binding location for global block
const std::string GUBName = "Globals";

// shader specific inputs
const std::string ShaderUniformBlockName = "ShaderData";

} // mat_spec

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
    }
}

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

class ShaderPreprocessor {
public:
    explicit ShaderPreprocessor(const std::filesystem::path& shader_include_dir = "shaders") : shader_include_dir{shader_include_dir} {}

    /**
     * @brief Simple text replacement for loading include files. Note this will ignore all other preprocessor directives
     * 
     * @param input_source 
     * @return std::string 
     */
    std::string preprocess(const std::string& input_source) const;

    std::filesystem::path get_shader_dir() const noexcept {return shader_include_dir;}

private:
    std::filesystem::path shader_include_dir;
};

/**
 * @brief Container for GPU Shader
 */
class Shader
{
public:
    explicit Shader(gl::GLSLShaderType type);
    ~Shader();

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&o) {
        *this = std::move(o);
    }

    Shader& operator=(Shader &&o) {
        std::swap(gl_reference, o.gl_reference);
        std::swap(type, o.type);
        return *this;
    }

    /**
     * @brief Compile or recompile all current source and return true on success
     * 
     * @param source    shader_source
     * @return true     after successful compilation, delete the source string
     * @return false    do not delete the source
     */
    bool compile(std::string_view source);

    /**
     * @brief Get the shader type
     *
     * @return ShaderType
     */
    gl::GLSLShaderType Type() const noexcept { return type; }

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
    std::string source;
    GLuint gl_reference;
    gl::GLSLShaderType type;
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

    void preprocess(const ShaderPreprocessor& pre);

    std::unique_ptr<Shader> make_shader_stage(ShaderType type, int version);

    /**
     * @brief Get the shader stages for this shader. Does not compile
     * 
     * @param version the version for this shader source ex: will put "#version 450" in shader for 450
     * @return std::vector<std::unique_ptr<Shader>> 
     */
    std::vector<std::unique_ptr<Shader>> get_shader_stages(int version);

    static ShaderBuilder make_default_shader_builder(const std::filesystem::path& shader_asset_path = {"shaders"});

private:
    std::string source{};
    std::filesystem::path shader_asset_path{};
    std::size_t last_shader_begin;
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

struct ProgramUniformBlockDescription
{
    GLint location_index = -1;  // Index in Program interface
    GLint block_size = -1; // total size in bytes of buffer

    struct Layout
    {
        GLint Offset = 0;      // offset in bytes from beginning of buffer
        GLint ArraySize = 0;   // number of array elements
        GLint ArrayStride = 0; // stride in bytes between array elements
    };
    std::unordered_map<std::string, int> layout_map;
    std::vector<Layout> layouts;

    Layout get_layout(const std::string &name)
    {
        auto itr = layout_map.find(name);
        if (itr != layout_map.end())
            return layouts[itr->second];
        return {-1, -1, -1};
    }

    Layout get_layout(int ind) {
        return layouts.at(ind);
    }

    GLint get_offset(const std::string &name)
    {
        auto itr = layout_map.find(name);
        if (itr != layout_map.end())
            return layouts[itr->second].Offset;
        throw std::out_of_range{name};
    }

    int get_index(const std::string &name) {
        auto itr = layout_map.find(name);
        if (itr != layout_map.end()) {
            return itr->second;
        }
        throw std::out_of_range{name};
    }

    GLint get_offset(int ind) {
        return layouts.at(ind).Offset;
    }

    bool is_valid() const noexcept {return location_index != -1;}

    /**
     * @brief Set a Shader parameter in the target uniform block buffer.
     * 
     * @tparam T 
     * @param paramName 
     * @param data 
     * @param shaderBuffer 
     * @return true 
     * @return false 
     */
    template <typename T>
    bool set_parameter(const std::string &paramName, const T &data, Buffer &shaderBuffer)
    {
        if (is_valid())
        {
            GLint uoff = get_offset(paramName);
            shaderBuffer.sub_data(data, (uint32_t)uoff);
            return true;
        }
        std::cerr << "Failed to set shader parameter " << paramName << std::endl;
        return false;
    }

    /**
     * @brief Set a Shader parameter in the target uniform block buffer.
     * 
     * @tparam T 
     * @param paramName 
     * @param data 
     * @param shaderBuffer 
     * @return true 
     * @return false 
     */
    template <typename T>
    bool set_parameter(int paramIndex, const T &data, Buffer &shaderBuffer)
    {
        if (is_valid())
        {
            GLint uoff = get_offset(paramIndex);
            shaderBuffer.sub_data(data, (uint32_t)uoff);
            return true;
        }
        std::cerr << "Failed to set shader parameter " << paramIndex << std::endl;
        return false;
    }

    /**
     * @brief Set the Shader Parameter in a buffer using this ProgramUniformBlockDescription
     * 
     * @tparam T 
     * @param paramName 
     * @param data 
     * @param shaderBuffer 
     * @return true 
     * @return false 
     */
    template <typename T>
    bool set_parameter(const std::string &paramName, const std::vector<T> &data, Buffer &shaderBuffer)
    {
        if (is_valid())
        {
            ProgramUniformBlockDescription::Layout layout = get_layout(paramName);
            GLint uoff = layout.Offset;
            GLint stride = layout.ArrayStride;
            if (uoff != -1 && layout.ArraySize >= data.size())
            {
                shaderBuffer.sub_data(data, uoff, stride);
                return true;
            }
        }
        std::cerr << "Failed to set shader parameter " << paramName << std::endl;
        return false;
    }

    /**
     * @brief bind a given buffer as a uniform buffer
     * @deprecated use buffer.bind_range
     * 
     * @param buffer 
     */
    void bind_buffer(const Buffer &buffer) {
        GL_CHECKED_CALL(
            glBindBufferRange(GL_UNIFORM_BUFFER, location_index, buffer.handle(), 0, buffer.get_capacity())
        );
    }
};

// GPU program specific data
class Program
{
public:
    Program();
    explicit Program(std::string name);
    virtual ~Program();

    Program(Program &&) = delete;
    Program(const Program &) = delete;

    Program &operator=(const Program &) = delete;

    /**
     * @brief Set the shader attached to a specific stage. The shader should already be built
     * 
     * @param 
     * @param shader 
     */
    void attachShader(const Shader* shader);

    /**
     * @brief  Set path for shader stage in this program
     * 
     * @param type 
     * @param path 
     * @param preprocessor 
     * @param version 
     */
    void loadShader(ShaderType type, const std::filesystem::path &path, const ShaderPreprocessor& preprocessor, int version = 450);

    /**
     * @brief Load, Compile, and Link shader programs
     */
    void link();

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
    bool isLinked() const;

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
    ProgramUniformDescription getUniformInfo(const std::string &uniformName) const
    {
        auto itr = uniforms.find(uniformName);
        if (itr != uniforms.end())
            return itr->second;
        return {-1, 0};
    }

    /**
     * @brief Get the Input description for given name
     *
     * @param inputName
     * @return ProgramInputDescription default initialized if it does not exist
     */
    ProgramInputDescription getInputInfo(const std::string &inputName) const
    {
        auto itr = inputs.find(inputName);
        if (itr != inputs.end())
            return itr->second;
        return {};
    }

    /**
     * @brief Get the Attribute Map. Mapping attribute index to binding location in the shader.
     * 
     * @return std::unordered_map<VertexAttributeType, int> mapping attribute identifier to binding location.
     */
    std::unordered_map<VertexAttributeLabel, uint32_t> getAttributeMap() const {
        std::unordered_map<VertexAttributeLabel, uint32_t> map;
        for (auto& mapping : inputs) {
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
    ProgramUniformBlockDescription getUniformBlockInfo(const std::string &uboName) const
    {
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
    bool attach_shader(GLuint shader_name);

public:
    /**
     * @brief Program name
     */
    std::string ProgramName;

protected:
    std::unordered_map<std::string, ProgramUniformDescription> uniforms;
    std::unordered_map<std::string, ProgramInputDescription> inputs;
    std::unordered_map<std::string, ProgramUniformBlockDescription> uniformBlocks;

    // counter for dependent objects. incremented when shader has been reloaded.
    uint32_t modifiedCount = 0;
    GLuint gl_reference;

private:
    // helper function to print program information
    friend std::ostream &operator<<(std::ostream &os, const Program &input);
};

} // namespace ev2

#endif // EV2_SHADER_H