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

#include "evpch.hpp"

#include "application.hpp"
#include "events/notifier.hpp"
#include "pcg/wfc.hpp"
#include "sc_wfc.hpp"
#include "object_database.hpp"

namespace ev2::pcg {

enum class NewDomainMode {
    Full = 0,
    Dependent
};

enum class RefreshNeighborhoodRadius {
    Never = 0,
    Always
};

enum class DiscoveryMode {
    EntropyOrder = 0,
    DiscoveryOrder
};

struct SCWFCSolverArgs {
    NewDomainMode domain_mode = NewDomainMode::Dependent;
    wfc::SolverValidMode validity_mode = wfc::SolverValidMode::Correct;
    DiscoveryMode solving_order = DiscoveryMode::DiscoveryOrder;
    
    RefreshNeighborhoodRadius node_neighborhood = RefreshNeighborhoodRadius::Never;

    float neighbor_radius_fac = 3.f;
    bool allow_revisit_node = false;
};

class SCWFCSolver {
private:
    struct BoundaryQueue  {
        virtual ~BoundaryQueue() = default;
        virtual void push(Ref<SCWFCGraphNode> node) = 0;
        virtual Ref<SCWFCGraphNode> pop_top() = 0;
        virtual std::size_t size() = 0;
    };
    struct BoundaryQueueFIFO;
    struct BoundaryQueueEntropy;

    SCWFCSolver(SCWFC& scwfc_node,
                std::shared_ptr<ObjectMetadataDB> obj_db,
                std::unique_ptr<std::mt19937> mt,
                std::shared_ptr<renderer::Mesh> unsolved_drawable,
                const SCWFCSolverArgs& args,
                std::unique_ptr<SCWFCSolver::BoundaryQueue> boundary_queue,
                std::unique_ptr<wfc::WFCSolver> wfc_solver);

public:
    static std::unique_ptr<SCWFCSolver> make_solver(
        SCWFC& scwfc_node, std::shared_ptr<ObjectMetadataDB> obj_db,
        std::random_device& rd,
        std::shared_ptr<renderer::Mesh> unsolved_drawable,
        const SCWFCSolverArgs& args);

    void sc_propagate(int n, int brf, float mass);

    std::vector<Ref<SCWFCGraphNode>> sc_propagate_from(SCWFCGraphNode* node, int n, float repulsion);

    void wfc_solve(int steps);

    void reevaluate_validity();

    void node_check_and_update(SCWFCGraphNode* s_node);

    Ref<SCWFCGraphNode> spawn_unsolved_node();

    /**
     * @brief Create a domain from a set of class ids. 
     *      This will create a set of wfc::Vals with patterns that are used for given
     *      class_id s
     * 
     * @param class_ids 
     * @return std::unordered_set<wfc::Val> 
     */
    std::unordered_set<wfc::Val> domain_from_class_ids(const std::unordered_set<int>& class_ids);

    glm::vec3 weighted_average_diagonal(const std::vector<wfc::Val>& domain);

    bool can_continue() const noexcept;

    void set_seed_node(Ref<SCWFCGraphNode> node);

    std::shared_ptr<ObjectMetadataDB> database() const noexcept {return obj_db;}

    void notify_node_added(SCWFCGraphNode* node) {}

    void notify_node_removed(SCWFCGraphNode* node) {
        m_discovered.erase(node->get_ref<SCWFCGraphNode>());
    }

    std::size_t get_boundary_size() const noexcept;
    std::size_t get_discovered_size() const noexcept;

private:
    struct LessThanByEntropy {
        LessThanByEntropy(wfc::WFCSolver* solver) : wfc_solver{solver} {}

        bool operator()(const Ref<SCWFCGraphNode>& lhs, const Ref<SCWFCGraphNode>& rhs) const
        {
            // low to high values
            return wfc_solver->node_entropy(lhs.get()) > wfc_solver->node_entropy(rhs.get());
        }

        wfc::WFCSolver* wfc_solver;
    };

public:
    DelegateListener<SCWFCGraphNode*> node_removed_listener{};
    DelegateListener<SCWFCGraphNode*> node_added_listener{};

    bool b_on_terrain = false;
    Application* app = nullptr;

private:
    SCWFC& scwfc_node;
    std::unique_ptr<std::mt19937> m_mt;
    std::shared_ptr<ObjectMetadataDB> obj_db;
    std::unique_ptr<wfc::WFCSolver> wfc_solver;
    std::shared_ptr<renderer::Mesh> unsolved_drawable;

    std::unique_ptr<BoundaryQueue> m_boundary;
    std::queue<Ref<SCWFCGraphNode>> m_boundary_expanding{};
    std::unordered_set<Ref<SCWFCGraphNode>> m_discovered{};

    SCWFCSolverArgs m_args{};
};

} // namespace ev2::pcg

#endif // EV2_PCG_SC_WFC_SOLVER_HPP