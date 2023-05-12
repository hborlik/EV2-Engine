#include "sc_wfc_solver.hpp"



#include "pcg/sc_wfc.hpp"
#include "pcg/wfc.hpp"
#include "reference_counted.hpp"
#include "resource.hpp"
#include "pcg/distributions.hpp"
#include "pcg/object_database.hpp"
#include "scene/node.hpp"

namespace ev2::pcg {

SCWFCSolver::SCWFCSolver(SCWFC& scwfc_node, 
                std::shared_ptr<ObjectMetadataDB> obj_db, 
                std::random_device& rd,
                std::shared_ptr<renderer::Drawable> unsolved_drawable)
    : scwfc_node{scwfc_node},
      m_mt{rd()},
      obj_db{obj_db},
      wfc_solver{std::make_unique<wfc::WFCSolver>(scwfc_node.get_graph(), obj_db->make_pattern_map(), m_mt)},
      unsolved_drawable{unsolved_drawable} {}

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
        std::unordered_set<int> new_domain{};

        // if spawning on an existing node
        if (node) {
            if (node->domain.size() < 1) // node should not have an empty domain
                return {};
            
            offsets.clear();

            // since we are propagating from an existing node, spawn a
            // node that contains set of valid neighbors for that existing
            // node.
            for (auto value : node->domain) {
                // for all patterns for the class id
                for (auto [itr, end] = wfc_solver->get_patterns(value); itr != end; ++itr) {
                    auto [id, p] = *itr;
                    // add all required classes
                    new_domain.insert(p.required_classes.begin(), p.required_classes.end());
                }
            }

            // random select a pattern and an object data to use for placement patterns
            // TODO check that all of these return more than 0 elements
            auto pattern = wfc_solver->weighted_pick(node);
            auto [obj_p, obj_e] = obj_db->objs_for_id(pattern);
            const auto& [id, obj] = *select_randomly(obj_p, obj_e, m_mt);

            for (const auto& obb : obj.propagation_patterns) {
                // values within 3 standard deviations account for 99.7% of samples
                std::normal_distribution<float> dist_x{0, obb.half_extents.x / 3};
                std::normal_distribution<float> dist_y{0, obb.half_extents.y / 3};
                std::normal_distribution<float> dist_z{0, obb.half_extents.z / 3};

                glm::vec3 pos_in_obb {
                    dist_x(m_mt),
                    dist_y(m_mt),
                    dist_z(m_mt)
                };

                // try to place sphere center in the OBB
                Sphere sph({}, n_radius);
                sph.center = node->get_linear_transform() * obb.get_transform() * glm::vec4{pos_in_obb, 1.f};
                
                glm::vec3 offset = sph.center;
                offset += mass * scwfc_node.sphere_repulsion(sph);

                offset.y = 0; // TODO placement rules

                offsets.push_back(offset);
            }

        } else { // populate domain with all available patterns
            auto [p_itr, p_end] = obj_db->get_patterns_iterator();
            std::vector<int> dest(obj_db->patterns_size());
            std::transform(p_itr, p_end, dest.begin(),
                [](auto &elem){ return elem.pattern_class; }
            );
            new_domain = {dest.begin(), dest.end()};
        }

        for (auto& offset : offsets) {
            auto nnode = scwfc_node.create_child_node<SCWFCGraphNode>("SGN " + std::to_string(scwfc_node.get_n_children()));
            // populate domain of new node
            nnode->domain = {new_domain.begin(), new_domain.end()};
            nnode->set_position(offset);
        
            // attach new node to all nearby neighbors
            // scwfc_node.update_all_adjacencies(nnode, n_radius);

            // update node state, may be destroyed
            node_check_and_update(nnode.get());
            
            if (!nnode->is_destroyed())
                nnodes.push_back(nnode);
        }

        if (i % brf == 0)
            node = select_randomly(nnodes.begin(), nnodes.end(), m_mt)->get();
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
        auto& n = m_boundary.front();
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

void SCWFCSolver::node_check_and_update(SCWFCGraphNode* s_node) {
    assert(s_node);

    if (s_node->is_finalized()) // node is already solved and complete
        return;

    auto model = unsolved_drawable;
    float extent{2 * s_node->get_bounding_sphere()
                            .radius};  // default to the same size

    if (s_node->domain.size() == 0) {
        // remove nodes with 0 valid objects in their domain from the scene
        s_node->destroy();
    } else if (s_node->domain.size() == 1) {
        const int class_id = s_node->domain[0];
        // obj_db will return all ObjestData's for a class id, but will
        // only use the first if it exists here.
        const ObjectData* obj{};
        if (auto [itr, end] = obj_db->objs_for_id(class_id);
            itr != end) {
            auto& [id, obj_data] = *itr;
            auto m = ResourceManager::get_singleton()
                            .get_model_relative_path(obj_data.asset_path);
            if (m) {
                model = m;
                extent = obj_data.extent;
                obj = &obj_data;
            }
        }

        float rotation_y = 0.f;
        if (obj)
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
        const auto& aabb = model->bounding_box;
        const float scale = extent / glm::length(aabb.diagonal()); // scale uniformly

        auto aabb_scaled = aabb.scale(glm::vec3{scale});

        s_node->set_model(model);
        s_node->set_scale(glm::vec3{scale});
        s_node->set_radius(aabb.min_diagonal() * scale / 2.f);
        s_node->rotate(glm::vec3{0, rotation_y, 0});
        s_node->set_position(s_node->get_position() * glm::vec3{1, 0, 1} + glm::vec3{0, -aabb_scaled.pMin.y, 0});
        s_node->set_finalized();

        if (scwfc_node.intersects_any_solved_neighbor(Ref{ s_node })) {
            // remove nodes whose final bounding volume intersects solved nodes
            s_node->destroy();
        }
    } else {
        // keep model as unsolved drawable
        const auto& aabb = model->bounding_box;
        const float scale = extent / glm::length(aabb.diagonal()); // scale uniformly

        s_node->set_model(model);
        s_node->set_scale(glm::vec3{scale});
        s_node->set_radius(aabb.min_diagonal() * scale / 2.f);
    }
}

bool SCWFCSolver::can_continue() const noexcept {
    return wfc_solver->can_continue();
}

void SCWFCSolver::set_seed_node(Ref<SCWFCGraphNode> node) {
    m_boundary.push(node);
}


}