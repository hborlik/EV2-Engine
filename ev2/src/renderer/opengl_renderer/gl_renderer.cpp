#include "renderer/opengl_renderer/gl_renderer.hpp"

#include "renderer/opengl_renderer/ev_gl.hpp"
#include "renderer/material.hpp"
#include "resource.hpp"
#include "core/engine.hpp"

namespace ev2::renderer {

// data structure matching GPU point light data definition
struct PointLightData {
    glm::vec3 position;
    float pad;
    glm::vec3 lightColor;
    float scale;
    float k_c;
    float k_l;
    float k_q;
    float radius;
};

void GLPointLight::set_color(const glm::vec3& color) {
    auto& point_lights = m_owner->point_lights;
    auto mi = point_lights.find(light_id);
    EV_CORE_ASSERT(mi != point_lights.end(), "GLPointLight invalid light_id!");
    if (mi != point_lights.end()) {
        mi->second.color = color;
    }
}

void GLPointLight::set_position(const glm::vec3& position) {
    auto& point_lights = m_owner->point_lights;
    auto mi = point_lights.find(light_id);
    EV_CORE_ASSERT(mi != point_lights.end(), "GLPointLight invalid light_id!");
    if (mi != point_lights.end()) {
        mi->second.position = position;
    }
}

void GLPointLight::set_k(const glm::vec3& k) {
    auto& point_lights = m_owner->point_lights;
    auto mi = point_lights.find(light_id);
    EV_CORE_ASSERT(mi != point_lights.end(), "GLPointLight invalid light_id!");
    if (mi != point_lights.end()) {
        mi->second.k = k;
    }
}

void GLDirectionalLight::set_color(const glm::vec3& color) {
    auto& directional_lights = m_owner->directional_lights;
    auto mi = directional_lights.find(light_id);
    EV_CORE_ASSERT(mi != directional_lights.end(), "GLDirectionalLight invalid light_id!");
    if (mi != directional_lights.end()) {
        mi->second.color = color;
    }
}

void GLDirectionalLight::set_position(const glm::vec3& position) {
    auto& directional_lights = m_owner->directional_lights;
    auto mi = directional_lights.find(light_id);
    EV_CORE_ASSERT(mi != directional_lights.end(), "GLDirectionalLight invalid light_id!");
    if (mi != directional_lights.end()) {
        mi->second.position = position;
    }
}

void GLDirectionalLight::set_ambient(const glm::vec3& ambient) {
    auto& directional_lights = m_owner->directional_lights;
    auto mi = directional_lights.find(light_id);
    EV_CORE_ASSERT(mi != directional_lights.end(), "GLDirectionalLight invalid light_id!");
    if (mi != directional_lights.end()) {
        mi->second.ambient = ambient;
    }
}


void GLDrawable::set_mesh(std::shared_ptr<Mesh> mesh) {
    auto gl_mesh = std::dynamic_pointer_cast<GLMesh>(mesh);
    EV_CORE_ASSERT(gl_mesh, "set_mesh requires a non-null GLMesh");
    m_mesh = gl_mesh;

    if (gl_vao != 0)
        glDeleteVertexArrays(1, &gl_vao);
    
    gl_vao = m_mesh->get_vertex_buffer().gen_vao_for_attributes(mat_spec::DefaultBindings);
}

void GLDrawable::set_material(std::shared_ptr<Material> material) {
    std::shared_ptr<GLMaterial> gl_material = std::dynamic_pointer_cast<GLMaterial>(material);
    if (!gl_material) {
        material_override = nullptr;
        return;
    }
    EV_CORE_ASSERT(gl_material->is_registered(), "Material must be registered");
    material_override = gl_material;
}

void GLDrawable::set_transform(const glm::mat4& transform) {
    this->transform = transform;
}

void GLDrawable::set_picking_id(std::size_t picking_id) {
    m_picking_id = picking_id;
    pack_id((uint32_t)picking_id ^ (picking_id >> 32));
    m_owner->update_picking_id_model_instance(this);
}

void GLInstancedDrawable::set_mesh(std::shared_ptr<Mesh> mesh) {
    auto gl_mesh = std::dynamic_pointer_cast<GLMesh>(mesh);
    EV_CORE_ASSERT(gl_mesh, "set_mesh requires a non-null GLMesh");
    this->m_mesh = gl_mesh;

    gl_vao = VAOFactory::gen_vao_for_attributes(m_mesh->get_vertex_buffer().get(), m_mesh->get_index_buffer().get(), mat_spec::DefaultBindings, instance_transform_buffer.get());
}

void GLRenderer::draw(GLMesh* dr, const ProgramData& prog, bool use_materials, GLuint gl_vao, int32_t material_override, int32_t n_instances) {
    if (n_instances == 0) {
        return; // nothing to do
    }
    
    if (dr->cull_mode == gl::CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)dr->cull_mode);
    }
    glFrontFace((GLenum)dr->front_facing);
    const int mat_loc = prog.mat_loc;
    const int vert_col_w_loc = prog.vert_col_w_loc;
    const int diffuse_sampler_loc = prog.diffuse_sampler_loc;
    const bool indexed = dr->m_index_buffer != nullptr;

    glBindVertexArray(gl_vao);
    for (auto& m : dr->m_primitives) {
        GLMaterial* material_ptr = nullptr;
        if (use_materials) {
            mat_slot_t material_slot = 0;
            if (m.material_ind >= 0 && material_override < 0) {
                material_ptr = dr->m_materials[m.material_ind].get();
                material_slot = material_ptr->material_slot;
            } else if (material_override >= 0) {
                material_ptr = materials.at(material_override);
                material_slot = material_ptr->material_slot;
            } else { // use error material if not set
                material_ptr = nullptr;
                material_slot = default_material_slot;
            }

            // TODO does this go here?
            // update the program material index
            if (mat_loc >= 0) {
                GL_CHECKED_CALL(glUniform1ui(mat_loc, material_slot));
            }

            if (vert_col_w_loc >= 0) {
                GL_CHECKED_CALL(glUniform1f(vert_col_w_loc, dr->vertex_color_weight));
            }
        }

        // textures
        if (diffuse_sampler_loc >= 0) {
            glActiveTexture(GL_TEXTURE0);
            if (use_materials && material_ptr && material_ptr->diffuse_tex)
                material_ptr->diffuse_tex->bind();
            else
                one_p_black_tex->bind();

            gl::glUniformSampler(0, diffuse_sampler_loc);
        } else {
            // glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (indexed) {
            // Buffer* el_buf = dr->vertex_buffer.get_buffer(dr->vertex_buffer.get_indexed()).get();
            // el_buf->bind();
            if (n_instances > 0) {
                glDrawElementsInstanced(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0, n_instances);
            } else {
                glDrawElements(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0);
            }
            // el_buf->unbind();
        } else {
            if (n_instances > 0) {
                glDrawArraysInstanced(GL_TRIANGLES, m.start_index, m.num_elements, n_instances);
            } else {
                glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
            }
        }

        // glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindVertexArray(0);
}

GLRenderer::GLRenderer(uint32_t width, uint32_t height) : 
    model_instances{},
    geometry_program{"Geometry Program"},
    directional_lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    ssao_buffer{gl::FBOTarget::RW},
    lighting_buffer{gl::FBOTarget::RW},
    depth_fbo{gl::FBOTarget::RW},
    bloom_thresh_combine{gl::FBOTarget::RW},
    bloom_blur_swap_fbo{FBO{gl::FBOTarget::RW}, FBO{gl::FBOTarget::RW}},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    lighting_materials{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    ssao_kernel_buffer{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height},
    m_preprocessor_settings{} {

    // material id queue
    for (mat_slot_t i = 0; i < MAX_N_MATERIALS; i++) {
        free_material_slots.push(i);
    }

    // create default material in material data
    default_material_slot = alloc_material_slot();
    material_data_buffer[default_material_slot].diffuse = {1.00,0.00,1.00};

    for (auto& mat : material_data_buffer) {
        mat = {};
    }
}

GLRenderer::~GLRenderer() {
    if (m_id_frame_save_async.valid())
        m_id_frame_save_async.wait();
}

void GLRenderer::init() {
    // precomputed (static) data
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    auto lerp = [](float a, float b, float f) -> float
    {
        return a + f * (b - a);
    };

    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0, // [-1, 1]
            randomFloats(generator) * 2.0 - 1.0, // [-1, 1]
            randomFloats(generator) // [0, 1]
        );
        sample  = glm::normalize(sample); // random direction
        sample *= randomFloats(generator);
        // bias samples towards the central pixel
        float scale = randomFloats(generator); // [0, 1]
        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    // random xy values for noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0, // [-1, 1]
            randomFloats(generator) * 2.0 - 1.0, // [-1, 1]
            randomFloats(generator) * 2.0 - 1.0);
        ssaoNoise.push_back(noise);
    }

    // ssao tiling noise texture
    ssao_kernel_noise = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    ssao_kernel_noise->set_image2D(gl::TextureInternalFormat::RGBA16F, 4, 4, gl::PixelFormat::RGB, gl::PixelType::FLOAT, (unsigned char*)ssaoNoise.data());
    ssao_kernel_noise->set_texture_wrap_s(gl::TextureWrapMode::REPEAT);
    ssao_kernel_noise->set_texture_wrap_t(gl::TextureWrapMode::REPEAT);

    ssao_kernel_color = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    ssao_kernel_color->recreate_storage2D(1, gl::TextureInternalFormat::R8, width, height);

    ssao_buffer.attach(ssao_kernel_color, gl::FBOAttachment::COLOR0, 0);

    if (!ssao_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};

    // black texture
    one_p_black_tex = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    one_p_black_tex->set_image2D(gl::TextureInternalFormat::RED, 1, 1, gl::PixelFormat::RED, gl::PixelType::UNSIGNED_BYTE, nullptr);
    one_p_black_tex->set_texture_wrap_s(gl::TextureWrapMode::REPEAT);
    one_p_black_tex->set_texture_wrap_t(gl::TextureWrapMode::REPEAT);
    one_p_black_tex->generate_mips();

    // set up FBO textures
    shadow_depth_tex = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    shadow_depth_tex->set_image2D(gl::TextureInternalFormat::DEPTH_COMPONENT16, ShadowMapWidth, ShadowMapHeight, gl::PixelFormat::DEPTH_COMPONENT, gl::PixelType::FLOAT, nullptr);
    // edge clamping is required for border color to be used
    shadow_depth_tex->set_texture_wrap_s(gl::TextureWrapMode::CLAMP_TO_EDGE);
    shadow_depth_tex->set_texture_wrap_t(gl::TextureWrapMode::CLAMP_TO_EDGE);
    shadow_depth_tex->set_border_color(glm::vec4{1.0f});
    depth_fbo.attach(shadow_depth_tex, gl::FBOAttachment::DEPTH);
    if (!depth_fbo.check())
        throw engine_exception{"Framebuffer is not complete"};
    
    obj_id_tex = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    obj_id_tex->recreate_storage2D(1, gl::TextureInternalFormat::RGB8UI, width, height);
    g_buffer.attach(obj_id_tex, gl::FBOAttachment::COLOR5, 5);

    material_tex = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    material_tex->recreate_storage2D(1, gl::TextureInternalFormat::R8UI, width, height);
    g_buffer.attach(material_tex, gl::FBOAttachment::COLOR3, 3);

    albedo_spec = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    albedo_spec->recreate_storage2D(1, gl::TextureInternalFormat::RGBA8, width, height);
    g_buffer.attach(albedo_spec, gl::FBOAttachment::COLOR2, 2);

    normals = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    normals->recreate_storage2D(1, gl::TextureInternalFormat::RGBA16F, width, height);
    g_buffer.attach(normals, gl::FBOAttachment::COLOR1, 1);

    position = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    position->recreate_storage2D(1, gl::TextureInternalFormat::RGBA16F, width, height);
    g_buffer.attach(position, gl::FBOAttachment::COLOR0, 0);

    emissive = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    emissive->recreate_storage2D(1, gl::TextureInternalFormat::RGBA16F, width, height);
    g_buffer.attach(emissive, gl::FBOAttachment::COLOR4, 4);

    g_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH_COMPONENT, width, height, gl::FBOAttachment::DEPTH);

    if (!g_buffer.check())
        throw engine_exception{"Geometry Framebuffer is not complete"};

    // lighting output HDR FBO
    hdr_texture = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    hdr_texture->recreate_storage2D(1, gl::TextureInternalFormat::RGBA16F, width, height);
    lighting_buffer.attach(hdr_texture, gl::FBOAttachment::COLOR0, 0);

    lighting_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH24_STENCIL8, width, height, gl::FBOAttachment::DEPTH_STENCIL);

    if (!lighting_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};

    // blur pass texture and FBOs
    bloom_blur_swap_tex[0] = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    bloom_blur_swap_tex[0]->set_image2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_blur_swap_fbo[0].attach(bloom_blur_swap_tex[0], gl::FBOAttachment::COLOR0, 0);

    bloom_blur_swap_tex[1] = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    bloom_blur_swap_tex[1]->set_image2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_blur_swap_fbo[1].attach(bloom_blur_swap_tex[1], gl::FBOAttachment::COLOR0, 0);

    // bloom threshold combine output HDR FBO
    hdr_combined = std::make_shared<GLTexture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST, gl::TextureFilterMode::NEAREST);
    hdr_combined->set_image2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_thresh_combine.attach(hdr_combined, gl::FBOAttachment::COLOR0, 0);

    bloom_thresh_combine.attach(bloom_blur_swap_tex[0], gl::FBOAttachment::COLOR1, 1);

    if (!bloom_thresh_combine.check())
        throw engine_exception{"Framebuffer is not complete"};

    // set up programs

    geometry_program.program->loadShader(ShaderType::VERTEX_SHADER, "geometry.glsl.vert", m_preprocessor_settings);
    geometry_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", m_preprocessor_settings);
    geometry_program.program->link();
    geometry_program.init();

    gp_m_location = geometry_program.program->getUniformInfo("M").Location;
    gp_mv_location = geometry_program.program->getUniformInfo("MV").Location;
    gp_g_location = geometry_program.program->getUniformInfo("G").Location;

    geometry_program_instanced.program->loadShader(ShaderType::VERTEX_SHADER, "geometry_instanced.glsl.vert", m_preprocessor_settings);
    geometry_program_instanced.program->loadShader(ShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", m_preprocessor_settings);
    geometry_program_instanced.program->link();
    geometry_program_instanced.init();

    gpi_m_location = geometry_program_instanced.program->getUniformInfo("M").Location;

    // Initialize the GLSL programs
    depth_program.program->loadShader(ShaderType::VERTEX_SHADER, "simpleDepth.glsl.vert", m_preprocessor_settings);
    depth_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "simpleDepth.glsl.frag", m_preprocessor_settings);
    depth_program.program->link();
    depth_program.init();

    sdp_m_location = depth_program.program->getUniformInfo("M").Location;
    sdp_lpv_location = depth_program.program->getUniformInfo("LPV").Location;


    directional_lighting_program.program->loadShader(ShaderType::VERTEX_SHADER, "sst.glsl.vert", m_preprocessor_settings);
    directional_lighting_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "directional.glsl.frag", m_preprocessor_settings);
    directional_lighting_program.program->link();
    directional_lighting_program.init();

    lp_p_location = directional_lighting_program.program->getUniformInfo("gPosition").Location;
    lp_n_location = directional_lighting_program.program->getUniformInfo("gNormal").Location;
    lp_as_location = directional_lighting_program.program->getUniformInfo("gAlbedoSpec").Location;
    lp_mt_location = directional_lighting_program.program->getUniformInfo("gMaterialTex").Location;
    lp_gao_location = directional_lighting_program.program->getUniformInfo("gAO").Location;
    lp_ls_location = directional_lighting_program.program->getUniformInfo("LS").Location;
    lp_sdt_location = directional_lighting_program.program->getUniformInfo("shadowDepth").Location;
    lp_ldir_location = directional_lighting_program.program->getUniformInfo("lightDir").Location;
    lp_lcol_location = directional_lighting_program.program->getUniformInfo("lightColor").Location;
    lp_lamb_location = directional_lighting_program.program->getUniformInfo("lightAmbient").Location;


    point_lighting_program.program->loadShader(ShaderType::VERTEX_SHADER, "point_lighting.glsl.vert", m_preprocessor_settings);
    point_lighting_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "point_lighting.glsl.frag", m_preprocessor_settings);
    point_lighting_program.program->link();
    point_lighting_program.init();

    plp_p_location       = point_lighting_program.program->getUniformInfo("gPosition").Location;
    plp_n_location       = point_lighting_program.program->getUniformInfo("gNormal").Location;
    plp_as_location      = point_lighting_program.program->getUniformInfo("gAlbedoSpec").Location;
    plp_mt_location      = point_lighting_program.program->getUniformInfo("gMaterialTex").Location;

    plp_ssbo_light_data_location = point_lighting_program.program->getProgramResourceLocation(GL_SHADER_STORAGE_BLOCK, "lights_in");

    point_light_data_buffer = std::make_unique<GLBuffer>(gl::BindingTarget::SHADER_STORAGE, gl::Usage::DYNAMIC_DRAW);


    ssao_program.program->loadShader(ShaderType::VERTEX_SHADER, "sst.glsl.vert", m_preprocessor_settings);
    ssao_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "ssao.glsl.frag", m_preprocessor_settings);
    ssao_program.program->link();
    ssao_program.init();

    ssao_p_loc         = ssao_program.program->getUniformInfo("gPosition").Location;
    ssao_n_loc         = ssao_program.program->getUniformInfo("gNormal").Location;
    ssao_tex_noise_loc = ssao_program.program->getUniformInfo("texNoise").Location;
    ssao_radius_loc    = ssao_program.program->getUniformInfo("radius").Location;
    ssao_bias_loc      = ssao_program.program->getUniformInfo("bias").Location;
    ssao_nSamples_loc  = ssao_program.program->getUniformInfo("nSamples").Location;

    load_ssao_uniforms();


    sky_program.program->loadShader(ShaderType::VERTEX_SHADER, "sky.glsl.vert", m_preprocessor_settings);
    sky_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "sky.glsl.frag", m_preprocessor_settings);
    sky_program.program->link();
    sky_program.init();
    sky_time_loc        = sky_program.program->getUniformInfo("time").Location;
    sky_cirrus_loc      = sky_program.program->getUniformInfo("cirrus").Location;
    sky_cumulus_loc     = sky_program.program->getUniformInfo("cumulus").Location;
    sky_sun_position_loc= sky_program.program->getUniformInfo("sun_position").Location;
    sky_output_mul_loc  = sky_program.program->getUniformInfo("output_mul").Location;

    post_fx_bloom_combine_program.program->loadShader(ShaderType::VERTEX_SHADER, "sst.glsl.vert", m_preprocessor_settings);
    post_fx_bloom_combine_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "post_fx_bloom_combine.glsl.frag", m_preprocessor_settings);
    post_fx_bloom_combine_program.program->link();
    post_fx_bloom_combine_program.init();
    post_fx_bc_hdrt_loc = post_fx_bloom_combine_program.program->getUniformInfo("hdrBuffer").Location;
    post_fx_bc_emist_loc = post_fx_bloom_combine_program.program->getUniformInfo("emissiveBuffer").Location;
    post_fx_bc_thresh_loc = post_fx_bloom_combine_program.program->getUniformInfo("bloom_threshold").Location;

    post_fx_bloom_blur.program->loadShader(ShaderType::VERTEX_SHADER, "sst.glsl.vert", m_preprocessor_settings);
    post_fx_bloom_blur.program->loadShader(ShaderType::FRAGMENT_SHADER, "post_fx_bloom_blur.glsl.frag", m_preprocessor_settings);
    post_fx_bloom_blur.program->link();
    post_fx_bloom_blur.init();
    post_fx_bb_hor_loc = post_fx_bloom_blur.program->getUniformInfo("horizontal").Location;
    post_fx_bb_bloom_in_loc = post_fx_bloom_blur.program->getUniformInfo("bloom_blur_in").Location;

    post_fx_program.program->loadShader(ShaderType::VERTEX_SHADER, "sst.glsl.vert", m_preprocessor_settings);
    post_fx_program.program->loadShader(ShaderType::FRAGMENT_SHADER, "post_fx.glsl.frag", m_preprocessor_settings);
    post_fx_program.program->link();
    post_fx_program.init();
    post_fx_gamma_loc           = post_fx_program.program->getUniformInfo("gamma").Location;
    post_fx_exposure_loc        = post_fx_program.program->getUniformInfo("exposure").Location;
    post_fx_bloom_falloff_loc   = post_fx_program.program->getUniformInfo("bloom_falloff").Location;
    post_fx_hdrt_loc            = post_fx_program.program->getUniformInfo("hdrBuffer").Location;
    post_fx_bloomt_loc          = post_fx_program.program->getUniformInfo("bloomBuffer").Location;

    // program block inputs
    globals_desc = geometry_program.program->getUniformBlockInfo("Globals");
    goffsets.P_ind = globals_desc.get_index("P");
    goffsets.PInv_ind = globals_desc.get_index("PInv");
    goffsets.View_ind = globals_desc.get_index("View");
    goffsets.VInv_ind = globals_desc.get_index("VInv");
    goffsets.VP_ind = globals_desc.get_index("VP");
    goffsets.CameraPos_ind = globals_desc.get_index("CameraPos");
    goffsets.CameraDir_ind = globals_desc.get_index("CameraDir");

    shader_globals.allocate(globals_desc.block_size);

    lighting_materials_desc = directional_lighting_program.program->getUniformBlockInfo("MaterialsInfo");
    lighting_materials.allocate(lighting_materials_desc.block_size);

    // extract all offsets for material buffer
    for (const auto& [name, ind] : lighting_materials_desc.layout_map) {
        std::size_t b_begin = name.find('[');
        std::size_t b_end = name.find(']');
        if (b_begin != std::string::npos) {
            if (b_end == std::string::npos)
                throw engine_exception{"Invalid array name??"};
            b_begin++;
            uint32_t index = std::stoi(name.substr(b_begin, b_end - b_begin));
            std::string var_name = name.substr(b_end + 2);
            MaterialData& data_ref = material_data_buffer[index];
            if (var_name == "diffuse") {
                data_ref.diffuse_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "emissive") {
                data_ref.emissive_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "metallic") {
                data_ref.metallic_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "subsurface") {
                data_ref.subsurface_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "specular") {
                data_ref.specular_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "roughness") {
                data_ref.roughness_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "specularTint") {
                data_ref.specularTint_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "clearcoat") {
                data_ref.clearcoat_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "clearcoatGloss") {
                data_ref.clearcoatGloss_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "anisotropic") {
                data_ref.anisotropic_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "sheen") {
                data_ref.sheen_offset = lighting_materials_desc.layouts[ind].Offset;
            } else if (var_name == "sheenTint") {
                data_ref.sheenTint_offset = lighting_materials_desc.layouts[ind].Offset;
            } else {
                throw engine_exception{"invalid material array name " + var_name};
            }
        }
    }

    ssao_kernel_desc = ssao_program.program->getUniformBlockInfo("Samples");
    ssao_kernel_buffer.allocate(ssao_kernel_desc.block_size);
    auto tgt_layout = ssao_kernel_desc.get_layout("samples[0]");
    ssao_kernel_buffer.sub_data(ssaoKernel, tgt_layout.Offset, tgt_layout.ArrayStride);
    
    int i = 0;
    for (auto& m : material_data_buffer) {
        update_material(i, m);
    }

    // light geometry
    auto point_light_cube = load_model(std::filesystem::path("models") / "cube.obj",
        Engine::get_singleton().asset_path);
    point_light_mesh = std::dynamic_pointer_cast<GLMesh>(make_mesh(point_light_cube.get()));

    // render back facing only
    point_light_mesh->front_facing =
        gl::FrontFacing::CW;
    
    const glm::vec3 scaling =
        glm::vec3{2} / (point_light_mesh->bounding_box.pMax -
                        point_light_mesh->bounding_box.pMin);

    point_light_geom_base_scale = scaling.x;

    point_light_gl_vao = point_light_mesh->get_vertex_buffer().gen_vao_for_attributes(point_lighting_program.program.getAttributeMap());

}

std::shared_ptr<Mesh> GLRenderer::make_mesh(const Model* model) {

}

std::shared_ptr<Material> GLRenderer::make_material() {

}

std::shared_ptr<PointLight> GLRenderer::make_point_light() {
    const auto light_deleter = [this](GLPointLight* light) {
        this->destroy_light(light);
    };
    int32_t nlid = next_light_id++;
    std::shared_ptr<GLPointLight> light(new GLPointLight{}, light_deleter);
    light->light_id = nlid;
    point_lights.insert_or_assign(nlid, LightData{});
    return light;
}

std::shared_ptr<DirectionalLight> GLRenderer::make_directional_light() {
    const auto light_deleter = [this](DirectionalLight* light) {
        this->destroy_light(light);
    };
    int32_t nlid = next_light_id++;
    if (shadow_directional_light_id < 0)
        shadow_directional_light_id = nlid;
    DirectionalLight l{};
    directional_lights.insert_or_assign(nlid, l);
    return {LID::Directional, nlid};
}

std::shared_ptr<Texture> GLRenderer::make_texture() {

}

std::shared_ptr<Drawable> GLRenderer::make_drawable() {
    const auto model_deleter = [this](GLDrawable* mi) {
        this->destroy_model_instance(mi);
    };

    int32_t id = next_model_instance_id++;
    ModelInstancePtr model(new GLDrawable(), model_deleter);
    model->id = id;
    auto [mi, inserted] = model_instances.emplace(id, model.get());

    assert(inserted);

    return model;
}

std::shared_ptr<InstancedDrawable> GLRenderer::make_instanced_drawable() {
    const auto instanced_drawable_deleter = [this](InstancedDrawable* id) {
        this->destroy_instanced_drawable(id);
    };

    int32_t id = next_instanced_drawable_id++;
    InstancedDrawablePtr model(new GLInstancedDrawable(), instanced_drawable_deleter);
    model->id = id;
    model->instance_transform_buffer = std::make_unique<Buffer>(gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW);

    auto [mi, inserted] = instanced_drawables.emplace(id, model.get());
    
    assert(inserted);

    return model;
}

void GLRenderer::destroy_model_instance(GLDrawable* model) {
    EV_CORE_ASSERT(model, "Cannot destroy null drawable");
    model_instances.erase(model->id);
    m_picking_id_map.erase(model->packed_id());
}


void GLRenderer::update_material(mat_slot_t material_slot, const MaterialData& material) {
    material_data_buffer[material_slot] = material;
    lighting_materials.sub_data(material.diffuse,        material.diffuse_offset);
    lighting_materials.sub_data(material.emissive,       material.emissive_offset);
    lighting_materials.sub_data(material.metallic,       material.metallic_offset);
    lighting_materials.sub_data(material.subsurface,     material.subsurface_offset);
    lighting_materials.sub_data(material.specular,       material.specular_offset);
    lighting_materials.sub_data(material.roughness,      material.roughness_offset);
    lighting_materials.sub_data(material.specularTint,   material.specularTint_offset);
    lighting_materials.sub_data(material.clearcoat,      material.clearcoat_offset);
    lighting_materials.sub_data(material.clearcoatGloss, material.clearcoatGloss_offset);
    lighting_materials.sub_data(material.anisotropic,    material.anisotropic_offset);
    lighting_materials.sub_data(material.sheen,          material.sheen_offset);
    lighting_materials.sub_data(material.sheenTint,      material.sheenTint_offset);
}

void GLRenderer::load_ssao_uniforms() {
    glProgramUniform1f(ssao_program.program->getHandle(), ssao_bias_loc, ssao_bias);
    glProgramUniform1f(ssao_program.program->getHandle(), ssao_radius_loc, ssao_radius);
    glProgramUniform1ui(ssao_program.program->getHandle(), ssao_nSamples_loc, ssao_kernel_samples);
}

std::shared_ptr<Material> GLRenderer::make_material() {

    const auto mat_deleter = [this](GLMaterial* mat) -> void {
        this->destroy_material(mat);
    };

    int32_t new_mat_slot = alloc_material_slot();
    if (new_mat_slot >= 0) {
        int32_t id = next_material_id++;
        auto new_material =
            std::shared_ptr<GLMaterial>(new GLMaterial{}, mat_deleter);
        materials[id] = new_material.get();
        new_material->material_id = id;
        new_material->material_slot = new_mat_slot;
        new_material->m_owner = this;
        return new_material;
    }
    return {};
}

int32_t GLRenderer::alloc_material_slot() {
    if (free_material_slots.size() > 0) {
        mat_slot_t slot = free_material_slots.front();
        free_material_slots.pop();
        return slot;
    } else {
        return -1;
    }
}

void GLRenderer::destroy_material(GLMaterial* material) {
    EV_CORE_ASSERT(material, "Cannot delete null material");
    if (material->material_id < 0 || material->material_slot < 0 || material->m_owner != this)
        return; // material not backed
    material_data_buffer[material->material_slot] = {};
    free_material_slots.push(material->material_slot);

    EV_CORE_ASSERT(materials.erase(material->material_id), "Failed to erase material");

    material->material_slot = -1;
    material->material_id = -1;
    material->m_owner = nullptr;
}

void GLRenderer::destroy_light(GLPointLight* light) {
    EV_CORE_ASSERT(light && light->is_valid());

    directional_lights.erase(light->light_id);
    point_lights.erase(light->light_id);
}

void GLRenderer::update_picking_id_model_instance(GLDrawable* drawable) {
    uint32_t dr_p_id = drawable->packed_id();
    m_picking_id_map.erase(dr_p_id);
    m_picking_id_map.insert_or_assign(dr_p_id, drawable->m_picking_id);
}

void GLRenderer::destroy_instanced_drawable(GLInstancedDrawable* drawable) {
    EV_CORE_ASSERT(drawable);
    instanced_drawables.erase(drawable->id);
}

void GLRenderer::render(const Camera &camera) {
    std::chrono::time_point<std::chrono::system_clock> start, end;

    start = std::chrono::system_clock::now();

    if (uniforms_dirty) {
        load_ssao_uniforms();
    }

    // pre render data updates
    for (auto& m : materials) {
        Material* material = m.second;
        material_data_buffer.at(material->material_slot).update_from(material);
    }

    for(mat_slot_t i = 0; i < MAX_N_MATERIALS; i++) {
        if (material_data_buffer[i].changed)
            update_material(i, material_data_buffer[i]);
    }

    
    const glm::mat4 P = camera.get_projection();
    const glm::mat4 V = camera.get_view();
    const glm::mat4 VP = P * V;
    
    if (!pause_cull) // update frustum
        cull_frustum = Frustum{VP};


    // update globals buffer with frame info
    globals_desc.set_parameter(goffsets.P_ind, P, shader_globals);
    globals_desc.set_parameter(goffsets.PInv_ind, glm::inverse(P), shader_globals);
    globals_desc.set_parameter(goffsets.View_ind, V, shader_globals);
    globals_desc.set_parameter(goffsets.VInv_ind, glm::inverse(V), shader_globals);
    globals_desc.set_parameter(goffsets.VP_ind, VP, shader_globals);
    globals_desc.set_parameter(goffsets.CameraPos_ind, camera.get_position(), shader_globals);
    globals_desc.set_parameter(goffsets.CameraDir_ind, camera.get_forward(), shader_globals);

    glm::mat4 light_vp;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Shadow Pass");

    if (shadow_directional_light_id >= 0) {
        //set up shadow shader
        depth_program.program->use();
        depth_fbo.bind();
        //set up light's depth map
        glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        const glm::vec3 directional_light_pos = camera.get_position() + directional_lights[shadow_directional_light_id].position;
        const float dist_to_camera = glm::length(directional_lights[shadow_directional_light_id].position);
        glm::mat4 LV =  glm::lookAt(directional_light_pos, camera.get_position(), camera.get_forward());
                    //    glm::inverse(glm::translate(glm::identity<glm::mat4>(), directional_light_pos));
        
        std::array<glm::vec3, 8> worldPoints = camera.extract_frustum_corners_world(100.f);
        float minX = INFINITY, maxX = -INFINITY, minY = INFINITY, maxY = -INFINITY;
        for (auto &point : worldPoints) {
            glm::vec3 ls_point = LV * glm::vec4{point, 1.0};
            if (ls_point.x < minX)
                minX = ls_point.x;
            if (ls_point.x > maxX)
                maxX = ls_point.x;
            if (ls_point.y < minY)
                minY = ls_point.y;
            if (ls_point.y > maxY)
                maxY = ls_point.y;
        }

        const float shadow_near = 0.1f;
        const float shadow_far = 2.f * dist_to_camera;
        const float shadow_bias = 2.f * shadow_bias_world / (shadow_far - shadow_near);
        const glm::mat4 LO = glm::ortho(minX, maxX, minY, maxY, shadow_near, shadow_far);
        const glm::mat4 LOV = LO * LV;
        const glm::mat4 bias_mat = {
            glm::vec4{.5f, 0, 0, 0},
            glm::vec4{0, .5f, 0, 0},
            glm::vec4{0, 0, .5f, 0},
            glm::vec4{.5f, .5f, .5f - shadow_bias, 0}
        };
        light_vp = bias_mat * LOV;

        //render scene
        ev2::gl::glUniform(LOV, sdp_lpv_location);
        for (auto &mPair : model_instances) {
            auto m = mPair.second;
            if (m->m_mesh) {
                ev2::gl::glUniform(m->transform, sdp_m_location);

                draw(m->m_mesh.get(), depth_program, false, m->gl_vao);
            }
        }
        depth_program.program->unbind();
        depth_fbo.unbind();
    }

    glPopDebugGroup();

    // render all geometry to g buffer
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Geometry Pass");

    g_buffer.bind();
    geometry_program.program->use();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


    // bind global shader UBO to shader
    shader_globals.bind_range(globals_desc.location_index);
    lighting_materials.bind_range(lighting_materials_desc.location_index);

    int cull_count = 0;
    for (auto &mPair : model_instances) {
        auto m = mPair.second;
        if (m->m_mesh) {
            bool visible = true;
            switch(m->m_mesh->frustum_cull) {
                case FrustumCull::None:
                    break;
                case FrustumCull::Sphere:
                    visible = intersect(cull_frustum, m->transform *m->m_mesh->bounding_sphere);
                    break;
                case FrustumCull::AABB:
                    visible = intersect(cull_frustum, m->transform * m->m_mesh->bounding_box);
                    break;
            }
            if (!visible && culling_enabled) {
                cull_count++;
                continue;
            }
            const glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(m->transform)));

            ev2::gl::glUniform(m->transform, gp_m_location);
            ev2::gl::glUniform(V * m->transform, gp_mv_location);
            ev2::gl::glUniform(G, gp_g_location);

            const int obj_id_loc = geometry_program.obj_id_loc;
            if (obj_id_loc >= 0) {
                GL_CHECKED_CALL(gl::glUniform(m->id_color, obj_id_loc));
            }

            int32_t mat_id_override = m->material_override ? m->material_override->get_material_id() : -1;

            draw(m->m_mesh.get(), geometry_program, true, m->gl_vao, mat_id_override);
        }
    }
    geometry_program.program->unbind();

    // render instanced geometry
    geometry_program_instanced.program->use();
    // bind global shader UBO to shader
    shader_globals.bind_range(globals_desc.location_index);
    lighting_materials.bind_range(lighting_materials_desc.location_index);
    for (auto &mPair : instanced_drawables) {
        auto m = mPair.second;
        if (m->m_mesh) {
            ev2::gl::glUniform(m->instance_world_transform, gpi_m_location);

            draw(m->m_mesh.get(), geometry_program_instanced, true, m->gl_vao, -1, m->n_instances);
        }
    }
    geometry_program_instanced.program->unbind();

    // terrain render
    const RenderState current_state{
        &g_buffer,
        &camera
    };

    for (auto& pass : m_passes) {
        pass->render(current_state);
    }

    g_buffer.unbind();

    glPopDebugGroup();

    // glFlush();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "SSAO");

    // ssao pass
    ssao_buffer.bind();

    // bind global shader UBO to shader
    shader_globals.bind_range(globals_desc.location_index);

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    
    ssao_program.program->use();

    if (ssao_p_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        position->bind();
        gl::glUniformSampler(0, ssao_p_loc);
    }

    if (ssao_n_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, ssao_n_loc);
    }

    if (ssao_tex_noise_loc >= 0) {
        glActiveTexture(GL_TEXTURE2);
        ssao_kernel_noise->bind();
        gl::glUniformSampler(2, ssao_tex_noise_loc);
    }

    // moved to glProgramUniforms
    // gl::glUniformf(ssao_bias, ssao_bias_loc);
    // gl::glUniformf(ssao_radius, ssao_radius_loc);
    // gl::glUniformui(ssao_kernel_samples, ssao_nSamples_loc);

    ssao_kernel_desc.bind_buffer(ssao_kernel_buffer);

    draw_screen_space_triangle();

    position->unbind();
    normals->unbind();
    ssao_kernel_noise->unbind();

    ssao_program.program->unbind();
    ssao_buffer.unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Lighting Pass");

    // lighting pass
    // glFlush();
    lighting_buffer.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // add all lighting contributions
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // stencil lighting areas
    glEnable(GL_STENCIL_TEST);
    // directional lighting programs discards when pos buffer zero
    glStencilMask(255);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // value to write in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // only write new value when fragment color is written

    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup lighting program
    directional_lighting_program.program->use();
    shader_globals.bind_range(globals_desc.location_index);
    lighting_materials.bind_range(lighting_materials_desc.location_index);


    if (lp_p_location >= 0) {
        glActiveTexture(GL_TEXTURE0);
        position->bind();
        gl::glUniformSampler(0, lp_p_location);
    }

    if (lp_n_location >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, lp_n_location);
    }

    if (lp_as_location >= 0) {
        glActiveTexture(GL_TEXTURE2);
        albedo_spec->bind();
        gl::glUniformSampler(2, lp_as_location);
    }

    if (lp_mt_location >= 0) {
        glActiveTexture(GL_TEXTURE3);
        material_tex->bind();
        gl::glUniformSampler(3, lp_mt_location);
    }

    if (lp_gao_location >= 0) {
        glActiveTexture(GL_TEXTURE4);
        ssao_kernel_color->bind();
        gl::glUniformSampler(4, lp_gao_location);
    }

    if (lp_sdt_location >= 0) {
        glActiveTexture(GL_TEXTURE5);
        shadow_depth_tex->bind();
        gl::glUniformSampler(5, lp_sdt_location);      
    }

    gl::glUniform(light_vp, lp_ls_location);

    for (auto& litr : directional_lights) {
        auto& l = litr.second;
        gl::glUniform(glm::normalize(l.position), lp_ldir_location);
        gl::glUniform(l.color, lp_lcol_location);
        gl::glUniform(l.ambient, lp_lamb_location);
        
        draw_screen_space_triangle();
    }


    position->unbind();
    normals->unbind();
    albedo_spec->unbind();
    material_tex->unbind();
    ssao_kernel_color->unbind();
    shadow_depth_tex->unbind();

    directional_lighting_program.program->unbind();

    // pointlight pass
    point_lighting_program.program->use();
    shader_globals.bind_range(globals_desc.location_index);

    if (plp_n_location >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, plp_n_location);
    }

    if (plp_as_location >= 0) {
        glActiveTexture(GL_TEXTURE2);
        albedo_spec->bind();
        gl::glUniformSampler(2, plp_as_location);
    }

    if (plp_mt_location >= 0) {
        glActiveTexture(GL_TEXTURE3);
        material_tex->bind();
        gl::glUniformSampler(3, plp_mt_location);
    }

    int index = 0;
    std::vector<PointLightData> point_light_data(point_lights.size());
    for (auto& litr : point_lights) {
        auto& l = litr.second;

        // from https://learnopengl.com/Advanced-Lighting/Deferred-Shading
        float constant  = l.k.x;
        float linear    = l.k.y;
        float quadratic = l.k.z;
        float lightMax  = std::fmaxf(std::fmaxf(l.color.r, l.color.g), l.color.b);
        float radius    = 
        (-linear +  sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) 
        / (2 * quadratic);

        PointLightData light_data;
        light_data.position = l.position;
        light_data.radius = radius;
        light_data.lightColor = l.color;
        light_data.scale = point_light_geom_base_scale * 2.f * radius;
        light_data.k_c = constant;
        light_data.k_l = linear;
        light_data.k_q = quadratic;

        point_light_data[index] = light_data;
        index++;
    }
    point_light_data_buffer->copy_data(point_light_data);

    // bind the point light data buffer to SSBO
    point_light_data_buffer->bind(plp_ssbo_light_data_location);

    draw(point_light_mesh.get(), point_lighting_program, false, point_light_gl_vao, -1, point_lights.size());

    point_light_data_buffer->unbind();

    normals->unbind();
    albedo_spec->unbind();
    material_tex->unbind();

    point_lighting_program.program->unbind();

    // sky program
    // draw into non lit pixels in hdr fbo
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);


    float time = (float)glfwGetTime() - 0.0f;
    glProgramUniform1f(sky_program.program->getHandle(), sky_time_loc, time*cloud_speed);
    glProgramUniform1f(sky_program.program->getHandle(), sky_sun_position_loc, sun_position);
    glProgramUniform1f(sky_program.program->getHandle(), sky_output_mul_loc, sky_brightness);

    sky_program.program->use();
    shader_globals->bind_range(globals_desc.location_index);
    draw_screen_space_triangle();

    sky_program.program->unbind();
    lighting_buffer.unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Bloom");

    // bloom threshold and combine
    bloom_thresh_combine.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // sst
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

    post_fx_bloom_combine_program.program.use();

    if (post_fx_bc_hdrt_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        hdr_texture->bind();
        gl::glUniformSampler(0, post_fx_bc_hdrt_loc);
    }

    if (post_fx_bc_emist_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        emissive->bind();
        gl::glUniformSampler(1, post_fx_bc_emist_loc);
    }

    gl::glUniformf(bloom_threshold, post_fx_bc_thresh_loc);

    draw_screen_space_triangle();

    post_fx_bloom_combine_program.program->unbind();
    bloom_thresh_combine.unbind();

    // bloom blur
    int bloom_output_tex_ind = 0;
    post_fx_bloom_blur.program->use();
    for (int i = 0; i < bloom_iterations * 2; i++) {
        bloom_output_tex_ind = (i + 1) % 2;

        bloom_blur_swap_fbo[bloom_output_tex_ind].bind();

        glActiveTexture(GL_TEXTURE0);
        bloom_blur_swap_tex[i%2]->bind();
        gl::glUniformSampler(0, post_fx_bb_bloom_in_loc);

        gl::glUniformi(bloom_output_tex_ind, post_fx_bb_hor_loc); // boolean

        draw_screen_space_triangle();

        bloom_blur_swap_fbo[bloom_output_tex_ind].unbind();
    }
    post_fx_bloom_blur.program->unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Post Pass");

    // post fx pass
    glDisable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    post_fx_program.program->use();

    gl::glUniformf(exposure, post_fx_exposure_loc);
    gl::glUniformf(bloom_falloff, post_fx_bloom_falloff_loc);
    gl::glUniformf(gamma, post_fx_gamma_loc);

    if (post_fx_hdrt_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        hdr_combined->bind();
        gl::glUniformSampler(0, post_fx_hdrt_loc);
    }

    if (post_fx_bloomt_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        bloom_blur_swap_tex[bloom_output_tex_ind]->bind();
        gl::glUniformSampler(1, post_fx_bloomt_loc);
    }

    draw_screen_space_triangle();

    hdr_texture->unbind();

    post_fx_program.program->unbind();

    glPopDebugGroup();

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    glFlush();
    glFinish();
    
    if (!m_id_frame_save_async.valid() && m_recording_id_frames) {
        // get image data from OpenGL
        // have to do it on this thread
        g_buffer.bind();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer((GLenum)gl::FBOAttachment::COLOR3);
        Image image{(int)width, (int)height, 1, 1};
        GL_CHECKED_CALL(glReadnPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, image.memory_requirement(), image.data()));
        g_buffer.unbind();

        // prepare and start file writing task
        auto save_task = [](Image image) {
            static int frame_num = 0;
            image.write(fs::path{"570_data"} / ("frame_" + std::to_string(frame_num++) + ".png"));
        };

        m_id_frame_save_async = std::async(std::launch::async, save_task, std::move(image));
    }

    if (m_id_frame_save_async.valid() &&
        m_id_frame_save_async.wait_for(std::chrono::milliseconds{0}) ==
            std::future_status::ready) {
        try {
            m_id_frame_save_async.get();
            Log::info_core<GLRenderer>("Frame Saved");
        } catch (std::exception& e) {
            Log::error_core<GLRenderer>(e.what());
        }
    }

    // std::cout << "size: " << (sizeof(data)) << " Center ID: ";
    // for (int i = 0; i < sizeof(data)/sizeof(data[0]); ++i) {
    //     std::cout << (int)data[i] << ", ";
    // } 
    // std::cout << "\n";


    // std::cout << "render() elapsed time: " << elapsed_seconds.count() << "s\n";
}

void GLRenderer::set_wireframe(bool enable) {
    wireframe = enable;
}

void GLRenderer::screenshot() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    Image image{(int)width, (int)height, 3, 1};
    GL_CHECKED_CALL(glReadnPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image.memory_requirement(), image.data()));

    try {
        std::string filename = "Screenshot_" + util::formatted_current_time() + ".png";
        image.write(filename);
        Log::info_core<GLRenderer>("Screenshot " + filename + " Saved");
    } catch (std::exception& e) {
        Log::error_core<GLRenderer>(e.what());
    }
}

std::size_t GLRenderer::read_obj_fb(const glm::uvec2& screen_point) {
    if (screen_point.x > width || screen_point.y > height) {
        return 0; // TODO should this throw exception??
    }
    g_buffer.bind();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer((GLenum)gl::FBOAttachment::COLOR5);
    Image image{2, 1, 3, 1};
    GL_CHECKED_CALL(glReadnPixels(screen_point.x, height-screen_point.y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, image.memory_requirement(), image.data()));
    g_buffer.unbind();

    glm::uvec3 id_vec{image.data()[0], image.data()[1], image.data()[2]};
    uint32_t id = unpack_uvec3_to_id(id_vec);
    std::size_t picking_id{};
    auto itr = m_picking_id_map.find(id);
    if (itr != m_picking_id_map.end())
        picking_id = itr->second;
    return picking_id;
}

void GLRenderer::set_resolution(uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;

    g_buffer.resize_all(width, height);
    ssao_buffer.resize_all(width, height);
    lighting_buffer.resize_all(width, height);
    bloom_thresh_combine.resize_all(width, height);
    bloom_blur_swap_fbo[0].resize_all(width, height);
    bloom_blur_swap_fbo[1].resize_all(width, height);
}

void GLRenderer::draw_screen_space_triangle() {
    glBindVertexArray(sst_vb.second);
    GL_CHECKED_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    glBindVertexArray(0);
}

}