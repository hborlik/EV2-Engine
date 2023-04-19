#include <pcg/procedural_grid.hpp>

#include <filesystem>

#include <resource.hpp>
#include <pcg/grid.hpp>
#include <pcg/sc_wfc.hpp>
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

    

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            pcg::DGraphNode* d_node = m_data->grid.at(i, j);
            if (d_node->domain.size() >= 1) {
                auto nnode = create_child_node<VisualInstance>("pcg visual [" + std::to_string(i) + ", " + std::to_string(j) + "]");
                nnode->set_model(d_node->domain.size() == 1 ? obj_db->get_model_for_id(d_node->domain[0]->cell_value.val) : obj_db->get_model_for_id(-1));
                nnode->set_position(glm::vec3{i * m_grid_spacing, 0, j * m_grid_spacing});
            }
        }
}

void ProceduralGrid::on_init() {
    // generate(10);

    auto cube0 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);

    auto cube1 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube1->materials[0]->diffuse = glm::vec3{1};

    auto cube2 = ResourceManager::get_singleton().get_model(fs::path("models") / "cube.obj", false);
    cube2->materials[0]->diffuse = glm::vec3{1, 0, 0};

    obj_db = std::make_shared<SCWFCObjectDatabase>();

    obj_db->add_model(cube0, 10);
    obj_db->add_model(cube1, 11);
    obj_db->add_model(cube2, -1);
}

void ProceduralGridEditor::show_editor(Node* node) {
    ProceduralGrid* n = dynamic_cast<ProceduralGrid*>(node);
    if (n) {
        if (ImGui::Button("Generate")) {
            n->generate(30);
        }
    }
}

}