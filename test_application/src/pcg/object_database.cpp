#include "object_database.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "io/serializers.hpp"
#include "pcg/wfc.hpp"
#include "core/engine.hpp"

namespace ev2::pcg {

void ObjectMetadataDB::set_class_name(std::string_view name, int class_id) {
    max_class_id = std::max(max_class_id, class_id);
    m_object_classes.insert_or_assign(class_id, name.data());
}

std::string ObjectMetadataDB::get_class_name(int class_id) const {
    std::string val{};
    if (auto itr = m_object_classes.find(class_id); itr != m_object_classes.end()) {
        val = itr->second;
    }
    return val;
}

template<typename K, typename V>
std::unordered_map<V, K> inverse_map(const std::unordered_map<K, V> &map)
{
    std::unordered_map<V, K> inv;
    std::for_each(map.begin(), map.end(),
                [&inv] (const std::pair<K, V> &p) {
                    inv.insert(std::make_pair(p.second, p.first));
                });
    return inv;
}

std::unique_ptr<ObjectMetadataDB> ObjectMetadataDB::load_object_database(std::string_view path) {
    using json = nlohmann::json;

    auto db = std::make_unique<ObjectMetadataDB>();

    std::unordered_map<std::string, int> object_classes;
    std::unordered_map<std::string, std::vector<ObjectData>> object_data;

    try {
        std::string json_str = io::read_file(path);
        json j = json::parse(json_str);
        j.at("object_data").get_to(object_data);
        j.at("object_classes").get_to(object_classes);
        j.at("patterns").get_to(db->m_patterns);

        db->m_object_classes = inverse_map(object_classes);

        for (auto& [class_name, obj_vec] : object_data) {
            if (!class_name.empty())
                for (auto& obj : obj_vec) {
                    obj.set_asset_path(obj.asset_path); // try loading asset path
                    db->m_obj_data.insert(std::make_pair(
                        object_classes.at(class_name), obj));
                }
        }

        db->refresh_class_id_pattern_map();

        db->check_max_ids();
    } catch (const std::exception& error) {
        Log::error_core<ObjectMetadataDB>("Failed to load " + std::string{path.data()} + ": " + error.what());
    }

    return db;
}

void ObjectMetadataDB::write_database(std::string_view path) const {
    using json = nlohmann::json;

    std::unordered_map<std::string, int> object_classes = inverse_map(m_object_classes);

    // convert the multimap to something nlohmann::json can manage 
    std::unordered_map<std::string, std::vector<ObjectData>> object_data;
    std::for_each(m_obj_data.begin(), m_obj_data.end(),
                [this, &object_data] (const auto &p) {
                    auto itr = m_object_classes.find(p.first);
                    if (itr != m_object_classes.end())
                        object_data[itr->second].push_back(p.second);
                });

    json j = json{{"object_data", object_data},
                  {"object_classes", object_classes},
                  {"patterns", m_patterns}};

    std::ofstream ostr{path.data(), std::ios::out};
    if (!ostr.is_open()) {
        throw std::ios_base::failure("file does not exist");
    }

    ostr << j.dump(4) << std::endl;
    ostr.close();
}

std::vector<std::pair<int, std::string>> ObjectMetadataDB::get_class_names() const {
    return {m_object_classes.begin(), m_object_classes.end()};
}

} // namespace ev2::pcg