/**
 * @file resource.h
 * @brief disk resource loader
 * @date 2022-04-18
 * 
 */
#ifndef EV2_RESOURCE_H
#define EV2_RESOURCE_H

#include <filesystem>

#include <glm/glm.hpp>

#include <renderer/mesh.h>
#include <renderer/renderer.h>
#include <renderer/texture.h>
#include <renderer/material.h>
#include <image.hpp>
#include <scene/scene.h>
#include <gltf.h>

namespace ev2 {

struct ImageResource {
    std::shared_ptr<renderer::Texture> texture;
    std::shared_ptr<Image> image;
};

struct MaterialData {
    std::string name = "default";

    glm::vec3 diffuse   = {1.00f,0.10f,0.85f};
    glm::vec3 emissive  = {};
    float metallic       = 0;
    float subsurface     = 0;
    float specular       = .5f;
    float roughness      = .5f;
    float specularTint   = 0;
    float clearcoat      = 0;
    float clearcoatGloss = 1.f;
    float anisotropic    = 0;
    float sheen          = 0;
    float sheenTint      = .5f;

    std::shared_ptr<ImageResource> ambient_tex;             // map_Ka
    std::shared_ptr<ImageResource> diffuse_tex;             // map_Kd
    std::shared_ptr<ImageResource> specular_tex;            // map_Ks
    std::shared_ptr<ImageResource> specular_highlight_tex;  // map_Ns
    std::shared_ptr<ImageResource> bump_tex;                // map_bump, map_Bump, bump
    std::shared_ptr<ImageResource> displacement_tex;        // disp
    std::shared_ptr<ImageResource> alpha_tex;               // map_d
    std::shared_ptr<ImageResource> reflection_tex;          // refl

    Ref<renderer::Material> create_renderer_material() const;
};

struct DrawObject {
    size_t start;
    size_t numTriangles;
    size_t material_id;
};

// single buffer model
class Model {
public:
    Model(const std::string &name,
          std::vector<DrawObject> draw_objects,
          std::vector<MaterialData> materials,
          glm::vec3 bmin,
          glm::vec3 bmax,
          std::vector<float> vb) : name{name},
                                   draw_objects{std::move(draw_objects)},
                                   materials{std::move(materials)},
                                   buffer{std::move(vb)},
                                   bmin{bmin},
                                   bmax{bmax}
                                   {}

    std::string             name;
    std::vector<DrawObject> draw_objects;
    std::vector<MaterialData>   materials;
    std::vector<float>      buffer;

    glm::vec3 bmin, bmax;
};

class ResourceManager : public Singleton<ResourceManager> {
public:
    struct MaterialLocation {
        int32_t update_internal();
        int32_t material_id = -1;
    };

    explicit ResourceManager(const std::filesystem::path& asset_path) : asset_path{asset_path}, model_lookup{} {}
    ~ResourceManager();

    void pre_render();

    /**
     * @brief Get the model object reference id, or load object if not available
     * 
     * @param filename 
     * @return renderer::MID 
     */
    std::shared_ptr<renderer::Drawable> get_model(const std::filesystem::path& filename, bool cache = true);

    std::shared_ptr<renderer::Drawable> create_model(std::shared_ptr<Model> model);

    std::shared_ptr<ImageResource> get_image(const std::filesystem::path& filename, bool ignore_asset_path = false);

    // Ref<GLTFScene> loadGLTF(const std::filesystem::path& filename, bool normalize = false);

    Ref<renderer::Material> get_material(const std::string& name);
    int32_t get_material_id(const std::string& name);

    const auto& get_materials() const {return materials;}

private:
    std::filesystem::path asset_path;

    std::unordered_map<std::string, std::weak_ptr<renderer::Drawable>> model_lookup;

    std::unordered_map<std::string, Ref<renderer::Material>> materials;

    std::unordered_map<std::string, std::shared_ptr<ImageResource>> images;
};

std::unique_ptr<Model> loadObj(const std::filesystem::path& filename, const std::filesystem::path& base_dir, ResourceManager* rm = nullptr);

std::unique_ptr<renderer::Texture> load_texture2D(const std::filesystem::path& filename);

}

#endif // EV2_RESOURCE_H