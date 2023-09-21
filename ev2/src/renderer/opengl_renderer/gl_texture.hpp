/**
 * @file gl_texture.h
 * @brief 
 * @date 2022-04-26
 * 
 */
#ifndef EV2_GL_TEXTURE_HPP
#define EV2_GL_TEXTURE_HPP

#include "evpch.hpp"

#include "renderer/texture.hpp"
#include "ev_gl.hpp"
#include "io/image.hpp"

namespace ev2::renderer {

class GLTexture {
public:
    GLTexture(gl::TextureType texture_type, gl::TextureFilterMode filterModeMin = gl::TextureFilterMode::NEAREST_MIPMAP_LINEAR, gl::TextureFilterMode filterModeMag = gl::TextureFilterMode::LINEAR);

    ~GLTexture() {
        if (glIsTexture(handle))
            glDeleteTextures(1, &handle);
    }

    GLTexture(GLTexture &&o) {
        swap(*this, o);
    }

    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    GLTexture& operator=(GLTexture&&) = delete;

    void set_border_color(const glm::vec4& color);

    void set_texture_wrap_r(gl::TextureWrapMode mode);
    void set_texture_wrap_s(gl::TextureWrapMode mode);
    void set_texture_wrap_t(gl::TextureWrapMode mode);

    void set_texture_filter_mode_mag(gl::TextureFilterMode mode);
    void set_texture_filter_mode_min(gl::TextureFilterMode mode);

    /**
     * @brief generate the mip maps for this texture
     * 
     */
    void generate_mips();

    /**
     * @brief Get the Handle to the texture
     * 
     * @return GLuint 
     */
    GLuint get_handle() const noexcept {return handle;}

    void bind() const {glBindTexture((GLenum)texture_type, handle);}
    void unbind() const {glBindTexture((GLenum)texture_type, 0);}

    gl::TextureType type() const noexcept {return texture_type;}
    gl::TextureInternalFormat get_internal_format() const noexcept {return internal_format;}

    /**
     * @brief glTexImage2D. Allocate and set the image data for a 2D image, texture must have the TextureType::TEXTURE_2D type
     * 
     * @param data nullptr to 0 fill texture memory
     * @param dataFormat 
     * @param dataType 
     * @param internalFormat 
     * @param width 
     * @param height 
     */
    void set_image2D(gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data);

    void set_data3D(gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data, gl::TextureTarget side);

    /**
     * @brief glTexStorage2D. specify the storage requirements for all levels of a two-dimensional texture or one-dimensional texture array simultaneously.
     * Note that this causes the texture storage to be immutable, so the texture size cannot change. It will also generate a new handle for the texture.
     * 
     * @param levels 
     * @param internalFormat 
     * @param width 
     * @param height 
     */
    void recreate_storage2D(GLsizei levels, gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height);

    int get_width() const noexcept {return width;}
    int get_height() const noexcept {return height;}

    bool is_valid() const noexcept {
        return width > 0 && height > 0 && glIsTexture(handle);
    }

private:
    /**
     * @brief GL API call to set the Filter Mode used when determining pixel color
     * 
     * @param filter filter function
     * @param mode behavior 
     */
    void set_filter_mode(gl::TextureParamFilter filter, gl::TextureFilterMode mode);

    /**
     * @brief GL API call to set the Texture wrapping behavior
     * 
     * @param wrap edge
     * @param mode behavior
     */
    void set_wrap_mode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode);

    /**
     * @brief update all gl texture params from member values
     * 
     */
    void set_params();

private:
    friend void swap(GLTexture& first, GLTexture& second) noexcept {
        using std::swap;

        std::swap(first.texture_type, second.texture_type);
        std::swap(first.internal_format, second.internal_format);
        std::swap(first.pixel_type, second.pixel_type);
        std::swap(first.m_wrap_r, second.m_wrap_r);
        std::swap(first.m_wrap_s, second.m_wrap_s);
        std::swap(first.m_wrap_t, second.m_wrap_t);
        std::swap(first.m_mag_filter, second.m_mag_filter);
        std::swap(first.m_min_filter, second.m_min_filter);
        std::swap(first.width, second.width);
        std::swap(first.height, second.height);
        std::swap(first.handle, second.handle);
    }

protected:
    gl::TextureType texture_type;
    gl::TextureInternalFormat internal_format;
    gl::PixelType pixel_type;

    // wrap modes
    gl::TextureWrapMode m_wrap_r = gl::TextureWrapMode::CLAMP_TO_BORDER;
    gl::TextureWrapMode m_wrap_s = gl::TextureWrapMode::CLAMP_TO_BORDER;
    gl::TextureWrapMode m_wrap_t = gl::TextureWrapMode::CLAMP_TO_BORDER;

    // sampling modes
    gl::TextureFilterMode m_mag_filter = gl::TextureFilterMode::NEAREST_MIPMAP_NEAREST;
    gl::TextureFilterMode m_min_filter = gl::TextureFilterMode::NEAREST_MIPMAP_NEAREST;

    GLuint handle = 0;
    int width = -1, height = -1;
};

class GLTexture1D : public GLTexture, public Texture1D {
public:
    GLTexture1D(gl::TextureFilterMode filterModeMin = gl::TextureFilterMode::NEAREST_MIPMAP_LINEAR, gl::TextureFilterMode filterModeMag = gl::TextureFilterMode::LINEAR) :
        GLTexture{ gl::TextureType::TEXTURE_1D, filterModeMin, filterModeMag } {}
};

class GLTexture2D : public GLTexture, public Texture2D {
public:
    GLTexture2D(gl::TextureFilterMode filterModeMin = gl::TextureFilterMode::NEAREST_MIPMAP_LINEAR, gl::TextureFilterMode filterModeMag = gl::TextureFilterMode::LINEAR) :
        GLTexture{ gl::TextureType::TEXTURE_2D, filterModeMin, filterModeMag } {}

    void generate_mips() override {
        GLTexture::generate_mips();
    }
};

class RenderBuffer {
public:
    RenderBuffer() {
        glGenRenderbuffers(1, &gl_reference);
    }

    ~RenderBuffer() {
        if (gl_reference != 0)
            glDeleteRenderbuffers(1, &gl_reference);
    }

    RenderBuffer(RenderBuffer&& o) {
        std::swap(gl_reference, o.gl_reference);
    }

    RenderBuffer(const RenderBuffer& o) = delete;
    RenderBuffer& operator              = (const RenderBuffer&) = delete;
    RenderBuffer& operator              = (RenderBuffer&&)      = delete;

    void bind() {glBindRenderbuffer(GL_RENDERBUFFER, gl_reference);}
    void unbind() {glBindRenderbuffer(GL_RENDERBUFFER, 0);}

    void set_data(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height) {
        bind();
        glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)format, width, height);
        unbind();
        this->format = format;
    }

    GLuint get_handle() const noexcept {return gl_reference;}

    gl::RenderBufferInternalFormat get_format() const noexcept {return format;}

private:
    GLuint gl_reference = 0;
    gl::RenderBufferInternalFormat format;
};

/**
 * @brief Frame Buffer Object
 * 
 */
class FBO {
public:
    FBO(gl::FBOTarget target) : target{target} {
        GL_CHECKED_CALL(glGenFramebuffers(1, &gl_reference));
    }

    ~FBO() {
        if (glIsFramebuffer(gl_reference))
            glDeleteFramebuffers(1, &gl_reference);
    }

    FBO(FBO&& o) {
        *this = std::move(o);
    }

    FBO& operator=(FBO&& o) {
        std::swap(target, o.target);
        std::swap(gl_reference, o.gl_reference);
        std::swap(width, o.width);
        std::swap(height, o.height);
        std::swap(attachments, o.attachments);
        std::swap(rb_attachments, o.rb_attachments);
        return *this;
    }

    FBO(const FBO& o) = delete;
    FBO& operator=(const FBO&) = delete;

    /**
     * @brief rebuild framebuffer at specified size by recreating backing textures
     * 
     * @param width 
     * @param height 
     * @return true 
     * @return false 
     */
    bool resize_all(GLsizei width, GLsizei height);

    /**
     * @brief Set the bind target. Do not change while framebuffer is bound
     * 
     * @param target 
     */
    void set_bind_target(gl::FBOTarget target) {this->target = target;}

    void bind() const {glBindFramebuffer((GLenum)target, gl_reference);}
    void unbind() const {glBindFramebuffer((GLenum)target, 0);}

    /**
     * @brief check that this framebuffer has all the required attachments for its specified binding target
     * 
     * @return true it is ready
     * @return false it is not ready
     */
    bool check();

    /**
     * @brief attach a texture to the framebuffer. currently only supports 2d textures
     * 
     * @param texture 
     * @return true 
     * @return false 
     */
    bool attach(std::shared_ptr<GLTexture> texture, gl::FBOAttachment attachment_point, int location = -1);

    bool attach_renderbuffer(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height, gl::FBOAttachment attachment_point);

    void detach(gl::FBOAttachment attachment_point);

    /**
     * @brief Get the pixel width of the framebuffer attachments
     * 
     * @return GLsizei 
     */
    GLsizei get_width() const noexcept {return width;}

    /**
     * @brief Get the pixel height of the framebuffer attachments
     * 
     * @return GLsizei 
     */
    GLsizei get_height() const noexcept {return height;}

    GLuint get_handle() noexcept {return gl_reference;}

private:
    gl::FBOTarget target;
    GLuint gl_reference = 0;
    int width = -1, height = -1;

    bool attach_texture(const GLTexture* texture, gl::FBOAttachment attachment_point);

    struct AttachmentBinding {
        int location = -1;
        std::shared_ptr<GLTexture> texture;
    };

    std::unordered_map<gl::FBOAttachment, AttachmentBinding> attachments;
    std::unordered_map<gl::FBOAttachment, RenderBuffer> rb_attachments;
};

} // namespace ev2::renderer

#endif // EV2_GL_TEXTURE_HPP