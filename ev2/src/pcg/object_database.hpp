/**
 * @file object_database.hpp
 * @brief 
 * @date 2023-04-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_PCG_OBJECT_DATABASE_HPP
#define EV2_PCG_OBJECT_DATABASE_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "wfc.hpp"
#include "renderer/renderer.hpp"
#include "geometry.hpp"

namespace ev2::pcg {

struct ObjectData {
    std::string name{};
    std::string asset_path{};
    int object_class{};
    std::unordered_map<std::string, float> properties{};
    std::vector<OBB> propagation_patterns{};

    float try_get_property(const std::string& p_name, float default_val = 0.f) {
        auto itr = properties.find(p_name);
        if(itr != properties.end()) {
            return itr->second;
        }
        return default_val;
    }
};

class ObjectMetadataDB {
public:
    ObjectMetadataDB() = default;
    
    std::shared_ptr<renderer::Drawable> get_model_for_id(int id);
    void add_model(std::shared_ptr<renderer::Drawable> d, int id);

    /**
     * @brief 
     * 
     * @param name 
     * @param id 
     * @return void 
     */
    void set_object_class_name(std::string_view name, int id);

    std::string object_class_name(int id);
    
    static std::unique_ptr<ObjectMetadataDB> load_object_database(std::string_view path);
    
    void write_database(std::string_view path);

public:
    std::vector<wfc::Pattern> patterns{};

private:
    std::unordered_map<int, std::shared_ptr<renderer::Drawable>> m_meshes{};
    std::vector<ObjectData> m_obj_data{};
    std::unordered_map<int, std::string> m_object_classes{};
};


} // namespace ev2::pcg

#endif // EV2_PCG_OBJECT_DATABASE_HPP