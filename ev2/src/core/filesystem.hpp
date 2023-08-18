/**
 * @file filesystem.hpp
 * @brief 
 * @date 2023-08-18
 * 
 * 
 */
#ifndef EV2_CORE_FILESYSTEM_HPP
#define EV2_CORE_FILESYSTEM_HPP

namespace ev2 {

class Filesystem {
public:
    static std::string get_shader_content(const std::filesystem::path& path);
};

std::string read_file(const std::filesystem::path& path);

}

#endif // EV2_CORE_FILESYSTEM_HPP