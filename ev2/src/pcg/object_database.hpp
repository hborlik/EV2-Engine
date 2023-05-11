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

#include <utility>
#include "evpch.hpp"

#include "wfc.hpp"
#include "renderer/renderer.hpp"
#include "geometry.hpp"

namespace ev2::pcg {

struct ObjectData {
    enum class Orientation {
        Free = 0,
        Lock,
        Stepped // 4 possible orientations N S E W
    };

    std::string name{};
    std::string asset_path{};
    std::unordered_map<std::string, float> properties{};
    std::vector<OBB> propagation_patterns{};
    float extent = 1.f;

    struct XYZ {
        union {
            Orientation v[3];
            struct {
                Orientation x;
                Orientation y;
                Orientation z;
            };
        }; 
    } axis_settings{};

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
    using object_data_map_t = std::unordered_multimap<int, ObjectData>;
    using pattern_list_t = std::list<wfc::Pattern>;
    using class_id_map_t = std::unordered_map<int, std::string>;

public:
    ObjectMetadataDB() = default;

    /**
     * @brief 
     * 
     * @param name 
     * @param id 
     * @return void 
     */
    void set_class_name(std::string_view name, int id);

    /**
     * @brief Get the name of a class given its id
     * 
     * @param id 
     * @return std::string empty if id not found
     */
    std::string get_class_name(int id) const;

    class_id_map_t::const_iterator erase_class(int id) {
        if (auto itr = m_object_classes.find(id); itr != m_object_classes.end())
            return m_object_classes.erase(itr);
        return m_object_classes.end();
    }

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

    object_data_map_t::iterator objs_erase(object_data_map_t::iterator iterator) {
        return m_obj_data.erase(iterator);
    }

    void objs_add(const ObjectData& obj, int id) {
        m_obj_data.emplace(std::make_pair(id, obj));
    }

    auto get_patterns_iterator() {
        return std::make_pair(m_patterns.cbegin(), m_patterns.cend());
    }

    wfc::PatternMap make_pattern_map() const {
        wfc::PatternMap pm{};
        pm.reserve(m_patterns_for_class.size());
        for (auto itr = m_patterns.begin(), end = m_patterns.end(); itr != end; ++itr) {
            pm.insert(std::make_pair(itr->pattern_class, *itr));
        }
        return pm;
    }

    pattern_list_t::const_iterator pattern_erase(pattern_list_t::const_iterator itr) {
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

    void pattern_change_class(pattern_list_t::const_iterator cit, int id) {
        auto it = to_internal_iterator(cit);

        remove_pattern_from_id_map(&*it);
        it->pattern_class = id;
        add_pattern_to_id_map(&*it);
    }

    std::vector<int>::const_iterator pattern_erase_requirement(pattern_list_t::const_iterator cit, std::vector<int>::const_iterator rqit) {
        auto it = to_internal_iterator(cit);
        return it->required_classes.erase(rqit);
    }

    void pattern_add_requirement(pattern_list_t::const_iterator cit, int rq) {
        auto it = to_internal_iterator(cit);
        it->required_classes.push_back(rq);
    }

    void pattern_set_weight(pattern_list_t::const_iterator cit, float weight) {
        auto it = to_internal_iterator(cit);
        it->weight = weight;
    }

private:
    void check_max_id() {
        max_id = 0;
        for(auto& [id, obj_class] : m_object_classes) {
            max_id = std::max(max_id, id);
        }
    }

    void remove_pattern_from_id_map(const wfc::Pattern* p) {
        for (auto [b,e] = m_patterns_for_class.equal_range(p->pattern_class); b != e; ++b) {
            auto& [id, pattern] = *b;
            if (pattern == p) {
                m_patterns_for_class.erase(b);
                break;
            }
        }
    }

    void add_pattern_to_id_map(wfc::Pattern* p) {
        m_patterns_for_class.emplace(std::make_pair(p->pattern_class, p));
    }

    void refresh_class_id_pattern_map() {
        m_patterns_for_class = {};
        for (auto& p : m_patterns) {
            m_patterns_for_class.insert(std::make_pair(p.pattern_class, &p));
        }
    }

    pattern_list_t::iterator to_internal_iterator(pattern_list_t::const_iterator cit) {
        // from https://www.technical-recipes.com/2012/how-to-convert-const_iterators-to-iterators-using-stddistance-and-stdadvance/
        // convert constant iterator to an iterator in our internal list
        using c_iter = pattern_list_t::const_iterator;
        auto it = m_patterns.begin();
        std::advance (it, std::distance<c_iter>(it, cit));
        return it;
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