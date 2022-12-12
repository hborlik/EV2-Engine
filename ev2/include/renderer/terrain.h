/**
 * @file terrain.h
 * @brief 
 * @date 2022-11-08
 * 
 */
#ifndef EV2_RENDERER_TERRAIN_H
#define EV2_RENDERER_TERRAIN_H

#include <memory>

#include <renderer/render_state.hpp>
#include <renderer/shader.h>
#include <image.hpp>

namespace ev2::renderer {

struct OpenGLManager;

class Terrain {
public:
    Terrain();

    // note destructor is needed for std::unique_ptr<OpenGLManager> destruction
    ~Terrain();

    void init(const RenderState& state, const ShaderPreprocessor& pre);

    void render(const RenderState& state);

    bool load_textures();

    bool load_buffers(const RenderState& state);

    bool load_vaos();

    bool load_programs(const RenderState& state, const ShaderPreprocessor& pre);

    bool load_queries();

    float height_query(float x, float y) const;

private:
    std::unique_ptr<OpenGLManager> m_glmanager;

    std::unique_ptr<Image> m_heightmap;
};

}

#endif // EV2_RENDERER_TERRAIN_H