#include <scene/visual_nodes.hpp>

#include <scene/scene.hpp>
#include <renderer/shader.hpp>

namespace ev2 {

void VisualInstance::on_init() {
    iid = renderer::Renderer::get_singleton().create_model_instance();
}

void VisualInstance::on_ready() {
    iid->transform = get_transform();
}

void VisualInstance::on_process(float delta) {
    
}

void VisualInstance::on_destroy() {
    renderer::Renderer::get_singleton().destroy_model_instance(iid);
}

void VisualInstance::pre_render() {
    iid->transform = get_transform();
}

void VisualInstance::set_model(std::shared_ptr<renderer::Drawable> model) {
    iid->set_drawable(model);
}

void VisualInstance::set_material_override(Ref<renderer::Material> material_override) {
    iid->set_material_override(material_override);
}

void InstancedGeometry::on_init() {
    renderer::VertexBufferLayout quad_layout;
    quad_layout .add_attribute(renderer::VertexAttributeLabel::Vertex)
                .add_attribute(renderer::VertexAttributeLabel::Normal)
                .add_attribute(renderer::VertexAttributeLabel::Texcoord)
                .finalize();
    geometry = std::make_shared<renderer::Drawable>(
        renderer::VertexBuffer::vbInitArrayVertexSpec(
            {
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
            },
            quad_layout),
        std::vector<renderer::Primitive>{renderer::Primitive{0, 12, -1}},
        std::vector<Ref<renderer::Material>>{},
        AABB{},
        Sphere{glm::vec3{0.f}, 0.05f},
        renderer::FrustumCull::Sphere,
        gl::CullMode::BACK,
        gl::FrontFacing::CCW
    );
    instance = ev2::renderer::Renderer::get_singleton().create_instanced_drawable();
    instance->set_drawable(geometry);
}

void InstancedGeometry::on_destroy() {
    
}

void InstancedGeometry::pre_render() {
    if (instance) {
        instance->instance_world_transform = get_transform();
        instance->instance_transform_buffer->copy_data(instance_transforms);
        instance->n_instances = instance_transforms.size();
    }
}

void InstancedGeometry::set_material_override(Ref<renderer::Material> material_override) {
    assert(geometry);
    if (material_override) {
        geometry->materials.resize(1);
        geometry->materials[0] = material_override;
        geometry->primitives[0].material_ind = 0;
    } else {
        geometry->primitives[0].material_ind = -1;
    }
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
    float r_aspect = ev2::renderer::Renderer::get_singleton().get_aspect_ratio();
    
    camera.set_perspective(fov, aspect * r_aspect, m_near, m_far);

    auto tr = get_transform();
    camera.set_position(glm::vec3(tr[3]));
    camera.set_rotation(glm::quat_cast(tr));
}

void DirectionalLightNode::on_init() {
    lid = ev2::renderer::Renderer::get_singleton().create_directional_light();
}

void DirectionalLightNode::on_ready() {

}

void DirectionalLightNode::on_process(float delta) {
    
}

void DirectionalLightNode::on_destroy() {
    ev2::renderer::Renderer::get_singleton().destroy_light(lid);
}

void DirectionalLightNode::pre_render() {
    ev2::renderer::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_transform()[3]));
}

void DirectionalLightNode::set_color(const glm::vec3& color) {
    ev2::renderer::Renderer::get_singleton().set_light_color(lid, color);
}

void DirectionalLightNode::set_ambient(const glm::vec3& color) {
    ev2::renderer::Renderer::get_singleton().set_light_ambient(lid, color);
}

// point

void PointLightNode::on_init() {
    lid = ev2::renderer::Renderer::get_singleton().create_point_light();
}

void PointLightNode::on_ready() {

}

void PointLightNode::on_process(float delta) {
    
}

void PointLightNode::on_destroy() {
    ev2::renderer::Renderer::get_singleton().destroy_light(lid);
}

void PointLightNode::pre_render() {
    ev2::renderer::Renderer::get_singleton().set_light_position(lid, glm::vec3(get_transform()[3]));
}

void PointLightNode::set_color(const glm::vec3& color) {
    ev2::renderer::Renderer::get_singleton().set_light_color(lid, color);
}

}