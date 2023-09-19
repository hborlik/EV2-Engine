/**
 * @file buffer.cpp
 * @author Hunter Borlik
 * @brief OpenGL buffer 
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */

#include "renderer/opengl_renderer/gl_buffer.hpp"

namespace ev2::renderer {

using namespace ev2;
using namespace ev2::gl;

void GLBuffer::allocate_impl(std::size_t size) {
    if(size > 0) {
        bool error = false;
        GL_ERROR_CHECK(glNamedBufferData((GLuint)gl_reference, size, NULL, (GLenum)usage), error);
        if (!error) capacity = size;
    }
}

void GLBuffer::allocate_impl(const void* data, std::size_t size) {
    if(size > 0) {
        bool error = false;
        GL_ERROR_CHECK(glNamedBufferData((GLuint)gl_reference, size, data, (GLenum)usage), error);
        if (!error) capacity = size;
    }
}

void GLBuffer::sub_bytes_impl(const void* data, std::size_t size, std::size_t offset) {
    if(data && size > 0) {
        GL_CHECKED_CALL(glNamedBufferSubData((GLuint)gl_reference, offset, size, data));
    }
}

} // namespace ev2::renderer