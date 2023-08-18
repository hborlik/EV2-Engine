/**
 * @file vertex_buffer.hpp
 * @brief 
 * @date 2023-08-11
 * 
 */
#ifndef EV2_VERTEX_BUFFER_HPP
#define EV2_VERTEX_BUFFER_HPP

#include "evpch.hpp"

#include "renderer/buffer.hpp"

namespace ev2::renderer {

struct VertexBufferAccessor {
    int         buffer_id   = -1;     // buffer in VertexBuffer
    size_t      byte_offset = 0;
    bool        normalized  = false;
    ShaderDataType type     = ShaderDataType::Float;
    size_t      count       = 0;       // required
    size_t      stride      = 0;
};

class VertexBuffer {
public:
    VertexBuffer() = default;

    void add_buffer(uint32_t buffer_id, std::shared_ptr<Buffer> buffer) {
        buffers.emplace(buffer_id, std::move(buffer));
    }

    std::shared_ptr<Buffer> get_buffer(uint32_t buffer_id) {
        return buffers.at(buffer_id);
    }

    inline void add_accessor(AttributeLabel accessor, uint32_t buffer_id,
                             size_t byte_offset, bool normalized,
                             ShaderDataType type, size_t count, size_t stride) {
        accessors.insert_or_assign(
            accessor, VertexBufferAccessor{(int)buffer_id, byte_offset,
                                           normalized, type, count, stride});
    }

    inline VertexBufferAccessor& get_accessor(AttributeLabel accessor) {
        return accessors.at(accessor);
    }

    /**
     * @brief Set the accessors from layout for a specific buffer in Vertex buffer set
     * 
     * @param buffer_id buffer that is the target of the layout. Buffer should be in buffers map
     * @param layout vertex buffer layout
     */
    void add_accessors_from_layout(int buffer_id, const VertexBufferLayout& layout);

    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& buffer);
    static VertexBuffer vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer);

    /**
     * @brief init vertex buffer for a screen space triangle (vertices only)
     * 
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitSST();

    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float). 
     *  Instanced array contains mat4 model matrices
     * 
     * @param buffer 
     * @param instance_buffer instance buffer to be used with the returned VAO
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitVertexDataInstanced(const std::vector<float>& buffer, const Buffer& instance_buffer, const VertexBufferLayout& layout);

    static VertexBuffer vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexBufferLayout& layout);
    static VertexBuffer vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexBufferLayout& layout);

    static VertexBuffer vbInitDefault();

    // buffer accessors
    std::map<AttributeLabel, VertexBufferAccessor> accessors;

private:
    std::unordered_map<int, std::shared_ptr<Buffer>> buffers;
};

} // namespace ev2::renderer

#endif // EV2_VERTEX_BUFFER_HPP