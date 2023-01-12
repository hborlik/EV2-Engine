#include <debug.h>
#include <renderer/renderer.h>
#include <resource.h>
#include <physics.h>

#include <imgui.h>

static bool enable_physics_timestep = true;

void show_material_editor_window() {
    ImGui::Begin("Material Editor");
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

void show_settings_editor_window(GameState* game) {
    ImGui::Begin("Settings");
    ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Time %.3f", game->time_day);
    ImGui::DragFloat("Time Speed", &(game->time_speed), 0.01f, 0.05f, 5.0f, "%.3f", 1.0f);
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
    if (ImGui::Checkbox("Enable Physics Timestep", &enable_physics_timestep)) {
        ev2::Physics::get_singleton().enable_simulation(enable_physics_timestep);
    }
    ImGui::Separator();
    ImGui::DragFloat("Sky Brightness", &(ev2::renderer::Renderer::get_singleton().sky_brightness), 0.01f, 0.01f, 2.f, "%.3f", 1.0f);
    ImGui::End();
}

void show_tree_window(GameState* game, ev2::Ref<TreeNode> selected_tree) {

    std::map<std::string, float> GUIParams = selected_tree->getParams();
    float fieldA = GUIParams.find("R_1")->second;
    float fieldB = GUIParams.find("R_2")->second;
    float fieldC = GUIParams.find("w_r")->second;
    float fieldDegree = ptree::radToDeg(GUIParams.find("a_0")->second);
    float fieldDegree2 = ptree::radToDeg(GUIParams.find("a_2")->second);
    float fieldDegree3 = ptree::radToDeg(GUIParams.find("d")->second);
    float thickness = GUIParams.find("thickness")->second;

    int counter = selected_tree->plantInfo.iterations;
    bool changed = false;
    
    ImGui::Begin(("Species #" + std::to_string(selected_tree->plantInfo.ID)).c_str(), &selected_tree->plantInfo.selected);                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("Edit your plant's genes!!! :D");               // Display some text (you can use a format strings too)
    
    if (ImGui::TreeNode("Color")) {
        changed |= ImGui::ColorPicker3("diffuse color 0", glm::value_ptr(selected_tree->c0), ImGuiColorEditFlags_InputRGB);
        changed |= ImGui::ColorPicker3("diffuse color 1", glm::value_ptr(selected_tree->c1), ImGuiColorEditFlags_InputRGB);
        ImGui::TreePop();
    }

    if (ImGui::SliderFloat("R_1", &fieldA, 0.001f, 1.0f))
    {
        GUIParams.find("R_1")->second = fieldA;
        changed = true;
    } 
    if (ImGui::SliderFloat("R_2", &fieldB, 0.001f, 2.0f))
    {
        GUIParams.find("R_2")->second = fieldB;
        changed = true;
    } 
    if (ImGui::SliderFloat("w_r", &fieldC, 0.001f, 1.0f))
    {
        GUIParams.find("w_r")->second = fieldC;
        changed = true;
    } 
    if (ImGui::SliderFloat("a_0 (degrees)", &fieldDegree, .5f, 60.0f))  
    {
        GUIParams.find("a_0")->second = ptree::degToRad(fieldDegree);
        changed = true;
    } 
    if (ImGui::SliderFloat("a_2 (degrees)", &fieldDegree2, .5f, 60.0f))
    {
        GUIParams.find("a_2")->second = ptree::degToRad(fieldDegree2);
        changed = true;
    } 
    if (ImGui::SliderFloat("d (degrees)", &fieldDegree3, 0.f, 360.0f))
    {
        GUIParams.find("d")->second = ptree::degToRad(fieldDegree3);
        changed = true;
    }
    if (ImGui::SliderFloat("thickness", &thickness, 0.2f, 10.0f))
    {
        GUIParams.find("thickness")->second = thickness;
        changed = true;
    }
    if (ImGui::Button("Increase iterations."))                           
    {
        selected_tree->plantInfo.iterations++;
        changed = true;
    }
    if (ImGui::Button("Decrease iterations."))                            
    {
        selected_tree->plantInfo.iterations--;
        if (selected_tree->plantInfo.iterations <= 0) {selected_tree->plantInfo.iterations = 0;}
        changed = true;
    }
    if (changed) 
    {                                               
        selected_tree->setParams(GUIParams, selected_tree->plantInfo.iterations, selected_tree->growth_current);
        changed = false;           // Edit 1 float using a slider from 0.0f to 1.0f
    }

    ImGui::Text("P = %d", selected_tree->plantInfo.iterations);
    ImGui::End();
}