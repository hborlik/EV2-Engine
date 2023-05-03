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
#include <cstdio>
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
    float extent = 1.f;

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
};

class ObjectMetadataDB {
public:
    using ObjectDataMap = std::unordered_multimap<int, ObjectData>;
    using pattern_list_t = std::list<wfc::Pattern>;

public:
    ObjectMetadataDB() = default;

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
    std::string get_object_class_name(int id) const;

    /**
     * @brief Get a new id for an object class
     * 
     * @return int 
     */
    int object_class_create_id() {return ++max_id;}
    
    static std::unique_ptr<ObjectMetadataDB> load_object_database(std::string_view path);
    
    void write_database(std::string_view path);

    std::vector<std::pair<int, std::string>> get_classes() const;

    auto objs_for_id(int id) {
        return m_obj_data.equal_range(id);
    }

    ObjectDataMap::iterator objs_erase(ObjectDataMap::iterator iterator) {
        return m_obj_data.erase(iterator);
    }

    void objs_add(const ObjectData& obj, int id) {
        m_obj_data.emplace(std::make_pair(id, obj));
    }

    auto get_patterns() {
        return std::make_pair(m_patterns.begin(), m_patterns.end());
    }

    pattern_list_t::iterator erase_pattern(pattern_list_t::iterator itr) {
        remove_pattern_from_id_map(&*itr);
        return m_patterns.erase(itr);
    }

    void add_pattern(const wfc::Pattern& pattern) {
        m_patterns.push_back(pattern);
        add_pattern_to_id_map(&m_patterns.back());
    }

    auto patterns_size() const noexcept {
        return m_patterns.size();
    }

    auto patterns_for_id(int id) {
        return m_patterns_for_class.equal_range(id);
    }

    void pattern_change_class(pattern_list_t::iterator itr, int id) {
        remove_pattern_from_id_map(&*itr);
        itr->pattern_class.val = id;
        add_pattern_to_id_map(&*itr);
    }

private:
    void check_max_id() {
        max_id = 0;
        for(auto& [id, obj_class] : m_object_classes) {
            max_id = std::max(max_id, id);
        }
    }

    void remove_pattern_from_id_map(const wfc::Pattern* p) {
        for (auto [b,e] = m_patterns_for_class.equal_range(p->pattern_class.val); b != e; ++b) {
            auto& [id, pattern] = *b;
            if (pattern == p) {
                m_patterns_for_class.erase(b);
                break;
            }
        }
    }

    void add_pattern_to_id_map(wfc::Pattern* p) {
        m_patterns_for_class.emplace(std::make_pair(p->pattern_class.val, p));
    }

    void refresh_class_id_pattern_map() {
        m_patterns_for_class = {};
        for (auto& p : m_patterns) {
            m_patterns_for_class.insert(std::make_pair(p.pattern_class.val, &p));
        }
    }

private:
    pattern_list_t m_patterns{};

    // map class ids to patterns that can be applied. 
    std::unordered_multimap<int, wfc::Pattern*> m_patterns_for_class{};

    std::unordered_multimap<int, ObjectData> m_obj_data{};
    std::unordered_map<int, std::string> m_object_classes{};
    int max_id = 0;
};


} // namespace ev2::pcg

#endif // EV2_PCG_OBJECT_DATABASE_HPP