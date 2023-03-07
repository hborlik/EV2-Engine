/**
 * @file procedural_grid.hpp
 * @brief 
 * @date 2023-03-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_PROCEDURAL_GRID_HPP
#define EV2_PROCEDURAL_GRID_HPP

#include <scene/node.hpp>

namespace ev2 {

class ProceduralGrid : public Node {
public:
    ProceduralGrid();

    void generate(int n);

private:
    
};

}

#endif // EV2_PROCEDURAL_GRID_HPP