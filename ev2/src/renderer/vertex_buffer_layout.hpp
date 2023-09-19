/**
 * @file vertex_buffer_layout.hpp
 * @brief 
 * @date 2023-09-09
 * 
 */
#ifndef EV2_VERTEX_BUFFER_LAYOUT_HPP
#define EV2_VERTEX_BUFFER_LAYOUT_HPP

#include "core/assert.hpp"

#include "renderer/shader_data_type.hpp"

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
                elements.push_back(Attribute{ type, ShaderDataType::Vec3f, 0, 0 });
                break;
            case AttributeLabel::Texcoord:
                elements.push_back(Attribute{type, ShaderDataType::Vec2f, 0, 0});
                break;
            default:
                break;
        }
        return *this;
    }

    VertexBufferLayout& add_attribute(AttributeLabel label, ShaderDataType data_type) {
        EV_CORE_ASSERT(!finalized(), "Layout should not be finalized!");
        elements.push_back(Attribute{label, data_type, 0, 0});
        return *this;
    }

    VertexBufferLayout& finalize() {
        EV_CORE_ASSERT(!finalized(), "Layout should not be finalized!");
        uint32_t total_size = 0;
        for (auto& l : elements) {
            l.byte_offset = total_size;
            total_size += ShaderDataTypeSize(l.type);
        }
        stride = total_size;

        // update all the Attribute stride values too just for sanity
        for (auto& l : elements) {
            l.stride = stride;
        }
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