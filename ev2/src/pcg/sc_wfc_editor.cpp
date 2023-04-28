#include "sc_wfc_editor.hpp"

#include <filesystem>

#include "distributions.hpp"
#include "../ui/imgui.hpp"
#include "../resource.hpp"

namespace fs = std::filesystem;

namespace ev2::pcg {

void SCWFCNodeEditor::show_editor(Node* node) {
    SCWFC* n = dynamic_cast<SCWFC*>(node);
    if (n) {

    }
}

struct SCWFCEditor::Data {
    wfc::WFCSolver solver;
};

SCWFCEditor::SCWFCEditor()
    : m_internal{std::make_shared<SCWFCEditor::Data>()} {}

void SCWFCEditor::show_editor_tool() {

    if (m_db_editor_open) show_db_editor_window(&m_db_editor_open);

    ImGui::Begin("SCWFCEditor", &m_is_open);

    if (m_scwfc_node) {
        ImGui::Text("Selected %s", m_scwfc_node->name.c_str());
    } else {
        ImGui::Text("Please select a SCWFC node");
    }

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Object DataBase")) {
        if (ImGui::Button("LoadDB")) {
            load_obj_db();
        }
        if (ImGui::Button("SaveDB")) {
            save_obj_db("asset/pcg/object_db.json");
        }
        if (ImGui::Button("Open DB Editor")) {
            m_db_editor_open = true;
        }
    }

    ImGui::BeginDisabled(m_scwfc_node == nullptr);
    if (ImGui::CollapsingHeader("Solver")) {
        static int sc_steps = 1;
        static int sc_brf = 1;
        static float sc_mass = 1;
        ImGui::InputInt("N Nodes", &sc_steps);
        ImGui::InputInt("Branching", &sc_brf);
        ImGui::SliderFloat("Mass", &sc_mass, 0.01f, 1.f);
        if (ImGui::Button("Spawn")) {
            sc_propagate_from(dynamic_cast<SCWFCGraphNode*>(m_editor->get_selected_node()), sc_steps, sc_brf, sc_mass);
        }
        if (ImGui::Button("Reset")) {
            m_scwfc_node->reset();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Remove all SCWFC nodes in scene");
        }

        ImGui::Separator();

        static int steps = 1;
        ImGui::InputInt("Steps", &steps);
        ImGui::SameLine();
        ImGui::BeginDisabled(m_internal->solver.next_node == nullptr);
        if (ImGui::Button("Solve")) {
            wfc_solve(steps);
        }
        ImGui::EndDisabled();

        auto selected_node = dynamic_cast<wfc::DGraphNode*>(m_editor->get_selected_node());
        ImGui::BeginDisabled(selected_node != nullptr);
        if (ImGui::Button("Set node as solver start")) {
            m_internal->solver.next_node = dynamic_cast<wfc::DGraphNode*>(selected_node);
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();

    ImGui::End();
}

void SCWFCEditor::db_editor_show_pattern_editor_widget() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        for (auto& p : obj_db->patterns) {
            const std::string pattern_name = obj_db->object_class_name(p.cell_value.val);
            // need to push id to differentiate between different selections
            ImGui::PushID(&p);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            bool node_open = ImGui::TreeNode("Pattern", "Pattern \"%s\"", pattern_name.c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            static PatternProperties prop{};
            if (ImGui::Button("Edit")) {
                ImGui::OpenPopup("Edit Pattern");
                prop = {
                    .pattern = &p,
                    .name = pattern_name,
                    .weight = p.weight
                };
            }

            show_pattern_property_editor_popup(prop);

            if (node_open) {
                for (std::size_t rv_i = 0; rv_i < p.required_values.size(); rv_i++) {
                    const std::string req_pattern_name = obj_db->object_class_name(p.required_values[rv_i].val);

                    ImGui::PushID(rv_i);
                    // ImGui::SetNextItemWidth(-FLT_MIN);
                    
                
                    if (ImGui::BeginPopupContextItem("pattern popup")) {
                        ImGui::Text("\"%s\"!", req_pattern_name.c_str());
                        if (ImGui::Button("Remove")) {

                        }
                        ImGui::EndPopup();
                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
                    ImGui::TreeNodeEx("Required", flags, "Requires \"%s\"", req_pattern_name.c_str());
                    ImGui::OpenPopupOnItemClick("pattern popup", ImGuiPopupFlags_MouseButtonRight);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", obj_db->object_class_name(p.required_values[rv_i].val).c_str());
                    ImGui::NextColumn();

                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

void SCWFCEditor::show_pattern_property_editor_popup(SCWFCEditor::PatternProperties& prop) {
    if (prop.pattern == nullptr)
        return;
    if (ImGui::BeginPopupModal("Edit Pattern", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Edit Pattern Properties");
        ImGui::Separator();

        char buf[64];
        sprintf(buf, "%s", prop.name.c_str());
        if (ImGui::InputText("Object Class Name", buf, IM_ARRAYSIZE(buf))) {
            prop.name = buf;
        }

        ImGui::InputFloat("Weight", &prop.weight);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Relative weight of this class for random selection during WFC node collapse step\n");
        }

        // static bool dont_ask_me_next_time = false;
        // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        // ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
        // ImGui::PopStyleVar();

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            obj_db->set_object_class_name(buf, prop.pattern->cell_value.val);
            prop.pattern->weight = std::max(prop.weight, 0.f);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SCWFCEditor::show_db_editor_window(bool* p_open) {
    if (ImGui::Begin("Object DB Editor", p_open)) {

        if (ImGui::TreeNode("Patterns")) {
            db_editor_show_pattern_editor_widget();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Objects")) {
            
            ImGui::TreePop();
        }
        ImGui::End();
    }
}

void SCWFCEditor::load_obj_db() {
    auto cube0 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);

    auto cube1 = ResourceManager::get_singleton().get_model(fs::path("models") / "Wagon.obj", false);
    // cube1->materials[0]->diffuse = glm::vec3{1};

    auto cube2 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube2->materials[0]->diffuse = glm::vec3{1, 0, 0};

    obj_db = std::make_shared<ObjectMetadataDB>();

    obj_db->add_model(cube0, 10);
    obj_db->add_model(cube1, 11);
    obj_db->add_model(cube2, -1); // required to represent unsolved domain

    wfc::Pattern PA{wfc::Value{10}, {wfc::Value{11}, wfc::Value{11}}};
    wfc::Pattern PB{wfc::Value{11}, {wfc::Value{10}}};

    obj_db->set_object_class_name("Class 10", 10);
    obj_db->set_object_class_name("Class 11", 11);

    obj_db->patterns = {PA, PB};
}

void SCWFCEditor::save_obj_db(std::string_view path) {
    if (obj_db)
        obj_db->write_database(path);
}

void SCWFCEditor::on_selected_node(Node* node) {
    if (node) {
        Ref<SCWFC> n = node->get_ref<SCWFC>();
        if (n) {
            m_scwfc_node = n;
        }
    }
}

void SCWFCEditor::sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass) {
    if (n <= 0)
        return;
    const float radius = 2.f;
    const float n_radius = 8.f;
    if (m_scwfc_node && obj_db) {
        Ref<SCWFCGraphNode> nnode{};
        for (int i = 0; i < n; ++i) {
            // pick a random spawn location
            const glm::vec2 pos = uniform_disk(uniform2d());
            glm::vec3 offset = glm::vec3{pos.x, 0, pos.y};

            // if spawning on an existing node
            if (node) {
                Sphere sph({}, n_radius);
                sph.center = node->get_position() + offset;
                
                offset += mass * m_scwfc_node->sphere_repulsion(sph);
                offset += node->get_position();

                offset.y = 0;
            }

            nnode = m_scwfc_node->create_child_node<SCWFCGraphNode>("SGN " + std::to_string(m_scwfc_node->get_n_children()), m_scwfc_node.get());
            nnode->set_radius(radius);
            nnode->set_scale(glm::vec3{radius});
            nnode->set_model(obj_db->get_model_for_id(-1));
            nnode->set_position(offset);

            // populate domain of new node 
            std::vector<const wfc::Pattern*> dest(obj_db->patterns.size());
            std::transform(obj_db->patterns.begin(), obj_db->patterns.end(), dest.begin(),
                [](auto &elem){ return &elem; }
            );
            nnode->domain = dest;

            // attach new node to all nearby neighbors
            m_scwfc_node->update_all_adjacencies(nnode, n_radius);

            if (i % brf == 0)
                node = nnode.get();
        }
        m_editor->set_selected_node(nnode.get());
    }
}

void SCWFCEditor::wfc_solve(int steps) {
    if (m_scwfc_node && m_internal) {
        m_internal->solver.graph = m_scwfc_node->get_graph();
        int cnt = 0;

        auto c_callback = [this](wfc::DGraphNode* node) -> void {
            auto* s_node = dynamic_cast<SCWFCGraphNode*>(node);
            assert(s_node);

            if (m_scwfc_node->intersects_any_solved_neighbor(Ref{ s_node }) ||
                node->domain.size() == 0) {

                s_node->destroy();
            } else {
                auto model = obj_db->get_model_for_id(node->domain[0]->cell_value.val);
                auto& aabb = model->bounding_box;
                const glm::vec3 scale = glm::vec3{ 2 * s_node->get_bounding_sphere().radius } / glm::length(aabb.diagonal()); // scale uniformly

                s_node->set_model(model);
                s_node->set_scale(scale);
            }
        };
        
        while(m_internal->solver.can_continue() && cnt++ < steps) {
            auto solved_node = m_internal->solver.step_wfc();
            c_callback(solved_node);
        }
    }
}

}