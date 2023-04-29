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

#include "pcg/wfc.hpp"
#include "ui/ui.hpp"
#include "ui/file_dialog.hpp"
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

    void load_default_obj_db();

    void load_obj_db(std::string_view path);

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
    bool show_pattern_edit_pattern_popup(PatternProperties& prop);
    bool show_object_data_editor_popup(ObjectData& prop);
    bool show_class_select_popup(std::string_view popup_name, int& item_current_idx, wfc::Value& selection_out, bool close_on_pick);

    void show_db_editor_window(bool* p_open);

private:
    std::shared_ptr<Data> m_internal{};
    Ref<SCWFC> m_scwfc_node{};
    std::shared_ptr<ObjectMetadataDB> obj_db{};

    bool m_db_editor_open = false;
    ui::FileDialogWindow m_file_dialog{};
};

}

 #endif // EV2_PCG_SCWFC_EDITOR_HPP