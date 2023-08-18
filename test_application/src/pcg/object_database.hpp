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

#include <memory>
#include <string_view>
#include <utility>
#include "evpch.hpp"

#include "resource.hpp"
#include "wfc.hpp"
#include "renderer/renderer.hpp"
#include "geometry.hpp"

namespace ev2::pcg {

class ObjectData {
public:
    enum class Orientation {
        Free = 0,
        Lock,
        Stepped // 4 possible orientations N S E W
    };

    struct XYZ {
        union {
            Orientation v[3];
            struct {
                Orientation x;
                Orientation y;
                Orientation z;
            };
        }; 
    };

    // serialized members
    std::string name{};
    std::string asset_path{};
    std::unordered_map<std::string, float> properties{};
    std::vector<OBB> propagation_patterns{};
    float extent = 1.f;
    XYZ axis_settings{};

    // instance members
    std::shared_ptr<renderer::Mesh> loaded_model{};

public:

    bool is_valid() const noexcept {
        return extent > 0.f && !name.empty() && !asset_path.empty();
    }

    float try_get_property(const std::string& p_name, float default_val = 0.f) {
        auto itr = properties.find(p_name);
        if(itr != properties.end()) {
            return itr->second;
        }
        return default_val;
    }

    void set_asset_path(std::string_view asset_path) {
        this->asset_path = asset_path;
        loaded_model = ResourceManager::get_singleton().get_model_relative_path(asset_path);
    }

    float get_model_scale() const {
        if (loaded_model)
            return extent / glm::length(loaded_model->bounding_box.diagonal()); // scale uniformly
        return 1.f;
    }

    AABB get_scaled_bounding_box() const {
        return loaded_model->bounding_box.scale(glm::vec3{get_model_scale()});
    }

};

class ObjectMetadataDB {
public:
    using object_data_map_t = std::unordered_multimap<int, ObjectData>; // class_id -> ObjectData[]
    using pattern_map_t = std::unordered_map<int, wfc::Pattern>; // pattern_id -> Pattern
    using class_id_map_t = std::unordered_map<int, std::string>; // class_id -> name

public:
    ObjectMetadataDB() = default;

    static std::unique_ptr<ObjectMetadataDB> load_object_database(std::string_view path);
    
    void write_database(std::string_view path) const;

    /**
     * @brief 
     * 
     * @param name 
     * @param id 
     * @return void 
     */
    void set_class_name(std::string_view name, int class_id);

    /**
     * @brief Get the name of a class given its id
     * 
     * @param class_id 
     * @return std::string empty if id not found
     */
    std::string get_class_name(int class_id) const;

    class_id_map_t::const_iterator erase_class(int class_id) {
        if (auto itr = m_object_classes.find(class_id); itr != m_object_classes.end())
            return m_object_classes.erase(itr);
        return m_object_classes.end();
    }

    /**
     * @brief Get a new id for an object class
     * 
     * @return int 
     */
    int create_class_id() {return ++max_class_id;}

    std::vector<std::pair<int, std::string>> get_class_names() const;

    // ObjectData interface

    auto objs_for_id(int class_id) {
        return m_obj_data.equal_range(class_id);
    }

    object_data_map_t::iterator objs_erase(object_data_map_t::iterator iterator) {
        return m_obj_data.erase(iterator);
    }

    void objs_add(const ObjectData& obj, int class_id) {
        m_obj_data.emplace(std::make_pair(class_id, obj));
    }

    // Pattern interface

    int create_pattern_id() {return ++max_pattern_id;}

    pattern_map_t::const_iterator pattern_erase(pattern_map_t::const_iterator itr) {
        remove_pattern_from_class_id_map(itr);
        return m_patterns.erase(itr);
    }

    bool add_pattern(const wfc::Pattern& pattern, int pattern_id) {
        auto [itr, inserted] = m_patterns.emplace(std::make_pair(pattern_id, pattern));
        if (inserted) add_pattern_to_class_id_map(itr);
        return inserted;
    }

    const wfc::Pattern* get_pattern(int pattern_id) const {
        const wfc::Pattern* out = nullptr;
        auto itr = m_patterns.find(pattern_id);
        if (itr != m_patterns.end())
            out = &itr->second;
        return out;
    }

    auto patterns_size() const noexcept {
        return m_patterns.size();
    }

    auto patterns_for_class_id(int obj_class_id) {
        return m_patterns_for_class_id.equal_range(obj_class_id);
    }

    void pattern_change_class(pattern_map_t::const_iterator cit, int obj_class_id) {
        auto it = to_internal_iterator(cit);

        remove_pattern_from_class_id_map(it);
        it->second.pattern_type = obj_class_id;
        add_pattern_to_class_id_map(it);
    }

    auto get_patterns_iterator() const {
        return std::make_pair(m_patterns.cbegin(), m_patterns.cend());
    }

    auto patterns_end() const {return m_patterns.cend();}

    /**
     * @brief Make map from pattern_id to patterns
     * 
     * @return wfc::PatternMap 
     */
    wfc::PatternMap make_pattern_map() const {
        wfc::PatternMap pm{};
        pm.reserve(m_patterns_for_class_id.size());
        for (auto itr = m_patterns.begin(), end = m_patterns.end(); itr != end; ++itr) {
            pm.insert(std::make_pair(itr->first, itr->second));
        }
        return pm;
    }

    // Requirement Interface

    std::vector<int>::const_iterator pattern_erase_requirement(pattern_map_t::const_iterator cit, std::vector<int>::const_iterator rqit) {
        auto it = to_internal_iterator(cit);
        return it->second.required_types.erase(rqit);
    }

    void pattern_add_requirement(pattern_map_t::const_iterator cit, int class_id) {
        auto it = to_internal_iterator(cit);
        it->second.required_types.push_back(class_id);
    }

    void pattern_set_weight(pattern_map_t::const_iterator cit, float weight) {
        auto it = to_internal_iterator(cit);
        it->second.weight = weight;
    }

private:
    void check_max_ids() {
        max_class_id = 0;
        for(auto& [id, obj_class] : m_object_classes) {
            max_class_id = std::max(max_class_id, id);
        }
    }

    void remove_pattern_from_class_id_map(pattern_map_t::const_iterator p) {
        for (auto [b,e] = m_patterns_for_class_id.equal_range(p->second.pattern_type); b != e; ++b) {
            // remove pattern matching p's pattern_id
            const auto& [c_id, p_id] = *b;
            if (p->first == p_id) {
                m_patterns_for_class_id.erase(b);
                break;
            }
        }
    }

    void add_pattern_to_class_id_map(pattern_map_t::const_iterator p) {
        m_patterns_for_class_id.emplace(std::make_pair(p->second.pattern_type, p->first));
    }

    void refresh_class_id_pattern_map() {
        m_patterns_for_class_id = {};
        for (auto& [pattern_id, p] : m_patterns) {
            m_patterns_for_class_id.insert(std::make_pair(p.pattern_type, pattern_id));
        }
    }

    pattern_map_t::iterator to_internal_iterator(pattern_map_t::const_iterator cit) {
        // from https://www.technical-recipes.com/2012/how-to-convert-const_iterators-to-iterators-using-stddistance-and-stdadvance/
        // convert constant iterator to an iterator in our internal list
        using c_iter = pattern_map_t::const_iterator;
        auto it = m_patterns.begin();
        std::advance (it, std::distance<c_iter>(it, cit));
        return it;
    }

private:
    pattern_map_t m_patterns{};
    int max_pattern_id = 0;

    std::unordered_multimap<int, ObjectData> m_obj_data{};
    std::unordered_map<int, std::string> m_object_classes{};
    int max_class_id = 0;

    // map class ids to patterns for those classes that can be applied.
    std::unordered_multimap<int, int> m_patterns_for_class_id{};
};


} // namespace ev2::pcg

#endif // EV2_PCG_OBJECT_DATABASE_HPP