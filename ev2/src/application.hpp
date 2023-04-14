/**
 * @file application.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_APPLICATION_H
#define EV2_APPLICATION_H

#include <string>

#include <input.hpp>

namespace ev2 {

class Application {
public:
    virtual ~Application() {}

    virtual void on_key(input::Key::Enum key, input::Modifier mods, bool down);
    virtual void on_char(uint32_t scancode);
    virtual void on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos);
    virtual void cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos);
    virtual void on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, input::MouseButton::Enum button, bool down);
    virtual void on_window_size_change(int32_t width, int32_t height);
    virtual void on_drop_file(const std::string& path);

protected:
    bool show_debug = false;

    int32_t window_width = 1920;
    int32_t window_height = 1080;
};

}

#endif // EV2_APPLICATION_H
