#include <iostream>

#include <io/serializers.hpp>

using namespace ev2;
using json = nlohmann::json;


int main() {

    OBB obb{
        .basis = glm::mat3{1},
        .position = glm::vec3{1, 1, 1},
        .half_extents = glm::vec3{0}
    };

    json j{obb};

    std::cout << j.dump() << std::endl;

}