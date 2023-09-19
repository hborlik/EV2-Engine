/**
 * @file application.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_APPLICATION_H
#define EV2_APPLICATION_H

#include "evpch.hpp"

#include "input.hpp"
#include "ui/ui.hpp"

#include "renderer/opengl_renderer/terrain_renderer.hpp"

namespace ev2 {

class Application {
public:
    Application();
    virtual ~Application() {}

    void process(float dt);

    void imgui();

    int run();

    virtual void on_process(float dt);
    virtual void on_key(input::Key::Enum key, input::Modifier mods, bool down);
    virtual void on_char(uint32_t scancode);
    virtual void on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos);
    virtual void cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos);
    virtual void on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, input::MouseButton::Enum button, bool down);
    virtual void on_window_size_change(int32_t width, int32_t height);
    virtual void on_drop_file(std::string_view path);

    virtual void on_draw_imgui();

    Ref<Node> get_current_scene() const noexcept {return current_scene;}

    void set_current_scene(Ref<Node> scene) {
        scene_tree.change_scene(scene);
        current_scene = scene;
    }

    auto get_current_camera() const noexcept {return current_camera;}

    void set_current_camera(Ref<CameraNode> camera) {
        current_camera = camera;
    }

#ifdef ENABLE_TERRAIN
    renderer::TerrainRenderer& get_terrain() {
        return *m_terrain_renderer.get();
    }
#endif // ENABLE_TERRAIN

    SceneTree& get_scene_tree() noexcept {return scene_tree;}

protected:
    bool show_debug = false;
    Editor scene_editor{};

    int32_t window_width = 1920;
    int32_t window_height = 1080;

private:
    SceneTree scene_tree{};
    Ref<Node> current_scene = nullptr;
    Ref<CameraNode> current_camera = nullptr;

#ifdef ENABLE_TERRAIN
    std::unique_ptr<renderer::TerrainRenderer> m_terrain_renderer{};
#endif // ENABLE_TERRAIN
};

}

#endif // EV2_APPLICATION_H
