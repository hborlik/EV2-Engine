#include <ui/ui.hpp>
#include <ui/imgui.h>

namespace ev2 {

void SceneEditor::show_scene_explorer(Scene* scene) {
    if (!ImGui::CollapsingHeader("Scene") || scene == nullptr)
        return;

    show_scene_node_widget(0, (Node*)scene);
}

void SceneEditor::show_scene_node_widget(int id, Node* node) {
    if (!node)
        return;
    const std::string node_name = std::to_string(id) + " " + node->name;
    if (!ImGui::TreeNode(node_name.c_str()))
        return;
    
    show_transform_editor(&node->transform);

    int i = 0;
    for (auto& node : node->get_children()) {
        show_scene_node_widget(i, node.get());
        i++;
    }

    ImGui::TreePop();
}

void SceneEditor::show_transform_editor(Transform* tr) {

    glm::vec3 v = tr->get_position();
    if (ImGui::InputFloat3("position", glm::value_ptr(v), "%.3f")) {
        tr->set_position(v);
    }

    glm::quat q = tr->get_rotation();
    if (ImGui::InputFloat4("rotation", glm::value_ptr(q), "%.3f")) {
        tr->set_rotation(q);
    }

    v = tr->get_scale();
    if (ImGui::InputFloat3("scale", glm::value_ptr(v), "%.3f")) {
        tr->set_scale(v);
    }

}


} // namespace ev2