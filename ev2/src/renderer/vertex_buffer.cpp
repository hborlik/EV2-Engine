#include "renderer/vertex_buffer.hpp"

namespace ev2::renderer {

std::unique_ptr<VertexBuffer> VertexBuffer::vbInitArrayVertexData(std::shared_ptr<Buffer> buffer) {
    std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>();

    const uint32_t FloatSize = ShaderDataTypeSize(ShaderDataType::Float);

    vb->add_buffer(0, buffer);

    vb->add_accessor(0, Attribute{
        .type=ShaderDataType::Vec3f,
        .label=AttributeLabel::Vertex,
        .stride=FloatSize * 9,
        .byte_offset=0
    });

    vb->add_accessor(0, Attribute{
        .type=ShaderDataType::Vec3f,
        .label=AttributeLabel::Normal,
        .stride=FloatSize * 9,
        .byte_offset=FloatSize * 3
    });

    vb->add_accessor(0, Attribute{
        .type=ShaderDataType::Vec3f,
        .label=AttributeLabel::Color,
        .stride=FloatSize * 9,
        .byte_offset=FloatSize * 6
    });

    // vb->buffers.emplace(mat_spec::VERTEX_BINDING_LOCATION, std::make_shared<GLBuffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertices));
    // vb.buffers.at(mat_spec::VERTEX_BINDING_LOCATION)->bind();
    // glEnableVertexAttribArray(mat_spec::VERTEX_BINDING_LOCATION);
    // glVertexAttribPointer(mat_spec::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    

    // vb.buffers.emplace(mat_spec::NORMAL_BINDING_LOCATION, std::make_shared<GLBuffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, normals));
    // vb.buffers.at(mat_spec::NORMAL_BINDING_LOCATION)->bind();
    // glEnableVertexAttribArray(mat_spec::NORMAL_BINDING_LOCATION);
    // glVertexAttribPointer(mat_spec::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 3));

    // vb.buffers.emplace(mat_spec::COLOR_BINDING_LOCATION, std::make_shared<GLBuffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertex_colors));
    // vb.buffers.at(mat_spec::COLOR_BINDING_LOCATION)->bind();
    // glEnableVertexAttribArray(mat_spec::COLOR_BINDING_LOCATION);
    // glVertexAttribPointer(mat_spec::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 6));

    // vb.buffers.at(mat_spec::COLOR_BINDING_LOCATION)->unbind();

    // glBindVertexArray(0);

    return vb;
}

std::unique_ptr<VertexBuffer> VertexBuffer::vbInitArrayVertexData(const std::vector<float>& buffer) {
    std::unique_ptr<VertexBuffer> vb = std::make_unique<VertexBuffer>();
    // pos(3float), normal(3float), color(3float), texcoord(2float)

    constexpr std::size_t vec3Size = ShaderDataTypeSize(ShaderDataType::Vec3f);
    constexpr std::size_t floatSize = ShaderDataTypeSize(ShaderDataType::Float);

    vb->add_accessor(0, AttributeLabel::Vertex,   ShaderDataType::Vec3f, floatSize * 11, 0         );
    vb->add_accessor(0, AttributeLabel::Normal,   ShaderDataType::Vec3f, floatSize * 11, vec3Size*1);
    vb->add_accessor(0, AttributeLabel::Color,    ShaderDataType::Vec3f, floatSize * 11, vec3Size*2);
    vb->add_accessor(0, AttributeLabel::Texcoord, ShaderDataType::Vec2f, floatSize * 11, vec3Size*3);

    // std::make_shared<Buffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer)

    auto r_buffer = std::shared_ptr(Buffer::make_buffer(BufferUsage::Vertex, BufferAccess::Static));
    r_buffer->allocate(buffer);
    vb->add_buffer(0, r_buffer);
    return vb;
}

// std::unique_ptr<VertexBuffer> VertexBuffer::vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer) {
//     VertexBuffer vb;

//     vb.buffers.emplace(0, std::make_shared<Buffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer));
//     vb.buffers.emplace(1, std::make_shared<Buffer>(gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer));

//     vb.indexed = 1; // id of index buffer

//     // pos(3float), normal(3float), color(3float), texcoord(2float)

//     vb.add_accessor(AttributeLabel::Vertex, 0, 0, false, gl::DataType::FLOAT, 3, sizeof(float) * 8);
//     vb.add_accessor(AttributeLabel::Normal, 0, sizeof(float) * 3, false, gl::DataType::FLOAT, 3, sizeof(float) * 8);
//     vb.add_accessor(AttributeLabel::Texcoord, 0, sizeof(float) * 6, false, gl::DataType::FLOAT, 2, sizeof(float) * 8);

    // vb.buffers.emplace(2, Buffer{gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW});
    // // mat4 instance info, note: max size for a vertex attribute is a vec4
    // constexpr std::size_t vec4Size = sizeof(glm::vec4);
    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 1);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 2);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 3);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION  , 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+1, 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+2, 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+3, 1);


//     return vb;
// }

std::unique_ptr<VertexBuffer> VertexBuffer::vbInitSST() {
    auto vb = std::make_unique<VertexBuffer>();

    std::vector<float> buffer{0, 0, 2, 0, 0, 2};

    auto render_buffer = std::shared_ptr(Buffer::make_buffer(BufferUsage::Vertex, BufferAccess::Static));
    render_buffer->allocate(buffer);

    vb->add_buffer(0, render_buffer);
    vb->add_accessor(0, AttributeLabel::Vertex, ShaderDataType::Vec3f, ShaderDataTypeSize(ShaderDataType::Vec3f), 0);
    
    return vb;
}

std::unique_ptr<VertexBuffer> VertexBuffer::vbInitArrayVertexSpec(std::shared_ptr<Buffer> buffer, const VertexBufferLayout& layout) {
    EV_CORE_ASSERT(layout.finalized());
    auto vb = std::make_unique<VertexBuffer>();
    vb->add_buffer(0, buffer);

    // map the attributes defined in the layout
    vb->add_accessors_from_layout(0, layout);

    return vb;
}

std::unique_ptr<VertexBuffer> VertexBuffer::vbInitDefault() {
    auto vb = std::make_unique<VertexBuffer>();
    return vb;
}

}