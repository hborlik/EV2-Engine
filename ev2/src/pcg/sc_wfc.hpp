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
#include <optional>

#include <scene/node.hpp>
#include <ui/ui.hpp>
#include <geometry.hpp>

namespace ev2::pcg {

class SCWFCGraphNode;

struct SCWFCObjectMetadataDB {
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

    std::shared_ptr<renderer::Drawable> get_model_for_id(int id);

    void add_model(std::shared_ptr<renderer::Drawable> d, int id);

    std::unordered_map<int, std::shared_ptr<renderer::Drawable>> m_meshes{};
};


std::unique_ptr<SCWFCObjectMetadataDB> load_object_database(const std::string& path);

class SCWFC : public Node {
public:
    explicit SCWFC(std::string name);

    void reset();

    void on_init() override;

    void on_child_removed(Ref<Node> child) override;

    void on_child_added(Ref<Node> child, int index) override;

    void update_all_adjacencies(Ref<class SCWFCGraphNode>& n, float radius);

    glm::vec3 sphere_repulsion(const Sphere& sph) const;

    /**
     * @brief Check if a node intersects any of its adjacent nodes in the scene.
     *  Note that this does not check for intersections among all children, only those nodes
     *  added by update_all_adjacencies()
     * 
     * @param n 
     * @return true 
     * @return false 
     */
    bool intersects_any_solved_neighbor(const Ref<class SCWFCGraphNode>& n);

private:
    friend class SCWFCEditor;
    struct Data;
    std::shared_ptr<Data> m_data{};
};

/**
 * @brief Defines the custom editor behavior for SCWFC nodes in scene
 * 
 */
class SCWFCNodeEditor : public NodeEditorT<SCWFC> {
public:
    void show_editor(Node* node) override;
};

class SCWFCEditor : public EditorTool {
public:
    void show_editor_tool() override;
    std::string get_name() const override {
        return "SCWFCEditor";
    }

    void load_obj_db();

    void on_selected_node(Node* node) override;

    void sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass);

    void wfc_solve(int steps);

private:
    struct Data;
    std::shared_ptr<Data> m_internal{};
    Ref<SCWFC> m_scwfc_node{};
    std::shared_ptr<SCWFCObjectMetadataDB> obj_db{};
};

}

#endif // EV2_SC_WHC_HPP