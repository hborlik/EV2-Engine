#include "renderer/terrain_renderer.hpp"
#include <application.hpp>

#include <memory>
#include <window.hpp>
#include <renderer/renderer.hpp>
#include <physics.hpp>
#include <ui/imgui.hpp>
#include <ui/imgui_impl_glfw.hpp>
#include <ui/imgui_impl_opengl3.hpp>

namespace ev2 {

Application::Application() : m_terrain_renderer(std::make_unique<renderer::TerrainRenderer>()){
    renderer::Renderer::get_singleton().add_pass(m_terrain_renderer.get());
}

void Application::process(float dt) {
    on_process(dt);
}

void Application::imgui() {
    GLFWwindow * window = ev2::window::getContextWindow();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    on_render_ui();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int Application::run() {
    // game->scene_tree.change_scene(game->scene.get());

    float dt = 0.05f;
    while(window::frame()) {
        process(dt); // application update
        scene_tree.update(dt); // scene graph update
        Physics::get_singleton().simulate(dt); // finally, physics update

        scene_tree.update_pre_render(); // compute all transforms in scene and pass to renderer

        auto camera_node = get_current_camera();

        renderer::Renderer::get_singleton().render(camera_node->get_camera()); // render scene

        imgui();
        dt = float(window::getFrameTime());
    }

    current_scene->destroy();
    
    return 0;
}

void Application::on_process(float dt) {

}

void Application::on_key(input::Key::Enum key, input::Modifier mods, bool down) {

}

void Application::on_char(uint32_t scancode) {

}

void Application::on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) {

}

void Application::cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) {
    if (!show_debug) {
        glm::vec2 scr_size = window::getWindowSize();
        glm::vec2 s_pos = window::getCursorPosition() / scr_size;
        input::SetMousePosition(s_pos);
    }
}

void Application::on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, input::MouseButton::Enum button, bool down) {
    ev2::input::SetMouseButton(button, down);
}

void Application::on_window_size_change(int32_t width, int32_t height) {
    if (ev2::renderer::Renderer::is_initialized())
        ev2::renderer::Renderer::get_singleton().set_resolution(width, height);
    window_width = width;
    window_height = height;
}

void Application::on_drop_file(std::string_view path) {

}

void Application::on_render_ui() {
    if (show_debug)
        scene_editor.show_editor(current_scene.get(), &current_camera->get_camera());
}


}