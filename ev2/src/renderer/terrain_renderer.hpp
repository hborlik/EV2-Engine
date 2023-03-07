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
#include <renderer/shader.hpp>
#include <renderer/material.hpp>
#include <image.hpp>

namespace ev2::renderer {

struct OpenGLManager;

class TerrainRenderer {
public:
    TerrainRenderer();

    ~TerrainRenderer();

    void init(const RenderState& state, const ShaderPreprocessor& pre);

    void render(const RenderState& state);

    bool load_textures();

    bool load_buffers(const RenderState& state);

    bool load_vaos();

    bool load_programs(const RenderState& state, const ShaderPreprocessor& pre);

    bool load_queries();

    // world space x, y to height on terrain
    float height_query(float x, float y) const;

private:
    std::unique_ptr<OpenGLManager> m_glmanager;

    std::unique_ptr<Image> m_heightmap;
    glm::mat4 m_model;

    Ref<Material> m_terrain_mat;
};

} // namespace ev2::renderer

#endif // EV2_RENDERER_TERRAIN_H