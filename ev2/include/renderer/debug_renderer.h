/**
 * @file debug_renderer.h
 * @brief 
 * @date 2022-05-23
 * 
 * 
 */
#ifndef EV2_DEBUG_RENDERER_H
#define EV2_DEBUG_RENDERER_H

#include <renderer/ev_gl.h>
#include <renderer/camera.h>

namespace ev2::renderer {

class DebugRenderer {
public:

    void init();

    void draw_bounding_box(const Camera& camera, const glm::vec3& bmin, const glm::vec3& bmax);
};

}

#endif // EV2_DEBUG_RENDERER_H