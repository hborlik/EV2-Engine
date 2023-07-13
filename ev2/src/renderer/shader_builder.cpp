#include "renderer/shader_builder.hpp"

#include "io/serializers.hpp"

namespace ev2 {

// UbiquitousShader

void ShaderBuilder::push_source_file(const std::filesystem::path &path) {
    last_shader_begin = m_source.length();
    auto shader_path = shader_asset_path / path;
    m_source += "\n//" + shader_path.generic_string() + "\n";
    m_source += io::read_file(shader_path);
}

void ShaderBuilder::push_source_string(const std::string& source_string) {
    last_shader_begin = m_source.length();
    m_source += source_string;
}

void ShaderBuilder::preprocess() {
    using namespace std;
    ostringstream result;

    istringstream iss(m_source);

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
            filename = m_preprocessor_settings.get_shader_dir() / filename;

            // do not process any includes in the header files
            result << io::read_file(filename);
        } else {
            result << line << std::endl;
        }
        line_num++;
    }
    m_source = result.str();
}

ShaderSource ShaderBuilder::make_shader_stage_source(ShaderType type, int version) {
    
    std::string out{};
    out += ("#version " + std::to_string(version) + "\n");
    out += ("#define " + ShaderTypeName(type) + "\n");
    out += (m_source);
    
    return ShaderSource{.source=out, .type=type};
}

}