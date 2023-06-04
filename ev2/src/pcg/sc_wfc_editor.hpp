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

#include "evpch.hpp"

#include "pcg/sc_wfc_solver.hpp"
#include "pcg/wfc.hpp"
#include "renderer/renderer.hpp"
#include "application.hpp"
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
    SCWFCEditor(Application* app);

    void show_editor_tool() override;

    std::string get_name() const override {
        return "SCWFC Editor";
    }

    void on_selected_node(Node* node) override;

    void load_default_obj_db();

    void load_obj_db(std::string_view path);

    void new_obj_db();

    void save_obj_db(std::string_view path);

    ObjectMetadataDB* get_object_db() noexcept {
        return m_obj_db.get();
    }

private:
    struct PatternProperties {
        int pattern_class = -1;
        float weight = 0.f;
    };

    struct ObjectClassProperties {
        std::string name{};
    };

private:
    void db_editor_show_pattern_editor_widget();
    void db_editor_show_object_class_editor_widget();

    bool show_dbe_edit_object_class_popup(std::string_view name, ObjectClassProperties& prop);
    bool show_dbe_edit_pattern_popup(std::string_view name, PatternProperties& prop);
    bool show_dbe_edit_object_data_popup(std::string_view name, ObjectData& prop);

    bool show_class_select_popup(std::string_view popup_name, int& item_current_idx, int& selection_out, bool close_on_pick);

    void show_db_editor_window(bool* p_open);

    void reset_solver();

private:
    Application* app = nullptr;
    std::random_device m_rd{};
    Ref<SCWFC> m_scwfc_node{};
    std::shared_ptr<ObjectMetadataDB> m_obj_db{};
    std::shared_ptr<renderer::Drawable> m_unsolved_drawable;

    SCWFCSolverArgs m_solver_args{};
    std::unique_ptr<SCWFCSolver> m_scwfc_solver;

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