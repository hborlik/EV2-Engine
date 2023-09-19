/**
 * @file resource.h
 * @brief disk resource loader
 * @date 2022-04-18
 * 
 */
#ifndef EV2_RESOURCE_H
#define EV2_RESOURCE_H

#include "evpch.hpp"

#include <glm/glm.hpp>

#include "core/reference_counted.hpp"
#include "renderer/renderer.hpp"
#include "renderer/material.hpp"
#include "io/image.hpp"
#include "io/model.hpp"
#include "gltf.hpp"

namespace ev2 {

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
     * @return std::shared_ptr<renderer::Mesh>
     */
    std::shared_ptr<renderer::Mesh> get_model(const std::filesystem::path& filename, bool cache = true, bool load_materials = true);

    /**
     * @brief Get the model from a relative path to cwd. This does not use asset_path, unlike get_model()
     * 
     * @param filename 
     * @param cache 
     * @param load_materials 
     * @return std::shared_ptr<renderer::Mesh>
     */
    std::shared_ptr<renderer::Mesh> get_model_relative_path(const std::filesystem::path& filename, bool cache = true, bool load_materials = true);

    std::shared_ptr<renderer::Texture> get_image(const std::filesystem::path& filename, bool ignore_asset_path = false, bool is_srgb = false);

    // Ref<GLTFScene> loadGLTF(const std::filesystem::path& filename, bool normalize = false);

    std::shared_ptr<renderer::Material> get_material(const std::string& name);

    const auto& get_materials() const {return materials;}

private:
    std::filesystem::path asset_path;

    std::unordered_map<std::string, std::weak_ptr<renderer::Mesh>> model_lookup;

    std::unordered_map<std::string, std::shared_ptr<renderer::Material>> materials;

    std::unordered_map<std::string, std::shared_ptr<renderer::Texture>> images;
};

std::unique_ptr<Model> load_model(const std::filesystem::path& filename, const std::filesystem::path& base_dir);

std::unique_ptr<renderer::Texture> load_texture2D(const std::filesystem::path& filename);

// std::string load_shader_content(const std::filesystem::path& source_path);

}

#endif // EV2_RESOURCE_H