#include "sc_wfc_solver.hpp"
#include <algorithm>
#include <iterator>
#include <unordered_set>



#include "glm/fwd.hpp"
#include "pcg/sc_wfc.hpp"
#include "pcg/wfc.hpp"
#include "core/reference_counted.hpp"
#include "resource.hpp"
#include "pcg/distributions.hpp"
#include "pcg/object_database.hpp"
#include "scene/node.hpp"
#include "timer.hpp"

namespace ev2::pcg {

struct SCWFCSolver::BoundaryQueueEntropy : public SCWFCSolver::BoundaryQueue {

    BoundaryQueueEntropy(wfc::WFCSolver* solver) : m_boundary{LessThanByEntropy{solver}} {

    }

    void push(Ref<SCWFCGraphNode> node) override {
        m_boundary.push(node);
    }

    Ref<SCWFCGraphNode> pop_top() override {
        auto top = m_boundary.top();
        m_boundary.pop();
        return top;
    }

    std::size_t size() override {
        return m_boundary.size();
    }

    std::priority_queue<Ref<SCWFCGraphNode>, std::vector<Ref<SCWFCGraphNode>>, LessThanByEntropy> m_boundary;
};

struct SCWFCSolver::BoundaryQueueFIFO : public SCWFCSolver::BoundaryQueue {

    void push(Ref<SCWFCGraphNode> node) override {
        m_boundary.push(node);
    }

    Ref<SCWFCGraphNode> pop_top() override {
        auto top = m_boundary.front();
        m_boundary.pop();
        return top;
    }

    std::size_t size() override {
        return m_boundary.size();
    }

    std::queue<Ref<SCWFCGraphNode>> m_boundary;
};

SCWFCSolver::SCWFCSolver(SCWFC& scwfc_node,
                         std::shared_ptr<ObjectMetadataDB> obj_db,
                         std::unique_ptr<std::mt19937> mt,
                         std::shared_ptr<renderer::Mesh> unsolved_drawable,
                         const SCWFCSolverArgs& args,
                         std::unique_ptr<SCWFCSolver::BoundaryQueue> boundary_queue,
                         std::unique_ptr<wfc::WFCSolver> wfc_solver)
    : node_removed_listener{decltype(node_added_listener)::delegate_t::create<
          SCWFCSolver, &SCWFCSolver::notify_node_removed>(this)},
      node_added_listener{decltype(node_added_listener)::delegate_t::create<
          SCWFCSolver, &SCWFCSolver::notify_node_added>(this)},
          
      scwfc_node{scwfc_node},
      m_mt{std::move(mt)},
      obj_db{obj_db},
      wfc_solver{std::move(wfc_solver)},
      unsolved_drawable{unsolved_drawable},
      m_boundary{std::move(boundary_queue)},
      m_args{args}
       {}

std::unique_ptr<SCWFCSolver> SCWFCSolver::make_solver(
    SCWFC& scwfc_node, std::shared_ptr<ObjectMetadataDB> obj_db,
    std::random_device& rd,
    std::shared_ptr<renderer::Mesh> unsolved_drawable,
    const SCWFCSolverArgs& args) {

    
    std::unique_ptr<BoundaryQueue> boundary_queue;
    auto mt = std::make_unique<std::mt19937>(rd());
    auto wfc_solver = std::make_unique<wfc::WFCSolver>(
        scwfc_node.get_graph(), obj_db->make_pattern_map(), *mt.get(),
        args.allow_revisit_node, args.validity_mode);

    switch (args.solving_order) {
        case DiscoveryMode::EntropyOrder:
            boundary_queue =
                std::make_unique<BoundaryQueueEntropy>(wfc_solver.get());
            break;
        case DiscoveryMode::DiscoveryOrder:
            boundary_queue = std::make_unique<BoundaryQueueFIFO>();
            break;
    }

    return std::unique_ptr<SCWFCSolver>{new SCWFCSolver{
        scwfc_node, obj_db, std::move(mt), unsolved_drawable, args,
        std::move(boundary_queue), std::move(wfc_solver)}};
}

void SCWFCSolver::sc_propagate(int n, int brf, float repulsion) {
    // spawn seed node if empty
    if (m_boundary_expanding.size() < 1)
        spawn_unsolved_node();
    
    // expand the boundary for n iterations
    for (int cnt = 0; cnt < n; ++cnt) {
        if (m_boundary_expanding.size() < 1)
            break;
        Ref<SCWFCGraphNode> node = m_boundary_expanding.front();
        m_boundary_expanding.pop();

        if (!node || (node && node->is_destroyed()))
            continue;

        auto new_nodes = sc_propagate_from(node.get(), brf, repulsion);
        for (const auto& e : new_nodes)
            m_boundary_expanding.push(e);
    }
}


std::vector<Ref<SCWFCGraphNode>> SCWFCSolver::sc_propagate_from(SCWFCGraphNode* node, int n, float repulsion) {
    if (n <= 0)
        return {};

    if (node && node->domain.size() < 1) // node should not have an empty domain
        return {};

    struct Spawn {
        std::vector<glm::vec3> positions;
        std::unordered_set<int> domain_class_ids{};
        std::unordered_set<wfc::Val> new_domain_vals{};
        float en_radius = 0.f;
    };

    // all available class_ids
    auto [p_itr, p_end] = obj_db->get_patterns_iterator();
    std::vector<int> dest(obj_db->patterns_size());
    std::transform(p_itr, p_end, dest.begin(),
        [](auto &elem){ return elem.second.pattern_type; }
    );
    const std::unordered_set<int> all_class_ids = {dest.begin(), dest.end()};
    
    std::vector<Ref<SCWFCGraphNode>> nnodes{};

    // Entry for each spawned node that will be spawned
    // set of required neighbor class ids for the propagating node
    // will be set of all possible patterns when node is null
    std::vector<Spawn> node_spawns{};

    // if spawning on an existing node
    if (node) {
        node_spawns.clear();

        // since we are propagating from an existing node, spawn a
        // node that possibly contains set of valid neighbors for that existing
        // node.

        // get repulsion
        const glm::vec3 r_vec = repulsion * ((repulsion > 0) ? scwfc_node.node_repulsion(node) : glm::vec3{});

        const float entropy = wfc_solver->node_entropy(node);
        for (auto domain_val : node->domain) {
            if (auto pattern = obj_db->get_pattern(domain_val.value);
                pattern != nullptr) {  // pattern_id is valid
                Spawn sp{};

                const float success = pattern->weight / (entropy ? entropy : 1.f);
                // does this current node need more nodes to become valid?
                switch(m_args.domain_mode) {
                    case NewDomainMode::Full:
                        sp.domain_class_ids = all_class_ids;
                    break;
                    
                    case NewDomainMode::Dependent:
                        if (wfc_solver->valid(domain_val, node) && binomial_trial(success, *m_mt.get())) {
                            // add all required classes
                            sp.domain_class_ids.insert(pattern->required_types.begin(), pattern->required_types.end());
                        } else {
                            sp.domain_class_ids = all_class_ids;
                        }
                    break;
                }

                // wfc::Val domain_val = wfc_solver->weighted_pick_domain(node);
                sp.new_domain_vals = domain_from_class_ids(sp.domain_class_ids);
                sp.en_radius = glm::length(weighted_average_diagonal({sp.new_domain_vals.begin(), sp.new_domain_vals.end()})) / 2.f;
                
                // all ObjectData for type
                auto [obj_p, obj_e] = obj_db->objs_for_id(pattern->pattern_type);                 

                // random trials for placements
                for (int i = 0; i < std::ceil(n * success); ++i) {
                    const auto& [id, obj] = *select_randomly(obj_p, obj_e, *m_mt.get());
                    const auto n_props = obj.propagation_patterns.size();
                    if (!binomial_trial(success / n_props, *m_mt.get()))
                        continue;

                    const auto& obb = *select_randomly(obj.propagation_patterns.begin(), obj.propagation_patterns.end(), *m_mt.get());
                    // values within 3 standard deviations account for 99.7% of samples
                    std::normal_distribution<float> dist_x{0, obb.half_extents.x / 3};
                    std::normal_distribution<float> dist_y{0, obb.half_extents.y / 3};
                    std::normal_distribution<float> dist_z{0, obb.half_extents.z / 3};

                    glm::vec3 pos_in_obb {
                        dist_x(*m_mt.get()),
                        dist_y(*m_mt.get()),
                        dist_z(*m_mt.get())
                    };

                    glm::vec3 position = node->get_linear_transform() * obb.get_transform() * glm::vec4{pos_in_obb, 1.f};
                    glm::vec3 prop_dir = node->get_linear_transform() * obb.get_transform() * glm::vec4{pos_in_obb, 0.f};

                    glm::vec3 final_pos = position + r_vec;

                    final_pos.y = 0; // TODO ground placement rules

                    if (!(glm::dot(glm::normalize(r_vec), glm::normalize(prop_dir)) < -.2))
                        sp.positions.push_back(final_pos);
                }
                node_spawns.emplace_back(std::move(sp));
            }
        }

    } else {
        node_spawns.emplace_back(
            Spawn{
                {glm::vec3{0}}, 
                all_class_ids
            });
    }


    for (const auto& spawn : node_spawns) {
        for (const auto& pos : spawn.positions) {
            auto nnode = scwfc_node.create_child_node<SCWFCGraphNode>("SGN " + std::to_string(scwfc_node.get_n_children()));
            // populate domain of new node
            nnode->domain = {spawn.new_domain_vals.begin(), spawn.new_domain_vals.end()};
            const float en_radius = spawn.en_radius;

            // get repulsion
            Sphere sph(pos, en_radius);
            glm::vec3 final_pos = sph.center;
            
            // const glm::vec3 r_vec = repulsion * ((repulsion > 0) ? scwfc_node.sphere_repulsion(sph) : glm::vec3{});
            // final_pos += r_vec;

            // final_pos.y = 0; // TODO placement rules

            nnode->set_radius(en_radius);
            nnode->set_neighborhood_radius(en_radius * m_args.neighbor_radius_fac);
            nnode->set_position(final_pos);
            if (node)
                nnode->set_rotation(node->get_rotation());

            // update visual node state, may be destroyed
            node_check_and_update(nnode.get());
            
            if (!nnode->is_destroyed())
                nnodes.push_back(nnode);
        }
    }

    // if (!nnodes.empty())
    //     node = select_randomly(nnodes.begin(), nnodes.end(), *m_mt.get())->get();

    // return node->get_ref<SCWFCGraphNode>();
    return nnodes;
}

void SCWFCSolver::wfc_solve(int steps) {
    wfc::WFCSolver::entropy_callback_t entropy_func =
        [this](auto* prop_node, auto* on_node) -> float {
            auto* prop_node_scene =
                dynamic_cast<const SCWFCGraphNode*>(prop_node);
            auto* on_node_scene = dynamic_cast<const SCWFCGraphNode*>(on_node);
            return wfc_solver->node_entropy(on_node) -
                    1 / glm::length(prop_node_scene->get_position() -
                                    on_node_scene->get_position());
        };

    wfc::WFCSolver::propagate_callback_t propagate_update = [this](auto* node) -> void {
        auto* s_node = dynamic_cast<SCWFCGraphNode*>(node);
        if (s_node)
            node_check_and_update(s_node);
    };

    // wfc_solver->set_entropy_func(entropy_func);
    wfc_solver->set_propagate_callback_func(propagate_update);
    for (int cnt = 0; cnt < steps;) {
        // Timer timer{"wfc_solve", false};
        if (m_boundary->size() < 1)
            break;
        auto n = m_boundary->pop_top();

        if (!n || (n && n->is_destroyed())) // m_boundary can contain destroyed nodes
            continue;

        // add all adjacent nodes to the solver boundary
        auto adjacent = scwfc_node.get_graph()->adjacent_nodes(n.get());
        // int domain_size = n->domain.size();
        // int adjacent_size = adjacent.size();
        std::for_each(
            adjacent.begin(), adjacent.end(), [this](auto* dgn) -> void {

                auto s_node = Ref{dynamic_cast<SCWFCGraphNode*>(dgn)};
                const bool not_found = m_discovered.find(s_node) == m_discovered.end();
                if (not_found) {
                    m_boundary->push(s_node);
                    m_discovered.insert(s_node);
                }
            });

        // solve node
        if (!n->is_finalized()) 
            wfc_solver->step_wfc((wfc::DGraphNode*)n.get());

        // node_check_and_update(n.get());
        ++cnt;

        // timer.stop();
        // std::cout << domain_size << "\t" << adjacent_size << "\t" << timer.elapsed_ms() << "\n";
    }
}

void SCWFCSolver::reevaluate_validity() {
    // note that m_discovered is modified when node is deleted
    for (auto itr = m_discovered.begin(); itr != m_discovered.end();) {
        auto s_node = *(itr++);
        if (s_node && !s_node->is_finalized() && !wfc_solver->valid(s_node->domain[0], s_node.get())) {
            s_node->destroy();
        }
    }
}

void SCWFCSolver::node_check_and_update(SCWFCGraphNode* s_node) {
    assert(s_node);

    if (s_node->is_finalized()) // node is marked as locked by user
        return;

    if (s_node->is_destroyed()) // node is already destroyed
        return;

    auto model = unsolved_drawable;

    glm::vec3 position = s_node->get_world_position() * glm::vec3{1, 0, 1};
    if (app && b_on_terrain)
        position.y = app->get_terrain().height_query(position.x, position.z);

    if (s_node->domain.size() == 0) {
        // remove nodes with 0 valid objects in their domain from the scene
        s_node->destroy();
    } else if (s_node->domain.size() == 1) {
        float rotation_y = 0.f;

        const int pattern_id = s_node->domain[0].value;
        // obj_db will return all ObjestData's for a class id, but will
        // only use the first if it exists here.
        const wfc::Pattern* pattern = obj_db->get_pattern(pattern_id);
        if (pattern) {
            const ObjectData* obj{};
            if (auto [itr, end] = obj_db->objs_for_id(pattern->pattern_type);
                itr != end) {
                auto& [id, obj_data] = *select_randomly(itr, end, *m_mt.get());
                auto m = obj_data.loaded_model;
                if (m) {
                    model = m;
                    obj = &obj_data;
                }
            }

            if (obj) {
                switch (obj->axis_settings.y) {
                    case ObjectData::Orientation::Free:
                        rotation_y = uniform1d() * 2.f * M_PI;
                        break;
                    case ObjectData::Orientation::Stepped:
                        rotation_y = (int)(4 * uniform1d()) / 4.f * 2.f * M_PI;
                        break;
                    case ObjectData::Orientation::Lock:
                        break;
                }

                // rescale the object so that it fits withing the extent
                const auto aabb_scaled = obj->get_scaled_bounding_box();
                const float scale = obj->get_model_scale(); // scale uniformly
                const float radius = aabb_scaled.min_diagonal() / 2.f;


                s_node->set_model(model);
                s_node->set_scale(glm::vec3{scale});
                s_node->set_radius(radius);

                if (m_args.node_neighborhood == RefreshNeighborhoodRadius::Always)
                    s_node->set_neighborhood_radius(radius * m_args.neighbor_radius_fac);

                position.y += -aabb_scaled.pMin.y;

                s_node->rotate(glm::vec3{0, rotation_y, 0});
                s_node->set_world_position(position);
                s_node->set_solved();

                if (scwfc_node.intersects_any_solved_neighbor(Ref{ s_node })) {
                    // remove nodes whose final bounding volume intersects solved nodes
                    s_node->destroy();
                }

            }
        }
    } else {
        // keep model as unsolved drawable
        float extent{2 * s_node->get_bounding_sphere()
                            .radius};  // default to the same size
        const auto& aabb = model->bounding_box;
        const float scale = extent / glm::length(aabb.diagonal()); // scale uniformly
        // float radius = aabb.min_diagonal() * scale / 2.f;
        float radius = glm::length(weighted_average_diagonal(s_node->domain)) / 2.f;//wfc_solver->node_entropy(s_node) +

        s_node->set_model(model);
        s_node->set_scale(glm::vec3{scale});
        s_node->set_radius(radius);
        s_node->set_world_position(position);

        if (m_args.node_neighborhood == RefreshNeighborhoodRadius::Always)
            s_node->set_neighborhood_radius(radius * m_args.neighbor_radius_fac);
    }
}

Ref<SCWFCGraphNode> SCWFCSolver::spawn_unsolved_node() {
    std::unordered_set<int> domain_class_ids{};
    // populate domain with all available pattern_ids
    auto [p_itr, p_end] = obj_db->get_patterns_iterator();
    std::vector<int> dest(obj_db->patterns_size());
    std::transform(p_itr, p_end, dest.begin(),
        [](auto &elem){ return elem.second.pattern_type; }
    );
    domain_class_ids = {dest.begin(), dest.end()};

    auto domain = domain_from_class_ids(domain_class_ids);

    auto nnode = scwfc_node.create_child_node<SCWFCGraphNode>("SGN " + std::to_string(scwfc_node.get_n_children()));
    // populate domain of new node
    nnode->domain = {domain.begin(), domain.end()};

    float en_radius = glm::length(weighted_average_diagonal(nnode->domain)) / 2.f;
    nnode->set_radius(en_radius);
    nnode->set_neighborhood_radius(en_radius * m_args.neighbor_radius_fac);

    node_check_and_update(nnode.get());

    m_boundary_expanding.push(nnode);

    return nnode;
}

std::unordered_set<wfc::Val> SCWFCSolver::domain_from_class_ids(const std::unordered_set<int>& class_ids) {
    std::unordered_set<wfc::Val> domain{};
    for(auto class_id : class_ids) {
        // get all pattern_ids that represent the given class_id
        auto [p_itr, p_end] = obj_db->patterns_for_class_id(class_id);
        // pairs of <class_id, pattern_id>
        for (; p_itr != p_end; ++p_itr)
            domain.insert(wfc::Val{
                .type = class_id,
                .value = p_itr->second
            });
    }
    return domain;
}

glm::vec3 SCWFCSolver::weighted_average_diagonal(const std::vector<wfc::Val>& domain) {
    glm::vec3 total_diagonal{};
    float total_weights = 0.f;
    for (auto pattern_id : domain) {
        // every node in domain should be valid for this node.
        glm::vec3 class_size{};

        if (auto pattern = obj_db->get_pattern(pattern_id.value); pattern != nullptr) { // pattern_id is valid
            // get the objects for this class and average their size
            auto [o_itr, o_end] = obj_db->objs_for_id(pattern->pattern_type);
            auto size = (float)std::distance(o_itr, o_end);
            if (size > 0) {
                std::for_each(o_itr, o_end, [&class_size](auto& pair) -> void {
                    auto& [class_id, obj] = pair;
                    class_size += obj.get_scaled_bounding_box().min_diagonal();
                });
                class_size /= size;
            }

            // get the weight for this pattern
            float class_weight = pattern->weight;

            // weight the size of this class by pattern weight
            total_diagonal += class_weight * class_size;
            total_weights += class_weight;
        }
    }
    if (total_weights > 0)
        return total_diagonal / total_weights;
    return {};
}

bool SCWFCSolver::can_continue() const noexcept {
    return !(m_boundary->size() == 0);
}

void SCWFCSolver::set_seed_node(Ref<SCWFCGraphNode> node) {
    if (node)
        m_boundary->push(node);
}

std::size_t SCWFCSolver::get_boundary_size() const noexcept {return m_boundary->size();}

std::size_t SCWFCSolver::get_discovered_size() const noexcept {return m_discovered.size();}


} // ev2::pcg

#if 0
Ref<SCWFCGraphNode> SCWFCSolver::sc_propagate_from(SCWFCGraphNode* node, int n, int brf, float mass) {
    if (n <= 0)
        return {};
    
    const float radius = 2.f;
    const float n_radius = radius * 4.f;
    std::vector<Ref<SCWFCGraphNode>> nnodes{};
    for (int i = 0; i < n; ++i) {
        std::vector<glm::vec3> offsets{glm::vec3{0}};

        // set of required neighbor class ids for the propagating node
        // will be set of all possible patterns when node is null
        std::unordered_set<int> domain_class_ids{};

        // if spawning on an existing node
        if (node) {
            if (node->domain.size() < 1) // node should not have an empty domain
                return {};
            
            offsets.clear();

            // since we are propagating from an existing node, spawn a
            // node that contains set of valid neighbors for that existing
            // node.
            for (auto pattern_id : node->domain) {
                if (auto p = obj_db->get_pattern(pattern_id.value); p != nullptr) {
                    // add all required classes
                    domain_class_ids.insert(p->required_types.begin(), p->required_types.end());
                }
            }

            // random select a pattern_id from domain and an object data to use for placement patterns
            // TODO check that all of these return more than 0 elements
            wfc::Val domain_val = wfc_solver->weighted_pick_domain(node);
            if (auto pattern = obj_db->get_pattern(domain_val.value); pattern != nullptr) { // pattern_id is valid
                auto [obj_p, obj_e] = obj_db->objs_for_id(pattern->pattern_type);
                const auto& [id, obj] = *select_randomly(obj_p, obj_e, *m_mt.get());

                for (const auto& obb : obj.propagation_patterns) {
                    // values within 3 standard deviations account for 99.7% of samples
                    std::normal_distribution<float> dist_x{0, obb.half_extents.x / 3};
                    std::normal_distribution<float> dist_y{0, obb.half_extents.y / 3};
                    std::normal_distribution<float> dist_z{0, obb.half_extents.z / 3};

                    glm::vec3 pos_in_obb {
                        dist_x(*m_mt.get()),
                        dist_y(*m_mt.get()),
                        dist_z(*m_mt.get())
                    };
                    glm::vec3 offset = node->get_linear_transform() * obb.get_transform() * glm::vec4{pos_in_obb, 1.f};
                    offsets.push_back(offset);
                }
            }

        } else { // populate domain with all available pattern_ids
            auto [p_itr, p_end] = obj_db->get_patterns_iterator();
            std::vector<int> dest(obj_db->patterns_size());
            std::transform(p_itr, p_end, dest.begin(),
                [](auto &elem){ return elem.second.pattern_type; }
            );
            domain_class_ids = {dest.begin(), dest.end()};
        }

        std::unordered_set<wfc::Val> new_domain_vals = domain_from_class_ids(domain_class_ids);

        for (const auto& offset : offsets) {
            auto nnode = scwfc_node.create_child_node<SCWFCGraphNode>("SGN " + std::to_string(scwfc_node.get_n_children()));
            // populate domain of new node
            nnode->domain = {new_domain_vals.begin(), new_domain_vals.end()};
            float en_radius = wfc_solver->node_entropy(nnode.get()) + glm::length(weighted_average_diagonal(nnode.get())) / 2.f;

            // get repulsion
            Sphere sph(offset, en_radius);
            glm::vec3 pos = sph.center;
            pos += mass * scwfc_node.sphere_repulsion(sph);

            pos.y = 0; // TODO placement rules

            nnode->set_radius(en_radius);
            nnode->set_position(pos);
        
            // attach new node to all nearby neighbors
            // scwfc_node.update_all_adjacencies(nnode, n_radius);

            // update visual node state, may be destroyed
            node_check_and_update(nnode.get());
            
            if (!nnode->is_destroyed())
                nnodes.push_back(nnode);
        }

        if (!nnodes.empty() && i % brf == 0)
            node = select_randomly(nnodes.begin(), nnodes.end(), *m_mt.get())->get();
    }
    return node->get_ref<SCWFCGraphNode>();
}

void SCWFCSolver::wfc_solve(int steps) {
    wfc::WFCSolver::entropy_callback_t entropy_func =
        [this](auto* prop_node, auto* on_node) -> float {
            auto* prop_node_scene =
                dynamic_cast<const SCWFCGraphNode*>(prop_node);
            auto* on_node_scene = dynamic_cast<const SCWFCGraphNode*>(on_node);
            return wfc_solver->node_entropy(on_node) -
                    1 / glm::length(prop_node_scene->get_position() -
                                    on_node_scene->get_position());
        };

    // wfc_solver->set_entropy_func(entropy_func);
    for (int cnt = 0; cnt < steps; ++cnt) {
        if (m_boundary.size() < 1)
            break;
        auto n = m_boundary.front();
        m_boundary.pop();

        // solve node
        wfc_solver->step_wfc((wfc::DGraphNode*)n.get());

        // add all adjacent nodes to the solver boundary
        auto adjacent = scwfc_node.get_graph()->adjacent_nodes(n.get());
        std::for_each(
            adjacent.begin(), adjacent.end(), [this](auto* dgn) -> void {

                auto s_node = Ref{dynamic_cast<SCWFCGraphNode*>(dgn)};
                if (!s_node->is_finalized() &&
                    m_discovered.find(s_node) == m_discovered.end()) {
                    
                    m_discovered.insert(s_node);
                    m_boundary.push(s_node);
                }
            });

        node_check_and_update(n.get());
    }
}
#endif