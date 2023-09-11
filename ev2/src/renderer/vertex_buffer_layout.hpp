/**
 * @file vertex_buffer_layout.hpp
 * @brief 
 * @date 2023-09-09
 * 
 */
#ifndef EV2_VERTEX_BUFFER_LAYOUT_HPP
#define EV2_VERTEX_BUFFER_LAYOUT_HPP

#include "renderer/buffer.hpp"

namespace ev2::renderer {

enum class AttributeLabel : int {
    Vertex = 0,
    Normal,
    Color,
    Texcoord,
    Tangent,
    Bitangent
};

struct Attribute {
    AttributeLabel  label       = AttributeLabel::Vertex;
    ShaderDataType  type        = ShaderDataType::Float;
    size_t          element_size= 0;    // size in bytes of a single element. eg 4 for Float
    size_t          count       = 0;    // number of elements (not size of array). eg 3 for vec3
    bool            normalized  = false;
    size_t          stride      = 0;    // bytes between consecutive elements
    size_t          byte_offset = 0;    // offset from beginning of buffer
};


struct VertexBufferLayout {

    VertexBufferLayout& add_attribute(AttributeLabel type) {
        EV_CORE_ASSERT(!finalized(), "Layout should not be finalized!");
        switch(type) {
            case AttributeLabel::Vertex:
            case AttributeLabel::Normal:
            case AttributeLabel::Color:
                elements.push_back(Attribute{ type, ShaderDataType::Float, 3, ShaderDataTypeSize(ShaderDataType::Float), 0 });
                break;
            case AttributeLabel::Texcoord:
                elements.push_back(Attribute{type, ShaderDataType::Float, 2, ShaderDataTypeSize(ShaderDataType::Float), 0});
                break;
            default:
                break;
        }
        return *this;
    }

    VertexBufferLayout& add_attribute(AttributeLabel label, ShaderDataType data_type, uint32_t count, uint32_t size) {
        EV_CORE_ASSERT(!finalized(), "Layout should not be finalized!");
        elements.push_back(Attribute{label, data_type, count, size, 0});
        return *this;
    }

    VertexBufferLayout& finalize() {
        EV_CORE_ASSERT(!finalized(), "Layout should not be finalized!");
        uint32_t total_size = 0;
        for (auto& l : elements) {
            l.byte_offset = total_size;
            total_size += l.element_size * l.count;
        }
        stride = total_size;
        return *this;
    }

    bool finalized() const noexcept {
        return stride != 0;
    }

    std::vector<Attribute> elements;
    uint32_t stride = 0;
};

} // namespace ev2::renderer

#endif // EV2_VERTEX_BUFFER_LAYOUT_HPP