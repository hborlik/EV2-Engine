#include <scene/procedural_grid.hpp>

#include <pcg/grid.hpp>
#include <pcg/wfc.hpp>

namespace ev2 {

struct ProceduralGrid::Data {
    Data(int w, int h) : grid{w, h} {}

    pcg::NodeGrid grid;
};

ProceduralGrid::ProceduralGrid() {
    
}

void ProceduralGrid::on_init() {
    m_data = std::make_unique<Data>(10, 10);
}

}