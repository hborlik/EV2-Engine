/**
 * @file gl_render_api.hpp
 * @brief 
 * @date 2023-09-20
 * 
 */
#ifndef EV2_GL_RENDER_API_HPP
#define EV2_GL_RENDER_API_HPP

#include "renderer/render_api.hpp"

namespace ev2::renderer {

class GLRenderAPI : public RenderAPI {
public:
    void init() override;

};

}

#endif // 