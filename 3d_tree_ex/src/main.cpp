
#include <iostream>
#include <filesystem>

#include <tests.h>
#include <tree.h>
//#define DR_MP3_IMPLEMENTATION
//#include "../extras/dr_mp3.h"   /* Enables MP3 decoding. */

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h> 

#include <ev.hpp>
#include <renderer/ev_gl.hpp>
#include <window.hpp>
#include <renderer/shader.hpp>
#include <application.hpp>
#include <renderer/camera.hpp>
#include <window.hpp>
#include <renderer/mesh.hpp>
#include <resource.hpp>
#include <Sphere.h>
#include <physics.hpp>
#include <scene/visual_nodes.hpp>
#include <scene/procedural_grid.hpp>
#include <debug.h>
#include <game.h>

#include <ui/imgui.hpp>

namespace fs = std::filesystem;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

class TestApp : public ev2::Application {
public:
    TestApp() {}
    
    TestApp(const fs::path& asset_path) :   asset_path{asset_path} {}

    fs::path asset_path = fs::path("asset");

    ev2::Ref<ev2::CameraNode> cam_orbital{};
    ev2::Ref<ev2::CameraNode> cam_fly{};
    ev2::Ref<ev2::Node> cam_orbital_root{};

    std::unique_ptr<GameState> game;

    glm::vec2 mouse_p{};
    glm::vec2 mouse_delta{};
    glm::vec2 move_input{};
    bool left_mouse_down = false;
    bool right_mouse_down = false;
    bool place_child = false;
    float cam_x = 0, cam_y = 0;
    float cam_boom_length = 10.0f;

    enum CameraMode {
        CAM_MODE_ORBIT = 0,
        CAM_MODE_FLY
    } m_camera_mode = CAM_MODE_ORBIT;

    ev2::Ref<ev2::CameraNode> getCameraNode() {
        switch(m_camera_mode) {
            case CAM_MODE_FLY:
                return cam_fly;
            case CAM_MODE_ORBIT:
                return cam_orbital;
            default:
                break;
        }
        return cam_orbital;
    }

    void on_render_ui() override {
        Application::on_render_ui();

        if (show_debug) {
            show_settings_editor_window(game.get());
        }
        if (game->selected_tree_1) {
            ImGui::SetNextWindowSize(ImVec2(window_width/5, window_height/5));
            ImGui::SetNextWindowPos(ImVec2(window_width - window_width/5, 0));
            show_tree_window(game.get(), game->selected_tree_1);
        }
        if (game->selected_tree_2) {
            ImGui::SetNextWindowSize(ImVec2(window_width/5, window_height/5));
            ImGui::SetNextWindowPos(ImVec2(window_width - window_width/5, window_height/5));
            show_tree_window(game.get(), game->selected_tree_2);
        }

    }

    void initialize() {
        set_current_scene(Node::create_node<Node>("scene0"));

        scene_editor.add_custom_node_editor(std::make_shared<ev2::ProceduralGridEditor>());

        get_current_scene()->create_child_node<ev2::ProceduralGrid>("procedural grid");

        game = std::make_unique<GameState>(this);

        cam_orbital      = get_current_scene()->create_child_node<ev2::CameraNode>("Orbital");
        cam_orbital_root = get_current_scene()->create_child_node<ev2::Node>("cam_orbital_root");

        cam_orbital_root->add_child(cam_orbital);

        cam_fly = get_current_scene()->create_child_node<ev2::CameraNode>("FlyCam");

        // ev2::ResourceManager::get_singleton().loadGLTF(fs::path("models") / "Box.gltf");
       
    }


    void toggleWireframe() {
        static bool enabled = false;
        enabled = !enabled;
        ev2::renderer::Renderer::get_singleton().set_wireframe(enabled);
    }


    void on_process(float dt) override {
        Application::on_process(dt);

        game->update(dt);

        set_current_camera(getCameraNode());

        ImGuiIO& io = ImGui::GetIO();

        if (show_debug && (right_mouse_down || ev2::window::getMouseCaptured()) && !io.WantCaptureMouse) {
            mouse_delta = ev2::window::getCursorPosition() - mouse_p;
            mouse_p = ev2::window::getCursorPosition();
            cam_x += mouse_delta.x * -.005f;
            cam_y = glm::clamp<float>(cam_y + mouse_delta.y * -.005f, glm::radians(-85.0f), glm::radians(85.0f));
        }

        switch(m_camera_mode) {
            case CAM_MODE_ORBIT:
            {
                glm::vec3 boom = {0, 0, cam_boom_length};
                glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_y, glm::vec3{1, 0, 0});
                cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_x, {0, 1, 0}) * cam_t;

                boom = cam_t * glm::vec4(boom, 1.0f);

                cam_orbital->set_position(boom);
                cam_orbital->set_rotation(glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0}));
            }
                break;
            case CAM_MODE_FLY:
            {
                glm::quat cam_q = glm::rotate(glm::identity<glm::quat>(), (float)cam_y, glm::vec3{1, 0, 0});
                cam_q = glm::rotate(glm::identity<glm::quat>(), (float)cam_x, {0, 1, 0}) * cam_q;

                cam_fly->set_rotation(cam_q);
            }
                break;
        }

        
        if (show_debug && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            switch(m_camera_mode) {
                case CAM_MODE_ORBIT:
                {
                    glm::vec3 cam_forward = glm::normalize(cam_orbital->get_camera().get_forward() * glm::vec3{1, 0, 1});
                    glm::vec3 cam_right = glm::normalize(cam_orbital->get_camera().get_right() * glm::vec3{1, 0, 1});
                    cam_orbital_root->set_position(
                        cam_orbital_root->get_position() + 
                        (cam_forward * 1.0f * cam_boom_length * dt * input.y + 
                        cam_right * 1.0f * cam_boom_length * dt * input.x)
                    ); // camera movement on y plane
                }
                    break;
                case CAM_MODE_FLY:
                {
                    glm::vec3 cam_forward = glm::normalize(cam_fly->get_camera().get_forward());
                    glm::vec3 cam_right = glm::normalize(cam_fly->get_camera().get_right());
                    cam_fly->set_position(
                        cam_fly->get_position()
                        + cam_forward * cam_boom_length * dt * input.y
                        + cam_right * cam_boom_length * dt * input.x
                    );
                }
                    break;
                default:
                    throw std::runtime_error{"invalid camera mode"};
            };
            
        }
    }

    void on_key(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {
        Application::on_key(key, mods, down);
        ImGuiIO& io = ImGui::GetIO();
        switch (key) {
            case ev2::input::Key::Esc:
                if (down) {
                    show_debug = !show_debug;
                    ev2::window::setMouseCaptured(!show_debug);
                    ev2::input::SetInputEnabled(!show_debug);
                } 
                break;
            default:
                break;
        }
        if (!show_debug)
            ev2::input::SetKeyState(key, mods, down);
        if (show_debug && !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            switch (key) {
                case ev2::input::Key::Tab:
                    if (down) {
                        m_camera_mode = m_camera_mode == CAM_MODE_FLY ? CAM_MODE_ORBIT : CAM_MODE_FLY;
                        cam_fly->set_position(cam_orbital->get_world_position());
                    }
                    break;
                case ev2::input::Key::KeyP:
                    break;
                case ev2::input::Key::KeyF:
                    break;
                case ev2::input::Key::KeyZ:
                    if (down)
                        toggleWireframe();
                    break;
                case ev2::input::Key::KeyW:
                    move_input.y = down ? 1.0f : 0.0f;
                    break;
                case ev2::input::Key::KeyS:
                    move_input.y = down ? -1.0f : 0.0f;
                    break;
                case ev2::input::Key::KeyA:
                    move_input.x = down ? -1.0f : 0.0f;
                    break;
                case ev2::input::Key::KeyD:
                    move_input.x = down ? 1.0f : 0.0f;
                    break;
                default:
                    break;
            }
        } else {
            move_input = glm::vec2{};
        }
    }

    void on_char(uint32_t scancode) override {}

    void on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {
        Application::on_scroll(mouse_x, mouse_y, scroll_pos);
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            static int32_t scroll_last = scroll_pos;
            int32_t scroll_delta = scroll_pos - scroll_last;
            scroll_last = scroll_pos;
            cam_boom_length = glm::clamp(cam_boom_length - scroll_delta, 0.1f, 1000.f);
        }
    }

    void cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {
        Application::cursor_pos(mouse_x, mouse_y, scroll_pos);
    }

    void on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {
        Application::on_mouse_button(mouse_x, mouse_y, scroll_pos, button, down);

        mouse_p = ev2::window::getCursorPosition();
        if (button == 1)
            left_mouse_down = down;
        if (button == 3)
            right_mouse_down = down;
    }
};

//Callback for miniaudio.
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput; // Unused.

    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    /* Reading PCM frames will loop based on what we specified when called ma_data_source_set_looping(). */
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);
}

int initAudio(fs::path asset_path) {
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;
    const char* filePath = "";

    filePath = (asset_path / "stickerbrush.mp3").generic_string().c_str();
    printf("%s\n", filePath);
    result = ma_decoder_init_file(filePath, NULL, &decoder);
    if (result != MA_SUCCESS) {
        return -2;
    }

    ma_data_source_set_looping(&decoder, MA_TRUE);

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    ev2::Args args{argc, argv};

    fs::path asset_path = fs::path("asset");
    fs::path log_path = fs::path("logs");

    ev2::EV2_init(args, asset_path, log_path);
    ev2::window::setWindowTitle("SC-WFC Sandbox");

    std::unique_ptr<TestApp> app = std::make_unique<TestApp>(asset_path);
    ev2::window::setApplication(app.get());
    //initAudio(asset_path);
    app->initialize();

    int rv = app->run();

    // shutdown
    ev2::window::setApplication(nullptr);
    app = {};
    ev2::EV2_shutdown();
    //TODO: deinit audio device and decoder.
    return rv;
}