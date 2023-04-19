#include <renderer/texture.hpp>

#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace ev2::renderer {

Texture::Texture(gl::TextureType texture_type, gl::TextureFilterMode filterModeMin, gl::TextureFilterMode filterModeMag) : texture_type{texture_type} {
    GL_CHECKED_CALL(glGenTextures(1, &handle));

    m_mag_filter = filterModeMag;
    m_min_filter = filterModeMin;

    set_params();
}

void Texture::set_wrap_mode(gl::TextureParamWrap wrap, gl::TextureWrapMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameteri((GLenum)texture_type, (GLenum)wrap, (GLenum)mode));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::set_border_color(const glm::vec4& color) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameterfv((GLenum)texture_type, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color)));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::set_texture_wrap_r(gl::TextureWrapMode mode) {
    m_wrap_r = mode;
    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_R, mode);
}
void Texture::set_texture_wrap_s(gl::TextureWrapMode mode) {
    m_wrap_s = mode;
    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, mode);
}
void Texture::set_texture_wrap_t(gl::TextureWrapMode mode) {
    m_wrap_t = mode;
    set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, mode);
}

void Texture::set_texture_filter_mode_mag(gl::TextureFilterMode mode) {
    m_mag_filter = mode;
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, mode);
}
void Texture::set_texture_filter_mode_min(gl::TextureFilterMode mode) {
    m_min_filter = mode;
    set_filter_mode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, mode);
}

void Texture::set_filter_mode(gl::TextureParamFilter filter, gl::TextureFilterMode mode) {
    glBindTexture((GLenum)texture_type, handle);
    GL_CHECKED_CALL(glTexParameteri((GLenum)texture_type, (GLenum)filter, (GLenum)mode));
    glBindTexture((GLenum)texture_type, 0);
}

void Texture::generate_mips() {
    bind();
    glGenerateMipmap((GLenum)texture_type);
    unbind();
}

void Texture::set_image2D(gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data) {
    bind();
    GL_CHECKED_CALL(glTexImage2D((GLenum)gl::TextureTarget::TEXTURE_2D, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data));
    unbind();
    internal_format = internalFormat;
    pixel_type = dataType;
    this->width = width;
    this->height = height;
}

void Texture::set_data3D(gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height, gl::PixelFormat dataFormat, gl::PixelType dataType, const unsigned char* data, gl::TextureTarget side) {
    bind();
    GL_CHECKED_CALL(glTexImage2D((GLenum)side, 0, (GLint)internalFormat, width, height, 0, (GLenum)dataFormat, (GLenum)dataType, data));
    unbind();
    internal_format = internalFormat;
    pixel_type = dataType;
    this->width = width;
    this->height = height;
}

void Texture::recreate_storage2D(GLsizei levels, gl::TextureInternalFormat internalFormat, GLsizei width, GLsizei height) {
    // recreate the texture to allow for new storage
    if (glIsTexture(handle))
        glDeleteTextures(1, &handle);
    
    GL_CHECKED_CALL(glGenTextures(1, &handle));
    bind();
    GL_CHECKED_CALL(glTexStorage2D((GLenum)texture_type, levels, (GLint)internalFormat, width, height));
    unbind();
    internal_format = internalFormat;
    this->width = width;
    this->height = height;

    // update new gl texture with previous settings
    set_params();
}

void Texture::set_params() {
    set_texture_filter_mode_mag(m_mag_filter);
    set_texture_filter_mode_min(m_min_filter);

    set_texture_wrap_r(m_wrap_r);
    set_texture_wrap_s(m_wrap_s);
    set_texture_wrap_t(m_wrap_t);
}

// FBO

bool FBO::resize_all(GLsizei width, GLsizei height) {
    for (const auto& [tex_at, bind] : attachments) {
        auto &tex = bind.texture;
        tex->recreate_storage2D(1, tex->get_internal_format(), width, height);
        attach_texture(tex.get(), tex_at);
    }
    for (auto& [_, rb_at] : rb_attachments) {
        rb_at.set_data(rb_at.get_format(), width, height);
    }
    this->width = width;
    this->height = height;
    return check();
}

bool FBO::check() {
    bind();
    // enable all drawbuffers
    int max_binding = 0;
    for (auto &a : attachments) {
        if (a.second.location >= 0) 
        max_binding = max_binding > a.second.location ? max_binding : a.second.location;
    }
    std::vector<GLenum> dbtgt(max_binding + 1);
    int i = 0;
    for (const auto & [at, binding] : attachments) {
        if (binding.location >= 0) 
            dbtgt[binding.location] = (GLenum)at;
    }

    GL_CHECKED_CALL(glDrawBuffers(dbtgt.size(), dbtgt.data()));

    GLenum err = glCheckFramebufferStatus((GLenum)target);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        // TODO error println
    }
    unbind();
    return err == GL_FRAMEBUFFER_COMPLETE;
}

bool FBO::attach(std::shared_ptr<Texture> texture, gl::FBOAttachment attachment_point, int location) {
    if ((attachment_point == gl::FBOAttachment::DEPTH) || (attachment_point == gl::FBOAttachment::DEPTH_STENCIL) || (attachment_point == gl::FBOAttachment::STENCIL)) 
    {
        assert(location == -1);
    }
    if (!(width > 0 && height > 0)) {
        width = texture->get_width();
        height = texture->get_height();
    }
    assert(width == texture->get_width() && height == texture->get_height());
    if (attachments.find(attachment_point) != attachments.end())
        return false;
    if (texture && texture->type() == gl::TextureType::TEXTURE_2D) {
        if (attach_texture(texture.get(), attachment_point)) {
            attachments.insert(std::pair{attachment_point, AttachmentBinding{location, texture}});
            rb_attachments.erase(attachment_point);

            return true;
        }
    }
    std::cout << "Failed to attach FBO attachment" << std::endl;
    return false;
}

bool FBO::attach_renderbuffer(gl::RenderBufferInternalFormat format, uint32_t width, uint32_t height, gl::FBOAttachment attachment_point) {
    // create new renderbuffer
    auto [ip, inserted] = rb_attachments.emplace(std::pair{attachment_point, RenderBuffer{}});
    if (!inserted)
        return false;

    RenderBuffer &r_buffer = ip->second;
    r_buffer.set_data(format, width, height);

    // attempt to attach the renderbuffer
    clearGLErrors();
    // glNamedFramebufferRenderbuffer(gl_reference, (GLenum)attachment_point, GL_RENDERBUFFER, r_buffer.get_handle());
    bind();
    glFramebufferRenderbuffer((GLenum)target, (GLenum)attachment_point, GL_RENDERBUFFER, r_buffer.get_handle());
    unbind();
    if (!isGLError()) {
        attachments.erase(attachment_point);

        return true;
    }
    return false;
}

bool FBO::attach_texture(const Texture* texture, gl::FBOAttachment attachment_point) {
    clearGLErrors();
    // glNamedFramebufferTexture(gl_reference, (GLenum)attachment_point, texture->get_handle(), 0);
    bind();
    glFramebufferTexture2D((GLenum)target, (GLenum)attachment_point, (GLenum)texture->type(), texture->get_handle(), 0);
    unbind();
    return !isGLError();
}

}