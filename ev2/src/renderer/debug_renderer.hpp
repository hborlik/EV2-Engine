/**
 * @file debug_renderer.h
 * @brief 
 * @date 2022-05-23
 * 
 * 
 */
#ifndef EV2_DEBUG_RENDERER_H
#define EV2_DEBUG_RENDERER_H

#include "renderer/ev_gl.hpp"
#include "renderer/camera.hpp"
#include "renderer/render_state.hpp"

namespace ev2::renderer {

class DebugRenderer {
public:

    void init();

    void render(const RenderState& state);

    void draw_bounding_box(const RenderState& state, const glm::vec3& bmin, const glm::vec3& bmax);
};

}

#endif // EV2_DEBUG_RENDERER_H