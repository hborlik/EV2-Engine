/**
 * @file renderer.hpp
 * @brief 
 * @date 2023-08-11
 * 
 */
#ifndef EV2_RENDERER_HPP
#define EV2_RENDERER_HPP

#include "core/singleton.hpp"
#include "io/model.hpp"
#include "renderer/mesh.hpp"
#include "renderer/material.hpp"
#include "renderer/camera.hpp"

namespace ev2::renderer {

enum class FrustumCull {
    None,
    Sphere,
    AABB
};

class PointLight {
public:
    virtual ~PointLight() = default;

    virtual void set_color(const glm::vec3& color) = 0;
    virtual void set_position(const glm::vec3& position) = 0;
    virtual void set_k(const glm::vec3& k) = 0;
};

class DirectionalLight {
public:
    virtual ~DirectionalLight() = default;

    virtual void set_color(const glm::vec3& color) = 0;
    virtual void set_position(const glm::vec3& position) = 0;
    virtual void set_ambient(const glm::vec3& ambient) = 0;
};

class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void set_mesh(std::shared_ptr<Mesh> mesh) = 0;
    virtual void set_material(std::shared_ptr<Material> material) = 0;
    virtual void set_transform(const glm::mat4& transform) = 0;
};

class InstancedDrawable {
public:
    virtual ~InstancedDrawable() = default;

    virtual void set_mesh(std::shared_ptr<Mesh> mesh) = 0;
    virtual void set_material(std::shared_ptr<Material> material) = 0;
    virtual void set_transform() = 0;
};

class Renderer : public Singleton<Renderer> {
public:
    virtual void init() = 0;

    virtual std::shared_ptr<Mesh> make_mesh(const Model* model = nullptr) = 0;
    virtual std::shared_ptr<Material> make_material(const MaterialData* material = nullptr) = 0;
    virtual std::shared_ptr<PointLight> make_point_light() = 0;
    virtual std::shared_ptr<DirectionalLight> make_directional_light() = 0;
    virtual std::shared_ptr<Texture> make_texture(TextureType type, const Image* image = nullptr) = 0;
    virtual std::shared_ptr<Drawable> make_drawable() = 0;
    virtual std::shared_ptr<InstancedDrawable> make_instanced_drawable() = 0;

    virtual void set_resolution(uint32_t width, uint32_t height) = 0;

    virtual void render(const Camera &camera) = 0;
};

}

#endif // EV2_RENDERER_HPP