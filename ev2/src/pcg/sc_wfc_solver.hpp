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

#include <unordered_set>
#include "events/notifier.hpp"
#include "evpch.hpp"

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

    void node_check_and_update(SCWFCGraphNode* s_node);

    bool can_continue() const noexcept;

    void set_seed_node(Ref<SCWFCGraphNode> node);

    std::shared_ptr<ObjectMetadataDB> database() const noexcept {return obj_db;}

    auto get_boundary_size() const noexcept {return m_boundary.size();}

    auto get_discovered_size() const noexcept {return m_discovered.size();}

    void notify_node_added(SCWFCGraphNode* node) {}

    void notify_node_removed(SCWFCGraphNode* node) {
        m_discovered.erase(node->get_ref<SCWFCGraphNode>());
    }

public:
    DelegateListener<SCWFCGraphNode*> node_removed_listener{};
    DelegateListener<SCWFCGraphNode*> node_added_listener{};

private:
    SCWFC& scwfc_node;
    std::mt19937 m_mt;
    std::shared_ptr<ObjectMetadataDB> obj_db;
    std::unique_ptr<wfc::WFCSolver> wfc_solver;
    std::shared_ptr<renderer::Drawable> unsolved_drawable;

    std::queue<Ref<SCWFCGraphNode>> m_boundary{};
    std::unordered_set<Ref<SCWFCGraphNode>> m_discovered{};
};

} // namespace ev2::pcg

#endif // EV2_PCG_SC_WFC_SOLVER_HPP