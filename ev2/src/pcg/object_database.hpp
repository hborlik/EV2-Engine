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

#include <algorithm>
#include <cstddef>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "wfc.hpp"
#include "renderer/renderer.hpp"
#include "geometry.hpp"

namespace ev2::pcg {

struct ObjectData {
    std::string name{};
    std::string asset_path{};
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
    using ObjectDataMap = std::unordered_multimap<int, ObjectData>;

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

    /**
     * @brief Get the name of a class given its id
     * 
     * @param id 
     * @return std::string empty if id not found
     */
    std::string object_class_name(int id) const;

    /**
     * @brief Delete the object class with the selected id and change all patterns that
     *  depend on it to the given id
     * 
     * @param id 
     * @param rename_id 
     * @return true 
     * @return false 
     */
    bool object_class_delete(int id, int rename_id);

    int object_class_create_id() {return ++max_id;}
    
    static std::unique_ptr<ObjectMetadataDB> load_object_database(std::string_view path);
    
    void write_database(std::string_view path);

    std::vector<std::pair<int, std::string>> get_classes() const;

    auto objs_for_id(int id) {
        return m_obj_data.equal_range(id);
    }

    void objs_erase(ObjectDataMap::iterator iterator) {
        m_obj_data.erase(iterator);
    }

    void objs_add(const ObjectData& obj, int id) {
        m_obj_data.emplace(std::make_pair(id, obj));
    }

private:
    void check_max_id() {
        max_id = 0;
        for(auto& [id, obj_class] : m_object_classes) {
            max_id = std::max(max_id, id);
        }
    }

public:
    std::list<wfc::Pattern> patterns{};

private:
    std::unordered_map<int, std::shared_ptr<renderer::Drawable>> m_meshes{};
    std::unordered_multimap<int, ObjectData> m_obj_data{};
    std::unordered_map<int, std::string> m_object_classes{};
    int max_id = 0;
};


} // namespace ev2::pcg

#endif // EV2_PCG_OBJECT_DATABASE_HPP