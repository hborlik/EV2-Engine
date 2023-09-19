#include "renderer/buffer.hpp"
#include "renderer/render_api.hpp"

#include "renderer/opengl_renderer/gl_buffer.hpp"

namespace ev2::renderer {

std::unique_ptr<Buffer> Buffer::make_buffer(BufferUsage usage, BufferAccess access) {
    switch(RenderAPI::get_api()) {
        case RenderAPI::API::OpenGL: {
            // convert usage into a binding target for GL buffer modification and usage
            // there is a bit of a naming conflict between the Vulkan and OpenGL idea of "usage".
            // OpenGL usage specifies how frequently memory will be accessed by CPU
            // Vulkan usage specifies what the buffer may be bound to.
            // In this rendering API, BufferUsage follows the Vulkan definition of usage 
            gl::BindingTarget gl_target = gl::BindingTarget::ARRAY;
            gl::Usage gl_usage = gl::Usage::STATIC_DRAW;

            switch(usage) {
                case BufferUsage::Index:
                    gl_target = gl::BindingTarget::ELEMENT_ARRAY;
                    break;
                case BufferUsage::Vertex:
                    gl_target = gl::BindingTarget::ARRAY;
                    break;
                default: 
                    EV_CORE_CRITICAL("BufferUsage value {} not implemented for OpenGL", usage);
                    return {};
            }

            switch(access) {
                case BufferAccess::Static:
                    gl_usage = gl::Usage::STATIC_DRAW;
                case BufferAccess::Dynamic:
                    gl_usage = gl::Usage::DYNAMIC_DRAW;
                default:
                    EV_CORE_CRITICAL("BufferAccess value {} not implemented for OpenGL", access);
                    return {};
            }

            auto shader = std::make_unique<GLBuffer>(gl_target, gl_usage);
            return shader;
        }
        default: 
            EV_CORE_ASSERT(false, "RendererAPI is currently not supported!");
            break;
    }
    return {};
}

}