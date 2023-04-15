#include <ui/ui.hpp>
#include <ui/imgui.hpp>

#include <physics.hpp>
#include <resource.hpp>

namespace ev2 {

void show_material_editor_window(bool* p_open) {
    ImGui::Begin("Material Editor", p_open);
    for (const auto& [name, mat] : ev2::ResourceManager::get_singleton().get_materials()) {
        if (ImGui::CollapsingHeader(("Material " + mat->name + " " + name).c_str())) {
            
            if (ImGui::TreeNode("Color")) {
                ImGui::ColorPicker3("diffuse", glm::value_ptr(mat->diffuse), ImGuiColorEditFlags_InputRGB);
                ImGui::ColorPicker3("emissive", glm::value_ptr(mat->emissive), ImGuiColorEditFlags_InputRGB);
                ImGui::TreePop();
            }
            ImGui::DragFloat("metallic",    &mat->metallic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("subsurface",  &mat->subsurface, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specular",    &mat->specular, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("roughness",   &mat->roughness, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specularTint",&mat->specularTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoat",   &mat->clearcoat, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoatGloss", &mat->clearcoatGloss, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("anisotropic", &mat->anisotropic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheen",       &mat->sheen, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheenTint",   &mat->sheenTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
        }
    }
    ImGui::End();
}

void show_settings_window(bool* p_open) {
    auto& renderer = ev2::renderer::Renderer::get_singleton();
    if (ImGui::Begin("Render Settings", p_open)) {
        ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("N Lights %i", renderer.get_n_pointlights());
        float ssao_radius = renderer.get_ssao_radius();
        if (ImGui::DragFloat("SSAO radius", &(ssao_radius), 0.01f, 0.0f, 3.0f, "%.3f", 1.0f)) {
            renderer.set_ssao_radius(ssao_radius);
        }
        int ssao_samples = renderer.get_ssao_kernel_samples();
        if (ImGui::DragInt("SSAO samples", &ssao_samples, 1, 1, 64)) {
            renderer.set_ssao_kernel_samples(ssao_samples);
        }
        float ssao_bias = renderer.get_ssao_bias();
        if (ImGui::DragFloat("SSAO Bias", &(ssao_bias), 0.01f, 0.0f, 1.0f, "%.3f", 1.0f)) {
            renderer.set_ssao_bias(ssao_bias);
        }
        ImGui::DragFloat("Exposure", &(renderer.exposure), 0.01f, 0.05f, 1.0f, "%.3f", 1.0f);
        ImGui::DragFloat("Gamma", &(renderer.gamma), 0.01f, 0.8f, 2.8f, "%.1f", 1.0f);
        ImGui::DragInt("Bloom Quality", &(renderer.bloom_iterations), 1, 1, 6);
        ImGui::DragFloat("Bloom Threshold", &(renderer.bloom_threshold), 0.005f, 0.01f, 5.0f, "%.5f", 1.0f);
        ImGui::DragFloat("Bloom Falloff", &(renderer.bloom_falloff), 0.005f, 0.1f, 3.0f, "%.5f", 1.0f);
        ImGui::DragFloat("Shadow Bias World", &(renderer.shadow_bias_world), 0.005f, 0.0001f, 1.0f, "%.5f", 1.0f);
        ImGui::Checkbox("Culling Enabled", &(renderer.culling_enabled));
        ImGui::Checkbox("Pause Culling", &(renderer.pause_cull));
        ImGui::Separator();
        ImGui::Text("World");

        bool enable_physics_timestep = ev2::Physics::get_singleton().is_simulation_enabled();
        if (ImGui::Checkbox("Enable Physics Timestep", &enable_physics_timestep)) {
            ev2::Physics::get_singleton().enable_simulation(enable_physics_timestep);
        }
        ImGui::Separator();
        ImGui::DragFloat("Sky Brightness", &(renderer.sky_brightness), 0.01f, 0.01f, 2.f, "%.3f", 1.0f);
    }
    ImGui::End();
}

void SceneEditor::editor(Node* scene) {

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

void SceneEditor::show_scene_explorer(Node* scene, bool* p_open) {
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
    ImGui::Text("Node type %s", util::name_demangle(typeid(*node).name()).c_str());
    std::string uuid_text = "UUID: " + node->uuid;
    ImGui::Text("%s", uuid_text.c_str());
    // ImGui::
    ImGui::Separator();
    ImGui::Text("Transform");
    show_transform_editor(node);
    if (auto itr = m_editor_types.find(typeid(*node)); itr != m_editor_types.end()) {
        ImGui::Separator();
        itr->second->show_editor(node);
    }
}

void SceneEditor::add_custom_node_editor(std::shared_ptr<NodeEditor> editor) {
    assert(editor);
    if (m_editor_types.find(editor->get_edited_type()) == m_editor_types.end())
        m_editor_types.emplace(editor->get_edited_type(), editor);
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

    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)id, node_flags, "%s", node_name.c_str());
    
    int node_clicked = -1;
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        node_clicked = id;
    
    if (test_drag_and_drop && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("SCENE_TREENODE", &node, sizeof(node));
        ImGui::Text("%s", node_name.c_str());
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

void SceneEditor::show_transform_editor(Node* node) {
    if (!node)
        return;

    glm::vec3 v = node->get_position();
    if (ImGui::DragFloat3("position", glm::value_ptr(v), 0.1, 0.f, 0.f, "%.3f")) {
        node->set_position(v);
    }

    glm::quat q = node->get_rotation();
    glm::vec3 euler = glm::eulerAngles(q);
    if (ImGui::DragFloat3("rotation", glm::value_ptr(euler), 0.01, 0.f, 0.f, "%.3f")) {
        q = glm::quat{euler};
        node->set_rotation(q);
    }

    v = node->get_scale();
    if (ImGui::DragFloat3("scale", glm::value_ptr(v), 0.05, 0.f, 0.f, "%.3f")) {
        node->set_scale(v);
    }

}


} // namespace ev2