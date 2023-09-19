#include "renderer/texture.hpp"
#include "renderer/render_api.hpp"

#include "renderer/opengl_renderer/gl_texture.hpp"

namespace ev2::renderer {

std::unique_ptr<Texture> Texture::make_texture(TextureType type) {
    switch(type) {
        case TextureType::Tex_1D:
            return Texture1D::make_texture();
        case TextureType::Tex_2D:
            return Texture2D::make_texture();
        default: return {};
    };
}

std::unique_ptr<Texture1D> Texture1D::make_texture() {
    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL:
            return std::make_unique<GLTexture1D>();
        default: 
            EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
            break;
    }
    return {};
}

std::unique_ptr<Texture2D> Texture2D::make_texture() {
    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL:
            return std::make_unique<GLTexture2D>();
        default: 
            EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
            break;
    }
    return {};
}

}