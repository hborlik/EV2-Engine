#include <scene/procedural_grid.hpp>

#include <pcg/grid.hpp>
#include <pcg/wfc.hpp>
#include <ui/imgui.hpp>

namespace ev2 {

struct ProceduralGrid::Data {
    Data(int w, int h) : grid{w, h} {}

    pcg::NodeGrid grid;
};

ProceduralGrid::ProceduralGrid() {
    
}

void ProceduralGrid::generate(int n) {
    m_data = std::make_unique<Data>(n, n);
}

void ProceduralGrid::on_init() {
    generate(10);
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