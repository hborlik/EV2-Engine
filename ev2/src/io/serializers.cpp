#include "serializers.hpp"

#include <initializer_list>
#include "glm/gtc/type_ptr.hpp"


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
