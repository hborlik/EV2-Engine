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

#include "evpch.hpp"

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

// Random element from container
// see https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    auto dist = std::distance(start, end);
    if (dist < 1)
        return end;
    std::uniform_int_distribution<> dis(0, dist - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename RandomGenerator>
bool binomial_trial(float success, RandomGenerator& g) {
    std::uniform_real_distribution<> dis{};
    return dis(g) < success;
}

}

#endif // EV2_PCG_DISTRIBUTIONS_HPP