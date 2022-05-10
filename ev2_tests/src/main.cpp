
#include <iostream>
#include <filesystem>

#include <tests.h>
#include <tree.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include <ev.h>
#include <ev_gl.h>
#include <window.h>
#include <shader.h>
#include <application.h>
#include <camera.h>
#include <window.h>
#include <mesh.h>
#include <resource.h>
#include <Sphere.h>
#include <renderer.h>
#include <scene.h>
#include <visual_nodes.h>


namespace fs = std::filesystem;

float randomFloatTo(float limit) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/limit));
}

struct Plant {
    glm::vec3 position;
    glm::vec3 color;
    glm::quat rot;
    Sphere geometry;
    Plant(glm::vec3 pos, glm::vec3 col, Sphere geo) {position = pos; color = col; geometry = geo; 
        rot = glm::quatLookAt(glm::vec3(randomFloatTo(2) - 1, 0, randomFloatTo(2) - 1), glm::vec3{0, 1, 0});
    }
};

    // Our state
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

class TestApp : public ev2::Application {
public:
    TestApp() : RM{std::make_unique<ev2::ResourceManager>(asset_path)},
                scene{std::make_unique<ev2::Scene>(RM)} {}
    
    TestApp(const fs::path& asset_path) :   asset_path{asset_path}, 
                                            RM{std::make_unique<ev2::ResourceManager>(asset_path)}, 
                                            scene{std::make_unique<ev2::Scene>(RM)} {}

    fs::path asset_path = fs::path("asset");

    ev2::Ref<ev2::CameraNode> cam_orbital{};
    ev2::Ref<ev2::Node> cam_orbital_root{};
    ev2::Ref<ev2::CameraNode> cam_fly{};
    ev2::Ref<ev2::CameraNode> cam_first_person{};

    ev2::Ref<TreeNode> tree{};

    std::shared_ptr<ev2::ResourceManager> RM;
    std::unique_ptr<ev2::Scene> scene;

    glm::vec2 mouse_p{};
    glm::vec2 mouse_delta{};
    glm::vec2 move_input{};
    bool mouse_down = false;
    float cam_x = 0, cam_y = 0;
    float cam_boom_length = 10.0f;

    enum CameraMode : uint8_t {
        FirstPerson = 0,
        Orbital
    } camera_type = Orbital;

    ev2::Ref<ev2::CameraNode> getActiveCam() {
        switch(camera_type) {
            case Orbital:
                return cam_orbital;
            case FirstPerson:
                return cam_first_person;
            default:
                return cam_orbital;
        }
    }

void imgui(GLFWwindow * window) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            //ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float fieldA = 0.9f;
            static float fieldB = 0.9f;
            static float fieldC = 0.7f;
            static float fieldDegree = 45.0f;
            static float fieldDegree2 = 45.0f;
            static float fieldDegree3 = 135.5f;
            
            static int counter = 0;
            std::map<std::string, float> GUIParams = {
                    {"R_1", fieldA},
                    {"R_2", fieldB},
                    {"a_0", ptree::degToRad(fieldDegree)},
                    {"a_2", ptree::degToRad(fieldDegree2)},
                    {"d",   ptree::degToRad(fieldDegree3)},
                    {"w_r", fieldC}
                };
            ImGui::Begin("Stem editor.");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("A preliminary editor for a plant's branch structure, each parameter is a \"gene\"");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::SliderFloat("float", &fieldA, 0.001f, 2.0f) |           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("float2", &fieldB, 0.001f, 2.0f) |           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("float3", &fieldC, 0.001f, 2.0f) |           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("floatDegree1", &fieldDegree, 1.0f, 360.0f) |           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("floatDegree2", &fieldDegree2, 1.0f, 360.0f) |           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("floatDegree3", &fieldDegree3, 1.0f, 360.0f)) 
                {
                    GUIParams.find("R_1")->second = fieldA;
                    GUIParams.find("R_2")->second = fieldB;
                    GUIParams.find("a_0")->second = fieldDegree;
                    GUIParams.find("a_2")->second = fieldDegree2;
                    GUIParams.find("d")->second = fieldDegree3;
                    GUIParams.find("w_r")->second = fieldC;
                                                            
                    tree->setParams(GUIParams, counter);
                };            // Edit 1 float using a slider from 0.0f to 1.0f
        

            if (ImGui::Button("Increase iterations."))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                counter++;
                //tree->generate(counter + 1);
                tree->setParams(GUIParams, counter);
            }

            if (ImGui::Button("Decrease iterations."))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                counter--;
                if (counter <= 0) {counter = 0;}
                //tree->generate(counter + 1);
                tree->setParams(GUIParams, counter);
            }
            ImGui::Text("P = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


    void initialize() {

        Sphere supershape(1.0f , 20, 20);
        // cube =  supershape.getModel();


        ev2::MID hid = RM->get_model( fs::path("models") / "rungholt" / "house.obj");
        ev2::MID ground = RM->get_model( fs::path("models") / "cube.obj");
        ev2::MID building0 = RM->get_model( fs::path("models") / "low_poly_houses.obj");

        // ground_cube->materials[0].diffuse = {0.1, 0.6, 0.05};
        // ground_cube->materials[0].shininess = 0.02;

        auto light = scene->create_node<ev2::DirectionalLightNode>("directional_light");
        light->transform.position = glm::vec3{10, 100, 0};

        auto h_node = scene->create_node<ev2::VisualInstance>("house");
        h_node->set_model(hid);
        h_node->transform.position = glm::vec3{30, 0, 0};
        h_node->transform.rotate({0.1, 0.5, 0});
        h_node->transform.scale = glm::vec3{0.1, 0.1, 0.1};

        auto lh_node = scene->create_node<ev2::VisualInstance>("building");
        lh_node->transform.position = glm::vec3{50, 1, -20};
        lh_node->set_model(building0);

        auto tree_bark = RM->create_material("bark");
        tree_bark.first->diffuse = glm::vec3{0.59,0.44,0.09};
        tree_bark.first->metallic = 0;
        RM->push_material_changed("bark");

        auto ground_material = RM->create_material("ground_mat");
        ground_material.first->diffuse = glm::vec3{0.529, 0.702, 0.478};
        ground_material.first->sheen = 0.2;
        RM->push_material_changed("ground_mat");

        auto g_node = scene->create_node<ev2::VisualInstance>("ground");
        g_node->set_model(ground);
        g_node->set_material_override(ground_material.second);
        g_node->transform.scale = glm::vec3{1000, 0.1, 1000};
        g_node->transform.position = glm::vec3{0, 0.4, 0};

        cam_orbital      = scene->create_node<ev2::CameraNode>("Orbital");
        cam_orbital_root = scene->create_node<ev2::Node>("cam_orbital_root");
        cam_first_person = scene->create_node<ev2::CameraNode>("FP");

        cam_orbital_root->add_child(cam_orbital);

        tree = scene->create_node<TreeNode>("Tree");
        tree->transform.position = glm::vec3{50, 0, 0};
        tree->set_material_override(tree_bark.second);
        
        ev2::Ref<TreeNode> tree2 = scene->create_node<TreeNode>("Tree2");
        tree2->transform.position = glm::vec3{25, 0, 0};
        tree2->set_material_override(tree_bark.second);    
    }

    void updateShape(float dt, Sphere geometry) {
        // cube = geometry.getModel();
    }


    int run() {
        float dt = 0.05f;
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

        GLFWwindow* window = ev2::window::getContextWindow();
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);


        while(ev2::window::frame()) {
            //Passing io to manage focus between app behavior and imgui behavior on mouse events.
            update(dt, io);
            RM->pre_render();
            ev2::Renderer::get_singleton().render(getActiveCam()->get_camera());
            imgui(window);
            dt = float(ev2::window::getFrameTime());
        }
        ev2::Renderer::shutdown();
        return 0;
    }

    void toggleWireframe() {
        static bool enabled = false;
        enabled = !enabled;
        ev2::Renderer::get_singleton().set_wireframe(enabled);
    }

    void update(float dt, ImGuiIO& io) {
        // first update scene
        scene->update(dt);

        if ((mouse_down || ev2::window::getMouseCaptured()) && !io.WantCaptureMouse) {
            mouse_delta = ev2::window::getCursorPosition() - mouse_p;
            mouse_p = ev2::window::getCursorPosition();
            cam_x += mouse_delta.x * -.005f;
            cam_y = glm::clamp<float>(cam_y + mouse_delta.y * -.005f, glm::radians(-85.0f), glm::radians(85.0f));
        }

        glm::vec3 boom = {0, 0, cam_boom_length};
        glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_y, glm::vec3{1, 0, 0});
        cam_t = glm::rotate(glm::mat4{1.0f}, (float)cam_x, {0, 1, 0}) * cam_t;

        boom = cam_t * glm::vec4(boom, 1.0f);

        cam_orbital->transform.position = boom;
        cam_orbital->transform.rotation = glm::quatLookAt(-glm::normalize(boom), glm::vec3{0, 1, 0});

        cam_first_person->transform.rotation = glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0});
        if (camera_type == FirstPerson && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_first_person->get_camera().get_forward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_first_person->get_camera().get_right() * glm::vec3{1, 0, 1});
            cam_first_person->transform.position = glm::vec3(
                cam_first_person->transform.position * glm::vec3{1, 0, 1} + 
                glm::vec3{0, 2, 0} + 
                cam_forward * 10.0f * dt * input.y + 
                cam_right * 10.0f * dt * input.x
            ); // force camera movement on y plane
        }
        else if (camera_type == Orbital && glm::length(move_input) > 0.0f) {
            glm::vec2 input = glm::normalize(move_input);
            glm::vec3 cam_forward = glm::normalize(cam_orbital->get_camera().get_forward() * glm::vec3{1, 0, 1});
            glm::vec3 cam_right = glm::normalize(cam_orbital->get_camera().get_right() * glm::vec3{1, 0, 1});
            cam_orbital_root->transform.position +=
                cam_forward * 1.0f * cam_boom_length * dt * input.y + 
                cam_right * 1.0f * cam_boom_length * dt * input.x
            ; // camera movement on y plane
        }
    }

    void onKey(ev2::input::Key::Enum key, ev2::input::Modifier mods, bool down) override {
        switch (key) {
            case ev2::input::Key::Tab:
                if (down)
                    ev2::window::setMouseCaptured(!ev2::window::getMouseCaptured());
                break;
            case ev2::input::Key::KeyP:
                break;
            case ev2::input::Key::Esc:
                break;
            case ev2::input::Key::KeyF:
                if (down)
                    camera_type = CameraMode((camera_type + 1) % 2);
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
    }

    void on_char(uint32_t scancode) override {}

    void on_scroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {
        static int32_t scroll_last = scroll_pos;
        int32_t scroll_delta = scroll_pos - scroll_last;
        scroll_last = scroll_pos;
        cam_boom_length = glm::clamp(cam_boom_length - scroll_delta, 0.f, 200.f);
    }

    void cursor_pos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) override {}

    void on_mouse_button(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, ev2::input::MouseButton::Enum button, bool down) override {
        mouse_p = ev2::window::getCursorPosition();
        mouse_down = down;
    }

    void on_window_size_change(int32_t width, int32_t height) override {
        if (ev2::Renderer::is_initialized())
            ev2::Renderer::get_singleton().set_resolution(width, height);
    }

    void on_drop_file(const std::string& path) override {}
};



int main(int argc, char *argv[]) {

    ev2::Args args{argc, argv};

    fs::path asset_path = fs::path("asset");

    ev2::EV2_init(args, asset_path);
    ev2::window::setWindowTitle("Plant Game");

    std::unique_ptr<TestApp> app = std::make_unique<TestApp>(asset_path);
    ev2::window::setApplication(app.get());

    app->initialize();

    return app->run();;
}