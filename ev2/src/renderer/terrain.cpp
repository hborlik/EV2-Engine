#include <renderer/terrain.h>

#include <stdio.h>
#include <map>
#include <vector>

#include <renderer/ev_gl.h>
#include <renderer/buffer.h>
#include <renderer/camera.h>
#include <renderer/shader.h>
#include <image.hpp>
#include <resource.h>

#define CBT_IMPLEMENTATION
#include <cbt.h>

#define LEB_IMPLEMENTATION
#include <leb.h>

#define LOG(x) printf(x)

template<typename T>
constexpr auto sqr(T a) noexcept {return a * a;}

#define TERRAIN
#ifdef TERRAIN

namespace ev2::renderer {

enum { FRAMEBUFFER_BACK, FRAMEBUFFER_SCENE, FRAMEBUFFER_COUNT };
enum { STREAM_TERRAIN_VARIABLES, STREAM_COUNT };
enum {
    VERTEXARRAY_EMPTY,
    VERTEXARRAY_SPHERE,
    VERTEXARRAY_MESHLET,
    VERTEXARRAY_COUNT
};
enum {
    BUFFER_LEB,
    BUFFER_TERRAIN_DRAW,
    BUFFER_TERRAIN_DRAW_MS,
    BUFFER_MESHLET_VERTICES,
    BUFFER_MESHLET_INDEXES,
    BUFFER_TERRAIN_DRAW_CS,     // compute shader path only
    BUFFER_TERRAIN_DISPATCH_CS, // compute shader path only
    BUFFER_SPHERE_VERTICES,
    BUFFER_SPHERE_INDEXES,
    BUFFER_CBT_NODE_COUNT,

    BUFFER_COUNT
};
enum {
    TEXTURE_CBUF,
    TEXTURE_ZBUF,
    TEXTURE_DMAP,
    TEXTURE_SMAP,
    TEXTURE_ATMOSPHERE_IRRADIANCE,
    TEXTURE_ATMOSPHERE_INSCATTER,
    TEXTURE_ATMOSPHERE_TRANSMITTANCE,
    TEXTURE_ROCK_DMAP,
    TEXTURE_ROCK_SMAP,

    TEXTURE_COUNT
};
enum {
    PROGRAM_VIEWER,
    PROGRAM_SPLIT,
    PROGRAM_MERGE,
    PROGRAM_RENDER_ONLY,    // compute shader path only
    PROGRAM_TOPVIEW,
    PROGRAM_LEB_REDUCTION,
    PROGRAM_LEB_REDUCTION_PREPASS,
    PROGRAM_BATCH,
    PROGRAM_SKY,
    PROGRAM_CBT_NODE_COUNT,

    PROGRAM_COUNT
};
enum {
    QUERY_NODE_COUNT,

    QUERY_COUNT
};
enum {
    UNIFORM_VIEWER_FRAMEBUFFER_SAMPLER,
    UNIFORM_VIEWER_GAMMA,

    UNIFORM_TERRAIN_DMAP_SAMPLER,
    UNIFORM_TERRAIN_SMAP_SAMPLER,
    UNIFORM_TERRAIN_DMAP_ROCK_SAMPLER,
    UNIFORM_TERRAIN_SMAP_ROCK_SAMPLER,
    UNIFORM_TERRAIN_DMAP_FACTOR,
    UNIFORM_TERRAIN_TARGET_EDGE_LENGTH,
    UNIFORM_TERRAIN_LOD_FACTOR,
    UNIFORM_TERRAIN_MIN_LOD_VARIANCE,
    UNIFORM_TERRAIN_SCREEN_RESOLUTION,
    UNIFORM_TERRAIN_INSCATTER_SAMPLER,
    UNIFORM_TERRAIN_IRRADIANCE_SAMPLER,
    UNIFORM_TERRAIN_TRANSMITTANCE_SAMPLER,

    UNIFORM_SPLIT_DMAP_SAMPLER,
    UNIFORM_SPLIT_SMAP_SAMPLER,
    UNIFORM_SPLIT_DMAP_ROCK_SAMPLER,
    UNIFORM_SPLIT_SMAP_ROCK_SAMPLER,
    UNIFORM_SPLIT_DMAP_FACTOR,
    UNIFORM_SPLIT_TARGET_EDGE_LENGTH,
    UNIFORM_SPLIT_LOD_FACTOR,
    UNIFORM_SPLIT_MIN_LOD_VARIANCE,
    UNIFORM_SPLIT_SCREEN_RESOLUTION,
    UNIFORM_SPLIT_INSCATTER_SAMPLER,
    UNIFORM_SPLIT_IRRADIANCE_SAMPLER,
    UNIFORM_SPLIT_TRANSMITTANCE_SAMPLER,

    UNIFORM_MERGE_DMAP_SAMPLER,
    UNIFORM_MERGE_SMAP_SAMPLER,
    UNIFORM_MERGE_DMAP_ROCK_SAMPLER,
    UNIFORM_MERGE_SMAP_ROCK_SAMPLER,
    UNIFORM_MERGE_DMAP_FACTOR,
    UNIFORM_MERGE_TARGET_EDGE_LENGTH,
    UNIFORM_MERGE_LOD_FACTOR,
    UNIFORM_MERGE_MIN_LOD_VARIANCE,
    UNIFORM_MERGE_SCREEN_RESOLUTION,
    UNIFORM_MERGE_INSCATTER_SAMPLER,
    UNIFORM_MERGE_IRRADIANCE_SAMPLER,
    UNIFORM_MERGE_TRANSMITTANCE_SAMPLER,

    UNIFORM_RENDER_DMAP_SAMPLER,
    UNIFORM_RENDER_SMAP_SAMPLER,
    UNIFORM_RENDER_DMAP_ROCK_SAMPLER,
    UNIFORM_RENDER_SMAP_ROCK_SAMPLER,
    UNIFORM_RENDER_DMAP_FACTOR,
    UNIFORM_RENDER_TARGET_EDGE_LENGTH,
    UNIFORM_RENDER_LOD_FACTOR,
    UNIFORM_RENDER_MIN_LOD_VARIANCE,
    UNIFORM_RENDER_SCREEN_RESOLUTION,
    UNIFORM_RENDER_INSCATTER_SAMPLER,
    UNIFORM_RENDER_IRRADIANCE_SAMPLER,
    UNIFORM_RENDER_TRANSMITTANCE_SAMPLER,

    UNIFORM_TOPVIEW_DMAP_SAMPLER,
    UNIFORM_TOPVIEW_DMAP_FACTOR,

    UNIFORM_SKY_SUN_DIR,
    UNIFORM_SKY_CAMERA_MATRIX,
    UNIFORM_SKY_VIEW_PROJECTION_MATRIX,
    UNIFORM_SKY_INSCATTER_SAMPLER,
    UNIFORM_SKY_IRRADIANCE_SAMPLER,
    UNIFORM_SKY_TRANSMITTANCE_SAMPLER,

    UNIFORM_COUNT
};
struct OpenGLManager {

    OpenGLManager() = default;

    std::unique_ptr<Program> programs[PROGRAM_COUNT] = {nullptr};
    GLuint framebuffers[FRAMEBUFFER_COUNT] = {0};
    GLuint textures[TEXTURE_COUNT] = {0};
    GLuint vertexArrays[VERTEXARRAY_COUNT] = {0};
    GLuint buffers[BUFFER_COUNT] = {0};
    GLuint queries[QUERY_COUNT] = {0};
    GLint uniforms[UNIFORM_COUNT] = {0};
    std::unique_ptr<Buffer> streams[STREAM_COUNT] = {nullptr};
} *g_gl;


// -----------------------------------------------------------------------------
// Terrain Manager
enum { METHOD_CS, METHOD_TS, METHOD_GS, METHOD_MS };
enum { SHADING_DIFFUSE, SHADING_NORMALS, SHADING_COLOR};
struct TerrainManager {
    struct { bool displace, cull, freeze, wire, topView; } flags;
    struct {
        std::string pathToFile;
        float width, height, zMin, zMax;
        float scale;
    } dmap;
    int method;
    int shading;
    int gpuSubd;
    float primitivePixelLengthTarget;
    float minLodStdev;
    int maxDepth;
    uint32_t nodeCount;
    float size;
} g_terrain = {
    {true, true, false, false, true},
    {std::string("" "./kauai.png"),
    //  52660.0f, 52660.0f, -400.0f, 1587.0f,
    5000.0f, 5000.0f, -200.0f, 14.0f,
     1.0f},
    METHOD_CS,
    SHADING_DIFFUSE,
    3,
    7.0f,
    0.1f,
    25,
    0,
    5000.0f
};

////////////////////////////////////////////////////////////////////////////////
// Program Configuration
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// set Terrain program uniforms
float computeLodFactor(const RenderState& state)
{
    if (state.camera->get_projection_mode() == CameraProjection::RECTILINEAR) {
        float tmp = 2.0f * tan(glm::radians(state.camera->get_fov()) / 2.0f)
            / state.target_fbo->get_height() * (1 << g_terrain.gpuSubd)
            * g_terrain.primitivePixelLengthTarget;

        return -2.0f * std::log2(tmp) + 2.0f;
    } else if (state.camera->get_projection_mode() == CameraProjection::ORTHOGRAPHIC) {
        float planeSize = 2.0f * tan(glm::radians(state.camera->get_fov() / 2.0f));
        float targetSize = planeSize * g_terrain.primitivePixelLengthTarget
                         / state.target_fbo->get_height() * (1 << g_terrain.gpuSubd);

        return -2.0f * std::log2(targetSize);
    } else if (state.camera->get_projection_mode() == CameraProjection::FISHEYE) {
        float tmp = 2.0f * tan(glm::radians(state.camera->get_fov()) / 2.0f)
            / state.target_fbo->get_height() * (1 << g_terrain.gpuSubd)
            * g_terrain.primitivePixelLengthTarget;

        return -2.0f * std::log2(tmp) + 2.0f;
    }

    return 1.0f;
}

void ConfigureTerrainProgram(const RenderState& state, GLuint glp, GLuint offset)
{
    float lodFactor = computeLodFactor(state);

    glProgramUniform1f(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_DMAP_FACTOR + offset],
        g_terrain.dmap.scale);
    glProgramUniform1f(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_LOD_FACTOR + offset],
        lodFactor);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_DMAP_SAMPLER + offset],
        TEXTURE_DMAP);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_SMAP_SAMPLER + offset],
        TEXTURE_SMAP);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_DMAP_ROCK_SAMPLER + offset],
        TEXTURE_ROCK_DMAP);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_SMAP_ROCK_SAMPLER + offset],
        TEXTURE_ROCK_SMAP);
    glProgramUniform1f(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_TARGET_EDGE_LENGTH + offset],
        g_terrain.primitivePixelLengthTarget);
    glProgramUniform1f(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_MIN_LOD_VARIANCE + offset],
        sqr(g_terrain.minLodStdev / 64.0f / g_terrain.dmap.scale));
    glProgramUniform2f(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_SCREEN_RESOLUTION + offset],
        state.target_fbo->get_width(), state.target_fbo->get_height());
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_INSCATTER_SAMPLER + offset],
        TEXTURE_ATMOSPHERE_INSCATTER);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_IRRADIANCE_SAMPLER+ offset],
        TEXTURE_ATMOSPHERE_IRRADIANCE);
    glProgramUniform1i(glp,
        g_gl->uniforms[UNIFORM_TERRAIN_TRANSMITTANCE_SAMPLER + offset],
        TEXTURE_ATMOSPHERE_TRANSMITTANCE);
}

void ConfigureTerrainPrograms(const RenderState& state)
{
    ConfigureTerrainProgram(state,
                            g_gl->programs[PROGRAM_SPLIT]->getHandle(),
                            UNIFORM_SPLIT_DMAP_SAMPLER - UNIFORM_TERRAIN_DMAP_SAMPLER);
    ConfigureTerrainProgram(state,
                            g_gl->programs[PROGRAM_MERGE]->getHandle(),
                            UNIFORM_MERGE_DMAP_SAMPLER - UNIFORM_TERRAIN_DMAP_SAMPLER);
    ConfigureTerrainProgram(state,
                            g_gl->programs[PROGRAM_RENDER_ONLY]->getHandle(),
                            UNIFORM_RENDER_DMAP_SAMPLER - UNIFORM_TERRAIN_DMAP_SAMPLER);
}

// -----------------------------------------------------------------------------
// set Terrain program uniforms
void ConfigureTopViewProgram()
{
    glProgramUniform1f(g_gl->programs[PROGRAM_TOPVIEW]->getHandle(),
        g_gl->uniforms[UNIFORM_TOPVIEW_DMAP_FACTOR],
        g_terrain.dmap.scale);
    glProgramUniform1i(g_gl->programs[PROGRAM_TOPVIEW]->getHandle(),
        g_gl->uniforms[UNIFORM_TOPVIEW_DMAP_SAMPLER],
        TEXTURE_DMAP);
}

// -----------------------------------------------------------------------------
// set Atmosphere program uniforms
void ConfigureSkyProgram()
{
    glProgramUniform1i(g_gl->programs[PROGRAM_SKY]->getHandle(),
                       g_gl->uniforms[UNIFORM_SKY_INSCATTER_SAMPLER],
                       TEXTURE_ATMOSPHERE_INSCATTER);
    glProgramUniform1i(g_gl->programs[PROGRAM_SKY]->getHandle(),
                       g_gl->uniforms[UNIFORM_SKY_IRRADIANCE_SAMPLER],
                       TEXTURE_ATMOSPHERE_IRRADIANCE);
    glProgramUniform1i(g_gl->programs[PROGRAM_SKY]->getHandle(),
                       g_gl->uniforms[UNIFORM_SKY_TRANSMITTANCE_SAMPLER],
                       TEXTURE_ATMOSPHERE_TRANSMITTANCE);
}

///////////////////////////////////////////////////////////////////////////////
// Program Loading
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
/**
 * Load the Terrain Rendering Program
 *
 * This program is responsible for updating and rendering the terrain.
 */
std::unique_ptr<Program> LoadTerrainProgram(const RenderState& state, const std::string& flag, GLuint uniformOffset, const ShaderPreprocessor& pre)
{
    std::unique_ptr<Program> djp = std::make_unique<Program>("Terrain Program");

    std::string source_string;

    LOG("Loading {Terrain-Program}\n");
    if (!g_terrain.flags.freeze)
        source_string += flag;
    if (g_terrain.method == METHOD_MS) {
        source_string += "#ifndef FRAGMENT_SHADER\n#extension GL_NV_mesh_shader : require\n#endif\n";
        source_string += "#extension GL_NV_shader_thread_group : require\n";
        source_string += "#extension GL_NV_shader_thread_shuffle : require\n";
        source_string += "#extension GL_NV_gpu_shader5 : require\n";
    }
    switch (state.camera->get_projection_mode()) {
    case CameraProjection::RECTILINEAR:
        source_string += "#define PROJECTION_RECTILINEAR\n";
        break;
    case CameraProjection::FISHEYE:
        source_string += "#define PROJECTION_FISHEYE\n";
        break;
    case CameraProjection::ORTHOGRAPHIC:
        source_string += "#define PROJECTION_ORTHOGRAPHIC\n";
        break;
    default:
        break;
    }
    source_string += "#define BUFFER_BINDING_TERRAIN_VARIABLES " + std::to_string(STREAM_TERRAIN_VARIABLES) + "\n";
    source_string += "#define BUFFER_BINDING_MESHLET_VERTICES " + std::to_string(BUFFER_MESHLET_VERTICES) + "\n";
    source_string += "#define BUFFER_BINDING_MESHLET_INDEXES " + std::to_string(BUFFER_MESHLET_INDEXES) + "\n";
    source_string += "#define TERRAIN_PATCH_SUBD_LEVEL " + std::to_string(g_terrain.gpuSubd) + "\n";
    source_string += "#define TERRAIN_PATCH_TESS_FACTOR " + std::to_string(1 << g_terrain.gpuSubd) + "\n";
    if (g_terrain.shading == SHADING_DIFFUSE)
        source_string += "#define SHADING_DIFFUSE 1\n";
    else if (g_terrain.shading == SHADING_NORMALS)
        source_string += "#define SHADING_NORMALS 1\n";
    else if (g_terrain.shading == SHADING_COLOR)
        source_string += "#define SHADING_COLOR 1\n";
    if (g_terrain.flags.displace)
        source_string += "#define FLAG_DISPLACE 1\n";
    if (g_terrain.flags.cull)
        source_string += "#define FLAG_CULL 1\n";
    if (g_terrain.flags.wire)
        source_string += "#define FLAG_WIRE 1\n";

    UbiquitousShader cull_shader{};
    cull_shader.push_source_string(source_string);
    cull_shader.push_source_file("./shaders/terrain/FrustumCulling.glsl");

    cull_shader.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    cull_shader.push_source_string("#define CBT_READ_ONLY\n");

    cull_shader.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
    cull_shader.push_source_file("./submodules/libleb/glsl/leb.glsl");
    cull_shader.push_source_file("./shaders/terrain/BrunetonAtmosphere.glsl");
    cull_shader.push_source_file("./shaders/terrain/TerrainRenderCommon.glsl");
    if (g_terrain.method == METHOD_CS) {
        if (strcmp("/* thisIsAHackForComputePass */\n", flag.c_str()) == 0) {
            if (g_terrain.flags.wire) {
                cull_shader.push_source_file("./shaders/terrain/TerrainRenderCS_Wire.glsl");
            } else {
                cull_shader.push_source_file("./shaders/terrain/TerrainRenderCS.glsl");
            }
        } else {
            cull_shader.push_source_file("./shaders/terrain/TerrainUpdateCS.glsl");
        }
    } else if (g_terrain.method == METHOD_TS) {
        if (g_terrain.flags.wire) {
            cull_shader.push_source_file("./shaders/terrain/TerrainRenderTS_Wire.glsl");
        } else {
            cull_shader.push_source_file("./shaders/terrain/TerrainRenderTS.glsl");
        }
    } else if (g_terrain.method == METHOD_GS) {
        int subdLevel = g_terrain.gpuSubd;

        if (g_terrain.flags.wire) {
            int vertexCnt = 3 << (2 * subdLevel);

            cull_shader.push_source_string("#define MAX_VERTICES " + std::to_string(vertexCnt) + "\n");
            cull_shader.push_source_file("./shaders/terrain/TerrainRenderGS_Wire.glsl");
        } else {
            int vertexCnt = subdLevel == 0 ? 3 : 4 << (2 * subdLevel - 1);

            cull_shader.push_source_string("#define MAX_VERTICES " + std::to_string(vertexCnt) + "\n");
            cull_shader.push_source_file("./shaders/terrain/TerrainRenderGS.glsl");
        }
    } else if (g_terrain.method == METHOD_MS) {
        cull_shader.push_source_file("./shaders/terrain/TerrainRenderMS.glsl");
    }

    for (auto& shader : cull_shader.get_shader_stages(450)) {
        if (!shader->compile(pre))
            throw shader_error{"TerrainProgram", "Failed to compile"};
        djp->attachShader(shader.get());
    }
    djp->link();

    g_gl->uniforms[UNIFORM_TERRAIN_DMAP_FACTOR + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_DmapFactor");
    g_gl->uniforms[UNIFORM_TERRAIN_LOD_FACTOR + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_LodFactor");
    g_gl->uniforms[UNIFORM_TERRAIN_DMAP_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_DmapSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_SMAP_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_SmapSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_DMAP_ROCK_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_DmapRockSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_SMAP_ROCK_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_SmapRockSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_TARGET_EDGE_LENGTH + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_TargetEdgeLength");
    g_gl->uniforms[UNIFORM_TERRAIN_MIN_LOD_VARIANCE + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_MinLodVariance");
    g_gl->uniforms[UNIFORM_TERRAIN_SCREEN_RESOLUTION + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "u_ScreenResolution");
    g_gl->uniforms[UNIFORM_TERRAIN_IRRADIANCE_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "skyIrradianceSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_INSCATTER_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "inscatterSampler");
    g_gl->uniforms[UNIFORM_TERRAIN_TRANSMITTANCE_SAMPLER + uniformOffset] =
        glGetUniformLocation(djp->getHandle(), "transmittanceSampler");

    ConfigureTerrainProgram(state, djp->getHandle(), uniformOffset);

    assert(glGetError() == GL_NO_ERROR);
    return std::move(djp); 
}

bool LoadTerrainPrograms(const RenderState& state, const ShaderPreprocessor& pre)
{
    bool v = true;

    if (v) v = v && (g_gl->programs[PROGRAM_SPLIT] = LoadTerrainProgram(state,
                                       "#define FLAG_SPLIT 1\n",
                                       UNIFORM_SPLIT_DMAP_FACTOR - UNIFORM_TERRAIN_DMAP_FACTOR,
                                       pre));
    if (v) v = v && (g_gl->programs[PROGRAM_MERGE] = LoadTerrainProgram(state,
                                       "#define FLAG_MERGE 1\n",
                                       UNIFORM_MERGE_DMAP_FACTOR - UNIFORM_TERRAIN_DMAP_FACTOR,
                                       pre));
    if (v) v = v && (g_gl->programs[PROGRAM_RENDER_ONLY] = LoadTerrainProgram(state,
                                       "/* thisIsAHackForComputePass */\n",
                                       UNIFORM_RENDER_DMAP_FACTOR - UNIFORM_TERRAIN_DMAP_FACTOR,
                                       pre));

    return v;
}

// -----------------------------------------------------------------------------
/**
 * Load the Reduction Program
 *
 * This program is responsible for precomputing a reduction for the
 * subdivision tree. This allows to locate the i-th bit in a bitfield of
 * size N in log(N) operations.
 */
bool LoadLebReductionProgram(const ShaderPreprocessor& pre)
{
    UbiquitousShader usp{};
    auto& djp = g_gl->programs[PROGRAM_LEB_REDUCTION] = std::make_unique<Program>("LebReductionProgram");

    LOG("Loading {Reduction-Program}\n");
    usp.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
    usp.push_source_file("./submodules/libcbt/glsl/cbt_SumReduction.glsl");
    usp.push_source_string("#ifdef COMPUTE_SHADER\n#endif");

    for (auto& shader : usp.get_shader_stages(450)) {
        if (!shader->compile(pre))
            return {};
        djp->attachShader(shader.get());
    }
    djp->link();

    return (glGetError() == GL_NO_ERROR);
}

bool LoadLebReductionPrepassProgram(const ShaderPreprocessor& pre)
{
    UbiquitousShader usp{};
    auto& djp = g_gl->programs[PROGRAM_LEB_REDUCTION_PREPASS] = std::make_unique<Program>("LebReductionPrepass");

    LOG("Loading {Reduction-Prepass-Program}\n");
    usp.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
    usp.push_source_file("./submodules/libcbt/glsl/cbt_SumReductionPrepass.glsl");
    usp.push_source_string("#ifdef COMPUTE_SHADER\n#endif");
    for (auto& shader : usp.get_shader_stages(450)) {
        if (!shader->compile(pre))
            return {};
        djp->attachShader(shader.get());
    }
    djp->link();

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load the Batch Program
 *
 * This program is responsible for preparing an indirect draw call
 */
bool LoadBatchProgram(const ShaderPreprocessor& pre)
{
    UbiquitousShader usp{};
    auto& djp = g_gl->programs[PROGRAM_BATCH] = std::make_unique<Program>("BatchProgram");

    LOG("Loading {Batch-Program}\n");
    if (GLAD_GL_ARB_shader_atomic_counter_ops) {
        usp.push_source_string("#extension GL_ARB_shader_atomic_counter_ops : require\n");
        usp.push_source_string("#define ATOMIC_COUNTER_EXCHANGE_ARB 1\n");
    } else if (GLAD_GL_AMD_shader_atomic_counter_ops) {
        usp.push_source_string("#extension GL_AMD_shader_atomic_counter_ops : require\n");
        usp.push_source_string("#define ATOMIC_COUNTER_EXCHANGE_AMD 1\n");
    }
    if (g_terrain.method == METHOD_MS) {
        usp.push_source_string("#define FLAG_MS 1\n");
        usp.push_source_string("#define BUFFER_BINDING_DRAW_MESH_TASKS_INDIRECT_COMMAND " + std::to_string(BUFFER_TERRAIN_DRAW_MS) + "\n");
    }
    if (g_terrain.method == METHOD_CS) {
        usp.push_source_string("#define FLAG_CS 1\n");
        usp.push_source_string("#define BUFFER_BINDING_DRAW_ELEMENTS_INDIRECT_COMMAND " + std::to_string(BUFFER_TERRAIN_DRAW_CS) + "\n");
        usp.push_source_string("#define BUFFER_BINDING_DISPATCH_INDIRECT_COMMAND " + std::to_string(BUFFER_TERRAIN_DISPATCH_CS) + "\n");
        usp.push_source_string("#define MESHLET_INDEX_COUNT " + std::to_string(3 << (2 * g_terrain.gpuSubd)) + "\n");
    }
    usp.push_source_string("#define LEB_BUFFER_COUNT 1\n");
    usp.push_source_string("#define BUFFER_BINDING_LEB " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_string("#define BUFFER_BINDING_DRAW_ARRAYS_INDIRECT_COMMAND " + std::to_string(BUFFER_TERRAIN_DRAW) + "\n");
#if 0
    djgp_push_file(djp, PATH_TO_SRC_DIRECTORY "./shaders/terrain/LongestEdgeBisection.glsl");
#else
    usp.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_string("#define CBT_READ_ONLY\n");
    usp.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
#endif
    usp.push_source_file("./shaders/terrain/TerrainBatcher.glsl");
    for (auto& shader : usp.get_shader_stages(450)) {
        if (!shader->compile(pre))
            return {};
        djp->attachShader(shader.get());
    }
    djp->link();

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load the Top View Program
 *
 * This program is responsible for rendering the terrain in a top view fashion
 */
bool LoadTopViewProgram(const ShaderPreprocessor& pre)
{
    UbiquitousShader usp{};
    auto& djp = g_gl->programs[PROGRAM_TOPVIEW] = std::make_unique<Program>("BatchProgram");

    LOG("Loading {Top-View-Program}\n");
    if (g_terrain.flags.displace)
        usp.push_source_string("#define FLAG_DISPLACE 1\n");
    usp.push_source_string("#define TERRAIN_PATCH_SUBD_LEVEL " + std::to_string(g_terrain.gpuSubd) + "\n");
    usp.push_source_string("#define TERRAIN_PATCH_TESS_FACTOR " + std::to_string(1 << g_terrain.gpuSubd) + "\n");
    usp.push_source_string("#define BUFFER_BINDING_TERRAIN_VARIABLES " + std::to_string(STREAM_TERRAIN_VARIABLES) + "\n");
    usp.push_source_string("#define LEB_BUFFER_COUNT 1\n");
    usp.push_source_string("#define BUFFER_BINDING_LEB " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_file("./shaders/terrain/FrustumCulling.glsl");
    usp.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_string("#define CBT_READ_ONLY\n");
    usp.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
    usp.push_source_file("./submodules/libleb/glsl/leb.glsl");
    usp.push_source_file("./shaders/terrain/TerrainRenderCommon.glsl");
    usp.push_source_file("./shaders/terrain/TerrainTopView.glsl");
    for (auto& shader : usp.get_shader_stages(450)) {
        if (!shader->compile(pre))
            return {};
        djp->attachShader(shader.get());
    }
    djp->link();

    g_gl->uniforms[UNIFORM_TOPVIEW_DMAP_FACTOR] =
        glGetUniformLocation(djp->getHandle(), "u_DmapFactor");
    g_gl->uniforms[UNIFORM_TOPVIEW_DMAP_SAMPLER] =
        glGetUniformLocation(djp->getHandle(), "u_DmapSampler");

    ConfigureTopViewProgram();

    return (glGetError() == GL_NO_ERROR);
}


// -----------------------------------------------------------------------------
/**
 * Load the Node Count Program
 *
 * This program is responsible for retrieving the number of nodes in the CBT
 */
bool LoadCbtNodeCountProgram(const ShaderPreprocessor& pre)
{
    UbiquitousShader usp{};
    auto& djp = g_gl->programs[PROGRAM_CBT_NODE_COUNT] = std::make_unique<Program>("BatchProgram");

    LOG("Loading {Cbt-Node-Count-Program}\n");
    
    usp.push_source_string("#define CBT_NODE_COUNT_BUFFER_BINDING " + std::to_string(BUFFER_CBT_NODE_COUNT) + "\n");
    usp.push_source_string("#define CBT_HEAP_BUFFER_BINDING " + std::to_string(BUFFER_LEB) + "\n");
    usp.push_source_string("#define CBT_READ_ONLY\n");
    usp.push_source_file("./submodules/libcbt/glsl/cbt.glsl");
    usp.push_source_file("./shaders/terrain/NodeCount.glsl");
    usp.push_source_string("#ifdef COMPUTE_SHADER\n#endif\n");
    for (auto& shader : usp.get_shader_stages(450)) {
        if (!shader->compile(pre))
            return {};
        djp->attachShader(shader.get());
    }
    djp->link();

    return (glGetError() == GL_NO_ERROR);
}


// -----------------------------------------------------------------------------
/**
 * Load All Programs
 *
 */
bool LoadPrograms(const RenderState& state, const ShaderPreprocessor& pre)
{
    bool v = true;

    // if (v) v &= LoadViewerProgram(pre);
    if (v) v &= LoadTerrainPrograms(state, pre);
    if (v) v &= LoadLebReductionProgram(pre);
    if (v) v &= LoadLebReductionPrepassProgram(pre);
    if (v) v &= LoadBatchProgram(pre);
    if (v) v &= LoadTopViewProgram(pre);
    // if (v) v &= LoadSkyProgram();
    if (v) v &= LoadCbtNodeCountProgram(pre);

    return v;
}

////////////////////////////////////////////////////////////////////////////////
// Texture Loading
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
/**
 * Load the Scene Framebuffer Textures
 *
 * Depending on the scene framebuffer AA mode, this function Load 2 or
 * 3 textures. In FSAA mode, two RGBA16F and one DEPTH24_STENCIL8 textures
 * are created. In other modes, one RGBA16F and one DEPTH24_STENCIL8 textures
 * are created.
 */
// bool LoadSceneFramebufferTexture()
// {
//     if (glIsTexture(g_gl->textures[TEXTURE_CBUF]))
//         glDeleteTextures(1, &g_gl->textures[TEXTURE_CBUF]);
//     if (glIsTexture(g_gl->textures[TEXTURE_ZBUF]))
//         glDeleteTextures(1, &g_gl->textures[TEXTURE_ZBUF]);
//     glGenTextures(1, &g_gl->textures[TEXTURE_ZBUF]);
//     glGenTextures(1, &g_gl->textures[TEXTURE_CBUF]);

//     switch (g_framebuffer.aa) {
//     case AA_NONE:
//         LOG("Loading {Z-Buffer-Framebuffer-Texture}\n");
//         glActiveTexture(GL_TEXTURE0 + TEXTURE_ZBUF);
//         glBindTexture(GL_TEXTURE_2D, g_gl->textures[TEXTURE_ZBUF]);
//         glTexStorage2D(GL_TEXTURE_2D,
//             1,
//             GL_DEPTH24_STENCIL8,
//             g_framebuffer.w,
//             g_framebuffer.h);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//         LOG("Loading {Color-Buffer-Framebuffer-Texture}\n");
//         glActiveTexture(GL_TEXTURE0 + TEXTURE_CBUF);
//         glBindTexture(GL_TEXTURE_2D, g_gl->textures[TEXTURE_CBUF]);
//         glTexStorage2D(GL_TEXTURE_2D,
//             1,
//             GL_RGBA32F,
//             g_framebuffer.w,
//             g_framebuffer.h);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//         break;
//     case AA_MSAA2:
//     case AA_MSAA4:
//     case AA_MSAA8:
//     case AA_MSAA16: {
//         int samples = 1 << g_framebuffer.aa;

//         int maxSamples;
//         int maxSamplesDepth;
//         //glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxSamples); //Wrong enum !
//         glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxSamples);
//         glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxSamplesDepth);
//         maxSamples = maxSamplesDepth < maxSamples ? maxSamplesDepth : maxSamples;

//         if (samples > maxSamples) {
//             LOG("note: MSAA is %ix\n", maxSamples);
//             samples = maxSamples;
//         }
//         LOG("Loading {Scene-MSAA-Z-Framebuffer-Texture}\n");
//         glActiveTexture(GL_TEXTURE0 + TEXTURE_ZBUF);
//         glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_gl->textures[TEXTURE_ZBUF]);
//         glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
//                                   samples,
//                                   GL_DEPTH24_STENCIL8,
//                                   g_framebuffer.w,
//                                   g_framebuffer.h,
//                                   g_framebuffer.msaa.fixed);

//         LOG("Loading {Scene-MSAA-RGBA-Framebuffer-Texture}\n");
//         glActiveTexture(GL_TEXTURE0 + TEXTURE_CBUF);
//         glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_gl->textures[TEXTURE_CBUF]);
//         glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
//                                   samples,
//                                   GL_RGBA32F,
//                                   g_framebuffer.w,
//                                   g_framebuffer.h,
//                                   g_framebuffer.msaa.fixed);
//     } break;
//     }
//     glActiveTexture(GL_TEXTURE0);

//     return (glGetError() == GL_NO_ERROR);
// }

// -----------------------------------------------------------------------------
/**
 * Load the Normal Texture Map
 *
 * This Loads an RG32F texture used as a slope map
 */
void LoadNmapTexture16(int smapID, const Image& dmap)
{
    int w = dmap.get_width();
    int h = dmap.get_height();
    const uint16_t *texels = (const uint16_t *)dmap.data();
    int mipcnt = mip_count(w, h, 1);

    std::vector<float> smap(w * h * 2);

    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i) {
            int i1 = std::max(0, i - 1);
            int i2 = std::min(w - 1, i + 1);
            int j1 = std::max(0, j - 1);
            int j2 = std::min(h - 1, j + 1);
            uint16_t px_l = texels[i1 + w * j]; // in [0,2^16-1]
            uint16_t px_r = texels[i2 + w * j]; // in [0,2^16-1]
            uint16_t px_b = texels[i + w * j1]; // in [0,2^16-1]
            uint16_t px_t = texels[i + w * j2]; // in [0,2^16-1]
            float z_l = (float)px_l / 65535.0f; // in [0, 1]
            float z_r = (float)px_r / 65535.0f; // in [0, 1]
            float z_b = (float)px_b / 65535.0f; // in [0, 1]
            float z_t = (float)px_t / 65535.0f; // in [0, 1]
            float slope_x = (float)w * 0.5f * (z_r - z_l);
            float slope_y = (float)h * 0.5f * (z_t - z_b);

            smap[    2 * (i + w * j)] = slope_x;
            smap[1 + 2 * (i + w * j)] = slope_y;
        }

    if (glIsTexture(g_gl->textures[smapID]))
        glDeleteTextures(1, &g_gl->textures[smapID]);

    glGenTextures(1, &g_gl->textures[smapID]);
    glActiveTexture(GL_TEXTURE0 + smapID);
    glBindTexture(GL_TEXTURE_2D, g_gl->textures[smapID]);
    glTexStorage2D(GL_TEXTURE_2D, mipcnt, GL_RG32F, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RG, GL_FLOAT, &smap[0]);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
}

void LoadNmapTexture8(int smapID, const Image& dmap)
{
    int w = dmap.get_width();
    int h = dmap.get_height();
    const uint8_t *texels = (const uint8_t *)dmap.data();
    int mipcnt = mip_count(w, h, 1);

    std::vector<float> smap(w * h * 2);

    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i) {
            int i1 = std::max(0, i - 1);
            int i2 = std::min(w - 1, i + 1);
            int j1 = std::max(0, j - 1);
            int j2 = std::min(h - 1, j + 1);
            uint8_t px_l = texels[i1 + w * j]; // in [0,2^16-1]
            uint8_t px_r = texels[i2 + w * j]; // in [0,2^16-1]
            uint8_t px_b = texels[i + w * j1]; // in [0,2^16-1]
            uint8_t px_t = texels[i + w * j2]; // in [0,2^16-1]
            float z_l = (float)px_l / 255.0f; // in [0, 1]
            float z_r = (float)px_r / 255.0f; // in [0, 1]
            float z_b = (float)px_b / 255.0f; // in [0, 1]
            float z_t = (float)px_t / 255.0f; // in [0, 1]
            float slope_x = (float)w * 0.5f * (z_r - z_l);
            float slope_y = (float)h * 0.5f * (z_t - z_b);

            smap[    2 * (i + w * j)] = slope_x;
            smap[1 + 2 * (i + w * j)] = slope_y;
        }

    if (glIsTexture(g_gl->textures[smapID]))
        glDeleteTextures(1, &g_gl->textures[smapID]);

    glGenTextures(1, &g_gl->textures[smapID]);
    glActiveTexture(GL_TEXTURE0 + smapID);
    glBindTexture(GL_TEXTURE_2D, g_gl->textures[smapID]);
    glTexStorage2D(GL_TEXTURE_2D, mipcnt, GL_RG32F, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RG, GL_FLOAT, &smap[0]);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT);
    glActiveTexture(GL_TEXTURE0);
}

// -----------------------------------------------------------------------------
/**
 * Load the Displacement Texture
 *
 * This Loads an R16 texture used as a displacement map
 */
bool LoadDmapTexture16(int dmapID, int smapID, const std::string& pathToFile)
{
    LOG("Loading {Dmap-Texture}\n");
    std::unique_ptr<Image> image = load_image_16(pathToFile);

    int w = image->get_width();
    int h = image->get_width();
    const uint16_t *texels = (const uint16_t*)image->data();
    int mipcnt = mip_count(w, h);
    std::vector<uint16_t> dmap(w * h * 2);

    for (int j = 0; j < h; ++j)
    for (int i = 0; i < w; ++i) {
        uint16_t z = texels[i + w * j]; // in [0,2^16-1]
        float zf = float(z) / float((1 << 16) - 1); // normalized height
        uint16_t z2 = zf * zf * ((1 << 16) - 1);

        dmap[    2 * (i + w * j)] = z;
        dmap[1 + 2 * (i + w * j)] = z2;
    }

    // Load nmap from dmap
    LoadNmapTexture16(smapID, *image.get());

    glActiveTexture(GL_TEXTURE0 + dmapID);
    if (glIsTexture(g_gl->textures[dmapID]))
        glDeleteTextures(1, &g_gl->textures[dmapID]);

    glGenTextures(1, &g_gl->textures[dmapID]);
    glActiveTexture(GL_TEXTURE0 + dmapID);
    glBindTexture(GL_TEXTURE_2D, g_gl->textures[dmapID]);
    glTexStorage2D(GL_TEXTURE_2D, mipcnt, GL_RG16, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RG, GL_UNSIGNED_SHORT, &dmap[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);

    return (glGetError() == GL_NO_ERROR);
}

bool LoadDmapTexture()
{
    std::string path = Engine::get().asset_path / g_terrain.dmap.pathToFile;
    std::cout << "Loading Dmap Texture: " << path << "\n";
    if (!g_terrain.dmap.pathToFile.empty()) {
        return LoadDmapTexture16(TEXTURE_DMAP,
                                 TEXTURE_SMAP,
                                 path);
    }

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load All Textures
 */
bool Terrain::load_textures() {
    bool v = true;

    // if (v) v &= LoadSceneFramebufferTexture();
    if (v) v &= LoadDmapTexture();
    // if (v) v &= LoadBrunetonAtmosphereTextures();

    return v;
}

////////////////////////////////////////////////////////////////////////////////
// Buffer Loading
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
/**
 * Load Terrain Variables UBO
 *
 * This procedure updates the transformation matrices; it is updated each frame.
 */
bool LoadTerrainVariables(const Camera& m_camera)
{
    static bool first = true;
    struct PerFrameVariables {
        glm::mat4 model,                // 16
                  modelView,            // 16
                  view,                 // 16
                  camera,               // 16
                  viewProjection,       // 16
                  modelViewProjection;  // 16
        glm::vec4 frustum[6];           // 24
        glm::vec4 align[2];             // 8
    } variables;

    if (first) {
        g_gl->streams[STREAM_TERRAIN_VARIABLES] = std::make_unique<Buffer>(gl::BindingTarget::UNIFORM, gl::Usage::STREAM_DRAW);
        first = false;
    }

    // extract view and projection matrices
    glm::mat4 projection = m_camera.get_projection();

    float width = g_terrain.dmap.width;
    float height = g_terrain.dmap.height;
    float zMin = g_terrain.dmap.zMin;
    float zMax = g_terrain.dmap.zMax;
    glm::vec3 scale = glm::vec3(width, zMax - zMin, height);
    // m_camera.get_view
    glm::mat4 viewInv = m_camera.get_view_inv();
    glm::mat4 view = glm::inverse(viewInv);
    glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(-width / 2.0f, zMin, +height / 2.0f))
            * glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale))
            * glm::rotate(glm::identity<glm::mat4>(), -(float)M_PI / 2.0f, glm::vec3(1, 0, 0));

    // set transformations (column-major)
    variables.model = model;
    //variables.normal = dja::transpose(dja::inverse(variables.model));
    variables.view = view;
    variables.camera = viewInv;
    variables.modelView = view * model;
    variables.modelViewProjection = projection * view * model;
    variables.viewProjection = projection * view;
    //variables.projection = dja::transpose(projection);

    // extract frustum planes
    glm::mat4 mvp = variables.modelViewProjection;
    
    m_camera.extract_frustum().copy_to_array(variables.frustum);

    // upLoad to GPU
    g_gl->streams[STREAM_TERRAIN_VARIABLES]->copy_data(sizeof(PerFrameVariables), (const void *)&variables);

    g_gl->streams[STREAM_TERRAIN_VARIABLES]->bind_range(STREAM_TERRAIN_VARIABLES, sizeof(PerFrameVariables));

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load LEB Buffer
 *
 * This procedure initializes the subdivision buffer.
 */
bool LoadLebBuffer()
{
    cbt_Tree *cbt = cbt_CreateAtDepth(g_terrain.maxDepth, 1);

    LOG("Loading {Subd-Buffer}\n");
    if (glIsBuffer(g_gl->buffers[BUFFER_LEB]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_LEB]);
    glGenBuffers(1, &g_gl->buffers[BUFFER_LEB]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_gl->buffers[BUFFER_LEB]);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 cbt_HeapByteSize(cbt),
                 cbt_GetHeap(cbt),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_LEB,
                     g_gl->buffers[BUFFER_LEB]);

    cbt_Release(cbt);

    return (glGetError() == GL_NO_ERROR);
}


// -----------------------------------------------------------------------------
/**
 * Load CBT Node Count Buffer
 *
 * This procedure initializes a buffer that stores the number of nodes in the CBT.
 */
bool LoadCbtNodeCountBuffer()
{
    LOG("Loading {Cbt-Node-Count-Buffer}\n");
    if (glIsBuffer(g_gl->buffers[BUFFER_CBT_NODE_COUNT]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_CBT_NODE_COUNT]);
    glGenBuffers(1, &g_gl->buffers[BUFFER_CBT_NODE_COUNT]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_gl->buffers[BUFFER_CBT_NODE_COUNT]);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                    sizeof(int32_t),
                    NULL,
                    GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_CBT_NODE_COUNT,
                     g_gl->buffers[BUFFER_CBT_NODE_COUNT]);

    return (glGetError() == GL_NO_ERROR);
}


// -----------------------------------------------------------------------------
/**
 * Load the indirect command buffer
 *
 * This procedure initializes the subdivision buffer.
 */
bool LoadRenderCmdBuffer()
{
    uint32_t drawArraysCmd[8] = {2, 1, 0, 0, 0, 0, 0, 0};
    uint32_t drawMeshTasksCmd[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    uint32_t drawElementsCmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t dispatchCmd[8] = {2, 1, 1, 0, 0, 0, 0, 0};

    if (glIsBuffer(g_gl->buffers[BUFFER_TERRAIN_DRAW]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW]);

    if (glIsBuffer(g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]);

    if (glIsBuffer(g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]);

    glGenBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawArraysCmd), drawArraysCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glGenBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawMeshTasksCmd), drawMeshTasksCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    glGenBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawElementsCmd), drawElementsCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);


    glGenBuffers(1, &g_gl->buffers[BUFFER_TERRAIN_DISPATCH_CS]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DISPATCH_CS]);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(dispatchCmd), dispatchCmd, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    return (glGetError() == GL_NO_ERROR);
}


// -----------------------------------------------------------------------------
/**
 * Load Meshlet Buffers
 *
 * This procedure creates a vertex and index buffer that represents a
 * subdividided triangle, which we refer to as a meshlet.
 */
bool LoadMeshletBuffers()
{
    std::vector<uint16_t> indexBuffer;
    std::vector<glm::vec2> vertexBuffer;
    std::map<uint32_t, uint16_t> hashMap;
    int lebDepth = 2 * g_terrain.gpuSubd;
    int triangleCount = 1 << lebDepth;
    int edgeTessellationFactor = 1 << g_terrain.gpuSubd;

    // compute index and vertex buffer
    for (int i = 0; i < triangleCount; ++i) {
        cbt_Node node = {(uint64_t)(triangleCount + i), 2 * g_terrain.gpuSubd};
        float attribArray[][3] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };

        leb_DecodeNodeAttributeArray(node, 2, attribArray);

        for (int j = 0; j < 3; ++j) {
            uint32_t vertexID = attribArray[0][j] * (edgeTessellationFactor + 1)
                              + attribArray[1][j] * (edgeTessellationFactor + 1) * (edgeTessellationFactor + 1);
            auto it = hashMap.find(vertexID);

            if (it != hashMap.end()) {
                indexBuffer.push_back(it->second);
            } else {
                uint16_t newIndex = (uint16_t)vertexBuffer.size();

                indexBuffer.push_back(newIndex);
                hashMap.insert(std::pair<uint32_t, uint16_t>(vertexID, newIndex));
                vertexBuffer.push_back(glm::vec2(attribArray[0][j], attribArray[1][j]));
            }
        }
    }

    if (glIsBuffer(g_gl->buffers[BUFFER_MESHLET_VERTICES]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_MESHLET_VERTICES]);

    if (glIsBuffer(g_gl->buffers[BUFFER_MESHLET_INDEXES]))
        glDeleteBuffers(1, &g_gl->buffers[BUFFER_MESHLET_INDEXES]);

    LOG("Loading {Meshlet-Buffers}\n");

    glGenBuffers(1, &g_gl->buffers[BUFFER_MESHLET_INDEXES]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_gl->buffers[BUFFER_MESHLET_INDEXES]);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(indexBuffer[0]) * indexBuffer.size(),
                 &indexBuffer[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &g_gl->buffers[BUFFER_MESHLET_VERTICES]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_gl->buffers[BUFFER_MESHLET_VERTICES]);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(vertexBuffer[0]) * vertexBuffer.size(),
                 &vertexBuffer[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load All Buffers
 *
 */
bool Terrain::load_buffers(const RenderState& state) {
    bool v = true;

    if (v) v &= LoadTerrainVariables(*state.camera);
    if (v) v &= LoadLebBuffer();
    if (v) v &= LoadRenderCmdBuffer();
    if (v) v &= LoadMeshletBuffers();
    if (v) v &= LoadCbtNodeCountBuffer();

    return v;
}

////////////////////////////////////////////////////////////////////////////////
// Vertex Array Loading
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
/**
 * Load an Empty Vertex Array
 *
 * This will be used to draw procedural geometry, e.g., a fullscreen quad.
 */
bool LoadEmptyVertexArray()
{
    LOG("Loading {Empty-VertexArray}\n");
    if (glIsVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]))
        glDeleteVertexArrays(1, &g_gl->vertexArrays[VERTEXARRAY_EMPTY]);

    glGenVertexArrays(1, &g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
    glBindVertexArray(0);

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load Meshlet Vertex Array
 *
 */
bool LoadMeshletVertexArray()
{
    LOG("Loading {Meshlet-VertexArray}\n");
    if (glIsVertexArray(g_gl->vertexArrays[VERTEXARRAY_MESHLET]))
        glDeleteVertexArrays(1, &g_gl->vertexArrays[VERTEXARRAY_MESHLET]);

    glGenVertexArrays(1, &g_gl->vertexArrays[VERTEXARRAY_MESHLET]);

    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_MESHLET]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl->buffers[BUFFER_MESHLET_VERTICES]);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, gl::buffer_offset(0));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_gl->buffers[BUFFER_MESHLET_INDEXES]);
    glBindVertexArray(0);

    return (glGetError() == GL_NO_ERROR);
}

// -----------------------------------------------------------------------------
/**
 * Load All Vertex Arrays
 *
 */
bool Terrain::load_vaos() {
    bool v = true;

    if (v) v &= LoadEmptyVertexArray();
    if (v) v &= LoadMeshletVertexArray();

    return v;
}

//// Terrain 

Terrain::Terrain() {
    m_glmanager = std::make_unique<OpenGLManager>();

    g_gl = m_glmanager.get(); // no, im not happy about this
}

Terrain::~Terrain() {
    //
}

void Terrain::init(const RenderState& state, const ShaderPreprocessor& pre) {
    bool v = true;

    if (v) v &= load_textures();
    if (v) v &= load_buffers(state);
    if (v) v &= load_vaos();
    if (v) v &= load_programs(state, pre);
    if (v) v &= load_queries();

    // TODO exception
    if (!v)
        throw std::runtime_error{"Terrain::init failure"};
}

bool Terrain::load_programs(const RenderState& state, const ShaderPreprocessor& pre) {
    return LoadPrograms(state, pre);
}

bool Terrain::load_queries() {
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// OpenGL Rendering
//
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
/**
 * Render top view
 *
 * This routine renders the terrain from a top view, which is useful for
 * debugging.
 */
void renderTopView(const RenderState& render_state)
{
    if (g_terrain.flags.topView) {
        glDisable(GL_CULL_FACE);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, g_gl->buffers[BUFFER_LEB]);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW]);
        glViewport(10, 10, 512, 512);
        glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
        glPatchParameteri(GL_PATCH_VERTICES, 1);

        glUseProgram(g_gl->programs[PROGRAM_TOPVIEW]->getHandle());
            glDrawArraysIndirect(GL_PATCHES, 0);

        glBindVertexArray(0);
        glViewport(0, 0, render_state.target_fbo->get_width(), render_state.target_fbo->get_height());
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, 0);
    }
}

// -----------------------------------------------------------------------------
/**
 * Reduction Pass -- Generic
 *
 * The reduction prepass is used for counting the number of nodes and
 * dispatch the threads to the proper node. This routine is entirely
 * generic and isn't tied to a specific pipeline.
 */
// LEB reduction step
void lebReductionPass()
{
    // djgc_start(g_gl->clocks[CLOCK_REDUCTION]);
    int it = g_terrain.maxDepth;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, g_gl->buffers[BUFFER_LEB]);
    glUseProgram(g_gl->programs[PROGRAM_LEB_REDUCTION_PREPASS]->getHandle());
    if (true) {
        int cnt = ((1 << it) >> 5);// / 2;
        int numGroup = (cnt >= 256) ? (cnt >> 8) : 1;
        int loc = glGetUniformLocation(g_gl->programs[PROGRAM_LEB_REDUCTION_PREPASS]->getHandle(),
                                       "u_PassID");

        // djgc_start(g_gl->clocks[CLOCK_REDUCTION00 + it - 1]);
        glUniform1i(loc, it);
        glDispatchCompute(numGroup, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        // djgc_stop(g_gl->clocks[CLOCK_REDUCTION00 + g_terrain.maxDepth - 1]);

        it-= 5;
    }

    glUseProgram(g_gl->programs[PROGRAM_LEB_REDUCTION]->getHandle());
    while (--it >= 0) {
        int loc = glGetUniformLocation(g_gl->programs[PROGRAM_LEB_REDUCTION]->getHandle(), "u_PassID");
        int cnt = 1 << it;
        int numGroup = (cnt >= 256) ? (cnt >> 8) : 1;

        // djgc_start(g_gl->clocks[CLOCK_REDUCTION00 + it]);
        glUniform1i(loc, it);
        glDispatchCompute(numGroup, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        // djgc_stop(g_gl->clocks[CLOCK_REDUCTION00 + it]);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, 0);
    // djgc_stop(g_gl->clocks[CLOCK_REDUCTION]);
}

// -----------------------------------------------------------------------------
/**
 * Batching Pass
 *
 * The batching pass prepares indirect draw calls for rendering the terrain.
 * This routine works for the
 */
void lebBatchingPassTsGs()
{
    glUseProgram(g_gl->programs[PROGRAM_BATCH]->getHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DRAW,
                     g_gl->buffers[BUFFER_TERRAIN_DRAW]);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DRAW, 0);
}
void lebBatchingPassMs()
{
    glUseProgram(g_gl->programs[PROGRAM_BATCH]->getHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DRAW,
                     g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DRAW_MS,
                     g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DRAW, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DRAW_MS, 0);
}
void lebBatchingPassCs()
{
    glUseProgram(g_gl->programs[PROGRAM_BATCH]->getHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DRAW,
                     g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DRAW_CS,
                     g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                     BUFFER_TERRAIN_DISPATCH_CS,
                     g_gl->buffers[BUFFER_TERRAIN_DISPATCH_CS]);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DRAW, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DRAW_CS, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_TERRAIN_DISPATCH_CS, 0);
}
void lebBatchingPass()
{
    // djgc_start(g_gl->clocks[CLOCK_BATCH]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, g_gl->buffers[BUFFER_LEB]);
    switch (g_terrain.method) {
    case METHOD_TS:
    case METHOD_GS:
        lebBatchingPassTsGs();
        break;
    case METHOD_CS:
        lebBatchingPassCs();
        break;
    case METHOD_MS:
        lebBatchingPassMs();
        break;
    default:
        break;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, 0);
    // djgc_stop(g_gl->clocks[CLOCK_BATCH]);
}

// -----------------------------------------------------------------------------
/**
 * Update Pass
 *
 * The update pass updates the LEB binary tree.
 * All pipelines except that the compute shader pipeline render the
 * terrain at the same time.
 */
void lebUpdateAndRenderTs(int pingPong)
{
    // set GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    // update and render
    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glPatchParameteri(GL_PATCH_VERTICES, 1);
    glUseProgram(g_gl->programs[PROGRAM_SPLIT + pingPong]->getHandle());
        glDrawArraysIndirect(GL_PATCHES, 0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reset GL state
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}
void lebUpdateAndRenderGs(int pingPong)
{
    // set GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    // update and render
    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW]);
    glUseProgram(g_gl->programs[PROGRAM_SPLIT + pingPong]->getHandle());
        glDrawArraysIndirect(GL_POINTS, 0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reset GL state
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}
void lebUpdateAndRenderMs(int pingPong)
{
    // set GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    // update and render
    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_EMPTY]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW_MS]);
    glUseProgram(g_gl->programs[PROGRAM_SPLIT + pingPong]->getHandle());
        glDrawMeshTasksIndirectNV(0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reset GL state
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}
void lebUpdateCs(int pingPong)
{
    // set GL state
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER,
                 g_gl->buffers[BUFFER_TERRAIN_DISPATCH_CS]);

    // update
    glUseProgram(g_gl->programs[PROGRAM_SPLIT + pingPong]->getHandle());
    glDispatchComputeIndirect(0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reset GL state
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
}
void lebUpdate()
{
    static int pingPong = 0;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, g_gl->buffers[BUFFER_LEB]);

    // djgc_start(g_gl->clocks[CLOCK_UPDATE]);
    switch (g_terrain.method) {
    case METHOD_TS:
        lebUpdateAndRenderTs(pingPong);
        break;
    case METHOD_GS:
        lebUpdateAndRenderGs(pingPong);
        break;
    case METHOD_CS:
        lebUpdateCs(pingPong);
        break;
    case METHOD_MS:
        lebUpdateAndRenderMs(pingPong);
        break;
    default:
        break;
    }
    // djgc_stop(g_gl->clocks[CLOCK_UPDATE]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, 0);
    pingPong = 1 - pingPong;
}

// -----------------------------------------------------------------------------
/**
 * Render Pass (Compute shader pipeline only)
 *
 * The render pass renders the geometry to the framebuffer.
 */
void lebRenderCs()
{
    // set GL state
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, g_gl->buffers[BUFFER_LEB]);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, g_gl->buffers[BUFFER_TERRAIN_DRAW_CS]);
    glBindVertexArray(g_gl->vertexArrays[VERTEXARRAY_MESHLET]);

    glActiveTexture(GL_TEXTURE0 + TEXTURE_DMAP);
    glBindTexture(GL_TEXTURE_2D, g_gl->textures[TEXTURE_DMAP]);

    glActiveTexture(GL_TEXTURE0 + TEXTURE_SMAP);
    glBindTexture(GL_TEXTURE_2D, g_gl->textures[TEXTURE_SMAP]);

    // render
    glUseProgram(g_gl->programs[PROGRAM_RENDER_ONLY]->getHandle());
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, gl::buffer_offset(0));

    // reset GL state
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BUFFER_LEB, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}
void lebRender()
{
    // djgc_start(g_gl->clocks[CLOCK_RENDER]);
    if (g_terrain.method == METHOD_CS) {
        lebRenderCs();
    }
    // djgc_stop(g_gl->clocks[CLOCK_RENDER]);
}

// -----------------------------------------------------------------------------
void renderTerrain(const Camera& camera)
{
    // djgc_start(g_gl->clocks[CLOCK_ALL]);

    LoadTerrainVariables(camera);
    lebUpdate();
    lebReductionPass();
    lebBatchingPass();
    lebRender(); // render pass (if applicable)

    // djgc_stop(g_gl->clocks[CLOCK_ALL]);
}

void renderScene(const Camera& camera)
{
    renderTerrain(camera);
    // RetrieveNodeCount();
    // renderSky();
}

void Terrain::render(const RenderState& state)
{
    state.target_fbo->bind();
    glViewport(0, 0, state.target_fbo->get_width(), state.target_fbo->get_height());
    // glClearColor(0.5, 0.5, 0.5, 1.0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(*state.camera);
}

}

#endif