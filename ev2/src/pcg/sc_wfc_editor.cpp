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
        ImGui::Text("Domain");
        auto p_itr = n->domain.begin();
        while (p_itr != n->domain.end()) {
            auto& p = **p_itr;
            const std::string pattern_name = obj_db->get_object_class_name(p.pattern_class);
            // need to push id to differentiate between different selections
            ImGui::PushID(&p);
            ImGui::Text("%s", pattern_name.c_str());
            ImGui::PopID();

            ++p_itr;
        }
    }
}

struct SCWFCEditor::Data {
    std::unique_ptr<wfc::WFCSolver> solver;
    std::shared_ptr<renderer::Drawable> unsolved_drawable;
};

SCWFCEditor::SCWFCEditor():
    rd{},
    mt{rd()},
    m_internal{std::make_shared<SCWFCEditor::Data>()},
    obj_db{std::make_shared<ObjectMetadataDB>()} {

    
    m_internal->unsolved_drawable = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    m_internal->unsolved_drawable->materials[0]->diffuse = glm::vec3{1, 0, 0};

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
            ImGui::SetTooltip("Remove all children nodes, resetting the entire generated scene");
        }

        ImGui::Separator();

        static int steps = 1;
        ImGui::InputInt("Steps", &steps);
        ImGui::SameLine();
        auto selected_node = dynamic_cast<wfc::DGraphNode*>(m_editor->get_selected_node());
        // ImGui::BeginDisabled(m_internal->solver->next_node == nullptr);
        ImGui::BeginDisabled(selected_node == nullptr && !m_internal->solver->can_continue());
        if (ImGui::Button("Solve")) {
            if (!m_internal->solver->can_continue())
                m_internal->solver->set_next_node(selected_node);
            wfc_solve(steps);
        }
        ImGui::EndDisabled();
    }
    ImGui::EndDisabled();

    ImGui::End();
}

void SCWFCEditor::db_editor_show_pattern_editor_widget() {
    if (!obj_db)
        return;
    
    static PatternProperties new_prop{};
    if(ImGui::Button("New Pattern")) {
        ImGui::OpenPopup("New Pattern");
        new_prop = {}; // reset to default
    }
    if (show_dbe_edit_pattern_popup("New Pattern", new_prop)) {
        obj_db->add_pattern(wfc::Pattern{
            new_prop.pattern_class,
            {},
            new_prop.weight
        });
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        auto [p_itr, p_end] = obj_db->get_patterns();
        while (p_itr != p_end) {
            auto& p = *p_itr;
            const std::string pattern_name = obj_db->get_object_class_name(p.pattern_class);
            // need to push id to differentiate between different selections
            ImGui::PushID(&p);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            if (ImGui::IsPopupOpen("Add Requirement")) ImGui::SetNextItemOpen(true);
            bool node_open = ImGui::TreeNode("Pattern", "Pattern \"%s\"", pattern_name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            static PatternProperties prop{};
            if (ImGui::Button("Edit")) {
                ImGui::OpenPopup("Edit Pattern");
                prop = {
                    .pattern_class = p.pattern_class,
                    .weight = p.weight
                };
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Edit Pattern \"%s\"", pattern_name.c_str());
            }
            if (show_dbe_edit_pattern_popup("Edit Pattern", prop)) { // true if saved
                obj_db->pattern_change_class(p_itr, prop.pattern_class);
                obj_db->pattern_set_weight(p_itr, std::max(prop.weight, 0.f));
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
                obj_db->pattern_add_requirement(p_itr, s_out);
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                p_itr = obj_db->pattern_erase(p_itr);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete Pattern \"%s\"", pattern_name.c_str());
            }

            if (node_open) {
                for (std::size_t rv_i = 0; rv_i < p.required_classes.size(); rv_i++) {
                    const std::string req_pattern_name = obj_db->get_object_class_name(p.required_classes[rv_i]);

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
                            obj_db->pattern_erase_requirement(p_itr, p.required_classes.begin() + rv_i);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", obj_db->get_object_class_name(p.required_classes[rv_i]).c_str());
                    ImGui::NextColumn();

                    ImGui::PopID();
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
            ++p_itr;
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

void SCWFCEditor::db_editor_show_object_class_editor_widget() {
    if (!obj_db)
        return;
    
    static ObjectClassProperties new_obj_class{};
    if(ImGui::Button("New Object Class")) {
        ImGui::OpenPopup("New Object Class");
        new_obj_class = {}; // reset to default
    }
    if (show_dbe_edit_object_class_popup("New Object Class", new_obj_class)) {
        int new_id = obj_db->object_class_create_id();
        obj_db->set_object_class_name(new_obj_class.name, new_id);
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        for (auto& [object_class_id, object_class_name] : obj_db->get_classes()) {
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
                obj_db->set_object_class_name(obj_class.name, object_class_id);
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
                obj_db->objs_add(obj_data_temp, object_class_id);
            }


            // ImGui::SameLine();
            // if (ImGui::Button("Delete")) {
            //     obj_db->patterns.erase(p_itr++);
            // }
            // if (ImGui::IsItemHovered()) {
            //     ImGui::SetTooltip("Delete Object Class \"%s\"", object_class_name.c_str());
            // }

            if (node_open) {
                // get all ObjectData's for the current class id 'object_class_id'
                for (auto [itr, range_end] =
                         obj_db->objs_for_id(object_class_id);
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
                        obj_db->objs_erase(rm);
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
                    obj_db->get_object_class_name(prop.pattern_class).c_str(),
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
    static OBB* selected_propagation_pattern = nullptr;
    static bool model_valid = false;
    static bool model_load_failed = false;
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
            prop.asset_path = spath;

            // loaded_model = ResourceManager::get_singleton().get_model_relative_path(spath);
            model_valid = false;
            model_load_failed = false;
        }

        ImGui::InputFloat("Extent", &prop.extent);

        static AABB model_aabb{};
        static std::array<float, 6> model_bounds; // pMin (3), pMax (3)
        if (!model_valid && !model_load_failed) {
            auto model = ResourceManager::get_singleton().get_model_relative_path(prop.asset_path);
            model_valid = (bool)model;
            model_load_failed = !model_valid;
            // change model AABB points to array
            if (model) {
                model_aabb = model->bounding_box;
                for (int i = 0; i < 3; ++i) {
                    model_bounds[i]     = model->bounding_box.pMin[i];
                    model_bounds[i+3]   = model->bounding_box.pMax[i];
                }
            }
        }


        // OBB editor
        ImVec2 viewport_size(0, -ImGui::GetFrameHeightWithSpacing()); // leave room for one vertical line below for Save and Cancel
        ImGui::BeginChildFrame(ImGui::GetID("OBB Editor"), viewport_size);

        static ImVec2 scrolling(0.0f, 0.0f);
        static float mouse_wheel = 10.f;
        static bool opt_enable_context_menu = true;

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
                           glm::value_ptr(glm::mat4{1}), 10.f);

        std::vector<glm::mat4> cube_matrix_vec{};
        if (model_valid) { // display cube for the model
            cube_matrix_vec.push_back(model_cube_mat);
        }
        for (auto obb_itr = prop.propagation_patterns.begin(),
                  end = prop.propagation_patterns.end();
             obb_itr != end; ++obb_itr) {
                OBB& obb = *obb_itr;

                glm::mat4 model = obb.make_transform(glm::vec3{1.f});
                cube_matrix_vec.push_back(model);
             }

        ImGuizmo::DrawCubes(glm::value_ptr(view),
                            glm::value_ptr(projection),
                            (float*)cube_matrix_vec.data(), cube_matrix_vec.size());

        // display gizmo if there is a selected OBB
        if (selected_propagation_pattern) {
            OBB& obb = *selected_propagation_pattern;

            glm::mat4 model = obb.make_transform(glm::vec3{1.f});
            // glm::mat4 cube_mm = model * glm::scale(glm::mat4{1}, obb.half_extents * 2.f);
            std::array<float, 6> obb_bounds; // pMin (3), pMax (3)
            for (int i = 0; i < 3; ++i) {
                obb_bounds[i]     = -.5;//-obb.half_extents[i];
                obb_bounds[i+3]   = .5;//obb.half_extents[i];
            }

            ImGuizmo::Manipulate(glm::value_ptr(view),
                                 glm::value_ptr(projection),
                                 m_current_gizmo_operation, current_gizmo_mode,
                                 glm::value_ptr(model), NULL, NULL, obb_bounds.data(), NULL);
            
            if (ImGuizmo::IsUsing())
                obb = OBB{model, glm::vec3{1}};
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
        const int controls_height = ImGui::GetFrameHeightWithSpacing() * 2 + ImGui::GetFrameHeight(); // below we render a couple rows of radio buttons in a frame
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
             pattern_itr != end; ++pattern_itr) {
        
            int node_flags = ImGuiTreeNodeFlags_Leaf;

            if (selected_propagation_pattern == &*pattern_itr)
                node_flags |= ImGuiTreeNodeFlags_Selected;

            auto& he = pattern_itr->half_extents;
            if (ImGui::TreeNodeEx("obb tree node", node_flags,
                                  "(%d) [%.2f, %.2f, %.2f]", propagation_idx,
                                  he[0], he[1], he[2]))
                ImGui::TreePop();

            if (ImGui::IsItemClicked()) {
                selected_propagation_pattern = &*pattern_itr;
            }
        }

        ImGui::PopItemWidth();
        ImGui::EndChild(); // end left pane

        ImGui::BeginChild("left pane controls", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, 0), true);

        ImGui::RadioButton("LOCAL", (int*)&current_gizmo_mode, (int)ImGuizmo::MODE::LOCAL); ImGui::SameLine();
        ImGui::RadioButton("WORLD", (int*)&current_gizmo_mode, (int)ImGuizmo::MODE::WORLD);

        if (ImGui::RadioButton("Translate",
                               m_current_gizmo_operation == ImGuizmo::TRANSLATE))
            m_current_gizmo_operation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate",
                               m_current_gizmo_operation == ImGuizmo::ROTATE))
            m_current_gizmo_operation = ImGuizmo::ROTATE;
        ImGui::SameLine();
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
            model_valid = false;
            model_load_failed = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndDisabled();
        // if (!prop.is_valid()) ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            model_valid = false;
            model_load_failed = false;
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

        auto class_names = obj_db->get_classes();
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
        LoadDB,
        LoadDefaultDB,
        SaveDB
    };

    MenuAction menu_action = None;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {

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

    if (obj_db) {
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

    obj_db->set_object_class_name("Class 10", 10);
    obj_db->set_object_class_name("Class 11", 11);

    obj_db->add_pattern(PA);
    obj_db->add_pattern(PB);
}

void SCWFCEditor::load_obj_db(std::string_view path) {
    obj_db = ObjectMetadataDB::load_object_database(path);
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
            m_internal->solver = std::make_unique<wfc::WFCSolver>(m_scwfc_node->get_graph(), &mt);
        }
    }
}

void SCWFCEditor::sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass) {
    if (!(m_scwfc_node || obj_db))
        return;

    if (n <= 0)
        return;
    
    const float radius = 2.f;
    const float n_radius = 8.f;
    Ref<SCWFCGraphNode> nnode{};
    for (int i = 0; i < n; ++i) {
        // pick a random spawn location
        const glm::vec2 pos = uniform_disk(uniform2d());
        glm::vec3 offset = glm::vec3{pos.x, 0, pos.y};

        // set of valid neighbors for the propagating node
        // will be set of all possible patterns when node is null
        std::unordered_set<const wfc::Pattern*> valid_neighbors{};

        // if spawning on an existing node
        if (node) {
            Sphere sph({}, n_radius);
            sph.center = node->get_position() + offset;
            
            offset += mass * m_scwfc_node->sphere_repulsion(sph);
            offset += node->get_position();

            offset.y = 0;

            // since we are propagating from an existing node, spawn a
            // node that contains set of valid neighbors for that existing
            // node.
            for (auto p : node->domain) {
                std::for_each(
                    p->required_classes.begin(), p->required_classes.end(),
                    [&valid_neighbors, this](auto& value)->void {
                        auto [p_b, p_e] = obj_db->patterns_for_id(value);
                        // insert the entire returned range into our set of valid patterns
                        for (; p_b != p_e; ++p_b)
                            valid_neighbors.insert((const wfc::Pattern*)p_b->second);
                    });
            }

        } else { // populate domain with all available patterns
            auto [p_itr, p_end] = obj_db->get_patterns();
            std::vector<const wfc::Pattern*> dest(obj_db->patterns_size());
            std::transform(p_itr, p_end, dest.begin(),
                [](auto &elem){ return &elem; }
            );
            valid_neighbors = {dest.begin(), dest.end()};
        }

        nnode = m_scwfc_node->create_child_node<SCWFCGraphNode>("SGN " + std::to_string(m_scwfc_node->get_n_children()), m_scwfc_node.get());
        nnode->set_radius(radius);
        nnode->set_scale(glm::vec3{radius});
        nnode->set_model(m_internal->unsolved_drawable);
        nnode->set_position(offset);

        // populate domain of new node
        nnode->domain = {valid_neighbors.begin(), valid_neighbors.end()};

        // attach new node to all nearby neighbors
        m_scwfc_node->update_all_adjacencies(nnode, n_radius);

        if (i % brf == 0)
            node = nnode.get();
    }
    m_editor->set_selected_node(nnode.get());
}

void SCWFCEditor::wfc_solve(int steps) {
    if (!(m_scwfc_node || m_internal))
        return;

    int cnt = 0;

    auto c_callback = [this](wfc::DGraphNode* node) -> void {
        auto* s_node = dynamic_cast<SCWFCGraphNode*>(node);
        assert(s_node);

        if (node->domain.size() == 0) {
            
            // remove nodes with 0 valid objects in their domain from the scene
            s_node->destroy();
        } else {
            const int class_id = node->domain[0]->pattern_class;
            auto model = m_internal->unsolved_drawable;
            float extent{2 * s_node->get_bounding_sphere()
                                    .radius};  // default to the same size
            // obj_db will return all ObjestData's for a class id, but will
            // only use the first if it exists here.
            if (auto [itr, end] = obj_db->objs_for_id(class_id);
                itr != end) {
                auto& [id, obj_data] = *itr;
                auto m = ResourceManager::get_singleton()
                                .get_model_relative_path(obj_data.asset_path);
                if (m) {
                    model = m;
                    extent = obj_data.extent;
                }
            }

            s_node->set_radius(extent / 2);
            if (m_scwfc_node->intersects_any_solved_neighbor(Ref{ s_node })) {
                // remove nodes whose final bounding volume intersects solved nodes
                s_node->destroy();
            } else {
                // rescale the object so that it fits withing the extent
                const auto& aabb = model->bounding_box;
                const glm::vec3 scale = glm::vec3{ extent } / glm::length(aabb.diagonal()); // scale uniformly

                s_node->set_model(model);
                s_node->set_scale(scale);
            }
        }
    };

    wfc::WFCSolver::entropy_callback_t entropy_func =
        [](auto* prop_node, auto* on_node) -> float {
            auto* prop_node_scene =
                dynamic_cast<const SCWFCGraphNode*>(prop_node);
            auto* on_node_scene = dynamic_cast<const SCWFCGraphNode*>(on_node);
            return on_node->entropy() -
                    1 / glm::length(prop_node_scene->get_position() -
                                    on_node_scene->get_position());
        };

    m_internal->solver->set_entropy_func(entropy_func);
    while (m_internal->solver->can_continue() && cnt++ < steps) {
        auto solved_node = m_internal->solver->step_wfc();
        c_callback(solved_node);
    }
}

}