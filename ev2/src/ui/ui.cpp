#include <ui/ui.hpp>
#include <ui/imgui.h>

#include <physics.h>
#include <resource.h>

namespace ev2 {

void show_material_editor_window(bool* p_open) {
    ImGui::Begin("Material Editor", p_open);
    for (auto& mas : ev2::ResourceManager::get_singleton().get_materials()) {
        if (ImGui::CollapsingHeader(("Material " + mas.second->name + " " + mas.first).c_str())) {
            
            if (ImGui::TreeNode("Color")) {
                ImGui::ColorPicker3("diffuse", glm::value_ptr(mas.second->diffuse), ImGuiColorEditFlags_InputRGB);
                ImGui::ColorPicker3("emissive", glm::value_ptr(mas.second->emissive), ImGuiColorEditFlags_InputRGB);
                ImGui::TreePop();
            }
            ImGui::DragFloat("metallic",    &mas.second->metallic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("subsurface",  &mas.second->subsurface, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specular",    &mas.second->specular, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("roughness",   &mas.second->roughness, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specularTint",&mas.second->specularTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoat",   &mas.second->clearcoat, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoatGloss", &mas.second->clearcoatGloss, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("anisotropic", &mas.second->anisotropic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheen",       &mas.second->sheen, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheenTint",   &mas.second->sheenTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
        }
    }
    ImGui::End();
}

void show_settings_window(bool* p_open) {
    if (ImGui::Begin("Render Settings", p_open)) {
        ImGui::Text("N Lights %i", ev2::renderer::Renderer::get_singleton().get_n_pointlights());
        float ssao_radius = ev2::renderer::Renderer::get_singleton().get_ssao_radius();
        if (ImGui::DragFloat("SSAO radius", &(ssao_radius), 0.01f, 0.0f, 3.0f, "%.3f", 1.0f)) {
            ev2::renderer::Renderer::get_singleton().set_ssao_radius(ssao_radius);
        }
        int ssao_samples = ev2::renderer::Renderer::get_singleton().get_ssao_kernel_samples();
        if (ImGui::DragInt("SSAO samples", &ssao_samples, 1, 1, 64)) {
            ev2::renderer::Renderer::get_singleton().set_ssao_kernel_samples(ssao_samples);
        }
        float ssao_bias = ev2::renderer::Renderer::get_singleton().get_ssao_bias();
        if (ImGui::DragFloat("SSAO Bias", &(ssao_bias), 0.01f, 0.0f, 1.0f, "%.3f", 1.0f)) {
            ev2::renderer::Renderer::get_singleton().set_ssao_bias(ssao_bias);
        }
        ImGui::DragFloat("Exposure", &(ev2::renderer::Renderer::get_singleton().exposure), 0.01f, 0.05f, 1.0f, "%.3f", 1.0f);
        ImGui::DragFloat("Gamma", &(ev2::renderer::Renderer::get_singleton().gamma), 0.01f, 0.8f, 2.8f, "%.1f", 1.0f);
        ImGui::DragInt("Bloom Quality", &(ev2::renderer::Renderer::get_singleton().bloom_iterations), 1, 1, 6);
        ImGui::DragFloat("Bloom Threshold", &(ev2::renderer::Renderer::get_singleton().bloom_threshold), 0.005f, 0.01f, 5.0f, "%.5f", 1.0f);
        ImGui::DragFloat("Bloom Falloff", &(ev2::renderer::Renderer::get_singleton().bloom_falloff), 0.005f, 0.1f, 3.0f, "%.5f", 1.0f);
        ImGui::DragFloat("Shadow Bias World", &(ev2::renderer::Renderer::get_singleton().shadow_bias_world), 0.005f, 0.0001f, 1.0f, "%.5f", 1.0f);
        ImGui::Separator();
        ImGui::Text("World");

        bool enable_physics_timestep = ev2::Physics::get_singleton().is_simulation_enabled();
        if (ImGui::Checkbox("Enable Physics Timestep", &enable_physics_timestep)) {
            ev2::Physics::get_singleton().enable_simulation(enable_physics_timestep);
        }
        ImGui::Separator();
        ImGui::DragFloat("Sky Brightness", &(ev2::renderer::Renderer::get_singleton().sky_brightness), 0.01f, 0.01f, 2.f, "%.3f", 1.0f);
    }
    ImGui::End();
}

void SceneEditor::editor(Scene* scene) {

    if (m_scene_editor_open)    show_scene_explorer(scene, &m_scene_editor_open);

    if (m_show_settings)        show_settings_window(&m_show_settings);

    if (m_material_editor_open) show_material_editor_window(&m_material_editor_open);

    if (m_demo_open)            ImGui::ShowDemoWindow();

    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Editor"))
        {
            // if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Scene Tree"))  {m_scene_editor_open = !m_scene_editor_open;}
            if (ImGui::MenuItem("Material Editor"))  {m_material_editor_open = !m_material_editor_open;}
            if (ImGui::MenuItem("Settings"))    {m_show_settings = !m_show_settings;}
            if (ImGui::MenuItem("ImGui Demo"))  {m_demo_open = !m_demo_open;}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void SceneEditor::show_scene_explorer(Scene* scene, bool* p_open) {
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1),    ImVec2(FLT_MAX, -1)); 
    if (!ImGui::Begin("Scene Window", p_open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // Left side
    ImGui::BeginChild("left pane", ImVec2(250, 0), true);
    show_scene_tree_widget(0, (Node*)scene);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right Side
    if (selected_node != nullptr)
    {
        ImGui::BeginGroup();

        ImVec2 child_size = ImVec2(300, -ImGui::GetFrameHeightWithSpacing());
        ImGui::BeginChild(ImGui::GetID("cfg_infos"), child_size, ImGuiWindowFlags_NoMove);

        ImGui::Text("Node %s (%s)", selected_node->name.c_str(), selected_node->get_path().c_str());
        ImGui::Separator();

        show_node_editor_widget(selected_node);

        ImGui::EndChild();
        
        ImGui::EndGroup();
    }

    ImGui::End();
}

void SceneEditor::show_node_editor_widget(Node* node) {
    if (!node)
        return;
    std::string uuid_text = "UUID: " + node->uuid;
    ImGui::Text(uuid_text.c_str());
    ImGui::Separator();
    ImGui::Text("Transform");
    show_transform_editor(&node->transform);
}

void SceneEditor::show_scene_tree_widget(int id, Node* node) {
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
        if (is_selected)
            selected_node = nullptr;
        else
            selected_node = node;
        // if (ImGui::GetIO().KeyCtrl)
        //     selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
        // else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
        //     selection_mask = (1 << node_clicked);           // Click to single-select
    }


    if (node_open) {
        int i = 0;
        for (auto& node : node->get_children()) {
            show_scene_tree_widget(i, node.get());
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