#include <pcg/sc_wfc.hpp>

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <pcg/wfc.hpp>
#include <renderer/renderer.hpp>
#include <ui/imgui.hpp>
#include <resource.hpp>

#include <io/serializers.hpp>

namespace fs = std::filesystem;

namespace ev2::pcg {

class SCWFCGraphNode : public VisualInstance, wfc::DGraphNode {
public:
    explicit SCWFCGraphNode(const std::string &name, SCWFC* scwfc) : VisualInstance{name}, wfc::DGraphNode{name, uuid_hash}, m_scwfc{scwfc} {}

    void on_init() override {
        VisualInstance::on_init();

        m_bounding_sphere = Sphere{get_world_position(), 1.f};
    }

    void on_transform_changed(Ref<ev2::Node> origin) override {
        VisualInstance::on_transform_changed(origin);

        m_bounding_sphere.center = get_world_position();
    }

    const Sphere& get_bounding_sphere() const noexcept {return m_bounding_sphere;}

private:
    SCWFC* m_scwfc = nullptr;
    Sphere m_bounding_sphere{};
};

class SCWFCAttractorNode : public VisualInstance {
public:
    explicit SCWFCAttractorNode(const std::string &name, SCWFC* scwfc) : VisualInstance{name}, m_scwfc{scwfc} {}

private:
    SCWFC* m_scwfc = nullptr;
};

std::shared_ptr<renderer::Drawable> SCWFCObjectDatabase::get_model_for_id(int id) {
    return m_meshes.at(id);
}

void SCWFCObjectDatabase::add_model(std::shared_ptr<renderer::Drawable> d, int id) {
    auto [_it, ins] = m_meshes.emplace(std::make_pair(id, d));
    if (!ins)
        throw std::runtime_error{"model already inserted for " + std::to_string(id)};
}



std::unique_ptr<SCWFCObjectDatabase> load_object_database(const std::string& path) {
    using json = nlohmann::json;
    auto db = std::make_unique<SCWFCObjectDatabase>();

    std::string json_str = io::read_file(path);

    json j = json::parse(json_str);
    for (auto& entry : j) {
        SCWFCObjectMetadata metadata;
        entry.get_to(metadata);
    }

    return db;
}

//////

struct SCWFC::Data {
    Data() : graph{}, solver{&graph} {}

    wfc::SparseGraph<wfc::DGraphNode> graph;
    wfc::WFCSolver solver;
};

SCWFC::SCWFC(std::string name): 
    Node{ std::move(name) } {

}

void SCWFC::wfc_solve(int steps) {
    int cnt = 0;
    while(m_data->solver.can_continue() && cnt++ < steps) {
        m_data->solver.step_wfc();
    }
}

void SCWFC::sc_spawn_points(int n) {

}

void SCWFC::sc_propagate_from(Ref<Node> node) {
    auto nnode = create_child_node<SCWFCGraphNode>(node->name + "+", this);
    nnode->set_model(obj_db->get_model_for_id(-1));
    nnode->set_position(node->get_position() + glm::vec3{1, 0, 1});
}

void SCWFC::reset() {
    m_data = std::make_shared<Data>();

    auto nnode = create_child_node<SCWFCGraphNode>("SC seed node", this);
}

void SCWFC::on_init() {
    auto cube0 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);

    auto cube1 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube1->materials[0]->diffuse = glm::vec3{1};

    auto cube2 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube2->materials[0]->diffuse = glm::vec3{1, 0, 0};

    obj_db = std::make_shared<SCWFCObjectDatabase>();

    obj_db->add_model(cube0, 10);
    obj_db->add_model(cube1, 11);
    obj_db->add_model(cube2, -1);

    reset();
}

void SCWFC::on_child_removed(Ref<Node> child) {

}

std::optional<glm::vec3> SCWFC::does_intersect_any(const Sphere& sph) {
    glm::vec3 net{};
    bool single = false;
    for (auto& c : get_children()) {
        auto graph_node = c.ref_cast<SCWFCGraphNode>();
        if (graph_node) {
            if (intersect(graph_node->get_bounding_sphere(), sph)) {
                net += graph_node->get_bounding_sphere().center - sph.center;
                single = true;
            }
        }
    }
    return single ? std::optional<glm::vec3>{net} : std::optional<glm::vec3>{};
}


void SCWFCNodeEditor::show_editor(Node* node) {
    SCWFC* n = dynamic_cast<SCWFC*>(node);
    if (n) {
        static int steps = 0;
        ImGui::InputInt("Solver Steps", &steps);
        steps = steps > 0 ? steps : 0;
        if (ImGui::Button("Generate")) {
            n->wfc_solve(steps);
        }
    }
}

void SCWFCEditor::show_editor_tool() {
    ImGui::Begin("SCWFCEditor", &m_is_open);

    if (m_scwfc_node) {
        ImGui::Text("Selected %s", m_scwfc_node->name.c_str());
    
        if (ImGui::Button("Spawn")) {
            if (m_scwfc_node)
                m_scwfc_node->sc_propagate_from(m_editor->get_selected_node()->get_ref<Node>());
        }
    } else {
        ImGui::Text("Please select a SCWFC node");
    }

    ImGui::End();
}

}