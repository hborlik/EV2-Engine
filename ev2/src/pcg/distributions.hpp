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
#include <random>

#include <glm/glm.hpp>

namespace ev2::pcg {

inline float uniform1d() {
    return (float)std::rand() / (float)RAND_MAX;
}

inline glm::vec2 uniform2d() {
    return {(float)std::rand() / (float)RAND_MAX, (float)std::rand() / (float)RAND_MAX};
}

inline glm::vec2 uniform_disk(const glm::vec2& uv) {
    float r = std::sqrt(uv.x);
    float th = 2 * M_PI * uv.y;
    return {r * std::cos(th), r * std::sin(th)};
}

// from https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

}

#endif // EV2_PCG_DISTRIBUTIONS_HPP