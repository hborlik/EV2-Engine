/**
 * @file serializers.hpp
 * @brief 
 * @date 2023-04-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_IO_SERIALIZERS_HPP
#define EV2_IO_SERIALIZERS_HPP

#include <vector>
#include <fstream>

#include <geometry.hpp>
#include <pcg/sc_wfc.hpp>

#include <json.hpp>

namespace ev2::io {

auto read_file(std::string_view path) -> std::string {
    // from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
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
    std::vector<float> barr(9);
    for(int i = 0; i < 3; i++) {
        barr[i*3+0] = {p.basis[i][0]};
        barr[i*3+1] = {p.basis[i][1]};
        barr[i*3+2] = {p.basis[i][2]};
    }
    j = nlohmann::json{{"basis", barr},
             {"position", {p.position.x, p.position.y, p.position.z}},
             {"half_extents", {p.half_extents.x, p.half_extents.y, p.half_extents.z}}};
}

void from_json(const nlohmann::json& j, OBB& p) {
    std::vector<float> barr{};
    j.at("basis").get_to(barr);

    p.basis = glm::mat3{
        {barr.at(0), barr.at(1), barr.at(2)},
        {barr.at(3), barr.at(4), barr.at(5)},
        {barr.at(6), barr.at(7), barr.at(8)}
    };

    j.at("position").get_to(barr);

    p.position = {barr.at(0), barr.at(1), barr.at(2)};

    j.at("half_extents").get_to(barr);

    p.half_extents = {barr.at(0), barr.at(1), barr.at(2)};
}

} // namespace ev2

namespace ev2::pcg {

void to_json(nlohmann::json& j, const ObjectData& p) {
    j = nlohmann::json{{"name", p.name}, {"asset_path", p.asset_path}, {"properties", p.properties}, {"propagation_patterns", p.propagation_patterns}};
}

void from_json(const nlohmann::json& j, ObjectData& p) {
    j.at("name").get_to(p.name);
    j.at("asset_path").get_to(p.asset_path);
    j.at("properties").get_to(p.properties);
    j.at("propagation_patterns").get_to(p.propagation_patterns);
}

} // namespace pcg



#endif // EV2_PCG_SERIALIZERS_HPP