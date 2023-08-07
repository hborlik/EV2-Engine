#include "renderer/shader.hpp"

#include "core/assert.hpp"
#include "renderer/render_api.hpp"

#include "renderer/opengl_renderer/gl_shader.hpp"

namespace ev2::renderer {

std::unique_ptr<Shader> make_shader(const ShaderSource& source) {

    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL: {
            auto shader = std::make_unique<GLShader>(source.type);
            shader->compile(source);
            return shader;
        }
        default: 
            EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
            break;
    }
    return nullptr;
}

} // namespace ev2
