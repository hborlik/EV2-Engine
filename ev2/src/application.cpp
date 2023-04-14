#include <application.hpp>

#include <window.hpp>
#include <renderer/renderer.hpp>

namespace ev2 {

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

void Application::on_drop_file(const std::string& path) {

}


}