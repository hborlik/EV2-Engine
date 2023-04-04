/**
 * @file 2d_geometry.hpp
 * @brief 
 * @date 2023-01-13
 * 
 */
#ifndef EV2_GEOMETRY_H
#define EV2_GEOMETRY_H

#include <vector>
#include <optional>
#include <glm/glm.hpp>

namespace ev2 {

struct LineSegment {
    glm::vec2 a,b;
};

std::optional<glm::vec2> intersect(const LineSegment& a, const LineSegment& b);

struct Polygon {
    std::vector<glm::vec2> vertices;

    bool is_degenerate() const;
};

} // namespace ev2

#endif // EV2_GEOMETRY_H