/**
 * @file sc_wfc.hpp
 * @brief 
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_SC_WHC_HPP
#define EV2_SC_WHC_HPP

#include <functional>

#include <scene/node.hpp>
#include <ui/ui.hpp>
#include <geometry.hpp>

namespace ev2 {

class renderer::Drawable;

struct SCWFCObjectMetadata {
    std::string name;
    std::string asset_path;
    std::unordered_map<std::string, float> properties;
    std::vector<OBB> propagation_patterns;

    float try_get_property(const std::string& p_name, float default_val = 0.f) {
        auto itr = properties.find(p_name);
        if(itr != properties.end()) {
            return itr->second;
        }
        return default_val;
    }
};

class SCWFCObjectDatabase {
public:

    std::shared_ptr<renderer::Drawable> get_model_for_id(int id);

    void add_model(std::shared_ptr<renderer::Drawable> d, int id);

    std::unordered_map<int, std::shared_ptr<renderer::Drawable>> m_meshes{};
};

std::unique_ptr<SCWFCObjectDatabase> load_object_database(const std::string& path);

class SCWFC : public Node {
public:
    explicit SCWFC(std::string name) : Node{std::move(name)} {}

    /**
     * @brief solve WFC domains steps times 
     * 
     * @param steps steps to run the solver
     */
    void wfc_solve(int steps);

    void sc_spawn_points(int n);

    void sc_propagate_from(Ref<Node> node);

    void reset();

    void on_init() override;

    void on_child_removed(Ref<Node> child) override;

private:
    friend class SCWFCGraphNode;

    Sphere* create_sphere();
    void destroy_sphere();

private:
    struct Data;
    std::shared_ptr<Data> m_data{};
};

/**
 * @brief Defines the custom editor behavior for SCWFC nodes in scene
 * 
 */
class SCWFCEditor : public NodeEditorT<SCWFC> {
public:
    void show_editor(Node* node) override;
};

}

#endif // EV2_SC_WHC_HPP