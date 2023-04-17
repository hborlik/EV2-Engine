#include <scene/procedural_grid.hpp>

#include <filesystem>

#include <resource.hpp>
#include <pcg/grid.hpp>
#include <pcg/wfc.hpp>
#include <ui/imgui.hpp>

namespace fs = std::filesystem;

namespace ev2 {

struct ProceduralGrid::Data {
    Data(int w, int h) : grid{w, h}, solver{&grid.get_graph()} {}

    pcg::NodeGrid grid;

    pcg::WFCSolver solver;
};

void ProceduralGrid::generate(int n) {

    auto l = get_children();
    for (auto c : l)
        c->destroy();

    m_data = std::make_shared<Data>(n, n);

    pcg::Pattern PA{pcg::Value{10}, {pcg::Value{11}, pcg::Value{11}}};
    pcg::Pattern PB{pcg::Value{11}, {pcg::Value{10}}};

    std::vector<const pcg::Pattern*> patterns{&PA, &PB};

    m_data->grid.reset_domains(patterns);

    m_data->solver.next_node = m_data->grid.at(0, 0);

    while(m_data->solver.can_continue()) {
        m_data->solver.step_wfc();
    }

    auto cube = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj");

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            pcg::DNode* d_node = m_data->grid.at(i, j);
            if (d_node->domain.size() != 0) {
                auto nnode = create_child_node<VisualInstance>("pcg visual []" + std::to_string(i) + ", " + std::to_string(j) + "]");
                nnode->set_model(cube);
                nnode->set_position(glm::vec3{i * m_grid_spacing, 0, j * m_grid_spacing});
            }
        }
}

void ProceduralGrid::on_init() {
    // generate(10);
}

void ProceduralGridEditor::show_editor(Node* node) {
    ProceduralGrid* n = dynamic_cast<ProceduralGrid*>(node);
    if (n) {
        if (ImGui::Button("Generate")) {
            n->generate(10);
        }
    }
}

}