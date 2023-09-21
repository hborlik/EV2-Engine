#include "resource.hpp"

#include "renderer/opengl_renderer/gl_buffer.hpp"
#include "renderer/opengl_renderer/ev_gl.hpp"
#include "renderer/renderer.hpp"
#include "io/serializers.hpp"
#include "core/engine.hpp"

#include <glm/glm.hpp>

namespace ev2 {

ResourceManager::~ResourceManager() {
    // TODO destroy models
    // for (auto v : model_lookup) {
    //     renderer::Renderer::get_singleton().destroy_render_obj()
    // }
}

void ResourceManager::pre_render() {

}

std::shared_ptr<renderer::Mesh> ResourceManager::get_model(const std::filesystem::path& filename, bool cache, bool load_materials) {
    const auto full_path = asset_path / filename;
    return get_model_relative_path(full_path, cache, load_materials);
}

std::shared_ptr<renderer::Mesh> ResourceManager::get_model_relative_path(const std::filesystem::path& filename, bool cache, bool load_materials) {
    auto itr = model_lookup.find(filename.generic_string());
    // check that the cached pointer is still good if it has been deleted
    if (itr != model_lookup.end() && cache && !itr->second.expired()) {
        return itr->second.lock();
    }
    std::shared_ptr<Model> loaded_model = load_model(
        filename.filename().generic_string(),
        filename.parent_path().generic_string());
    if (loaded_model) {
        auto mesh = renderer::Renderer::get_singleton().make_mesh(loaded_model.get());
        if (cache)
            model_lookup.insert_or_assign(filename.generic_string(), mesh);
        return mesh;
    } else {
        Log::error_core<ResourceManager>("Failed to load model {}", filename.generic_string());
        return {};
    }
}

std::shared_ptr<renderer::Texture> ResourceManager::get_image(const std::filesystem::path& filename, bool ignore_asset_path, bool is_srgb) {
    auto itr = images.find(filename.generic_string());
    if (itr != images.end()) { // already loaded
        return itr->second;
    }

    std::filesystem::path base_path = ignore_asset_path ? "" : asset_path;
    std::string input_file = (base_path / filename).generic_string();

    std::shared_ptr<Image> image = load_image(input_file);
    if (image) {
        gl::TextureInternalFormat internal_format;
        gl::PixelFormat pixel_format;
        switch(image->comp()) {
            case 1:
                internal_format = gl::TextureInternalFormat::RED;
                pixel_format = gl::PixelFormat::RED;
                break;
            case 2:
                internal_format = gl::TextureInternalFormat::RG;
                pixel_format = gl::PixelFormat::RG;
                break;
            case 3:
                internal_format = is_srgb ? gl::TextureInternalFormat::SRGB8 : gl::TextureInternalFormat::RGB;
                pixel_format = gl::PixelFormat::RGB;
                break;
            case 4:
                internal_format = is_srgb ? gl::TextureInternalFormat::SRGB8_ALPHA8 : gl::TextureInternalFormat::RGBA;
                pixel_format = gl::PixelFormat::RGBA;
                break;
            default:
                Log::error_core<ResourceManager>("unable to load {}. unsupported texture format. Channels: {}", filename, image->comp());
                return {};
        }

        auto texture = std::shared_ptr(renderer::Texture2D::make_texture());
        // texture->set_texture_wrap_s(gl::TextureWrapMode::REPEAT);
        // texture->set_texture_wrap_t(gl::TextureWrapMode::REPEAT);
        texture->set_image2D(internal_format, image->width(), image->height(), pixel_format, gl::PixelType::UNSIGNED_BYTE, image->data());
        texture->generate_mips();

        images.insert({filename.generic_string(), texture});
        Log::info_core<ResourceManager>("Loaded image {}", filename.generic_string());
        return texture;
    } else {
        Log::error_core<ResourceManager>("Failed to load image {}", filename.generic_string());
        return {};
    }
}

std::shared_ptr<renderer::Material> ResourceManager::get_material(const std::string& name) {
    std::shared_ptr<renderer::Material> mat = nullptr;
    auto itr = materials.find(name);
    if (itr == materials.end()) {
        mat = renderer::Renderer::get_singleton().make_material();
        materials.insert({name, mat});
    } else {
        mat = itr->second;
    }
    return mat;
}

std::unique_ptr<renderer::Texture> load_texture2D(const std::filesystem::path& filename) {
    std::unique_ptr<renderer::Texture> out;
    bool error = false;
    if(!filename.empty()) {
        std::string file = filename.generic_string();
        // stbi_set_flip_vertically_on_load(false);
        // unsigned char *image = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
        auto image = load_image(file);
        if(image) {
            int width = image->width();
            int height = image->height();
            int nrChannels = image->comp();
            Log::trace_core("Loaded image data: " + file + ", w = " + std::to_string(width)
                + ", h = " + std::to_string(height) + ", channels = " + std::to_string(nrChannels));

            out = renderer::Texture2D::make_texture();
            if(nrChannels == 1) {
                out->set_image2D(gl::TextureInternalFormat::RED, width, height, gl::PixelFormat::RED, gl::PixelType::UNSIGNED_BYTE, image->data());
            } else if(nrChannels == 2) { // rg
                out->set_image2D(gl::TextureInternalFormat::RG, width, height, gl::PixelFormat::RG, gl::PixelType::UNSIGNED_BYTE, image->data()); 
            } else if(nrChannels == 3) { // rgb
                out->set_image2D(gl::TextureInternalFormat::RGB, width, height, gl::PixelFormat::RGB, gl::PixelType::UNSIGNED_BYTE, image->data());
            } else if(nrChannels == 4) { // rgba
                out->set_image2D(gl::TextureInternalFormat::RGBA, width, height, gl::PixelFormat::RGBA, gl::PixelType::UNSIGNED_BYTE, image->data());
            } else {
                Log::error_core("unable to load texture for image {}. unsupported texture format. Channels: {}", file, nrChannels);
            }
            
            out->generate_mips();
        } else {
            Log::error_core("Failed to load texture: {}", file);
        }
    }
    
    return out;
}


// std::string load_shader_content(const std::filesystem::path& source_path) {
//     std::ifstream in{source_path};

//     if (!in.is_open()) {
//         Log::error_core("Unable to load shader content from {}", source_path);
//         throw std::runtime_error{"Shader File not found at " + source_path.generic_string()};
//     }
//     // copy out file contents
//     std::string content{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
//     in.close();
//     content += '\n';

//     return content;
// }

} // namespace ev2