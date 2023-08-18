#include "serializers.hpp"

#include <initializer_list>
#include "glm/gtc/type_ptr.hpp"


namespace ev2::io {

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

} // namespace ev2::io

namespace ev2 {


void to_json(nlohmann::json& j, const OBB& p) {
    std::vector<float> barr(glm::value_ptr(p.rotation), glm::value_ptr(p.rotation) + 4);
    std::vector<float> carr(glm::value_ptr(p.center), glm::value_ptr(p.rotation) + 3);
    j = nlohmann::json{ 
        { "rotation", barr },
        {"center", carr},
        { "half_extents", { p.half_extents.x, p.half_extents.y, p.half_extents.z } } };
}

void from_json(const nlohmann::json& j, OBB& p) {
    std::vector<float> barr{};
    j.at("rotation").get_to(barr);

    p.rotation = glm::make_quat(barr.data());

    j.at("center").get_to(barr);

    p.center = glm::make_vec3(barr.data());

    j.at("half_extents").get_to(barr);

    p.half_extents = { barr.at(0), barr.at(1), barr.at(2) };
}

} // namespace ev2
