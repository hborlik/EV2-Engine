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

#include <cassert>
#include <string>
#include <string_view>

#include "pcg/wfc.hpp"
#include "ui/ui.hpp"
#include "ui/file_dialog.hpp"
#include "object_database.hpp"
#include "sc_wfc.hpp"

namespace ev2::pcg {

class SCWFCGraphNode;

/**
 * @brief Custom editor tool
 * 
 */
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

    ObjectMetadataDB* get_object_db() noexcept {
        return obj_db.get();
    }

private:
    struct PatternProperties {
        int pattern_class = -1;
        float weight = 0.f;
    };

    struct ObjectClassProperties {
        std::string name{};
    };

    struct Data;

private:
    void db_editor_show_pattern_editor_widget();
    void db_editor_show_object_class_editor_widget();

    bool show_dbe_edit_object_class_popup(std::string_view name, ObjectClassProperties& prop);
    bool show_dbe_edit_pattern_popup(std::string_view name, PatternProperties& prop);
    bool show_dbe_edit_object_data_popup(std::string_view name, ObjectData& prop);

    bool show_class_select_popup(std::string_view popup_name, int& item_current_idx, int& selection_out, bool close_on_pick);

    void show_db_editor_window(bool* p_open);

private:
    std::random_device rd;
    std::mt19937 mt;
    std::shared_ptr<Data> m_internal{};
    Ref<SCWFC> m_scwfc_node{};
    std::shared_ptr<ObjectMetadataDB> obj_db{};

    bool m_db_editor_open = false;
    ui::FileDialogWindow m_file_dialog{};
};

/**
 * @brief Defines the custom editor behavior for SCWFC nodes in scene
 * 
 */
class SCWFCGraphNodeEditor : public NodeEditorT<SCWFCGraphNode> {
public:
    SCWFCGraphNodeEditor(SCWFCEditor* editor) : m_scwfc_editor{editor} {
        assert(editor);
    }

    void show_editor(Node* node) override;

private:
    SCWFCEditor* m_scwfc_editor = nullptr;
};

}

 #endif // EV2_PCG_SCWFC_EDITOR_HPP