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

namespace ev2::pcg {

class ObjectMetadataDB;

class ProceduralGrid : public Node {
public:
    explicit ProceduralGrid(std::string name) : Node{std::move(name)} {}

    void generate(int n);

    void on_init() override;

private:
    float m_grid_spacing = 1.f;

private:
    struct Data;
    std::shared_ptr<Data> m_data{};
    std::shared_ptr<ObjectMetadataDB> obj_db;
};

class ProceduralGridEditor : public NodeEditorT<ProceduralGrid> {
public:
    void show_editor(Node* node) override;
};

}

#endif // EV2_PROCEDURAL_GRID_HPP