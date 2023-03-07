/**
 * @file material_storage.hpp
 * @brief 
 * @date 2023-03-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_RENDERER_MATERIAL_STORAGE_HPP
#define EV2_RENDERER_MATERIAL_STORAGE_HPP

#include <unordered_map>
#include <array>
#include <queue>
#include <cstdint>

#include <renderer/shader.hpp>

namespace ev2::renderer {

constexpr uint16_t MAX_N_MATERIALS = 255;

class RendererMaterialStorage {
public:

    struct UBOElement {
        int size;
        int offset;
    };

    struct MaterialData {
        std::vector<uint8_t> material_data_buffer; // cpu side material data
        Buffer uniform_buffer;
    };

    struct Material {
        MaterialData* material = nullptr;
        Program* shader = nullptr;
    };


    void material_allocate();
    void material_free();

    void shader_allocate();
    void shader_free();

private:
    // material management
    std::unordered_map<int32_t, Material*> materials;
    std::queue<int32_t> free_material_slots; // queue of free slots in material_data_buffer
    int32_t next_material_id = 1234;

};

} // namespace ev2::renderer

#endif // EV2_RENDERER_MATERIAL_STORAGE_HPP