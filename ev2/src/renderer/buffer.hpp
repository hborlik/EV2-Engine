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
#include <fmt/core.h>
#include <fmt/format.h>

namespace ev2::renderer {

/**
 * @brief frequency of access (modification and usage)
 * 
 */
enum class BufferAccess {
    Static,     // The data store contents will be modified once and used many times.
    Dynamic     // The data store contents will be modified repeatedly and used many times.
};

/**
 * @brief the nature of that memory access.
 * 
 */
enum class BufferUsage {
    Vertex,     // contains vertex data
    Index,      // contains index data
};

class Buffer {
public:
    virtual ~Buffer() = default;

    virtual void allocate_impl(std::size_t size) = 0;
    virtual void allocate_impl(const void* data, std::size_t size) = 0;

    /**
     * @brief write data into buffer starting at offset bytes from beginning of buffer
     * 
     * @param data pointer to data
     * @param size length in bytes of data to write
     * @param offset offset in bytes from start of buffer to write data
     */
    virtual void sub_bytes_impl(const void* data, std::size_t size, std::size_t offset) = 0;

    template<typename T>
    void sub_bytes(const T& source, std::size_t offset) {
        sub_bytes_impl(&source, sizeof(T), offset);
    }

    /**
     * @brief Update part of data in buffer. Buffer should have data allocated before call is made to sub data
     * 
     * @tparam T 
     * @param source 
     * @param offset 
     */
    template<typename T>
    void sub_bytes(const std::vector<T>& source, uint32_t offset, uint32_t stride) {
        if(!source.empty()) {
            for(size_t i = 0; i < source.size(); i++) {
                sub_bytes_impl(&source[i], sizeof(T), offset + i * stride);
            }
        }
    }

    /**
     * @brief Allocate buffer large enough to contain all data in source and copy data into buffer.
     * 
     * @tparam T 
     * @param source 
     */
    template<typename T>
    void allocate(const std::vector<T>& source) {
        allocate_impl(source.data(), sizeof(T) * source.size());
    }

    void allocate(std::size_t size) {
        allocate_impl(size);
    }

    static std::unique_ptr<Buffer> make_buffer(BufferUsage usage, BufferAccess access);
};

} // namespace ev2::renderer

// Formatters
template <> struct fmt::formatter<ev2::renderer::BufferAccess>: fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.

    auto format(ev2::renderer::BufferAccess c, format_context& ctx) const {
        string_view name = "unknown";
        switch (c) {
            case ev2::renderer::BufferAccess::Static:   name = "Static"; break;
            case ev2::renderer::BufferAccess::Dynamic:   name = "Dynamic"; break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

template <> struct fmt::formatter<ev2::renderer::BufferUsage>: fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.

    auto format(ev2::renderer::BufferUsage c, format_context& ctx) const {
        string_view name = "unknown";
        switch (c) {
            case ev2::renderer::BufferUsage::Vertex:   name = "Vertex"; break;
            case ev2::renderer::BufferUsage::Index:   name = "Index"; break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

#endif // EV2_BUFFER_HPP