#include "sc_wfc_editor.hpp"

#include <cfloat>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <random>
#include <unordered_set>

#include "distributions.hpp"
#include "geometry.hpp"
#include "glm/common.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "pcg/sc_wfc.hpp"
#include "pcg/sc_wfc_solver.hpp"
#include "ui/imgui.hpp"
#include "ui/imgui_internal.hpp"
#include "ui/imgui_stdlib.h"
#include "ui/ImGuizmo-1.83/ImGuizmo.h"
#include "resource.hpp"
#include "pcg/object_database.hpp"
#include "pcg/wfc.hpp"

namespace fs = std::filesystem;

namespace ev2::pcg {

void SCWFCGraphNodeEditor::show_editor(Node* node) {
    SCWFCGraphNode* n = dynamic_cast<SCWFCGraphNode*>(node);
    auto* obj_db = m_scwfc_editor->get_object_db();
    if (n && obj_db) {
        ImGui::Text("Radius: %f", n->get_radius());
        ImGui::Separator();

        ImGui::Text("Neighborhood Radius: %f", n->get_neighborhood_radius());
        ImGui::Separator();

        ImVec4 color = n->is_finalized() ? ImVec4(0, 255, 0, 1) : ImVec4(255, 0, 0, 1);
        constexpr const char* finalized_text[]{"Not ", ""};
        ImGui::TextColored(color, "%sFinalized", finalized_text[n->is_finalized()]);

        ImGui::Text("Domain:");
        ImGui::Indent(10);
        for (auto& val : n->domain) {
            // need to push id to differentiate between different selections
            ImGui::PushID(std::hash<wfc::Val>()(val));

            const wfc::Pattern* pattern = obj_db->get_pattern(val.value);\
            if (pattern) {
                const std::string pattern_name = obj_db->get_class_name(pattern->pattern_type);

                ImGui::Text("Pattern %d for class \"%s\"", val.value, pattern_name.c_str());
            } else {
                ImGui::Text("Pattern %d for class \"%s\"", val.value, "Unknown");
            }
            ImGui::PopID();
        }
        ImGui::Indent(-10);
        ImGui::Separator();

        ImGui::Text("Adjacent Nodes");
        SCWFCGraphNode* s_adjacent_hovered = nullptr;
        if (auto s_node = node->get_parent().ref_cast<SCWFC>()) {
            ImGui::Indent(10);
            for (const auto adjacent : s_node->get_graph()->adjacent_nodes(n)) {
                SCWFCGraphNode* s_adjacent = dynamic_cast<SCWFCGraphNode*>(adjacent);
                // need to push id to differentiate between different selections
                ImGui::PushID(adjacent);
                if (ImGui::Selectable(s_adjacent->name.c_str()))
                    m_editor->set_selected_node(Ref{s_adjacent});
                if (ImGui::IsItemHovered())
                    s_adjacent_hovered = s_adjacent;
                ImGui::PopID();
            }
            ImGui::Indent(-10);
        }

        // draw some lines to show adjacent nodes
        ImDrawList* background = ImGui::GetBackgroundDrawList();
        if (auto parent_node = node->get_parent().ref_cast<SCWFC>()) {
            for (const auto adjacent : parent_node->get_graph()->adjacent_nodes(n)) {
                SCWFCGraphNode* s_adjacent = dynamic_cast<SCWFCGraphNode*>(adjacent);

                glm::mat4 mat = m_editor->current_camera()->get_projection() *
                                m_editor->current_camera()->get_view();
                
                glm::vec2 p0 = m_editor->to_screen_point(mat, node->get_world_position());
                glm::vec2 p1 = m_editor->to_screen_point(mat, s_adjacent->get_world_position());
                
                uint32_t color = (s_adjacent_hovered == s_adjacent)
                                     ? IM_COL32(0xAF, 0xBF, 0xBF, 0xFF)
                                     : IM_COL32(0x50, 0x40, 0x40, 0xFF);
                background->AddLine(ImVec2{p0.x, p0.y}, ImVec2{p1.x, p1.y},
                                    color, 2.f);
            }
        }
    }
}

SCWFCEditor::SCWFCEditor(Application* app) : app{app}, m_rd{}, m_obj_db{} {
    m_unsolved_drawable = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    m_unsolved_drawable->materials[0]->diffuse = glm::vec3{1, 0, 0};
}

void SCWFCEditor::show_editor_tool() {

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("SCWFC DB Editor")) m_db_editor_open = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_db_editor_open) show_db_editor_window(&m_db_editor_open);

    // begin displaying our main tool window
    if (!m_is_open)
        return;

    if (!ImGui::Begin("SCWFCEditor", &m_is_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Open DB Editor")) {
        m_db_editor_open = true;
    }

    ImGui::Spacing();
    
    if (m_scwfc_node) {
        ImGui::Text("Selected %s", m_scwfc_node->name.c_str());
    } else {
        ImGui::Text("Please select a SCWFC node");
    }

    ImGui::BeginDisabled(m_scwfc_node == nullptr);
    if (ImGui::CollapsingHeader("Solver")) {

        if (m_scwfc_solver == nullptr)
            reset_solver();

        ImGui::Text("Solver Settings");

        constexpr const char* DomainMode[] = {"Full", "Dependent"};
        if (ImGui::Combo("New Node Domain Mode", (int*)&m_solver_args.domain_mode, DomainMode, IM_ARRAYSIZE(DomainMode))) {
            reset_solver();
        }

        constexpr const char* DiscoveryMode[] = {"EntropyOrder", "DiscoveryOrder"};
        if (ImGui::Combo("Solving Order", (int*)&m_solver_args.solving_order, DiscoveryMode, IM_ARRAYSIZE(DiscoveryMode))) {
            reset_solver();
        }

        constexpr const char* ValidityModes[] = {"Correct", "Approximate"};
        if (ImGui::Combo("Solver Validity Mode", (int*)&m_solver_args.validity_mode, ValidityModes, IM_ARRAYSIZE(ValidityModes))) {
            reset_solver();
        }

        constexpr const char* NeighborhoodRadiusModes[] = {"Never", "Always"};
        if (ImGui::Combo("Refresh Neighborhood Radius", (int*)&m_solver_args.node_neighborhood, NeighborhoodRadiusModes, IM_ARRAYSIZE(NeighborhoodRadiusModes))) {
            reset_solver();
        }

        if (ImGui::Checkbox("Allow Revisiting", &m_solver_args.allow_revisit_node)) {
            reset_solver();
        }

        ImGui::Separator();

        struct SolverWork {
            int in_progress = 0;
            int total = 0;

            bool check(int i) noexcept {
                if (in_progress < total) {
                    in_progress++;
                    return true;
                }
                return false;
            }

            float progress() noexcept {
                return in_progress / (float)total;
            }
        };

        ImGui::Text("SC propagate");
        static int sc_steps = 500;
        static int sc_brf = 20;
        static float sc_mass = 0.2f;
        static SolverWork sc_work{};
        ImGui::InputInt("N Nodes", &sc_steps);
        ImGui::InputInt("Branching", &sc_brf);
        ImGui::SliderFloat("Repulsion", &sc_mass, 0.0f, 5.f);
        ImGui::Checkbox("Place on terrain", &m_scwfc_solver->b_on_terrain);

        ImGui::BeginDisabled(m_scwfc_solver == nullptr);
        if (ImGui::Button("Propagate")) {
            sc_work.total = sc_steps;
            sc_work.in_progress = 0;
        }
        if (sc_work.check(1)) {
            m_scwfc_solver->sc_propagate(1, sc_brf, sc_mass);
            ImGui::ProgressBar(sc_work.progress(), ImVec2(-100, 0));
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                sc_work = {};
            }
        }

        if (ImGui::Button("Propagate From Selected")) {
            auto selected_node = dynamic_cast<SCWFCGraphNode*>(m_editor->get_selected_node().get());
            auto nnode = m_scwfc_solver->sc_propagate_from(selected_node, sc_steps, sc_mass);
            if (nnode.size() > 0)
                m_editor->set_selected_node(nnode.at(0));
        }
        ImGui::Separator();
        if (ImGui::Button("Unsolved Node")) {
            auto nnode = m_scwfc_solver->spawn_unsolved_node();
            m_editor->set_selected_node(nnode);
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::Text("WFC solver");

        static int solver_steps = 100;
        static SolverWork solver_work{};
        ImGui::InputInt("Steps", &solver_steps);
        ImGui::SameLine();
        auto selected_node = m_editor->get_selected_node().ref_cast<SCWFCGraphNode>();
        ImGui::BeginDisabled(m_scwfc_solver == nullptr || (selected_node == nullptr && !m_scwfc_solver->can_continue()));
        if (ImGui::Button("Solve")) {
            if (m_scwfc_solver && !m_scwfc_solver->can_continue())
                m_scwfc_solver->set_seed_node(selected_node);

            solver_work.total = sc_steps;
            solver_work.in_progress = 0;
        }
        ImGui::EndDisabled();
        if (solver_work.check(1)) {
            if (m_scwfc_solver) m_scwfc_solver->wfc_solve(1);
            ImGui::ProgressBar(solver_work.progress(), ImVec2(-100, 0));
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || (m_scwfc_solver && !m_scwfc_solver->can_continue())) {
                solver_work = {};
            }
        }
        if (m_scwfc_solver) {
            ImGui::Text("%lu boundary nodes", m_scwfc_solver->get_boundary_size());
            ImGui::Text("%lu discovered nodes", m_scwfc_solver->get_discovered_size());
        }
    }

    if (ImGui::Button("Reevaluate discovered nodes")) {
        if (m_scwfc_solver)
            m_scwfc_solver->reevaluate_validity();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("remove any nodes that do not have their requirements satisfied");
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::BeginDisabled(m_scwfc_solver == nullptr);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
    if (ImGui::Button("Reset")) {
        m_scwfc_node->reset();
        reset_solver();
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Remove all children nodes, resetting the entire generated scene");
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Unsolved")) {
        m_scwfc_node->remove_all_unsolved();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Remove all unsolved children nodes");
    }
    ImGui::EndDisabled();

    ImGui::End();
}

void SCWFCEditor::db_editor_show_pattern_editor_widget() {
    if (!m_obj_db)
        return;
    
    static PatternProperties new_prop{};
    if(ImGui::Button("New Pattern")) {
        ImGui::OpenPopup("New Pattern");
        new_prop = {}; // reset to default
    }
    if (show_dbe_edit_pattern_popup("New Pattern", new_prop)) {
        m_obj_db->add_pattern(wfc::Pattern{
            new_prop.pattern_class,
            {},
            new_prop.weight
        }, m_obj_db->create_pattern_id());
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        auto [p_itr, p_end] = m_obj_db->get_patterns_iterator();
        while (p_itr != p_end) {
            auto& [pattern_id, pattern] = *p_itr;
            const std::string pattern_name = m_obj_db->get_class_name(pattern.pattern_type);
            // need to push id to differentiate between different selections
            ImGui::PushID(&pattern);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            if (ImGui::IsPopupOpen("Add Requirement")) ImGui::SetNextItemOpen(true);
            bool node_open = ImGui::TreeNode("Pattern", "Pattern %d \"%s\"", pattern_id, pattern_name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            static PatternProperties prop{};
            if (ImGui::Button("Edit")) {
                ImGui::OpenPopup("Edit Pattern");
                prop = {
                    .pattern_class = pattern.pattern_type,
                    .weight = pattern.weight
                };
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Edit Pattern \"%s\"", pattern_name.c_str());
            }
            if (show_dbe_edit_pattern_popup("Edit Pattern", prop)) { // true if saved
                m_obj_db->pattern_change_class(p_itr, prop.pattern_class);
                m_obj_db->pattern_set_weight(p_itr, std::max(prop.weight, 0.f));
            }

            ImGui::SameLine();
            static int item_current_idx = -1;
            if (ImGui::Button("+")) {
                ImGui::OpenPopup("Add Requirement");
                item_current_idx = -1;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add Adjacency Requirement to \"%s\"", pattern_name.c_str());
            }
            int s_out{};
            if (show_class_select_popup("Add Requirement", item_current_idx, s_out, false)) {
                m_obj_db->pattern_add_requirement(p_itr, s_out);
            }

            ImGui::SameLine();
            bool delete_pattern = false;
            if (ImGui::Button("Delete")) {
                delete_pattern = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete Pattern \"%s\"", pattern_name.c_str());
            }

            if (node_open) {
                for (std::size_t rv_i = 0; rv_i < pattern.required_types.size(); rv_i++) {
                    const std::string req_pattern_name = m_obj_db->get_class_name(pattern.required_types[rv_i]);

                    ImGui::PushID(rv_i);
                    // ImGui::SetNextItemWidth(-FLT_MIN);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
                    ImGui::TreeNodeEx("Required", flags, "Requires \"%s\"", req_pattern_name.c_str());
                    if (ImGui::BeginPopupContextItem()) {
                        ImGui::Text("\"%s\"!", req_pattern_name.c_str());
                        if (ImGui::Button("Remove Requirement")) {
                            m_obj_db->pattern_erase_requirement(p_itr, pattern.required_types.begin() + rv_i);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", m_obj_db->get_class_name(pattern.required_types[rv_i]).c_str());
                    ImGui::NextColumn();

                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
            if (delete_pattern)
                p_itr = m_obj_db->pattern_erase(p_itr);
            else
                ++p_itr;
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

void SCWFCEditor::db_editor_show_object_class_editor_widget() {
    if (!m_obj_db)
        return;
    
    static ObjectClassProperties new_obj_class{};
    if(ImGui::Button("New Object Class")) {
        ImGui::OpenPopup("New Object Class");
        new_obj_class = {}; // reset to default
    }
    if (show_dbe_edit_object_class_popup("New Object Class", new_obj_class)) {
        int new_id = m_obj_db->create_class_id();
        m_obj_db->set_class_name(new_obj_class.name, new_id);
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        for (auto& [object_class_id, object_class_name] : m_obj_db->get_class_names()) {
            // need to push id to differentiate between different selections
            ImGui::PushID(object_class_id);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            // if (ImGui::IsPopupOpen("Add Requirement")) ImGui::SetNextItemOpen(true);
            bool node_open = ImGui::TreeNode("Object Class", "Object Class \"%s\" (%d)", object_class_name.c_str(), object_class_id);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            static ObjectClassProperties obj_class{};
            if (ImGui::Button("Edit")) {
                ImGui::OpenPopup("Edit Object Class");
                obj_class = {
                    .name = object_class_name
                };
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Edit Object Class \"%s\"", object_class_name.c_str());
            }
            if (show_dbe_edit_object_class_popup("Edit Object Class", obj_class)) {
                m_obj_db->set_class_name(obj_class.name, object_class_id);
            }

            ImGui::SameLine();
            static ObjectData obj_data_temp{};
            if (ImGui::Button("Add Object Data")) {
                ImGui::OpenPopup("Add Object Data");
                obj_data_temp = {};
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add Object Data to class \"%s\"", object_class_name.c_str());
            }
            if (show_dbe_edit_object_data_popup("Add Object Data", obj_data_temp)) {
                m_obj_db->objs_add(obj_data_temp, object_class_id);
            }


            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                m_obj_db->erase_class(object_class_id);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete Object Class \"%s\"", object_class_name.c_str());
            }

            if (node_open) {
                // get all ObjectData's for the current class id 'object_class_id'
                for (auto [itr, range_end] =
                         m_obj_db->objs_for_id(object_class_id);
                     itr != range_end;) {
                    
                    auto& [id, obj_data] = *itr;
                    const std::string obj_data_name = obj_data.name;

                    ImGui::PushID(&obj_data);
                    // ImGui::SetNextItemWidth(-FLT_MIN);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();

                    bool remove = false;
                    ImGuiTreeNodeFlags flags =
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_NoTreePushOnOpen |
                        ImGuiTreeNodeFlags_Bullet;
                    ImGui::TreeNodeEx("object data", flags, "Object \"%s\"",
                                      obj_data_name.c_str());
                    if (ImGui::BeginPopupContextItem()) {
                        ImGui::Text("\"%s\"", obj_data_name.c_str());
                        static ObjectData obj_data_temp_edit{};
                        if (ImGui::Button("Edit")) {
                            ImGui::OpenPopup("Edit Object Data");
                            obj_data_temp_edit = obj_data;
                        }
                        if (show_dbe_edit_object_data_popup(
                                "Edit Object Data", obj_data_temp_edit)) {
                            obj_data = obj_data_temp_edit;
                        }
                        if (ImGui::Button("Remove")) {
                            ImGui::CloseCurrentPopup();
                            remove = true;
                        }
                        ImGui::EndPopup();
                    }

                    // advance to next item, if remove was set true, remove the previous iterator value (rm)
                    if (auto rm = itr++; remove) {
                        m_obj_db->objs_erase(rm);
                    }

                    // static ObjectData obj_data_temp{};
                    // if (open_edit_popup) {
                    //     ImGui::OpenPopup("Edit Object Data");
                    //     obj_data_temp = {};
                    // }
                    // if (show_dbe_edit_object_data_popup("Edit Object Data", obj_data_temp)) {
                    //     obj_db->objs_add(obj_data_temp, object_class_id);
                    // }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", obj_data.asset_path.c_str());
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

bool SCWFCEditor::show_dbe_edit_object_class_popup(
    std::string_view name, ObjectClassProperties& prop) {
    bool saved = false;

    if (ImGui::BeginPopupModal(name.data(), NULL,
                               ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Edit Object Class Properties");
        ImGui::Separator();

        ImGui::InputText("Class Name", &prop.name);

        ImGui::BeginDisabled(prop.name.empty());
        if (ImGui::Button("Done")) {
            saved = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    return saved;
}

bool SCWFCEditor::show_dbe_edit_pattern_popup(std::string_view name,
                                              PatternProperties& prop) {
    bool saved = false;

    if (ImGui::BeginPopupModal(name.data(), NULL,
                               ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Edit Pattern Properties");
        ImGui::Separator();

        static int item_current_idx = -1;
        ImGui::Text("Class \"%s\" (%d)",
                    m_obj_db->get_class_name(prop.pattern_class).c_str(),
                    prop.pattern_class);
        ImGui::SameLine();
        if (ImGui::Button("Select Class")) {
            ImGui::OpenPopup("Select Pattern Class");
            item_current_idx = -1;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Select Pattern Class");
        }
        int s_out{};
        if (show_class_select_popup("Select Pattern Class", item_current_idx,
                                    s_out, true)) {
            prop.pattern_class = s_out;
        }

        ImGui::InputFloat("Weight", &prop.weight);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Relative weight of this class for random selection during WFC "
                "node collapse step\n");
        }

        // static bool dont_ask_me_next_time = false;
        // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        // ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
        // ImGui::PopStyleVar();

        ImGui::BeginDisabled(prop.pattern_class < 0);
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            saved = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();
        ImGui::SetItemDefaultFocus();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return saved;
}

bool SCWFCEditor::show_dbe_edit_object_data_popup(std::string_view name, ObjectData& prop) {
    static ImGuizmo::MODE current_gizmo_mode{ImGuizmo::WORLD};
    static ImGuizmo::OPERATION m_current_gizmo_operation{ImGuizmo::TRANSLATE};
    static ImVec2 scrolling(0.0f, 0.0f);
    static float mouse_wheel = 10.f;
    static bool opt_enable_context_menu = true;

    static AABB model_aabb{};
    static std::array<float, 6> model_bounds; // pMin (3), pMax (3)

    // the following need to be reset when the popup is closed
    static OBB* selected_propagation_pattern = nullptr;

    ImGuiIO& io = ImGui::GetIO();

    bool saved = false;
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.6, io.DisplaySize.y * 0.6), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(name.data(), NULL,
                            //    ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("%s", name.data());
        if (!prop.name.empty()) {
            ImGui::SameLine();
            ImGui::TextWrapped("%s", prop.name.data());
        }
        ImGui::Separator();

        ImGui::InputText("Object Name", &prop.name);

        ImGui::TextWrapped("Asset Path %s", prop.asset_path.c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MAX);
        if (ImGui::Button("Select Path")) {
            ImGui::OpenPopup("Select Asset Path");
        }

        std::string spath{};
        if (m_file_dialog.show_file_dialog_modal("Select Asset Path", &spath)) {
            prop.set_asset_path(spath);
        }

        ImGui::InputFloat("Extent", &prop.extent);

        constexpr const char* AxisNames[]{"X", "Y", "Z"};
        for (int i = 0; i < 3; ++i) {
            ImGui::PushID(i);
            ImGui::Spacing();
            ImGui::Text("Axis %s", AxisNames[i]);
            ImGui::RadioButton("Free", (int*)&prop.axis_settings.v[i], (int)ObjectData::Orientation::Free); ImGui::SameLine();
            ImGui::RadioButton("Lock", (int*)&prop.axis_settings.v[i], (int)ObjectData::Orientation::Lock); ImGui::SameLine(); 
            ImGui::RadioButton("Stepped", (int*)&prop.axis_settings.v[i], (int)ObjectData::Orientation::Stepped);
            ImGui::PopID();
        }


        auto model = prop.loaded_model;
        // change model AABB points to array
        if (model) {
            model_aabb = model->bounding_box;
            for (int i = 0; i < 3; ++i) {
                model_bounds[i]     = model->bounding_box.pMin[i];
                model_bounds[i+3]   = model->bounding_box.pMax[i];
            }
        }


        // OBB editor
        ImVec2 viewport_size(0, -ImGui::GetFrameHeightWithSpacing()); // leave room for one vertical line below for Save and Cancel
        ImGui::BeginChildFrame(ImGui::GetID("OBB Editor"), viewport_size);

        // ImGui::Checkbox("Enable context menu", &opt_enable_context_menu);

        ImVec2 window_p0 = ImGui::GetCursorPos();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        // Draw border and background color
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        // draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        const float WindowWidth = (float)ImGui::GetWindowWidth();
        const float WindowHeight = (float)ImGui::GetWindowHeight();

         // build scale matrix to scale unit cube as bounding box of loaded model
         // model_scale is scaling value applied to actual model when it is spawned
         // model_cube mat is for scaling unit cube
        const glm::vec3 model_scale = glm::vec3{ prop.extent } / glm::length(model_aabb.diagonal());
        const glm::mat4 model_cube_mat = glm::scale(glm::mat4{1}, model_scale * model_aabb.diagonal());

        const glm::mat4 grid_mat = glm::translate(glm::mat4{1}, model_scale * glm::vec3{0, -model_aabb.diagonal().y/2, 0});

        const glm::vec3 camera_boom =
            glm::rotate(glm::mat4{1.f}, -10.f * scrolling.x / WindowWidth,
                        glm::vec3{0, 1, 0}) *
            (mouse_wheel * glm::vec4{1, 1, 1, 0});

        glm::mat4 view =
            glm::lookAt(camera_boom, glm::vec3{0}, glm::vec3{0, 1, 0});
        glm::mat4 projection = glm::perspective(
            glm::radians(60.f), WindowWidth / WindowHeight, 0.05f, 100.f);

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                          WindowWidth, WindowHeight);
        ImGuizmo::DrawGrid(glm::value_ptr(view), glm::value_ptr(projection),
                           glm::value_ptr(grid_mat), 10.f);

        const glm::vec3 cube_half_extents{.5f};
        std::vector<glm::mat4> cube_matrix_vec{};
        if (model) { // display cube for the model
            cube_matrix_vec.push_back(model_cube_mat);
        }
        for (auto obb_itr = prop.propagation_patterns.begin(),
                  end = prop.propagation_patterns.end();
             obb_itr != end; ++obb_itr) {
                OBB& obb = *obb_itr;

                glm::mat4 model = obb.make_transform(cube_half_extents);
                cube_matrix_vec.push_back(model);
             }

        ImGuizmo::DrawCubes(glm::value_ptr(view),
                            glm::value_ptr(projection),
                            (float*)cube_matrix_vec.data(), cube_matrix_vec.size());

        // display gizmo if there is a selected OBB
        if (selected_propagation_pattern) {
            OBB& obb = *selected_propagation_pattern;

            glm::mat4 model = obb.make_transform(cube_half_extents);
            // glm::mat4 cube_mm = model * glm::scale(glm::mat4{1}, obb.half_extents * 2.f);
            std::array<float, 6> obb_bounds; // pMin (3), pMax (3)
            for (int i = 0; i < 3; ++i) {
                obb_bounds[i]     = -cube_half_extents[i];//-obb.half_extents[i];
                obb_bounds[i+3]   = cube_half_extents[i];//obb.half_extents[i];
            }

            ImGuizmo::Manipulate(glm::value_ptr(view),
                                 glm::value_ptr(projection),
                                 m_current_gizmo_operation, current_gizmo_mode,
                                 glm::value_ptr(model), NULL, NULL, obb_bounds.data(), NULL);
            
            if (ImGuizmo::IsUsing())
                obb = OBB{model, cube_half_extents};
        }


        if (!ImGuizmo::IsOver()) {
            // This will catch our interactions
            ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonRight);
            const bool is_hovered = ImGui::IsItemHovered(); // Hovered
            const bool is_active = ImGui::IsItemActive();   // Held
            const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;

                // std::cout << scrolling.x << std::endl;
            }

            ImGui::SetItemUsingMouseWheel(); // capture the mouse wheel with invisible button
            if (is_hovered) {
                float wheel = -ImGui::GetIO().MouseWheel;
                mouse_wheel = glm::clamp(mouse_wheel + wheel, 1.f, 20.f);
            }
        }

        ImGui::SetCursorPos(window_p0);

        // left side display list of all patterns for this object
        const int controls_height = ImGui::GetFrameHeightWithSpacing() * 4 + ImGui::GetFrameHeight(); // below we render a couple rows of radio buttons in a frame
        ImGui::BeginChild("left pane", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, -controls_height), true);
        ImGui::PushItemWidth(-FLT_MIN); // align to right side

        if (ImGui::Button("Add OBB"))
            prop.propagation_patterns.push_back(OBB{});

        ImGui::Separator();

        ImGui::Text("Select Propagaion OBB to edit");

        // display list of propagation patterns
        int propagation_idx = 0;
        for (auto pattern_itr = prop.propagation_patterns.begin(),
                  end = prop.propagation_patterns.end();
             pattern_itr != end; ++pattern_itr, ++propagation_idx) {

            ImGui::PushID(propagation_idx);
        
            int node_flags = 0;

            if (selected_propagation_pattern == &*pattern_itr)
                node_flags |= ImGuiTreeNodeFlags_Selected;

            auto& he = pattern_itr->half_extents;
            if (ImGui::TreeNodeEx((void*)&propagation_idx, node_flags,
                                  "(%d) [%.2f, %.2f, %.2f]", propagation_idx,
                                  he[0], he[1], he[2])) {
                ImGui::InputFloat3("Half Extents", glm::value_ptr(he));
                ImGui::InputFloat3("Position", glm::value_ptr(pattern_itr->center));
                ImGui::TreePop();
            }

            if (ImGui::IsItemClicked()) {
                selected_propagation_pattern = &*pattern_itr;
            }

            ImGui::PopID();
        }

        ImGui::PopItemWidth();
        ImGui::EndChild(); // end left pane

        ImGui::BeginChild("left pane controls", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, 0), true);

        ImGui::RadioButton("LOCAL", (int*)&current_gizmo_mode, (int)ImGuizmo::MODE::LOCAL); ImGui::SameLine();
        ImGui::RadioButton("WORLD", (int*)&current_gizmo_mode, (int)ImGuizmo::MODE::WORLD);

        if (ImGui::RadioButton("Translate",
                               m_current_gizmo_operation == ImGuizmo::TRANSLATE))
            m_current_gizmo_operation = ImGuizmo::TRANSLATE;

        // ImGui::SameLine();
        if (ImGui::RadioButton("Rotate",
                               m_current_gizmo_operation == ImGuizmo::ROTATE))
            m_current_gizmo_operation = ImGuizmo::ROTATE;


        // ImGui::SameLine();
        if (ImGui::RadioButton("Scale",
                               m_current_gizmo_operation == ImGuizmo::SCALE))
            m_current_gizmo_operation = ImGuizmo::SCALE;

        ImGui::EndChild(); // end left pane controls

        ImGui::EndChildFrame();
        // end of OBB editor

        // if (!prop.is_valid()) ImGui::PushStyleColor(ImGuiCol_Button,
        // (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
        ImGui::BeginDisabled(!prop.is_valid());
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            saved = true;
            selected_propagation_pattern = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndDisabled();
        // if (!prop.is_valid()) ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            selected_propagation_pattern = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return saved;
}

bool SCWFCEditor::show_class_select_popup(
    std::string_view popup_name,
    int& item_current_idx,
    int& selection_out,
    bool close_on_pick) {

    bool selected = false;
    
    if (ImGui::BeginPopupContextItem(popup_name.data(), ImGuiPopupFlags_MouseButtonLeft)) {
        ImGui::TextWrapped("Select Object Class");
        ImGui::Separator();

        auto class_names = m_obj_db->get_class_names();
        float child_height =
            ImGui::GetTextLineHeightWithSpacing() *
            (std::min<float>(class_names.size(), 5) + .25f);

        int idx = 0;
        if (ImGui::BeginListBox("##listbox requirements",
                                ImVec2{0, child_height})) {
            for (auto& [id, class_name] : class_names) {

                const bool is_selected = (item_current_idx == idx);
                if (ImGui::Selectable(class_name.c_str(), is_selected)) {
                    item_current_idx = idx;
                }
                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                idx++;
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Choose")) {
            if (item_current_idx >= 0 && item_current_idx < class_names.size()) {
                selection_out = class_names[item_current_idx].first;
                if (close_on_pick) ImGui::CloseCurrentPopup();
                selected = true;
            }
        }

        ImGui::EndPopup();
    }

    return selected;
}

void SCWFCEditor::show_db_editor_window(bool* p_open) {
    if (!ImGui::Begin("Object DB Editor", p_open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    enum MenuAction {
        None,
        NewDB,
        LoadDB,
        LoadDefaultDB,
        SaveDB
    };

    MenuAction menu_action = None;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("New DB"))
                menu_action = NewDB;

            if (ImGui::MenuItem("Load DB"))
                menu_action = LoadDB;

            if (ImGui::MenuItem("Load Default DB"))
                menu_action = LoadDefaultDB;

            if (ImGui::MenuItem("Save DB"))
                menu_action = SaveDB;

            // if (ImGui::MenuItem("Save DB"))

            // if (ImGui::MenuItem("Exit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    switch (menu_action) {
        case NewDB:
            new_obj_db();
            break;
        case LoadDB:
            ImGui::OpenPopup("Load DB From File");
            break;
        case SaveDB:
            ImGui::OpenPopup("Save DB to File");
            break;
        case LoadDefaultDB:
            load_default_obj_db();
            break;
        case None:
        default:
            break;
    }

    std::string spath{};
    if (m_file_dialog.show_file_dialog_modal("Load DB From File", &spath)) {
        load_obj_db(spath);
    }

    if (m_file_dialog.show_file_dialog_modal("Save DB to File", &spath)) {
        save_obj_db(spath);
    }

    if (m_obj_db) {
        if (ImGui::TreeNode("Object Classes")) {
            db_editor_show_object_class_editor_widget();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Patterns")) {
            db_editor_show_pattern_editor_widget();
            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void SCWFCEditor::load_default_obj_db() {

    wfc::Pattern PA{10, {11, 11}};
    wfc::Pattern PB{11, {10}};

    m_obj_db->set_class_name("Class 10", 10);
    m_obj_db->set_class_name("Class 11", 11);

    m_obj_db->add_pattern(PA, m_obj_db->create_pattern_id());
    m_obj_db->add_pattern(PB, m_obj_db->create_pattern_id());
}

void SCWFCEditor::load_obj_db(std::string_view path) {
    m_obj_db = ObjectMetadataDB::load_object_database(path);
}

void SCWFCEditor::new_obj_db() {
    m_obj_db = std::make_shared<ObjectMetadataDB>();
}

void SCWFCEditor::save_obj_db(std::string_view path) {
    if (m_obj_db)
        m_obj_db->write_database(path);
}

void SCWFCEditor::on_selected_node(Node* node) {
    if (node) {
        Ref<SCWFC> n = node->get_ref<SCWFC>();
        if (n) {
            m_scwfc_node = n;

            reset_solver();
        }
    }
}

void SCWFCEditor::reset_solver() {
    if (m_obj_db) {
        m_scwfc_solver = SCWFCSolver::make_solver(*m_scwfc_node, m_obj_db, m_rd, m_unsolved_drawable, m_solver_args);
        // attach notification events scene nodes being removed
        m_scwfc_solver->node_added_listener.subscribe(&m_scwfc_node->child_node_added);
        m_scwfc_solver->node_removed_listener.subscribe(&m_scwfc_node->child_node_removed);

        m_scwfc_solver->app = app;
    }
}

}