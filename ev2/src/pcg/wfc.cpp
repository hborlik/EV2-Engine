#include <pcg/wfc.hpp>

#include <memory>
#include <unordered_set>
#include <iostream>

namespace wfc {

float ford_fulkerson(const DenseGraph<Node>& dg, const Node* source, const Node* sink, DenseGraph<Node>* residual_graph) {
    // based on https://www.geeksforgeeks.org/ford-fulkerson-algorithm-for-maximum-flow-problem/
    assert(source && sink);

    DenseGraph<Node> residual = dg;

    float max_flow = 0.f;

    int s_ind = residual.get_node_index(source);
    int t_ind = residual.get_node_index(sink);

    std::vector<int> parent;
    while (residual.bfs(s_ind, t_ind, parent)) {
        // find the minimum residual capacity along the 
        // path in residual graph
        float path_flow = INFINITY;
        for (int v = t_ind; v != s_ind; v = parent[v]) {
            int u = parent[v];
            path_flow = std::min(path_flow, residual.adjacent(u, v));
        }

        // update residual capacities on the path edges
        for (int v = t_ind; v != s_ind; v = parent[v]) {
            int u = parent[v];
            residual.adjacent(u, v) -= path_flow;
            residual.adjacent(v, u) += path_flow;
        }

        max_flow += path_flow;
    }
    if (residual_graph)
        *residual_graph = residual;
    return max_flow;
}

float DNode::entropy() const {
    float sum = 0;
    for (auto& p : domain) {
        sum += p->weight;
    }
    return sum;
}

bool Pattern::valid(const std::vector<DNode*>& neighborhood) const {
    /* matching problem (edges are between required values and neighbors with that value in domain)
     * map required values one-to-one (perfect matching) with available neighbors
     * if all requirements are satisfied, return true
     * */
    if (required_values.size() == 0)
        return true;

    if (neighborhood.size() == 0)
        return false;

    struct ValueAndNode {
        Value v;
        Node* p;
    };
    std::vector<ValueAndNode> neigh_values{};   // values in neighborhood with associated node in flow graph
    std::vector<ValueAndNode> required{};       // required values and associated node in flow graph

    // extra two capacity for source and sink
    DenseGraph<Node> dg{ (int)neighborhood.size() + (int)required_values.size() + 2, true };

    /* construct flow graph */

    // node references
    auto source = std::make_unique<Node>("source", 2);
    auto sink = std::make_unique<Node>("sink", 3);
    std::vector<std::unique_ptr<Node>> req_nodes{};
    std::vector<std::unique_ptr<Node>> neigh_nodes{};
    
    // for each required value, create a node in flow graph and connect it to source
    int id = 4;
    for (auto& rv : required_values) {
        auto req_node = std::make_unique<Node>("req:" + std::to_string(rv.cell_id), ++id);
        dg.add_edge(source.get(), req_node.get(), 1.f);
        required.push_back(ValueAndNode{
            .v = rv,
            .p = req_node.get()
        });

        // save reference for cleanup
        req_nodes.emplace_back(std::move(req_node));
    }

    // for each neighbor, create a node in flow graph and connect it to sink
    for (const auto& node : neighborhood) {
        auto domain_node = std::make_unique<Node>(node->identifier, ++id);
        dg.add_edge(domain_node.get(), sink.get(), 1.f);
        for (const auto& val : node->domain) {
            neigh_values.emplace_back(ValueAndNode{
                .v = val->cell_value,
                .p = domain_node.get()
                });
        }

        // save reference for cleanup
        neigh_nodes.emplace_back(std::move(domain_node));
    }

    auto sort_vn = [](const ValueAndNode& a, const ValueAndNode& b) {
        // sorted high to low cell values
        return a.v.cell_id < b.v.cell_id;
    };

    // sort by the cell values
    std::sort(required.begin(), required.end(), sort_vn);
    std::sort(neigh_values.begin(), neigh_values.end(), sort_vn);

    // find the nodes associated with each required value by stepping through the sorted lists
    auto vn_pos = neigh_values.begin();
    for (const auto& req : required) {
        bool has_single_neighbor = false;
        while(vn_pos != neigh_values.end()) {
            if (vn_pos->v == req.v) {
                // connect required value node with neighbor node that
                // has required value in its domain
                dg.add_edge(req.p, vn_pos->p, 1.f);
                // there is at least one node satisfying the requirement
                has_single_neighbor = true;
            } else if (vn_pos->v.cell_id > req.v.cell_id) {
                if (has_single_neighbor == false)
                    return false;
                break; // did not match value
            }
            ++vn_pos;
        }
    }

    float flow = ford_fulkerson(dg, source.get(), sink.get(), &dg);

    // check that all requirements are met
    if (flow != required_values.size())
        return false;

    return true;
}

}