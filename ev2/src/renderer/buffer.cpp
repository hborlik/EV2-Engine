/**
 * @file buffer.cpp
 * @author Hunter Borlik
 * @brief OpenGL buffer 
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */

#include <renderer/buffer.hpp>

namespace ev2::renderer {

using namespace ev2;
using namespace ev2::gl;

void Buffer::allocate(std::size_t bytes) {
    glBindBuffer((GLenum)target, (GLuint)gl_reference);
    GL_CHECKED_CALL(glBufferData((GLenum)target, bytes, NULL, (GLenum)usage));
    glBindBuffer((GLenum)target, 0);
    capacity = bytes;
}

void Buffer::copy_data(std::size_t size, const void* data) {
    glBindBuffer((GLenum)target, (GLuint)gl_reference);
    glBufferData((GLenum)target, size, data, (GLenum)usage);
    glBindBuffer((GLenum)target, 0);
    capacity = size;
}

} // namespace ev2::renderer