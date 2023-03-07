/**
 * @file geometry.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_GEOMETRY_H
#define EV2_GEOMETRY_H

#include <optional>
#include <limits>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <util.h>
#include <ev.h>
#include <reference_counted.h>

namespace ev2 {

struct SurfaceInteraction
{
    double t = 0;
    glm::vec3 point;
    glm::vec3 incoming;
    glm::vec3 normal;
    Ref<Object> hit;

    SurfaceInteraction() = default;
    SurfaceInteraction(glm::vec3 normal,
                       float t,
                       glm::vec3 point,
                       glm::vec3 incoming)
        : t{t}, point{point}, incoming{incoming},
          normal{normal}
    {
    }
};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction) : origin{origin}, direction{glm::normalize(direction)} {}

    Ray transform(const glm::mat4& tr) const {
        return {tr * glm::vec4{origin, 1.0f}, tr * glm::vec4{direction, .0f}};
    }

    glm::vec3 eval(float t) const {return origin + t * direction;}

private:
    friend Ray operator*(const glm::mat4& tr, const Ray& r) {
        return r.transform(tr);
    }
};

struct Sphere {
    glm::vec3   center{0.f};
    float       radius = 0.f;

    Sphere() = default;
    Sphere(const glm::vec3& center, float radius) noexcept : center{center}, radius{radius} {}

    bool ray_cast(const Ray& ray, SurfaceInteraction& hit) {
        using namespace glm;

        vec3 e_c = (ray.origin - center);
        float a = dot(ray.direction, ray.direction);
        float b = 2 * dot(ray.direction, e_c);
        float c = dot(e_c,e_c) - radius*radius;
        float det_sq = b*b - 4*a*c;
        if(det_sq <= 0)
            return false;
        det_sq = glm::sqrt(det_sq);
        float t = -b - det_sq;
        t /= 2*a;
        if(t < 0)
            return false;

        auto hit_point = ray.eval(t);
        // hit info into surface interaction
        SurfaceInteraction h {
            normalize(hit_point - center),
            t,
            hit_point,
            ray.direction
        };
        hit = h;
        return true;
    }

private:

    friend Sphere operator*(const glm::mat4& tr, const Sphere& s) {
        return {tr * glm::vec4{s.center, 1.0f}, s.radius};
    }    
};

struct Plane {
    glm::vec4 p = {0, 1, 0, 0};

    Plane() = default;

    Plane(glm::vec3 normal, float d) noexcept : p{normal.x, normal.y, normal.z, d} {
        normalize();
    }

    Plane(glm::vec3 normal, glm::vec3 point) noexcept : p{normal.x, normal.y, normal.z, -glm::dot(normal, point)} {
        normalize();
    }

    Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c) noexcept : p{glm::cross(b - a, c - a), -glm::dot(a, glm::cross(b - a, c - a))} {
        normalize();
    }

    explicit Plane(const glm::vec4& p) noexcept : p{p} {
        normalize();
    }

    inline void normalize() noexcept {
        p /= glm::length(glm::vec3(p));
    }

    /**
     * @brief Normalized plane required
     * 
     * @param point 
     * @return float 
     */
    inline float distance_from_plane(glm::vec3 point) const noexcept {
        return glm::dot(p, glm::vec4(point, 1.0f));
    }

    /**
     * @brief intersect ray and plane, ray direction and plane should be normalized
     * 
     * @param ray 
     * @param hit 
     * @return true 
     * @return false 
     */
    bool ray_cast(const Ray& ray, SurfaceInteraction& hit) {
        using namespace glm;
        const float denom = dot(ray.direction, vec3(p));
        if (denom == 0.0) // parallel
            return false;
        const float t = (p[3] - dot(ray.origin, vec3(p))) / denom;
        if (t >= 0.01f) {
            SurfaceInteraction h {
                vec3(p),
                t,
                ray.eval(t),
                ray.direction
            };
            hit = h;
            return true;
        }
        return false;
    }
};

/**
 * @brief axis aligned box. 
 * 
 */
class AABB {
public:
    const glm::vec3 pMin;
    const glm::vec3 pMax;

    AABB() : pMin{(float)-INFINITY}, pMax{(float)INFINITY} {}
    AABB(glm::vec3 pMin, glm::vec3 pMax) : pMin{pMin}, pMax{pMax} {}

    glm::vec3 diagonal() const noexcept {return pMax - pMin;}

    float volume() const noexcept {
        auto d = diagonal();
        return d.x * d.y * d.z;
    }

    static AABB from_points(const glm::vec3 points[8]) noexcept {
        glm::vec3 minp{INFINITY};
        glm::vec3 maxp{-INFINITY};
        for (int i = 0; i < 8; ++i) {
            minp = glm::min(minp, points[i]);
            maxp = glm::max(maxp, points[i]);
        }
        return {minp, maxp};
    }

    /**
     * @brief intersect a ray with the axis aligned bounds
     * 
     * @param r ray
     * @param hit_t0 optional t0 close hit
     * @param hit_t1 optional t1 far hit
     * @return true got an intersection
     * @return false miss
     */
    bool ray_cast(const Ray& r, float* hit_t0 = nullptr, float* hit_t1 = nullptr) const {
        float t0 = 0;
        float t1 = INFINITY;
        // for each axis, check axis aligned plane t = (v[i] - o[i]) / d[i]
        for (int i = 0; i < 3; ++i) {
            float invRD = 1 / r.direction[i];
            float tn = (pMin[i] - r.origin[i]) * invRD;
            float tf = (pMax[i] - r.origin[i]) * invRD;
            if (tn > tf) // planes are ordered incorrectly, swap values
                std::swap(tn, tf);
            // 
            // keep the min far and max near t values
            t0 = std::max(tn, t0);
            t1 = std::min(tf, t1);
            if (t0 > t1) // shortcut check for overlap -> miss
                return false;
        }
        if (hit_t0)
            *hit_t0 = t0;
        if (hit_t1)
            *hit_t1 = t1;
        return true;
    }

private:

    friend AABB operator*(const glm::mat4& tr, const AABB& aabb) noexcept {
        using namespace glm;
        // 8 corners of the box
        vec3 corners[8] = {
            vec3(aabb.pMin.x, aabb.pMin.y, aabb.pMin.z),
            vec3(aabb.pMax.x, aabb.pMin.y, aabb.pMin.z),
            vec3(aabb.pMax.x, aabb.pMax.y, aabb.pMin.z),
            vec3(aabb.pMin.x, aabb.pMax.y, aabb.pMin.z),
            vec3(aabb.pMin.x, aabb.pMin.y, aabb.pMax.z),
            vec3(aabb.pMax.x, aabb.pMin.y, aabb.pMax.z),
            vec3(aabb.pMax.x, aabb.pMax.y, aabb.pMax.z),
            vec3(aabb.pMin.x, aabb.pMax.y, aabb.pMax.z),
        };

        // transform with tr
        vec3 world_corners[8];
        for (int i = 0; i < 8; ++i) {
            world_corners[i] = vec3(tr * vec4(corners[i], 1.0f));
        }

        // find min and max
        return from_points(world_corners);
    }
};

struct Box {
    glm::mat4   transform;
    AABB     local_bounds;
};

struct Frustum {

    Frustum() = default;
    explicit Frustum(const glm::mat4& view_projection) {
        using namespace glm;
        // 4 corners of the view frustum in clip space
        vec4 clip_corners[8] = {
            vec4(-1, -1, -1, 1),
            vec4( 1, -1, -1, 1),
            vec4( 1,  1, -1, 1),
            vec4(-1,  1, -1, 1),
            vec4(-1, -1,  1, 1),
            vec4( 1, -1,  1, 1),
            vec4( 1,  1,  1, 1),
            vec4(-1,  1,  1, 1),
        };

        // transform to world space
        mat4 inv_view_projection = inverse(view_projection);
        vec4 world_corners[8];
        for (int i = 0; i < 8; ++i) {
            world_corners[i] = inv_view_projection * clip_corners[i];
            world_corners[i] /= world_corners[i].w;
        }

        // 6 planes
        // left
        planes[0] = Plane(world_corners[0], world_corners[1], world_corners[2]);
        // right
        planes[1] = Plane(world_corners[4], world_corners[5], world_corners[6]);
        // top
        planes[2] = Plane(world_corners[2], world_corners[3], world_corners[6]);
        // bottom
        planes[3] = Plane(world_corners[0], world_corners[1], world_corners[4]);
        // near
        planes[4] = Plane(world_corners[0], world_corners[3], world_corners[4]);
        // far
        planes[5] = Plane(world_corners[1], world_corners[2], world_corners[5]);
    }

    // 6 inward pointing planes
    Plane planes[6];

    Plane& get_left() noexcept  {return planes[0];}
    Plane& get_right() noexcept {return planes[1];}
    Plane& get_top() noexcept   {return planes[2];}
    Plane& get_bottom() noexcept{return planes[3];}
    Plane& get_near() noexcept  {return planes[4];}
    Plane& get_far() noexcept   {return planes[5];}

    const Plane& get_left() const noexcept  {return planes[0];}
    const Plane& get_right() const noexcept {return planes[1];}
    const Plane& get_top() const noexcept   {return planes[2];}
    const Plane& get_bottom() const noexcept{return planes[3];}
    const Plane& get_near() const noexcept  {return planes[4];}
    const Plane& get_far() const noexcept   {return planes[5];}

    void normalize() noexcept {
        for (int i = 0; i < 6; ++i) {
            planes[i].normalize();
        }
    }

    void copy_to_array(glm::vec4 (&tgt)[6]) {
        for (int i = 0; i < sizeof(tgt) / sizeof(glm::vec4); ++i) {
            tgt[i] = planes[i].p;
        }
    }
};

/**
 * @brief Extract projection frustum planes
 * 
 * @param comp 
 * @return Frustum 
 */
inline Frustum extract_frustum(const glm::mat4& comp) noexcept {
    using namespace glm;
    Frustum f{};

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++) {
            f.planes[i * 2 + j].p.x = comp[0][3] + (j == 0 ? comp[0][j] : -comp[0][j]);
            f.planes[i * 2 + j].p.y = comp[1][3] + (j == 0 ? comp[1][j] : -comp[1][j]);
            f.planes[i * 2 + j].p.z = comp[2][3] + (j == 0 ? comp[2][j] : -comp[2][j]);
            f.planes[i * 2 + j].p.w = comp[3][3] + (j == 0 ? comp[3][j] : -comp[3][j]);
            f.planes[i * 2 + j].normalize();
        }

    return f;
}

/**
 * @brief check if a sphere is touching or inside a frustum
 * 
 * @param f 
 * @param s 
 * @return bool
 */
inline bool intersect(const Frustum& f, const Sphere& s) noexcept {
    float dist = 0;
    for (int i = 0; i < 6; i++) {
        dist = f.planes[i].distance_from_plane(s.center);
        //test against each plane
        if (dist < 0 && std::abs(dist) > s.radius) {
            return false;
        }
    }
    return true;
};

/**
 * @brief check if an axis aligned lies completely inside a frustum
 * 
 * @param f 
 * @param b 
 * @return true 
 * @return false 
 */
inline bool intersect(const Frustum& f, const AABB& b) noexcept {
    // code partly created by Copilot https://copilot.github.com/
    glm::vec3 points_to_test[8] = {
        b.pMin,
        b.pMax,
        {b.pMin.x, b.pMin.y, b.pMax.z},
        {b.pMin.x, b.pMax.y, b.pMin.z},
        {b.pMin.x, b.pMax.y, b.pMax.z},
        {b.pMax.x, b.pMin.y, b.pMin.z},
        {b.pMax.x, b.pMin.y, b.pMax.z},
        {b.pMax.x, b.pMax.y, b.pMin.z},
    };

    // for each plane, check if all points are outside
    for (int i = 0; i < 6; ++i) {
        int out = 0;
        for (int j = 0; j < 8; ++j) {
            // planes are ordered inward, so if the distance is negative, the point is outside
            if (f.planes[i].distance_from_plane(points_to_test[j]) < 0) {
                out++;
            }
        }
        // all points are outside of the frustum plane
        if (out == 8) {
            return false;
        }
    }
    return true;
}


} // namespace ev2

#endif // EV2_GEOMETRY_H