#include "renderer/buffer.hpp"
#include "renderer/render_api.hpp"

#include "renderer/opengl_renderer/gl_buffer.hpp"

namespace ev2::renderer {

std::unique_ptr<Buffer> Buffer::make_buffer(BufferUsage usage, BufferAccess access) {
    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL: {
            // convert usage into a binding target for GL buffer modification and usage
            gl::BindingTarget target = gl::BindingTarget::ARRAY;
            switch(usage) {
                case BufferUsage::Index:
                    target = gl::BindingTarget::ELEMENT_ARRAY;
                case BufferUsage::Vertex:
                    target = gl::BindingTarget::ARRAY;
                default: break;
            }

            auto shader = std::make_unique<GLBuffer>(target, usage);
            return shader;
        }
        default: 
            EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
            break;
    }
    return nullptr;
}

}