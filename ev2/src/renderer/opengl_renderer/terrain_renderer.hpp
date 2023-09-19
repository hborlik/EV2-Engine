/**
 * @file terrain.h
 * @brief 
 * @date 2022-11-08
 * 
 */
#ifndef EV2_RENDERER_TERRAIN_H
#define EV2_RENDERER_TERRAIN_H

#ifdef ENABLE_TERRAIN

#include "evpch.hpp"

#include "renderer/renderer.hpp"
#include "renderer/material.hpp"
#include "io/image.hpp"

#include "renderer/opengl_renderer/gl_renderer.hpp"
#include "renderer/opengl_renderer/render_state.hpp"
#include "renderer/opengl_renderer/gl_shader.hpp"

namespace ev2::renderer {

struct OpenGLManager;

class TerrainRenderer : public RenderPass {
public:
    TerrainRenderer();

    ~TerrainRenderer();

    void init(const RenderState& state, const PreprocessorSettings& pre) override;

    void render(const RenderState& state) override;

    bool load_textures();

    bool load_buffers(const RenderState& state);

    bool load_vaos();

    bool load_programs(const RenderState& state, const PreprocessorSettings& pre);

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

#endif // ENABLE_TERRAIN

#endif // EV2_RENDERER_TERRAIN_H