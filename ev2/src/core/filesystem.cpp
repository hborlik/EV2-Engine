#include "filesystem.hpp"

#include "core/engine.hpp"

namespace ev2 {

auto read_file(const std::filesystem::path& path) -> std::string {
    // from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.generic_string());
    stream.exceptions(std::ios_base::badbit);

    if (!stream) {
        throw std::ios_base::failure("file does not exist");
    }

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

std::string Filesystem::get_shader_content(const std::filesystem::path& path) {
    return read_file(Engine::get_singleton().asset_path / path);
}

}