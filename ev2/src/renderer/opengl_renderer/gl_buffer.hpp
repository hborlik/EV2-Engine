/**
 * @file gl_buffer.h
 * @author Hunter Borlik
 * @brief opengl buffer
 * @version 0.2
 * @date 2019-09-21
 * 
 * 
 */
#ifndef EV2_GL_BUFFER_HPP
#define EV2_GL_BUFFER_HPP

#include "evpch.hpp"

#include "ev_gl.hpp"
#include "util.hpp"

#include "renderer/buffer.hpp"

namespace ev2::renderer {

class GLBuffer : public Buffer {
public:

    GLBuffer(gl::BindingTarget target, gl::Usage usage) : capacity{}, gl_reference{0}, target{target}, usage{usage} {
        glCreateBuffers(1, &gl_reference.v);
    }

    template<typename T>
    GLBuffer(gl::BindingTarget target, gl::Usage usage, const std::vector<T>& data) : target{target}, usage{usage} {
        glCreateBuffers(1, &gl_reference.v);
        allocate(data);
    }

    GLBuffer(gl::BindingTarget target, gl::Usage usage, const void* data, std::size_t size) : target{target}, usage{usage} {
        glCreateBuffers(1, &gl_reference.v);
        allocate_impl(data, size);
    }

    virtual ~GLBuffer() {
        glDeleteBuffers(1, &gl_reference.v);
    }

    // GLBuffer(const GLBuffer& o) = delete;
    // GLBuffer& operator=(const GLBuffer&) = delete;

    GLBuffer(GLBuffer&& o) = default;
    GLBuffer& operator=(GLBuffer&& o) = default;

    void sub_bytes_impl(const void* data, std::size_t size, std::size_t offset) override;

    /**
     * @brief Allocate buffer data
     * 
     * @param size number of bytes to allocate
     */
    void allocate_impl(std::size_t size) override;
    void allocate_impl(const void* data, std::size_t size) override;

    /**
     * @brief Bind this buffer to its target
     */
    void bind() const { glBindBuffer((GLenum)target, (GLuint)gl_reference); }

    /**
     * @brief Bind this buffer at given index of the binding point.
     * Buffer target must be one of GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER.
     */
    void bind(GLuint index) {glBindBufferBase((GLenum)target, index, (GLuint)gl_reference);}

    /**
     * @brief binds the range to the generic buffer binding point specified by target and to given index of the binding point.
     * Buffer target must be one of GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER.
     * 
     * @param index index of the binding point 
     * @param size  amount of data that should be available to read (from the offset) while the buffer is bound to this index
     * @param offset offset into buffer
     */
    void bind_range(GLuint index, size_t size = std::numeric_limits<size_t>::max(), GLint offset = 0) {
        // offset can be positive or negative
        int buf_offset = ((offset % capacity) + capacity) % capacity;
        size_t buf_size = std::min(size, capacity);
        assert(buf_offset + buf_size <= capacity);
        glBindBufferRange((GLenum)target, index, (GLuint)gl_reference, buf_offset, buf_size);
    }

    /**
     * @brief Unbind this buffer from its target
     */
    void unbind() const {glBindBuffer((GLenum)target, 0);}

    GLuint handle() const noexcept { return (GLuint)gl_reference; }

    /**
     * @brief Get the size in bytes of buffer
     * 
     * @return size_t 
     */
    size_t get_capacity() const noexcept { return capacity; }

    gl::BindingTarget get_binding_target() const noexcept {return target;}

    gl::Usage get_usage() const noexcept {return usage;}

private:
    /**
     * @brief size in bytes of copied data 
     */
    size_t capacity = 0;

    util::non_copyable<GLuint> gl_reference{0};

    /**
     * @brief binding target of this buffer
     */
    gl::BindingTarget target;

    /**
     * @brief usage type of buffer
     */
    gl::Usage usage;
};

} // ev2

#endif // EV2_GL_BUFFER_HPP