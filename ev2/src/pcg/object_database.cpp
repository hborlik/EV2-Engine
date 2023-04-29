#include "object_database.hpp"

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
    m_object_classes.insert_or_assign(id, name.data());
}

std::string ObjectMetadataDB::object_class_name(int id) {
    std::string val{};
    if (auto itr = m_object_classes.find(id); itr != m_object_classes.end()) {
        val = itr->second;
    }
    return val;
}

std::unique_ptr<ObjectMetadataDB> ObjectMetadataDB::load_object_database(std::string_view path) {
    using json = nlohmann::json;

    auto db = std::make_unique<ObjectMetadataDB>();

    try {
        std::string json_str = io::read_file(path);
        json j = json::parse(json_str);
        j.at("m_obj_data").get_to(db->m_obj_data);
        j.at("m_object_classes").get_to(db->m_object_classes);
        j.at("patterns").get_to(db->patterns);
    } catch (const json::type_error& error) {
        Engine::log("Failed to load " + std::string{path.data()} + ": " + error.what());
    }

    return db;
}

void ObjectMetadataDB::write_database(std::string_view path) {
    using json = nlohmann::json;

    json j = json{{"m_obj_data", m_obj_data},
                  {"m_object_classes", m_object_classes},
                  {"patterns", patterns}};

    std::ofstream ostr{path.data(), std::ios::out};
    if (!ostr.is_open()) {
        throw std::ios_base::failure("file does not exist");
    }

    ostr << j.dump() << std::endl;
    ostr.close();
}

std::vector<std::pair<int, std::string>> ObjectMetadataDB::get_classes() const {
    return {m_object_classes.begin(), m_object_classes.end()};
}

} // namespace ev2::pcg