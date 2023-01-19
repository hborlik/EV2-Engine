#include <ui/ui.hpp>
#include <ui/imgui.h>

namespace ev2 {

void SceneEditor::show_scene_explorer(Scene* scene) {
    show_scene_node_widget(0, (Node*)scene);
}

void SceneEditor::show_scene_node_widget(int id, Node* node) {
    if (!node)
        return;
    const std::string node_name = std::to_string(id) + " " + node->name;
    
    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    static bool test_drag_and_drop = true;
    // ImGui::Checkbox("Align label with current X position", &align_label_with_current_x_position);
    // ImGui::Checkbox("Test tree node as drag source", &test_drag_and_drop);

    // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
    // To alter selection we use IsItemClicked() && !IsItemToggledOpen(), so clicking on an arrow doesn't alter selection.
    ImGuiTreeNodeFlags node_flags = base_flags;
    const bool is_selected = selected_node == node;
    if (is_selected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if (node->get_n_children() == 0)
        node_flags |= ImGuiTreeNodeFlags_Leaf;

    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)id, node_flags, node_name.c_str());
    
    int node_clicked = -1;
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        node_clicked = id;
    
    if (test_drag_and_drop && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("SCENE_TREENODE", &node, sizeof(node));
        ImGui::Text(node_name.c_str());
        ImGui::EndDragDropSource();
    }
    
    if (test_drag_and_drop && ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* drag_drop = ImGui::AcceptDragDropPayload("SCENE_TREENODE")) {
            Node* moved_node = *(Node**)drag_drop->Data;
            std::cout << "TODO: impl move node (" << moved_node->name << ") " << moved_node->get_path() << " to (" << node->name << ") " << node->get_path() << std::endl;
        }
        ImGui::EndDragDropTarget();
    }

    if (node_clicked != -1)
    {
        // Update selection state
        // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
        selected_node = node;
        // if (ImGui::GetIO().KeyCtrl)
        //     selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
        // else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
        //     selection_mask = (1 << node_clicked);           // Click to single-select
    }


    if (node_open) {
        int i = 0;
        for (auto& node : node->get_children()) {
            show_scene_node_widget(i, node.get());
            i++;
        }
        ImGui::TreePop();
    }
}

void SceneEditor::show_transform_editor(Transform* tr) {
    if (!tr)
        return;

    glm::vec3 v = tr->get_position();
    if (ImGui::DragFloat3("position", glm::value_ptr(v), 0.1, 0.f, 0.f, "%.3f")) {
        tr->set_position(v);
    }

    glm::quat q = tr->get_rotation();
    glm::vec3 euler = glm::eulerAngles(q);
    if (ImGui::DragFloat3("rotation", glm::value_ptr(euler), 0.01, 0.f, 0.f, "%.3f")) {
        q = glm::quat{euler};
        tr->set_rotation(q);
    }

    v = tr->get_scale();
    if (ImGui::DragFloat3("scale", glm::value_ptr(v), 0.05, 0.f, 0.f, "%.3f")) {
        tr->set_scale(v);
    }

}


} // namespace ev2