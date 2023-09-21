#include "scene/visual_nodes.hpp"
#include <utility>

#include "renderer/opengl_renderer/gl_shader.hpp"

namespace ev2 {

void VisualInstance::on_init() {
    iid = renderer::Renderer::get_singleton().make_drawable();
    iid->set_picking_id(uuid_hash);
}

void VisualInstance::on_ready() {
    iid->set_transform(get_world_transform());
}

void VisualInstance::on_destroy() {
    iid.reset();
}

void VisualInstance::pre_render() {
    iid->set_transform(get_world_transform());
}

void VisualInstance::set_mesh(std::shared_ptr<renderer::Mesh> mesh) {
    iid->set_mesh(mesh);
}

void VisualInstance::set_material_override(std::shared_ptr<renderer::Material> material_override) {
    iid->set_material(material_override);
}

void InstancedGeometry::on_init() {
    renderer::VertexBufferLayout quad_layout;
    quad_layout .add_attribute(renderer::AttributeLabel::Vertex)
                .add_attribute(renderer::AttributeLabel::Normal)
                .add_attribute(renderer::AttributeLabel::Texcoord)
                .finalize();
    auto buffer = std::shared_ptr(
        renderer::Buffer::make_buffer(renderer::BufferUsage::Vertex, renderer::BufferAccess::Static));
    buffer->allocate(std::vector<float>{
            // positions         normals         texcoords
            -0.05f,  0.05f, .0f, .0f, .0f, -1.f, 1.f, 1.f,
            0.05f, -0.05f, .0f, .0f, .0f, -1.f, 0.f, 0.f,
            -0.05f, -0.05f, .0f, .0f, .0f, -1.f, 1.f, 0.f,

            -0.05f,  0.05f, .0f, .0f, .0f, -1.f, 1.f, 1.f,
            0.05f,  0.05f, .0f, .0f, .0f, -1.f, 0.f, 1.f,
            0.05f, -0.05f, .0f, .0f, .0f, -1.f, 0.f, 0.f,
            
            // back
            -0.05f,  0.05f, .0f, .0f, .0f, 1.f, 1.f, 1.f,
            -0.05f, -0.05f, .0f, .0f, .0f, 1.f, 1.f, 0.f,
            0.05f, -0.05f, .0f, .0f, .0f, 1.f, 0.f, 0.f,

            -0.05f,  0.05f, .0f, .0f, .0f, 1.f, 1.f, 1.f,
            0.05f, -0.05f, .0f, .0f, .0f, 1.f, 0.f, 0.f,
            0.05f,  0.05f, .0f, .0f, .0f, 1.f, 0.f, 1.f,
        });
    auto vb = std::shared_ptr(renderer::VertexBuffer::vbInitArrayVertexSpec(
        buffer,
        quad_layout));
    
    geometry = renderer::Mesh::make_mesh(
        vb,
        {}, // index buffer unused
        std::vector<renderer::Primitive>{renderer::Primitive{0, 12, -1}},
        std::vector<std::shared_ptr<renderer::Material>>{},
        AABB{},
        Sphere{glm::vec3{0.f}, 0.05f},
        renderer::FrustumCull::Sphere,
        renderer::CullMode::Back,
        renderer::FrontFacing::CCW
    );
    instance = ev2::renderer::Renderer::get_singleton().make_instanced_drawable();
    instance->set_mesh(geometry);
}

void InstancedGeometry::on_destroy() {
    instance.reset();
}

void InstancedGeometry::pre_render() {
    if (instance && m_transforms_modified) {
        m_transforms_modified = false;
        instance->instance_transform_buffer->copy_data(instance_transforms);
        instance->n_instances = instance_transforms.size();
    }
}

void InstancedGeometry::set_material_override(std::shared_ptr<renderer::Material> material_override) {
    EV_CORE_ASSERT(geometry != nullptr);
    if (material_override) {
        geometry->materials.resize(1);
        geometry->materials[0] = material_override;
        geometry->primitives[0].material_ind = 0;
    } else {
        geometry->primitives[0].material_ind = -1;
    }
}

void InstancedGeometry::on_transform_changed(Ref<Node> origin) {
    Node::on_transform_changed(origin);

    instance->set_transform(get_world_transform());
}

void InstancedGeometry::set_instance_transforms(std::vector<glm::mat4> tr) {
    instance_transforms = std::move(tr);
    m_transforms_modified = true;
}

// camera

void CameraNode::on_ready() {
    
}

void CameraNode::on_process(float dt) {
    
}

void CameraNode::on_destroy() {
    
}

void CameraNode::pre_render() {
    update_internal();
}

void CameraNode::update_internal() {
    float r_aspect = ev2::renderer::GLRenderer::get_singleton().get_aspect_ratio();
    
    camera.set_perspective(fov, aspect * r_aspect, m_near, m_far);

    auto tr = get_world_transform();
    camera.set_position(glm::vec3(tr[3]));
    camera.set_rotation(glm::quat_cast(tr));
}

void DirectionalLightNode::on_init() {
    lid = ev2::renderer::Renderer::get_singleton().make_directional_light();
}

void DirectionalLightNode::on_ready() {

}

void DirectionalLightNode::on_process(float delta) {
    
}

void DirectionalLightNode::on_destroy() {
    // ev2::renderer::Renderer::get_singleton().destroy_light(lid);
}

void DirectionalLightNode::pre_render() {
    // ev2::renderer::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_world_transform()[3]));
    lid->set_position(glm::vec3(get_world_transform()[3]));
}

void DirectionalLightNode::set_color(const glm::vec3& color) {
    // ev2::renderer::Renderer::get_singleton().set_light_color(lid, color);
    lid->set_color(color);
}

void DirectionalLightNode::set_ambient(const glm::vec3& color) {
    // ev2::renderer::Renderer::get_singleton().set_light_ambient(lid, color);
    lid->set_color(color);
}

// point

void PointLightNode::on_init() {
    lid = ev2::renderer::Renderer::get_singleton().make_point_light();
}

void PointLightNode::on_ready() {

}

void PointLightNode::on_process(float delta) {
    
}

void PointLightNode::on_destroy() {
    // ev2::renderer::Renderer::get_singleton().destroy_light(lid);
}

void PointLightNode::pre_render() {
    // ev2::renderer::GLRenderer::get_singleton().set_light_position(lid, glm::vec3(get_world_transform()[3]));
    lid->set_position(glm::vec3(get_world_transform()[3]));
}

void PointLightNode::set_color(const glm::vec3& color) {
    // ev2::renderer::GLRenderer::get_singleton().set_light_color(lid, color);
    lid->set_color(color);
}

}