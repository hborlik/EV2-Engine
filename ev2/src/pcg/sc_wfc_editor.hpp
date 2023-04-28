/**
 * @file sc_wfc_editor.hpp
 * @brief 
 * @date 2023-04-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
 #ifndef EV2_PCG_SCWFC_EDITOR_HPP
 #define EV2_PCG_SCWFC_EDITOR_HPP

#include "ui/ui.hpp"
#include "object_database.hpp"
#include "sc_wfc.hpp"

namespace ev2::pcg {

class SCWFCGraphNode;

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
    SCWFCEditor();

    void show_editor_tool() override;

    std::string get_name() const override {
        return "SCWFC Editor";
    }

    void on_selected_node(Node* node) override;

    void load_obj_db();

    void save_obj_db(std::string_view path);

    void sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass);

    void wfc_solve(int steps);

private:
    struct PatternProperties {
        wfc::Pattern* pattern;
        std::string name;
        float weight;
    };

    struct Data;

private:
    void db_editor_show_pattern_editor_widget();
    void show_pattern_property_editor_popup(PatternProperties& prop);

    void show_db_editor_window(bool* p_open);

private:
    std::shared_ptr<Data> m_internal{};
    Ref<SCWFC> m_scwfc_node{};
    std::shared_ptr<ObjectMetadataDB> obj_db{};

    bool m_db_editor_open = false;
};

}

 #endif // EV2_PCG_SCWFC_EDITOR_HPP