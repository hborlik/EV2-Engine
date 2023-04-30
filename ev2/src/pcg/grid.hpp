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

namespace ev2::pcg {


class NodeGrid {
public:
    NodeGrid(int width, int height): width{ width }, height{ height }, m_grid{}, m_sparse_graph{false} {
        if (width <= 0 || height <= 0)
            throw std::logic_error("Grid::Grid invalid size argument");
        for (int i = 0; i < height; ++i)
            for(int j = 0; j < width; ++j) {
                const int n_ind = i * width + j;
                m_grid.emplace_back(std::make_unique<wfc::DGraphNode>("n(" + std::to_string(i) + ", " + std::to_string(j) + ")", n_ind+1));
                wfc::DGraphNode* node = m_grid[n_ind].get();

                if (j > 0) m_sparse_graph.add_edge(node, m_grid[i * width + (j-1)].get(), 1.f);
                if (i > 0) m_sparse_graph.add_edge(node, m_grid[(i-1) * width + j].get(), 1.f);
            }
    }

    ~NodeGrid() {
        
    }

    /**
     * @brief reset the domains available for each node in grid
     * 
     * @param patterns 
     */
    void reset_domains(const std::vector<const wfc::Pattern*>& patterns) {
        for (int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x) {
                auto* n = at(x, y);
                n->domain = patterns;
            }
        }
    }

    const wfc::DGraphNode* at(int x, int y) const {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return m_grid[x * width + y].get();
    }

    wfc::DGraphNode* at(int x, int y) {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return m_grid[x * width + y].get();
    }

    auto& get_graph() {return m_sparse_graph;}

private:
    friend std::string to_string(const NodeGrid& node_grid) {
        std::stringstream str;
        int max_w = 0;

        auto print_domain = [](const wfc::DGraphNode* n) -> std::string {
            std::stringstream dstr;
            dstr << "[";
            for (const auto& v : n->domain)
                dstr << v->pattern_class.val << ",";
            dstr << "]";
            return dstr.str();
        };

        for (int y = 0; y < node_grid.height; ++y)
            for(int x = 0; x < node_grid.width; ++x) {
                const wfc::DGraphNode* n = node_grid.at(x, y);
                max_w = std::max(max_w, (int)print_domain(n).length());
            }
        
        for (int y = 0; y < node_grid.height; ++y) {
            for(int x = 0; x < node_grid.width; ++x) {
                const wfc::DGraphNode* n = node_grid.at(x, y);
                str << std::setw(3) << n->node_id;
                str << std::setw(max_w) << print_domain(n);            
            }
            str << "\n";
        }
        return str.str();
    }

public:
    const int width, height;

private:
    std::vector<std::unique_ptr<wfc::DGraphNode>> m_grid;
    wfc::SparseGraph<wfc::DGraphNode> m_sparse_graph;
};

}

#endif // EV2_GRID_HPP