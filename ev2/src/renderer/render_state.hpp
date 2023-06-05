/**
 * @file render_state.hpp
 * @author Hunter Borlik
 * @brief Render state passed to different effects from the Renderer class
 * @date 2022-11-16
 * 
 * 
 */
#ifndef EV2_RENDERER_STATE_H
#define EV2_RENDERER_STATE_H

#include "renderer/opengl_renderer/texture.hpp"
#include "renderer/camera.hpp"

namespace ev2::renderer {

struct RenderState {
    const FBO* target_fbo;
    const Camera* camera;
};

}

#endif // EV2_RENDERER_STATE_H