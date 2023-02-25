#include <pcg/wfc.hpp>

#include <memory>
#include <iomanip>
#include <unordered_set>

namespace wfc {

SparseGraph::internal_node* SparseGraph::add_node(Node* node) {
    internal_node* i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        // node does not exist
        if (itr == node_map.end()) {
            auto [p, inserted] = node_map.emplace(node->node_id, internal_node{ get_next_mat_coord(), node, {} });

            // if false, failed to create, we probably already have the same id
            if (inserted)
                i_node = &p->second;
        } else
            i_node = &itr->second;
    }
    return i_node;
}

const SparseGraph::internal_node* SparseGraph::get_node(Node* node) const {
    const internal_node* i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        if (itr != node_map.end())
            i_node = &itr->second;
    }
    return i_node;
}

bool DenseGraph::bfs(const Node* a, const Node* b, std::vector<Node*>& path) const {
    assert(a && b);
    path.clear();

    if (m_nodeid_to_nodeind.find(a->node_id) == m_nodeid_to_nodeind.end() ||
        m_nodeid_to_nodeind.find(b->node_id) == m_nodeid_to_nodeind.end())
        return false;

    auto parent = std::vector<int>{};

    int a_ind = m_nodeid_to_nodeind.at(a->node_id);
    int b_ind = m_nodeid_to_nodeind.at(b->node_id);

    if (bfs(a_ind, b_ind, parent)) {
        int next = b_ind;
        while (next != -1) {
            path.push_back(m_nodes[next]);
            next = parent[next];
        }
        std::reverse(path.begin(), path.end());
        return true;
    }
    return false;
}

bool DenseGraph::bfs(int a_ind, int b_ind, std::vector<int>& parent) const {
    assert(a_ind >= 0 && b_ind >= 0);
    parent.clear();
    parent = std::vector<int>(m_nodes.size(), -1);

    auto visited = std::vector<bool>(m_nodes.size(), false);

    std::queue<int> q{};

    q.push(a_ind);
    visited[a_ind] = true;

    while (!q.empty()) {
        int u_ind = q.front();
        q.pop();
        // visit n, find index in adjacency matrix
        for (int v_ind = 0; v_ind < m_nodes.size(); ++v_ind) {
            // if this node has not been visited and adjacent (from u to v)
            if (!visited[v_ind] && m_adjacency_matrix[ind(u_ind, v_ind)] > 0.f) {
                q.push(v_ind);
                visited[v_ind] = true;
                parent[v_ind] = u_ind; // v was reached from u

                if (v_ind == b_ind) {
                    // found target, starting from target, record all parents into path
                    return true;
                }
            }
        }
    }
    return false;
}

std::ostream& operator<< (std::ostream& out, const DenseGraph& graph) {
    out << std::setprecision(5);
    for (int i = 0; i < graph.get_n_nodes(); ++i) {
        for (int j = 0; j < graph.get_n_nodes(); ++j)
            out << graph.m_adjacency_matrix[graph.ind(i, j)] << " ";
        out << "\n";
    }

    return out;
}

float ford_fulkerson(const DenseGraph& dg, const Node* source, const Node* sink, DenseGraph* residual_graph) {
    // based on https://www.geeksforgeeks.org/ford-fulkerson-algorithm-for-maximum-flow-problem/
    assert(source && sink);

    DenseGraph residual = dg;

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
    DenseGraph dg{ (int)neighborhood.size() + (int)required_values.size() + 2, true };

    /* construct flow graph */

    // node references
    auto source = std::make_unique<Node>("source", 0);
    auto sink = std::make_unique<Node>("sink", 1);
    std::vector<std::unique_ptr<Node>> req_nodes{};
    std::vector<std::unique_ptr<Node>> neigh_nodes{};
    
    // for each required value, create a node in flow graph and connect it to source
    int id = 2;
    for (auto& rv : required_values) {
        auto req_node = std::make_unique<Node>("req_value:" + std::to_string(rv.cell_id), ++id);
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
        return a.v.cell_id > b.v.cell_id;
    };

    // sort by the cell values
    std::sort(required.begin(), required.end(), sort_vn);
    std::sort(neigh_values.begin(), neigh_values.end(), sort_vn);

    // find the nodes associated with each required value by stepping through the sorted lists
    auto vn_pos = neigh_values.begin();
    for (const auto& req : required) {
        while(vn_pos != neigh_values.end()) {
            if (vn_pos->v == req.v) {
                // connect required value node with neighbor node that
                // has required value in its domain
                dg.add_edge(req.p, vn_pos->p, 1.f);
            } else {
                break; // did not match value
            }

            ++vn_pos;
        }
    }

    ford_fulkerson(dg, source.get(), sink.get(), &dg);

    // check that all requirements are met
    for (const auto& n : req_nodes) {
        float f = dg.adjacent(source.get(), n.get());
        if (f < 1.f) {
            return false;
        }
    }

    return true;
}

}