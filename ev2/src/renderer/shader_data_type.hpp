/**
 * @file shader_data_type.hpp
 * @brief 
 * @date 2023-09-15
 * 
 */
#ifndef EV2_RENDERER_SHADER_DATA_TYPE_HPP
#define EV2_RENDERER_SHADER_DATA_TYPE_HPP

#include <stdint.h>

namespace ev2::renderer {

enum class ShaderDataType : int {
    Short=0             ,
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
    Mat4d               ,
    COUNT
};

static constexpr uint32_t ShaderDataTypeSize(ShaderDataType type) {
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

    // EV_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

}

#endif // EV2_RENDERER_SHADER_DATA_TYPE_HPP