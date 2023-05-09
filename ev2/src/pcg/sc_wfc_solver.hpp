/**
 * @file sc_wfc_solver.hpp
 * @brief SCWFC Solver. places nodes and solves their domains. Also updates node models
 * @date 2023-05-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_PCG_SC_WFC_SOLVER_HPP
#define EV2_PCG_SC_WFC_SOLVER_HPP

#include <memory>
#include <random>
#include "pcg/wfc.hpp"
#include "sc_wfc.hpp"
#include "object_database.hpp"

namespace ev2::pcg {

class SCWFCSolver {
public:
    SCWFCSolver(SCWFC& scwfc_node, 
                std::shared_ptr<ObjectMetadataDB> obj_db, 
                std::random_device& rd,
                std::shared_ptr<renderer::Drawable> unsolved_drawable);

    Ref<SCWFCGraphNode> sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass);

    void wfc_solve(int steps);

    void update_node_model(wfc::DGraphNode* node);

    bool can_continue() const noexcept;

    void set_seed_node(wfc::DGraphNode* node);

private:
    SCWFC& scwfc_node;
    std::mt19937 m_mt;
    std::shared_ptr<ObjectMetadataDB> obj_db;
    std::unique_ptr<wfc::WFCSolver> wfc_solver;
    std::shared_ptr<renderer::Drawable> unsolved_drawable;
};

} // namespace ev2::pcg

#endif // EV2_PCG_SC_WFC_SOLVER_HPP