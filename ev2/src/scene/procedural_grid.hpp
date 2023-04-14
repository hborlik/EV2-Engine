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
#include <ui/ui.hpp>

namespace ev2 {

class ProceduralGrid : public Node {
public:
    ProceduralGrid();

    void generate(int n);

    void on_init() override;

private:
    struct Data;
    std::unique_ptr<Data> m_data;
};

class ProceduralGridEditor : public NodeEditorT<ProceduralGrid> {
public:
    void show_editor(Node* node) override;
};

}

#endif // EV2_PROCEDURAL_GRID_HPP