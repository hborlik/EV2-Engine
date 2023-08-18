/**
 * @file renderer.h
 * @brief 
 * @date 2022-04-27
 * 
 */
#ifndef EV2_RENDERER_H
#define EV2_RENDERER_H

#include <memory>
#include "evpch.hpp"

#include "core/singleton.hpp"
#include "renderer/shader_builder.hpp"
#include "renderer/camera.hpp"
#include "renderer/renderer.hpp"
#include "gl_shader.hpp"
#include "gl_texture.hpp"
#include "render_state.hpp"
#include "gl_material.hpp"
#include "gl_mesh.hpp"

#include "geometry.hpp"

namespace ev2::renderer {

constexpr uint16_t MAX_N_MATERIALS = 255;

using mat_slot_t = uint8_t;

/**
 * @brief light id
 * 
 */
struct GLLight : public Light {
    enum LightType {
        Point,
        Directional
    } _type;
    int32_t _v = -1;

    bool is_valid() const noexcept {return _v != -1;}
};

struct LightData {
    glm::vec3 color{};
    glm::vec3 position{};
    glm::vec3 k{0.0, 0.0, 2.5}; // k_c, k_l, k_q
};

struct DirectionalLight {
    glm::vec3 color     = {1, 1, 1};
    glm::vec3 ambient   = {0.05, 0.05, 0.05};
    glm::vec3 position = {0, -1, 0};
};

/**
 * @brief material data internal
 * 
 */
struct MaterialData {
    glm::vec3 diffuse       = {0.5, 0.4, 0.0};
    glm::vec3 emissive      = {};
    float metallic          = 0;
    float subsurface        = 0;
    float specular          = .5f;
    float roughness         = .5f;
    float specularTint      = 0;
    float clearcoat         = 0;
    float clearcoatGloss    = 1.f;
    float anisotropic       = 0;
    float sheen             = 0;
    float sheenTint         = .5f;

    GLint diffuse_offset        = 0;
    GLint emissive_offset       = 0;
    GLint metallic_offset       = 0;
    GLint subsurface_offset     = 0;
    GLint specular_offset       = 0;
    GLint roughness_offset      = 0;
    GLint specularTint_offset   = 0;
    GLint clearcoat_offset      = 0;
    GLint clearcoatGloss_offset = 0;
    GLint anisotropic_offset    = 0;
    GLint sheen_offset          = 0;
    GLint sheenTint_offset      = 0;

    bool changed = false;
};

// picking id packing functions

inline glm::uvec3 pack_id_to_uvec3(uint32_t id) noexcept {
    glm::uvec3 target;
    target[0] = id & (0xFF << 0);
    target[1] = (id & (0xFF << 8)) >> 8;
    target[2] = (id & (0xFF << 16)) >> 16;
    return target;
}

inline uint32_t unpack_uvec3_to_id(const glm::uvec3& vec) noexcept {
    uint32_t id = vec[0] | (vec[1] << 8) | (vec[2] << 16);
    return id;
}

struct GLDrawable : public Drawable {
    ~GLDrawable() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    void set_mesh(std::shared_ptr<Mesh> mesh) override;
    void set_material(std::shared_ptr<Material> material) override;
    void set_transform() override;

    void set_material_override(std::shared_ptr<Material> material);
    void set_picking_id(std::size_t picking_id);

private:
    void pack_id(uint32_t value) noexcept {
        id_color = pack_id_to_uvec3(value);
    }

    uint32_t packed_id() const noexcept {
        return unpack_uvec3_to_id(id_color);
    }

public:
    glm::mat4   transform = glm::identity<glm::mat4>();

private:
    friend class GLRenderer;

    GLDrawable() = default;

    std::shared_ptr<GLMaterial>               material_override{};

    int32_t                     id = -1;
    std::shared_ptr<Mesh>       m_mesh = nullptr;
    GLuint                      gl_vao = 0;

    glm::uvec3 id_color{};
    std::size_t m_picking_id{};
};

using ModelInstancePtr = std::shared_ptr<GLDrawable>;


struct GLInstancedDrawable : public InstancedDrawable {
    GLInstancedDrawable(GLInstancedDrawable &&o) = default;

    GLInstancedDrawable& operator=(GLInstancedDrawable &&o) = default;

    ~GLInstancedDrawable() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    void set_mesh(std::shared_ptr<Mesh> drawable);

public:
    glm::mat4               instance_world_transform = glm::identity<glm::mat4>();
    std::unique_ptr<Buffer> instance_transform_buffer{};
    uint32_t                n_instances;

private:
    friend class GLRenderer;

    GLInstancedDrawable() = default;

    int32_t                     id = -1;
    std::shared_ptr<Mesh>       m_mesh = nullptr;
    GLuint                      gl_vao = 0;
};

using InstancedDrawablePtr = std::shared_ptr<InstancedDrawable>;

class RenderPass {
public:
    virtual ~RenderPass() = default;

    virtual void init(const RenderState& state, const PreprocessorSettings& pre) = 0;
    virtual void render(const RenderState& state) = 0;
};

class GLRenderer : public Singleton<GLRenderer>, public Renderer {
public:

    struct ProgramData {
        ProgramData() = default;
        ProgramData(std::string name) : program{std::move(name)} {}

        void init() {
            mat_loc = program.getUniformInfo("materialId").Location;
            obj_id_loc = program.getUniformInfo("id_color").Location;
            vert_col_w_loc = program.getUniformInfo("vertex_color_weight").Location;
            diffuse_sampler_loc = program.getUniformInfo("diffuse_tex").Location;
        }

        std::unique_ptr<GLProgram> program{};
        int mat_loc = -1;
        int obj_id_loc = -1;
        int vert_col_w_loc = -1;
        int diffuse_sampler_loc = -1;
    };

public:
    GLRenderer(uint32_t width, uint32_t height);
    ~GLRenderer();

    void init();

    std::shared_ptr<Mesh> make_mesh() override;
    std::shared_ptr<Material> make_material() override;
    std::shared_ptr<Light> make_light() override;
    std::shared_ptr<Texture> make_texture() override;

    LID create_point_light();
    LID create_directional_light();
    void set_light_position(LID lid, const glm::vec3& position);
    void set_light_color(LID lid, const glm::vec3& color);
    void set_light_ambient(LID lid, const glm::vec3& color);
    void destroy_light(LID lid);

    ModelInstancePtr create_model_instance();

    InstancedDrawablePtr create_instanced_drawable();

    void render(const Camera &camera);

    void set_wireframe(bool enable);
    void set_debug_draw(bool enable);
    std::size_t read_obj_fb(const glm::uvec2& screen_point);

    void set_resolution(uint32_t width, uint32_t height);
    float get_aspect_ratio() const noexcept {return width / (float)height;}

    void draw_screen_space_triangle();

    // ======================
    //  Diag functions
    // ======================

    int get_n_pointlights() const {return point_lights.size();}

    void set_recording(bool enable) { m_recording_id_frames = enable;}

    // ======================

    float get_ssao_radius() const {return ssao_radius;}
    float get_ssao_bias() const {return ssao_bias;}
    int get_ssao_kernel_samples() const {return ssao_kernel_samples;}

    void set_ssao_radius(float radius) {ssao_radius = radius; uniforms_dirty = true;}
    void set_ssao_bias(float bias) {ssao_bias = bias; uniforms_dirty = true;}
    void set_ssao_kernel_samples(int samples) {ssao_kernel_samples = abs(samples); uniforms_dirty = true;}

    void add_pass(RenderPass* pass) {
        // additional effect initialization
        Camera default_camera{};
        RenderState terrain_target_state{
            &g_buffer, // right now only gbuffer passes are supported
            &default_camera
        };
        pass->init(terrain_target_state, m_preprocessor_settings);
        m_passes.push_back(pass);
    }

    void screenshot();


private:
    friend GLMaterial;
    friend GLDrawable;

    void draw(Mesh* dr, const ProgramData& prog, bool use_materials, GLuint gl_vao, int32_t material_override = -1, int32_t n_instances = -1);

    void update_material(mat_slot_t material_slot, const MaterialData& material);

    void load_ssao_uniforms();

    int32_t alloc_material_slot();

    /**
     * @brief internal function for Material object to remove itself before deletion
     * 
     * @param material 
     */
    void destroy_material(GLMaterial* material);

    void destroy_model_instance(GLDrawable* model);

    void update_picking_id_model_instance(GLDrawable* drawable);

    void destroy_instanced_drawable(InstancedDrawable* drawable);

public:
    float exposure      = .8f;
    float gamma         = 2.2f;
    float bloom_falloff = 0.9f;

    float sky_brightness = .22f;
    float sun_position  = .0f;
    float cloud_speed   = .1f;

    const uint32_t ShadowMapWidth = 4096;
    const uint32_t ShadowMapHeight = 4096;
    float shadow_bias_world = 0.17f;

    int32_t bloom_iterations = 2;
    float bloom_threshold = 2.17f;

    bool pause_cull = false;
    bool culling_enabled = true;


private:
    bool m_recording_id_frames = false;
    std::future<void> m_id_frame_save_async{};

    Frustum cull_frustum{};

    // uniform constants
    bool uniforms_dirty = true;
    float ssao_radius = 0.5f;
    float ssao_bias = 0.025f;
    uint32_t ssao_kernel_samples = 32;

    // material management
    std::unordered_map<int32_t, GLMaterial*> materials;
    int32_t next_material_id = 1234;

    std::array<MaterialData, MAX_N_MATERIALS> material_data_buffer; // cpu side material data array
    std::queue<mat_slot_t> free_material_slots; // queue of free slots in material_data_buffer

    // single instance of a drawable
    std::unordered_map<int32_t, GLDrawable*> model_instances;
    uint32_t next_model_instance_id = 100;

    // instances of a drawable with instanced draw calls
    std::unordered_map<int32_t, InstancedDrawable*> instanced_drawables;
    uint32_t next_instanced_drawable_id = 100;

    // picking id map small hash to original id
    std::unordered_map<uint32_t, std::size_t> m_picking_id_map{};

    // std::unordered_map<int32_t, RenderObj> meshes;
    // uint32_t next_mesh_id = 1;

    // std::unordered_map<int32_t, MeshInstance> mesh_instances;
    // uint32_t next_mesh_instance_id = 1;

    // lights
    std::unordered_map<uint32_t, LightData> point_lights;
    std::unique_ptr<Buffer> point_light_data_buffer;
    std::unordered_map<uint32_t, DirectionalLight> directional_lights;
    int32_t next_light_id = 1000;
    int32_t shadow_directional_light_id = -1;

    ProgramData geometry_program;
    int gp_m_location;
    int gp_mv_location;
    int gp_g_location;

    ProgramData geometry_program_instanced;
    int gpi_m_location;

    ProgramData depth_program;
    int sdp_m_location;
    int sdp_lpv_location;

    ProgramData directional_lighting_program;
    int lp_p_location, lp_n_location, lp_as_location, lp_mt_location, lp_gao_location, lp_ls_location, lp_sdt_location, lp_ldir_location, lp_lcol_location, lp_lamb_location;

    ProgramData point_lighting_program;
    int plp_p_location, plp_n_location, plp_as_location, plp_mt_location;
    int plp_ssbo_light_data_location;

    ProgramData ssao_program;
    int ssao_p_loc, ssao_n_loc, ssao_tex_noise_loc, ssao_radius_loc, ssao_bias_loc, ssao_nSamples_loc;

    ProgramData sky_program;
    int sky_time_loc, sky_cirrus_loc, sky_cumulus_loc, sky_sun_position_loc, sky_output_mul_loc;

    ProgramData post_fx_bloom_combine_program;
    int post_fx_bc_hdrt_loc, post_fx_bc_emist_loc, post_fx_bc_thresh_loc;

    ProgramData post_fx_bloom_blur;
    int post_fx_bb_hor_loc, post_fx_bb_bloom_in_loc;

    ProgramData post_fx_program;
    int post_fx_gamma_loc, post_fx_exposure_loc, post_fx_bloom_falloff_loc, post_fx_hdrt_loc, post_fx_bloomt_loc;


    FBO g_buffer;
    FBO ssao_buffer;
    FBO lighting_buffer;
    FBO depth_fbo;
    FBO bloom_thresh_combine;
    std::array<FBO, 2> bloom_blur_swap_fbo;
    
    std::pair<VertexBuffer, GLuint> sst_vb;

    std::shared_ptr<GLTexture> shadow_depth_tex;

    // g buffer fbo attachment textures
    std::shared_ptr<GLTexture> obj_id_tex;
    std::shared_ptr<GLTexture> material_tex;
    std::shared_ptr<GLTexture> albedo_spec;
    std::shared_ptr<GLTexture> normals;
    std::shared_ptr<GLTexture> position;
    std::shared_ptr<GLTexture> emissive;

    std::shared_ptr<GLTexture> ssao_kernel_noise;
    std::shared_ptr<GLTexture> ssao_kernel_color;

    std::shared_ptr<GLTexture> one_p_black_tex;

    std::shared_ptr<GLTexture> hdr_texture;
    std::shared_ptr<GLTexture> hdr_combined;

    // texture in the 0 index is used for the bloom combined output
    std::array<std::shared_ptr<GLTexture>, 2> bloom_blur_swap_tex;

    Buffer shader_globals;
    ProgramUniformBlockDescription globals_desc;
    struct GlobalsOffsets {
        int P_ind;
        int PInv_ind;
        int View_ind;
        int VInv_ind;
        int VP_ind;
        int CameraPos_ind;
        int CameraDir_ind;
    } goffsets;

    Buffer lighting_materials;
    ProgramUniformBlockDescription lighting_materials_desc;

    Buffer ssao_kernel_buffer;
    ProgramUniformBlockDescription ssao_kernel_desc;

    uint32_t width, height;
    bool wireframe = false;
    bool draw_bounding_boxes = false;

    float point_light_geom_base_scale;
    std::shared_ptr<Mesh> point_light_drawable;
    GLuint point_light_gl_vao = 0;

    int32_t default_material_slot = 0;

    std::vector<RenderPass*> m_passes{};

    PreprocessorSettings m_preprocessor_settings{};
};

}

#endif // EV2_RENDERER_H
