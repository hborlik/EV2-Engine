#include <pcg/sc_wfc.hpp>

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "wfc.hpp"
#include "distributions.hpp"
#include "../ui/imgui.hpp"
#include "../io/serializers.hpp"
#include "../resource.hpp"

namespace fs = std::filesystem;

namespace ev2::pcg {

class SCWFCGraphNode : public VisualInstance, public wfc::DGraphNode {
public:
    explicit SCWFCGraphNode(const std::string &name, SCWFC* scwfc) : VisualInstance{name}, wfc::DGraphNode{name, (int)uuid_hash}, m_scwfc{scwfc} {}

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

std::shared_ptr<renderer::Drawable> SCWFCObjectMetadataDB::get_model_for_id(int id) {
    return m_meshes.at(id);
}

void SCWFCObjectMetadataDB::add_model(std::shared_ptr<renderer::Drawable> d, int id) {
    auto [_it, ins] = m_meshes.emplace(std::make_pair(id, d));
    if (!ins)
        throw std::runtime_error{"model already inserted for " + std::to_string(id)};
}



std::unique_ptr<SCWFCObjectMetadataDB> load_object_database(const std::string& path) {
    using json = nlohmann::json;
    auto db = std::make_unique<SCWFCObjectMetadataDB>();

    std::string json_str = io::read_file(path);

    json j = json::parse(json_str);
    for (auto& entry : j) {
        SCWFCObjectMetadataDB metadata;
        entry.get_to(metadata);
    }

    return db;
}

//////

struct SCWFC::Data {
    wfc::SparseGraph<wfc::DGraphNode> graph;
};

SCWFC::SCWFC(std::string name): 
    Node{ std::move(name) } {

}


void SCWFC::reset() {
    // for (auto& c : get_children()) {
    //     c->destroy();
    // }
    for (auto c : get_children())
        c->destroy();

    m_data = std::make_shared<Data>();
}

void SCWFC::on_init() {
    reset();
}

void SCWFC::on_child_removed(Ref<Node> child) {

}

void SCWFC::on_child_added(Ref<Node> child, int index) {
    if (auto n = child.ref_cast<SCWFCGraphNode>()) {
        // update_all_adjacencies(n);
    }
}

void SCWFC::update_all_adjacencies(Ref<SCWFCGraphNode>& n, float radius) {
    Sphere s = n->get_bounding_sphere();
    s.radius = radius;
    for (auto& c : get_children()) {
        auto graph_node = c.ref_cast<SCWFCGraphNode>();
        if (graph_node && c != n) {
            if (intersect(graph_node->get_bounding_sphere(), s)) {
                m_data->graph.add_edge((wfc::DGraphNode*)n.get(), (wfc::DGraphNode*)c.get(), 1);
            } else {
                m_data->graph.remove_edge((wfc::DGraphNode*)n.get(), (wfc::DGraphNode*)c.get());
            }
        }
    }
}

std::optional<glm::vec3> SCWFC::does_intersect_any(const Sphere& sph) {
    glm::vec3 net{};
    bool single = false;
    for (auto& c : get_children()) {
        auto graph_node = c.ref_cast<SCWFCGraphNode>();
        if (graph_node) {
            const Sphere& bounds = graph_node->get_bounding_sphere();
            if (intersect(bounds, sph)) {
                glm::vec3 c2c = bounds.center - sph.center;
                net += -glm::normalize(c2c) * (sph.radius + bounds.radius);
                single = true;
            }
        }
    }
    return single ? std::optional<glm::vec3>{net} : std::optional<glm::vec3>{};
}


void SCWFCNodeEditor::show_editor(Node* node) {
    SCWFC* n = dynamic_cast<SCWFC*>(node);
    if (n) {

    }
}

struct SCWFCEditor::Data {
    wfc::WFCSolver solver;

    std::vector<wfc::Pattern> patterns;
};

void SCWFCEditor::show_editor_tool() {
    ImGui::Begin("SCWFCEditor", &m_is_open);

    if (m_scwfc_node) {
        ImGui::Text("Selected %s", m_scwfc_node->name.c_str());

        if (ImGui::Button("LoadDB")) {
            load_obj_db();
        }
        if (ImGui::Button("Spawn")) {
            sc_propagate_from(dynamic_cast<SCWFCGraphNode*>(m_editor->get_selected_node()));
        }
        if (ImGui::Button("Reset")) {
            m_scwfc_node->reset();
        }
        ImGui::Separator();
        static int steps = 1;
        ImGui::InputInt("Steps", &steps);
        ImGui::SameLine();
        if (ImGui::Button("Solve")) {
            wfc_solve(steps);
        }
        if (m_internal->solver.next_node == nullptr) {
            if (ImGui::Button("Select current node as solver start")) {
                m_internal->solver.next_node = dynamic_cast<wfc::DGraphNode*>(m_editor->get_selected_node());
            }
        }

        if (ImGui::TreeNode("Patterns")) {

            ImGui::TreePop();
        }

    } else {
        ImGui::Text("Please select a SCWFC node");
    }

    ImGui::End();
}

void SCWFCEditor::load_obj_db() {
    auto cube0 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);

    auto cube1 = ResourceManager::get_singleton().get_model(fs::path("models") / "Wagon.obj", false);
    // cube1->materials[0]->diffuse = glm::vec3{1};

    auto cube2 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube2->materials[0]->diffuse = glm::vec3{1, 0, 0};

    obj_db = std::make_shared<SCWFCObjectMetadataDB>();

    obj_db->add_model(cube0, 10);
    obj_db->add_model(cube1, 11);
    obj_db->add_model(cube2, -1);

    wfc::Pattern PA{wfc::Value{10}, {wfc::Value{11}, wfc::Value{11}}};
    wfc::Pattern PB{wfc::Value{11}, {wfc::Value{10}}};

    m_internal->patterns = {PA, PB};
}

void SCWFCEditor::on_selected_node(Node* node) {
    if (node) {
        Ref<SCWFC> n = node->get_ref<SCWFC>();
        if (n) {
            m_scwfc_node = n;
            m_internal = std::make_shared<SCWFCEditor::Data>();
            m_internal->solver.graph = &m_scwfc_node->m_data->graph;
        }
    }
}

void SCWFCEditor::sc_propagate_from(SCWFCGraphNode* node) {
    if (m_scwfc_node && obj_db) {

        const glm::vec2 pos = uniform_disk(uniform2d());
        glm::vec3 offset = glm::vec3{pos.x, 0, pos.y};
        if (node) {
            Sphere sph({}, 1);
            sph.center = node->get_position() + offset;
            auto o_offset = m_scwfc_node->does_intersect_any(sph);
            if (o_offset) offset = *o_offset;
            offset += node->get_position();
        }

        auto nnode = m_scwfc_node->create_child_node<SCWFCGraphNode>("SCWFCGraphNode", m_scwfc_node.get());
        nnode->set_model(obj_db->get_model_for_id(-1));


        std::vector<const wfc::Pattern*> dest(m_internal->patterns.size());
        std::transform(m_internal->patterns.begin(), m_internal->patterns.end(), dest.begin(),
            [](auto &elem){ return &elem; }
        );
        nnode->domain = dest;

        m_editor->set_selected_node(nnode.get());

        nnode->set_position(offset);

        m_scwfc_node->update_all_adjacencies(nnode, 5.f);
    }
}

void SCWFCEditor::wfc_solve(int steps) {
    if (m_scwfc_node) {
        int cnt = 0;
        auto c_callback = [this](wfc::DGraphNode* node) -> void {
            auto* s_node = dynamic_cast<SCWFCGraphNode*>(node);
            assert(s_node);
            s_node->set_model(
                obj_db->get_model_for_id(node->domain[0]->cell_value.val)
            );
        };
        while(m_internal->solver.can_continue() && cnt++ < steps) {
            m_internal->solver.step_wfc(c_callback);
        }
    }
}

}