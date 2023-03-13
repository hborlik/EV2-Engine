/**
 * @file grid.hpp
 * @brief 
 * @date 2023-03-03
 * 
 */
#ifndef EV2_GRID_HPP
#define EV2_GRID_HPP

#include <stdexcept>
#include <memory>
#include <iostream>

#include <pcg/wfc.hpp>

namespace pcg {


class NodeGrid {
public:
    NodeGrid(int width, int height): width{ width }, height{ height } {
        if (width <= 0 || height <= 0)
            throw std::logic_error("Grid::Grid invalid size argument");
        for (int i = 0; i < height; ++i)
            for(int j = 0; j < width; ++j) {
                const int n_ind = i * width + j;
                m_grid.emplace_back(std::make_unique<DNode>("n(" + std::to_string(i) + ", " + std::to_string(j) + ")", n_ind+1));
                DNode* node = m_grid[n_ind].get();

                if (j > 0) m_sparse_graph.add_edge(node, m_grid[i * width + (j-1)].get(), 1.f);
                if (i > 0) m_sparse_graph.add_edge(node, m_grid[(i-1) * width + j].get(), 1.f);
            }
    }

    ~NodeGrid() {
        
    }

    const DNode* at(int x, int y) const {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return m_grid[x * width + y].get();
    }

    DNode* at(int x, int y) {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return m_grid[x * width + y].get();
    }

    auto& get_graph() {return m_sparse_graph;}

public:
    const int width, height;

private:
    std::vector<std::unique_ptr<DNode>> m_grid;
    SparseGraph<DNode> m_sparse_graph;
};

}

#endif // EV2_GRID_HPP