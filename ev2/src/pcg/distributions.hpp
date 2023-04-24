/**
 * @file distributions.hpp
 * @brief 
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_PCG_DISTRIBUTIONS_HPP
#define EV2_PCG_DISTRIBUTIONS_HPP

#include <cmath>

#include <glm/glm.hpp>

namespace ev2::pcg {

glm::vec2 uniform2d() {
    return {(float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX};
}

glm::vec2 uniform_disk(const glm::vec2& uv) {
    float r = std::sqrt(uv.x);
    float th = 2 * M_PI * uv.y;
    return {r * std::cos(th), r * std::sin(th)};
}

}

#endif // EV2_PCG_DISTRIBUTIONS_HPP