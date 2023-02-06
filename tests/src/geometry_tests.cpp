#include <geometry.h>

void test_intersect_frustum_boundingbox() {
    using namespace glm;
    using namespace ev2;

    const vec3 min = vec3(-1.f, -1.f, -1.f);
    const vec3 max = vec3(1.f, 1.f, 1.f);
    const AABB box = AABB(min, max);

    const vec3 eye = vec3(0.f, 0.f, 0.f);
    const vec3 center = vec3(0.f, 0.f, -1.f);
    const vec3 up = vec3(0.f, 1.f, 0.f);
    const mat4 view = lookAt(eye, center, up);

    const float fov = 90.f;
    const float aspect = 1.f;
    const float near = 0.1f;
    const float far = 100.f;
    const mat4 proj = perspective(radians(fov), aspect, near, far);

    const mat4 viewproj = proj * view;

    const Frustum frustum = Frustum(viewproj);

    const bool b_intersect = intersect(frustum, box);

    assert(b_intersect);
}

void test_no_intersect_frustum_boundingbox() {
    using namespace glm;
    using namespace ev2;

    const vec3 min = vec3(0.f, 0.f, 0.f);
    const vec3 max = vec3(1.f, 1.f, 1.f);
    const AABB box = AABB(min, max);

    const vec3 eye = vec3(0.f, 0.f, -2.f);
    const vec3 center = vec3(0.f, 0.f, -1.f);
    const vec3 up = vec3(0.f, 1.f, 0.f);
    const mat4 view = lookAt(eye, center, up);

    const float fov = 90.f;
    const float aspect = 1.f;
    const float near = 0.1f;
    const float far = 100.f;
    const mat4 proj = perspective(radians(fov), aspect, near, far);

    const mat4 view_proj = proj * view;

    const Frustum frustum = Frustum(view_proj);

    const bool b_intersect = intersect(frustum, box);

    assert(!b_intersect);
}

int main() {
    test_intersect_frustum_boundingbox();
    test_no_intersect_frustum_boundingbox();
    return 0;
}