/**
 * @file buffer.hpp
 * @brief 
 * @date 2023-06-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef RV2_BUFFER_HPP
#define RV2_BUFFER_HPP

#include "evpch.hpp"

#include "core/assert.hpp"

namespace ev2::renderer {

enum class ShaderDataType : int {
    Short               ,
    UnsignedShort       ,
    Int                 ,
    Int2                ,
    Int3                ,
    Int4                ,
    Uint                ,
    Uint2               ,
    Uint3               ,
    Uint4               ,
    HalfFloat           ,
    Float               ,
    Vec2f               ,
    Vec3f               ,
    Vec4f               ,
    Mat2f               ,
    Mat3f               ,
    Mat4f               ,
    Double              ,
    Vec2d               ,
    Vec3d               ,
    Vec4d               ,
    Mat2d               ,
    Mat3d               ,
    Mat4d               
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type)
    {
        case ShaderDataType::Short:         return 2;
        case ShaderDataType::UnsignedShort: return 2;
        case ShaderDataType::Int:           return 4;
        case ShaderDataType::Int2:          return 4 * 2;
        case ShaderDataType::Int3:          return 4 * 3;
        case ShaderDataType::Int4:          return 4 * 4;
        case ShaderDataType::Uint:          return 4;
        case ShaderDataType::Uint2:         return 4 * 2;
        case ShaderDataType::Uint3:         return 4 * 3;
        case ShaderDataType::Uint4:         return 4 * 4;
        case ShaderDataType::HalfFloat:     return 2;
        case ShaderDataType::Float:         return 4;
        case ShaderDataType::Vec2f:         return 4 * 2;
        case ShaderDataType::Vec3f:         return 4 * 3;
        case ShaderDataType::Vec4f:         return 4 * 4; 
        case ShaderDataType::Mat2f:         return 4 * 2 * 2; 
        case ShaderDataType::Mat3f:         return 4 * 3 * 3;
        case ShaderDataType::Mat4f:         return 4 * 4 * 4;
        case ShaderDataType::Double:        return 8;
        case ShaderDataType::Vec2d:         return 8 * 2;
        case ShaderDataType::Vec3d:         return 8 * 3;
        case ShaderDataType::Vec4d:         return 8 * 4;
        case ShaderDataType::Mat2d:         return 8 * 2 * 2;
        case ShaderDataType::Mat3d:         return 8 * 3 * 3;
        case ShaderDataType::Mat4d:         return 8 * 4 * 4;
        default: break;
    }

    EV_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

enum class AttributeLabel : int {
    Vertex = 0,
    Normal,
    Color,
    Texcoord,
    Tangent,
    Bitangent
};

struct Attribute {
    AttributeLabel      label           = AttributeLabel::Vertex;
    ShaderDataType      type            = ShaderDataType::Float;
    uint32_t            count           = 0;
    uint32_t            element_size    = 0;
    uint32_t            offset          = 0;    // calculated offset.
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
            l.offset = total_size;
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

class Buffer {
public:
    virtual void allocate(std::size_t bytes);
};

}

#endif // EV2_BUFFER_HPP