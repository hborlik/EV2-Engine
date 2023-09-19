/**
 * @file shader.cpp
 * @author Hunter Borlik 
 * @brief GPU Shader
 * @version 0.1
 * @date 2019-09-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "core/engine.hpp"
#include "gl_shader.hpp"
#include "resource.hpp"

using namespace ev2;

namespace ev2::renderer {

// overload stream operator
std::ostream& operator<<(std::ostream& os, const GLProgram& input) {
    os << "=== ShaderProgram: " << input.ProgramName << " ===" << "\n";
    // for(auto& v : input.attachedShaders) {
    //     os << v.second->shaderPath() << "\n";
    // }
    os << "GLProgram inputs: (name, location)" << "\n";
    for(const auto& [name, desc] : input.inputs) {
        os << "    - " << name << ", " << desc.Location << "\n";
    }
    os << "GLProgram uniforms: (name, location)" << "\n";
    for(const auto& [name, desc] : input.uniforms) {
        os << "    - " << name << ", " << desc.Location << "\n";
    }
    os << "GLProgram uniform blocks:" << "\n";
    for(const auto& [name, desc] : input.uniformBlocks) {
        os << "    UB: " << name << " at " << desc.location_index << " size: " << desc.block_size << " bytes" << "\n";
        for(const auto& [l_name, l_desc] : desc.layout_map) {
            os << "       U: " << l_name << " offset " << desc.layouts[l_desc].Offset << " len " << desc.layouts[l_desc].ArraySize << " stride " << desc.layouts[l_desc].ArrayStride << "\n";
        }
    }
    os << "    shader " << (input.is_linked() ? "linked" : "not linked") << "\n";
    os << "=== ShaderProgram ===" << "\n";
    return os;
}

//// SHADER ////

GLShader::GLShader(gl::GLSLShaderType type) : Shader{}, m_type{type} {
    gl_reference = glCreateShader((GLenum)m_type);
    EV_CHECK_THROW(gl_reference != 0, "Failed to create shader");
}

GLShader::~GLShader() {
    if(gl_reference != 0)
        glDeleteShader(gl_reference);
}

bool GLShader::compile(const ShaderSource& source) {
    const GLchar* codeArray = source.source.data();
    glShaderSource(gl_reference, 1, &codeArray, nullptr);
    glCompileShader(gl_reference);

    // get shader compile results
    GLint result;
    glGetShaderiv(gl_reference, GL_COMPILE_STATUS, &result);
    if(result == GL_TRUE) {

    } else { // ask for more info on failure
        Log::error_core<GLShader>("Failed to compile shader");

        GLint logLen;
        glGetShaderiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetShaderInfoLog(gl_reference, logLen, &written, log.data());

            Log::error_core<GLShader>(
                "GLShader error {} \
                \n------BEGIN SHADER SOURCE-----\n \
                {}\
                \n------END SHADER SOURCE-----\n\n",
                std::string{log.begin(), log.end() - 1},
                source.source);
        }
    }
    return result == GL_TRUE;
}

// GLProgram

GLProgram::GLProgram() {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create GLShader Program"};
}

GLProgram::GLProgram(std::string name) : 
    ProgramName{std::move(name)} {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create GLShader Program"};
}

GLProgram::~GLProgram() {
    if(gl_reference != 0)
        glDeleteProgram(gl_reference);
}

void GLProgram::attach_shader(std::shared_ptr<Shader> shader) {
    auto gl_shader = std::dynamic_pointer_cast<GLShader>(shader);
    EV_CORE_ASSERT(gl_shader, "shader must be non null and of type GLShader!");
    if (!(gl_shader && gl_shader->IsCompiled() && gl_attach_shader(gl_shader->getHandle())))
        throw shader_error{ProgramName, "GLShader could not be attached"};
}

void GLProgram::loadShader(ShaderType type, const std::filesystem::path& path, const PreprocessorSettings& settings, int version) {

    ShaderBuilder shader_builder = ShaderBuilder::make_default_shader_builder();

    // shader_builder.push_source_string("#version " + std::to_string(version) + " core\n");
    shader_builder.push_source_string("#line 1\n");
    shader_builder.push_source_file(path);
    shader_builder.preprocess(settings);

    ShaderSource source = shader_builder.make_shader_stage_source(type, version);
    
    std::shared_ptr<GLShader> s = std::make_shared<GLShader>(glShaderType(type));
    s->compile(source);

    auto suc = gl_attach_shader(s->getHandle());
    if (!suc)
        throw shader_error{ProgramName, "Loaded shader could not be attached " + path.generic_string()};
}

void GLProgram::link() {
    modifiedCount++;

    glLinkProgram(gl_reference);
    // get status
    if(is_linked()) {
        // initialize lists of attributes and uniform variables
        updateProgramInputInfo();
        updateProgramUniformBlockInfo();
        updateProgramUniformInfo();

        // additional configuration
        onBuilt();
    } else {
        Log::error_core<GLProgram>("Failed to link program {}",  ProgramName);
        
        GLint logLen;
        glGetProgramiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetProgramInfoLog(gl_reference, logLen, &written, log.data());

            std::string log_str{log.begin(), log.end()-1};
            log_str += "\nProgram Log for " + ProgramName + "\n";
            Log::error_core<GLProgram>("Program error for " + ProgramName);
            Log::error_core<GLProgram>(log_str);
        }
        throw shader_error{ProgramName, "Failed to link program"};
    }
}

void GLProgram::use() const {
    // set program to active
    glUseProgram(gl_reference);
}

bool GLProgram::is_linked() const {
    GLint status;
    glGetProgramiv(gl_reference, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

bool GLProgram::setUniformBlockBinding(const std::string& uniformName, uint32_t location) {
    int loc = get_uniform_block_layout(uniformName).location_index;
    if(loc != -1) {
        glUniformBlockBinding(gl_reference, loc, location);
        return true;
    }
    return false;
}

GLuint GLProgram::getProgramResourceLocation(GLenum resource, const std::string& name) {
    GLuint resource_index = glGetProgramResourceIndex(gl_reference, resource, name.c_str());
    const GLenum properties[] = {GL_BUFFER_BINDING};

    GLint values[1];
    GL_CHECKED_CALL(glGetProgramResourceiv(gl_reference, resource, resource_index, 1, properties, 1, NULL, values));

    return values[0];
}

void GLProgram::updateProgramInputInfo() {
    GLint numAttribs = 0;
    glGetProgramInterfaceiv(gl_reference, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);
    const GLenum properties[3] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};

    inputs.clear();

    for(int attr = 0; attr < numAttribs; ++attr) {
        GLint values[3];
        glGetProgramResourceiv(gl_reference, GL_PROGRAM_INPUT, attr, 3, properties, 3, NULL, values);

        // Get the name
        std::vector<char> name{};
        name.resize(values[0]);
        glGetProgramResourceName(gl_reference, GL_PROGRAM_INPUT, attr, name.size(), NULL, name.data());
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        // save attr information
        std::string pname{name.begin(), name.end()};
        ProgramInputDescription& pid = inputs[pname];
        pid.Location = values[2];
        pid.Type = values[1];
    }
}

void GLProgram::updateProgramUniformInfo() {
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(gl_reference, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
    const GLenum properties[4] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

    uniforms.clear();

    for(uint32_t unif = 0; unif < numUniforms; ++unif) {
        GLint values[4];
        glGetProgramResourceiv(gl_reference, GL_UNIFORM, unif, 4, properties, 4, NULL, values);

        // Skip any uniforms that are in a block.
        if(values[0] != -1)
            continue;


        // get Length of uniform array
        GLuint props[]  = {GL_UNIFORM_SIZE};
        GLuint unifs[]  = {unif};
        GLint rets[]    = {0};
        glGetActiveUniformsiv(gl_reference, 1, unifs, GL_UNIFORM_SIZE, rets);

        // Get the name
        std::vector<char> name{};
        name.resize(values[2]);
        glGetProgramResourceName(gl_reference, GL_UNIFORM, unif, name.size(), NULL, name.data());
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        // save uniform information
        std::string uname{name.begin(), name.end()};
        ProgramUniformDescription& pud = uniforms[uname]; // reference uniform description
        pud.Type        = values[1];
        pud.Location    = values[3];
        pud.Length      = rets[0];
    }
}

void GLProgram::updateProgramUniformBlockInfo() {
    GLint numBlocks = 0;
    glGetProgramInterfaceiv(gl_reference, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    const GLenum blockProperties[1] = {GL_NUM_ACTIVE_VARIABLES};
    const GLenum activeUnifProp[1] = {GL_ACTIVE_VARIABLES};

    uniformBlocks.clear();

    for(int blockIx = 0; blockIx < numBlocks; ++blockIx) {
        GLint numActiveVars = 0;
        glGetProgramResourceiv(gl_reference, GL_UNIFORM_BLOCK, blockIx, 1, blockProperties, 1, NULL, &numActiveVars);

        if(!numActiveVars)
            continue;

        // get size of uniform block
        GLint blockSize;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

        // get the binding of the uniform block
        GLint binding;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_BINDING, &binding);

        // get the uniform block name
        GLint nameLength;
        glGetActiveUniformBlockiv(gl_reference, blockIx, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
        std::vector<char> namebuffer{};
        namebuffer.resize(nameLength);
        glGetActiveUniformBlockName(gl_reference, blockIx, nameLength, nullptr, namebuffer.data());

        std::string blockName{namebuffer.begin(), namebuffer.end()};
        blockName.erase(std::find(blockName.begin(), blockName.end(), '\0'), blockName.end());

        ProgramBlockLayout& pubd = uniformBlocks[blockName];
        pubd.location_index = binding;
        pubd.block_size = blockSize;
        pubd.layouts = {};

        std::vector<GLuint> unifIndices(numActiveVars); // array of active variable indices associated with an active uniform block
        glGetProgramResourceiv(gl_reference, GL_UNIFORM_BLOCK, blockIx, 1, activeUnifProp, numActiveVars, NULL, (GLint*)unifIndices.data());

        std::vector<GLint> offsets(numActiveVars);
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_OFFSET, offsets.data());

        std::vector<GLint> strides(numActiveVars); // stride between consecutive elements in array
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_ARRAY_STRIDE, strides.data());
        // for(int i = 0; i < numActiveVars; ++i) {
        //     std::cout << strides[i] << std::endl;
        // }

        std::vector<GLint> sizes(numActiveVars); // number of items in uniform, greater than 1 for arrays
        glGetActiveUniformsiv(gl_reference, numActiveVars, unifIndices.data(), GL_UNIFORM_SIZE, sizes.data());

        // for each uniform in block
        for(int unifIx = 0; unifIx < numActiveVars; ++unifIx) {
            const GLenum unifProperties[3] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};
            GLint values[3];
            glGetProgramResourceiv(gl_reference, GL_UNIFORM, unifIndices[unifIx], 3, unifProperties, 3, NULL, values);

            // Get the name
            std::string name{};
            name.resize(values[0]);
            glGetProgramResourceName(gl_reference, GL_UNIFORM, unifIndices[unifIx], name.size(), NULL, &name[0]);
            name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

            //std::cout << "UB: " << name << ", GL_UNIFORM_BLOCK " << unifIndices[unifIx] << ", Location " << values[2] << std::endl;

            pubd.layouts.emplace_back(ProgramBlockLayout::Layout{offsets[unifIx], sizes[unifIx], strides[unifIx]});
            pubd.layout_map[name] = pubd.layouts.size() - 1;

        }

    }
}

bool GLProgram::gl_attach_shader(GLuint shader_name) {
    bool error = false;
    GL_ERROR_CHECK(glAttachShader(gl_reference, shader_name), error);
    return !error;
}

} // namespace ev2::renderer