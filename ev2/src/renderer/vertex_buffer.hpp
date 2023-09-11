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
#include "renderer/vertex_buffer_layout.hpp"

namespace ev2::renderer {

struct VertexBufferAccessor {
    int         buffer_id   = -1;       // buffer in VertexBuffer::buffers
    Attribute   attribute{};
};

class VertexBuffer {
public:
    VertexBuffer() = default;

    void add_buffer(int buffer_id, std::shared_ptr<Buffer> buffer) {
        buffers.emplace(buffer_id, std::move(buffer));
    }

    std::shared_ptr<Buffer> get_buffer(int buffer_id) {
        return buffers.at(buffer_id);
    }

    const std::shared_ptr<Buffer> get_buffer(int buffer_id) const {
        return buffers.at(buffer_id);
    }

    void add_accessor(int buffer_id, const Attribute& attr) {
        accessors.insert_or_assign(attr.label, VertexBufferAccessor{buffer_id,  attr});
    }

    void add_accessor(int buffer_id,
        AttributeLabel label,
        ShaderDataType type,
        size_t count,
        bool normalized,
        size_t stride,
        size_t byte_offset) {

        const Attribute attr { 
            .label=label, 
            .type=type,
            .element_size=ShaderDataTypeSize(type),
            .count=count,
            .normalized=normalized,
            .stride=stride,
            .byte_offset=byte_offset};
        
        add_accessor(buffer_id, attr);
    }

    VertexBufferAccessor& get_accessor(AttributeLabel label) {
        return accessors.at(label);
    }

    /**
     * @brief Set the accessors from layout for a specific buffer in Vertex buffer set
     * 
     * @param buffer_id buffer that is the target of the layout. Buffer should be in buffers map
     * @param layout vertex buffer layout
     */
    void add_accessors_from_layout(int buffer_id, const VertexBufferLayout& layout) {
        // map the elements defined in the layout
        for (auto& attr : layout.elements) {
            add_accessor(buffer_id, attr);
        }
    }


    /**
     * @brief vertex buffer format pos(3float), normal(3float), color(3float)
     * 
     * @param buffer
     * @return std::unique_ptr<VertexBuffer> 
     */
    static std::unique_ptr<VertexBuffer> vbInitArrayVertexData(std::shared_ptr<Buffer> buffer);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return std::unique_ptr<VertexBuffer> 
     */
    static std::unique_ptr<VertexBuffer> vbInitArrayVertexData(const std::vector<float>& buffer);
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