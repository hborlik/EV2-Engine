/**
 * @file renderer.hpp
 * @brief 
 * @date 2023-08-11
 * 
 */
#ifndef EV2_RENDERER_HPP
#define EV2_RENDERER_HPP

#include "renderer/mesh.hpp"
#include "renderer/material.hpp"

namespace ev2::renderer {

enum class FrustumCull {
    None,
    Sphere,
    AABB
};

class Light {
public:
    virtual ~Light() = default;

    virtual void set_color() = 0;
    virtual void set_position() = 0;
};

class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void set_mesh(std::shared_ptr<Mesh> mesh) = 0;
    virtual void set_material(std::shared_ptr<Material> material) = 0;
    virtual void set_transform() = 0;
};

class InstancedDrawable {
public:
    virtual ~InstancedDrawable() = default;

    virtual void set_mesh(std::shared_ptr<Mesh> mesh) = 0;
    virtual void set_material(std::shared_ptr<Material> material) = 0;
    virtual void set_transform() = 0;
};

class Renderer {
public:
    virtual std::shared_ptr<Mesh> make_mesh() = 0;
    virtual std::shared_ptr<Material> make_material() = 0;
    virtual std::shared_ptr<Light> make_light() = 0;
    virtual std::shared_ptr<Texture> make_texture() = 0;
    virtual std::shared_ptr<Drawable> make_drawable() = 0;
    virtual std::shared_ptr<InstancedDrawable> make_instanced_drawable() = 0;
};

}

#endif // EV2_RENDERER_HPP