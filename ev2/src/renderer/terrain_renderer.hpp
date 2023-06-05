/**
 * @file terrain.h
 * @brief 
 * @date 2022-11-08
 * 
 */
#ifndef EV2_RENDERER_TERRAIN_H
#define EV2_RENDERER_TERRAIN_H

#include "evpch.hpp"

#include "renderer.hpp"
#include "renderer/render_state.hpp"
#include "renderer/opengl_renderer/shader.hpp"
#include "renderer/material.hpp"
#include "io/image.hpp"

namespace ev2::renderer {

struct OpenGLManager;

class TerrainRenderer : public RenderPass {
public:
    TerrainRenderer();

    ~TerrainRenderer();

    void init(const RenderState& state, const ShaderPreprocessor& pre) override;

    void render(const RenderState& state) override;

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

    std::shared_ptr<Material> m_terrain_mat;
};

} // namespace ev2::renderer

#endif // EV2_RENDERER_TERRAIN_H