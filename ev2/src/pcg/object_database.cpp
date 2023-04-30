#include "object_database.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "io/serializers.hpp"
#include "pcg/wfc.hpp"
#include "engine.hpp"

namespace ev2::pcg {

std::shared_ptr<renderer::Drawable> ObjectMetadataDB::get_model_for_id(int id) {
    return m_meshes.at(id);
}

void ObjectMetadataDB::add_model(std::shared_ptr<renderer::Drawable> d, int id) {
    auto [_it, ins] = m_meshes.emplace(std::make_pair(id, d));
    if (!ins)
        throw std::runtime_error{"model already inserted for " + std::to_string(id)};
}

void ObjectMetadataDB::set_object_class_name(std::string_view name, int id) {
    max_id = std::max(max_id, id);
    m_object_classes.insert_or_assign(id, name.data());
}

std::string ObjectMetadataDB::object_class_name(int id) const {
    std::string val{};
    if (auto itr = m_object_classes.find(id); itr != m_object_classes.end()) {
        val = itr->second;
    }
    return val;
}

bool ObjectMetadataDB::object_class_delete(int id, int rename_id) {

    return false;
}

template<typename K, typename V>
std::unordered_map<V, K> inverse_map(std::unordered_map<K, V> &map)
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

        j.at("patterns").get_to(db->patterns);
        db->m_object_classes = inverse_map(object_classes);

        for (auto& [class_name, obj_vec] : object_data) {
            for (auto& obj : obj_vec) {
                db->m_obj_data.insert(std::make_pair(
                    object_classes.at(class_name), obj));
            }
        }

        db->check_max_id();
    } catch (const json::type_error& error) {
        Engine::log("Failed to load " + std::string{path.data()} + ": " + error.what());
    }

    return db;
}

void ObjectMetadataDB::write_database(std::string_view path) {
    using json = nlohmann::json;

    std::unordered_map<std::string, int> object_classes = inverse_map(m_object_classes);

    // convert the multimap to something nlohmann::json can manage 
    std::unordered_map<std::string, std::vector<ObjectData>> object_data;
    std::for_each(m_obj_data.begin(), m_obj_data.end(),
                [this, &object_data] (const auto &p) {
                    object_data[m_object_classes[p.first]].push_back(p.second);
                });

    json j = json{{"object_data", object_data},
                  {"object_classes", object_classes},
                  {"patterns", patterns}};

    std::ofstream ostr{path.data(), std::ios::out};
    if (!ostr.is_open()) {
        throw std::ios_base::failure("file does not exist");
    }

    ostr << j.dump(4) << std::endl;
    ostr.close();
}

std::vector<std::pair<int, std::string>> ObjectMetadataDB::get_classes() const {
    return {m_object_classes.begin(), m_object_classes.end()};
}

} // namespace ev2::pcg