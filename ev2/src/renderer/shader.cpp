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

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <algorithm>

#include <renderer/shader.h>

using namespace ev2;

namespace ev2::renderer {

// overload stream operator
std::ostream& operator<<(std::ostream& os, const Program& input) {
    os << "=== ShaderProgram: " << input.ProgramName << " ===" << std::endl;
    // for(auto& v : input.attachedShaders) {
    //     os << v.second->shaderPath() << std::endl;
    // }
    os << "Program inputs: (name, location)" << std::endl;
    for(const auto& [name, desc] : input.inputs) {
        os << "    - " << name << ", " << desc.Location << std::endl;
    }
    os << "Program uniforms: (name, location)" << std::endl;
    for(const auto& [name, desc] : input.uniforms) {
        os << "    - " << name << ", " << desc.Location << std::endl;
    }
    os << "Program uniform blocks:" << std::endl;
    for(const auto& [name, desc] : input.uniformBlocks) {
        os << "    UB: " << name << " at " << desc.location_index << " size: " << desc.block_size << " bytes" << std::endl;
        for(const auto& [l_name, l_desc] : desc.layouts) {
            os << "       U: " << l_name << " offset " << l_desc.Offset << " len " << l_desc.ArraySize << " stride " << l_desc.ArrayStride << std::endl;
        }
    }
    os << "    shader " << (input.isLinked() ? "linked" : "not linked") << std::endl;
    os << "=== ShaderProgram ===" << std::endl;
    return os;
}

std::string ShaderPreprocessor::preprocess(const std::string& input_source) const {
    using namespace std;
    ostringstream result;

    istringstream iss(input_source);

    const string include = "#include";
    size_t line_num = 0;
    for (string line; getline(iss, line); ) {
        auto pos = line.find(include);
        if (pos != string::npos) {
            size_t s = string::npos, e = string::npos;
            while (pos < line.size()) {
                if (line[pos] == '\"') {
                    if (s == string::npos) {
                        s = pos + 1;
                    } else {
                        e = pos;
                        break;
                    }
                }
                pos++;
            }
            if (s == string::npos || e == string::npos) {
                throw shader_error{"Preprocessor", std::to_string(line_num)};
            }
            std::filesystem::path filename{line.substr(s, e - s)};
            filename = shader_include_dir / filename;

            // do not process any includes in the header files
            result << load_shader_content(filename);
        } else {
            result << line << std::endl;
        }
        line_num++;
    }
    return result.str();
}

std::string load_shader_content(const std::filesystem::path& source_path) {
    std::ifstream in{source_path};

    if (!in.is_open()) {
        throw shader_error{"Preprocessor", "Shader File not found at " + source_path.generic_string()};
    }
    // copy out file contents
    std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    in.close();
    content += '\n';

    return content;
}



//// SHADER ////

Shader::Shader(gl::GLSLShaderType type) : type{type} {
    gl_reference = glCreateShader((GLenum)type);
    EV2_CHECK_THROW(gl_reference != 0, "Failed to create shader");
}

Shader::~Shader() {
    if(gl_reference != 0)
        glDeleteShader(gl_reference);
}

void Shader::push_source_file(const std::filesystem::path& path) {
    this->path = path;

    source += load_shader_content(path);
}

void Shader::push_source_string(const std::string& source_string) {
    source += source_string;
}

bool Shader::compile(ShaderPreprocessor pre, bool delete_source) {
    std::string final_source = pre.preprocess(source);
    const GLchar* codeArray = final_source.c_str();
    glShaderSource(gl_reference, 1, &codeArray, nullptr);
    glCompileShader(gl_reference);

    // get shader compile results
    GLint result;
    glGetShaderiv(gl_reference, GL_COMPILE_STATUS, &result);
    if(result == GL_TRUE) {
        if (delete_source) {
            source = {};
        }
    } else { // ask for more info on failure
        std::cout << "Failed to compile shader " << path << std::endl;

        GLint logLen;
        glGetShaderiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetShaderInfoLog(gl_reference, logLen, &written, log.data());

            std::cout << "Shader error log dumped to log file for " << path << "\n";
            Engine::get().log_file<Shader>(
                std::string{log.begin(), log.end() - 1} +
                "\n------BEGIN SHADER SOURCE-----\n\n" +
                source +
                "\n------END SHADER SOURCE-----\n\n");
        }
    }
    return result == GL_TRUE;
}

// UbiquitousShader

void UbiquitousShader::push_source_file(const std::filesystem::path &path) {
    last_shader_begin = source.length();
#if not NDEBUG
    source += "\n//" + path.generic_string() + "\n";
#endif
    source += load_shader_content(path);
}

void UbiquitousShader::push_source_string(const std::string& source_string) {
    last_shader_begin = source.length();
    source += source_string;
}

std::unique_ptr<Shader> create_shader_stage(const std::string& stage_define, const std::string& source, gl::GLSLShaderType type, int version) {
    auto out = std::make_unique<Shader>(type);
    out->push_source_string("#version " + std::to_string(version) + "\n");
    out->push_source_string("#define " + stage_define + "\n");
    out->push_source_string(source);
    
    return out;
}

std::vector<std::unique_ptr<Shader>> UbiquitousShader::get_shader_stages(int version) {
    std::vector<std::unique_ptr<Shader>> output_shaders{};
    GLSLShaderTypeFlag stages{};
    if (source.find("VERTEX_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::VERTEX_SHADER;
        output_shaders.push_back(create_shader_stage("VERTEX_SHADER", source, gl::GLSLShaderType::VERTEX_SHADER, version));
    }
    if (source.find("FRAGMENT_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::FRAGMENT_SHADER;
        output_shaders.push_back(create_shader_stage("FRAGMENT_SHADER", source, gl::GLSLShaderType::FRAGMENT_SHADER, version));
    }
    if (source.find("GEOMETRY_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::GEOMETRY_SHADER;
        output_shaders.push_back(create_shader_stage("GEOMETRY_SHADER", source, gl::GLSLShaderType::GEOMETRY_SHADER, version));
    }
    if (source.find("TESS_CONTROL_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::TESS_CONTROL_SHADER;
        output_shaders.push_back(create_shader_stage("TESS_CONTROL_SHADER", source, gl::GLSLShaderType::TESS_CONTROL_SHADER, version));
    }
    if (source.find("TESS_EVALUATION_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::TESS_EVALUATION_SHADER;
        output_shaders.push_back(create_shader_stage("TESS_EVALUATION_SHADER", source, gl::GLSLShaderType::TESS_EVALUATION_SHADER, version));
    }
    if (source.find("COMPUTE_SHADER", last_shader_begin) != std::string::npos) {
        stages.v |= GLSLShaderTypeFlag::COMPUTE_SHADER;
        output_shaders.push_back(create_shader_stage("COMPUTE_SHADER", source, gl::GLSLShaderType::COMPUTE_SHADER, version));
    }
    
    return output_shaders;
}

// Program

Program::Program() {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create Shader Program"};
}

Program::Program(std::string name) : 
    ProgramName{std::move(name)} {

    gl_reference = glCreateProgram();
    if(gl_reference == 0)
        throw engine_exception{"Failed to create Shader Program"};
}

Program::~Program() {
    if(gl_reference != 0)
        glDeleteProgram(gl_reference);
}

void Program::attachShader(const Shader* shader) {
    if (!(shader && shader->IsCompiled() && attach_shader(shader->getHandle())))
        throw shader_error{ProgramName, "Shader could not be attached"};
}

void Program::loadShader(gl::GLSLShaderType type, const std::filesystem::path& path, const ShaderPreprocessor& preprocessor, int version) {
    std::shared_ptr<Shader> s = std::make_shared<Shader>(type);

    s->push_source_string("#version " + std::to_string(version) + " core\n");
    s->push_source_string("#line 1\n");
    s->push_source_file(Engine::get().shader_path / path);
    s->compile(preprocessor);
    auto suc = attach_shader(s->getHandle());
    if (!suc)
        throw shader_error{ProgramName, "Loaded shader could not be attached " + path.generic_string()};
}

void Program::link() {
    modifiedCount++;

    glLinkProgram(gl_reference);
    // get status
    if(isLinked()) {
        // initialize lists of attributes and uniform variables
        updateProgramInputInfo();
        updateProgramUniformBlockInfo();
        updateProgramUniformInfo();

        // additional configuration
        onBuilt();
    } else {
        std::cerr << "Failed to link program " << ProgramName << std::endl;
        
        GLint logLen;
        glGetProgramiv(gl_reference, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            std::vector<char> log{};
            log.resize(logLen);
            
            GLsizei written;
            glGetProgramInfoLog(gl_reference, logLen, &written, log.data());

            std::cout << "Program error log dumped to log file for " << ProgramName << "\n";
            std::string log_str{log.begin(), log.end()-1};
            log_str += "\nProgram Log for " + ProgramName + "\n";
            Engine::get().log_file<Program>(log_str);

            std::cerr << log_str << std::endl;
        }
        throw shader_error{ProgramName, "Failed to link program"};
    }
}

void Program::use() const {
    // set program to active
    glUseProgram(gl_reference);
}

bool Program::isLinked() const {
    GLint status;
    glGetProgramiv(gl_reference, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

bool Program::setUniformBlockBinding(const std::string& uniformName, uint32_t location) {
    int loc = getUniformBlockInfo(uniformName).location_index;
    if(loc != -1) {
        glUniformBlockBinding(gl_reference, loc, location);
        return true;
    }
    return false;
}

GLuint Program::getProgramResourceLocation(GLenum resource, const std::string& name) {
    GLuint resource_index = glGetProgramResourceIndex(gl_reference, resource, name.c_str());
    const GLenum properties[] = {GL_BUFFER_BINDING};

    GLint values[1];
    GL_CHECKED_CALL(glGetProgramResourceiv(gl_reference, resource, resource_index, 1, properties, 1, NULL, values));

    return values[0];
}

void Program::updateProgramInputInfo() {
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

void Program::updateProgramUniformInfo() {
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

void Program::updateProgramUniformBlockInfo() {
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

        ProgramUniformBlockDescription& pubd = uniformBlocks[blockName];
        pubd.location_index = binding;
        pubd.block_size = blockSize;

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

            pubd.layouts[name] = ProgramUniformBlockDescription::Layout{offsets[unifIx], sizes[unifIx], strides[unifIx]};

        }

    }
}

bool Program::attach_shader(GLuint shader_name) {
    bool error = false;
    GL_ERROR_CHECK(glAttachShader(gl_reference, shader_name), error);
    return !error;
}

} // namespace ev2::renderer