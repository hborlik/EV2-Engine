#include <algorithm>
#include <cstddef>

#include "pcg/sc_wfc.hpp"
#include "timer.hpp"

namespace ev2::pcg {

struct SCWFC::Data {
    wfc::SparseGraph<wfc::DGraphNode> graph;
};

SCWFC::SCWFC(std::string name): 
    Node{ std::move(name) } {

}


void SCWFC::reset() {
    // for (auto& c : get_children()) {
    //     c->destroy();
    // }
    for (auto c : get_children())
        c->destroy();

    m_data = std::make_shared<Data>();
}

void SCWFC::remove_all_unsolved() {
    for (auto c : get_children()) {
        auto s_node = c.ref_cast<SCWFCGraphNode>();
        if (s_node && s_node->domain.size() > 1) {
            c->destroy();
        }
    }
}

void SCWFC::on_init() {
    reset();
}

void SCWFC::on_child_removed(Ref<Node> child) {
    if (auto n = child.ref_cast<SCWFCGraphNode>()) {
        // remove node from graph
        m_data->graph.remove_node(static_cast<wfc::DGraphNode*>(n.get()));
        child_node_removed.notify(n.get());
    }
}

void SCWFC::on_child_added(Ref<Node> child, int index) {
    if (auto n = child.ref_cast<SCWFCGraphNode>()) {
        // update_all_adjacencies(n);
        child_node_added.notify(n.get());
    }
}

void SCWFC::update_all_adjacencies(Ref<SCWFCGraphNode> n) {
    // Timer timer{__FUNCTION__};
    Sphere s = n->get_bounding_sphere();
    s.radius = n->get_neighborhood_radius();
    for (auto& c : get_children()) {
        auto c_wfc_node = c.ref_cast<SCWFCGraphNode>();
        if (c_wfc_node && c_wfc_node != n) {
            auto* n_graph_node = static_cast<wfc::DGraphNode*>(n.get());
            auto* c_graph_node = static_cast<wfc::DGraphNode*>(c_wfc_node.get());
            if (intersect(c_wfc_node->get_bounding_sphere(), s)) {
                m_data->graph.add_edge(n_graph_node, c_graph_node, 1);
            } else {
                m_data->graph.remove_edge(n_graph_node, c_graph_node);
            }
        }
    }
    // timer.stop();
    // std::cout << get_n_children() << "\t" << timer.elapsed_ms() << "ms" << "\n";
}

glm::vec3 SCWFC::sphere_repulsion(const Sphere& sph) const {
    glm::vec3 net{};
    for (auto& c : get_children()) {
        auto graph_node = c.ref_cast<SCWFCGraphNode>();
        if (graph_node) {
            const Sphere& bounds = graph_node->get_bounding_sphere();
            // if (intersect(bounds, sph)) {
                glm::vec3 c2c = bounds.center - sph.center;
                float r2 = glm::dot(c2c, c2c);
                net += -glm::normalize(c2c) * (bounds.radius * sph.radius) / r2;
            // }
        }
    }
    return net;
}

bool SCWFC::intersects_any_solved_neighbor(const Ref<SCWFCGraphNode>& n) {
    // for every node that has been added as an adjacent one
    for (auto& node : m_data->graph.adjacent_nodes(n.get())) {
        auto sc_node = dynamic_cast<SCWFCGraphNode*>(node);
        if (sc_node && sc_node->is_solved()) { // is it solved
            const Sphere& bounds = sc_node->get_bounding_sphere();
            if (intersect(bounds, n->get_bounding_sphere())) {
                return true;
            }
        }
    }
    return false;
}

bool SCWFC::intersects_any(const Sphere& n) {
    for (auto& c : get_children()) {
        auto sc_node = c.ref_cast<SCWFCGraphNode>();
        if (sc_node) {
            const Sphere& bounds = sc_node->get_bounding_sphere();
            if (intersect(bounds, n)) {
                return true;
            }
        }
    }
    return false;
}

wfc::SparseGraph<wfc::DGraphNode>* SCWFC::get_graph() {
    return &m_data->graph;
}

}