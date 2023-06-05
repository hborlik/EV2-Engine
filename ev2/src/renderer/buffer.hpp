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

namespace ev2 {

enum class ShaderDataType : int {
    BYTE                = 0,
    UNSIGNED_BYTE       ,
    SHORT               ,
    UNSIGNED_SHORT      ,
    INT                 ,
    UNSIGNED_INT        ,
    FIXED               ,
    HALF_FLOAT          ,
    FLOAT               ,
    VEC2F               ,
    VEC3F               ,
    VEC4F               ,
    MAT2F               ,
    MAT3F               ,
    MAT4F               ,
    DOUBLE              ,
    VEC2D               ,
    VEC3D               ,
    VEC4D               ,
    MAT2D               ,
    MAT3D               ,
    MAT4D               
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type)
    {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
    }

    EV_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

struct BufferElement {
    VertexAttributeLabel attribute   = VertexAttributeLabel::Vertex;
    gl::DataType        type        = gl::DataType::FLOAT;
    uint16_t            count       = 0;
    uint16_t            element_size= 0;
    uint16_t            offset = 0;    // calculated offset. This is populated by finalize()
};

struct BufferLayout {
    

};

class Buffer {
public:
    virtual void allocate(std::size_t bytes);
};

}

#endif // EV2_BUFFER_HPP