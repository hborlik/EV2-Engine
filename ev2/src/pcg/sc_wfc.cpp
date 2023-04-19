#include <pcg/sc_wfc.hpp>

#include <filesystem>
#include <fstream>

#include <pcg/wfc.hpp>
#include <renderer/renderer.hpp>
#include <ui/imgui.hpp>

#include <scene/serializers.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace ev2 {

class SCWFCGraphNode : public VisualInstance, pcg::DNode {
public:
    explicit SCWFCGraphNode(const std::string &name) : VisualInstance{name}, pcg::DNode{name, uuid_hash} {}
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
    auto db = std::make_unique<SCWFCObjectDatabase>();

    std::string json_str = read_file(path);

    return db;
}

//////

struct SCWFC::Data {
    Data() : graph{}, solver{&graph} {}

    pcg::SparseGraph<pcg::DNode> graph;
    pcg::WFCSolver solver;
};

void SCWFC::wfc_solve(int steps) {
    int cnt = 0;
    while(m_data->solver.can_continue() && cnt++ < steps) {
        m_data->solver.step_wfc();
    }
}

void SCWFC::sc_spawn_points(int n) {

}

void SCWFC::sc_propagate_from(Ref<Node> node) {

}

void SCWFC::reset() {
    m_data = std::make_shared<Data>();
}

void SCWFC::on_init() {
    reset();
}

void SCWFC::on_child_removed(Ref<Node> child) {

}


void SCWFCEditor::show_editor(Node* node) {
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

}