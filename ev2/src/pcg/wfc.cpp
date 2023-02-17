#include <pcg/wfc.hpp>

#include <memory>
#include <iomanip>
#include <unordered_set>

namespace wfc {

SparseGraph::internal_node* SparseGraph::add_node(Node *node)
{
    internal_node *i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        // node does not exist
        if (itr == node_map.end()) {
            auto [p, inserted] = node_map.emplace(node->node_id, internal_node{get_next_mat_coord(), node, {}});

            // if false, failed to create, we probably already have the same id
            if (inserted)
                i_node = &p->second;
        } else
            i_node = &itr->second;
    }
    return i_node;
}

const SparseGraph::internal_node* SparseGraph::get_node(Node *node) const 
{
    const internal_node *i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        if (itr != node_map.end())
            i_node = &itr->second;
    }
    return i_node;
}

bool DenseGraph::bfs(const Node *a, const Node *b, std::vector<Node*>& path) const {
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
        while(next != -1) {
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

std::ostream& operator<< (std::ostream &out, const DenseGraph &graph) {
    out << std::setprecision(5);
    for (int i = 0; i < graph.get_n_nodes(); ++i) {
        for (int j = 0; j < graph.get_n_nodes(); ++j)
            out << graph.m_adjacency_matrix[graph.ind(i, j)] << " ";
        out << "\n";
    }

    return out;
}

float ford_fulkerson(const DenseGraph& dg, const Node *source, const Node *sink, DenseGraph* residual_graph) {
    // based on https://www.geeksforgeeks.org/ford-fulkerson-algorithm-for-maximum-flow-problem/
    assert(source && sink);

    DenseGraph residual = dg;

    float max_flow = 0.f;

    int s_ind = residual.get_node_index(source);
    int t_ind = residual.get_node_index(sink);

    std::vector<int> parent;
    while(residual.bfs(s_ind, t_ind, parent)) {
        // find the minimum residual capacity along the 
        // path in residual graph
        float path_flow = INFINITY;
        for(int v = t_ind; v != s_ind; v = parent[v]) {
            int u = parent[v];
            path_flow = std::min(path_flow, residual.adjacent(u, v));
        }

        // update residual capacities on the path edges
        for(int v = t_ind; v != s_ind; v = parent[v]) {
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

float Node::entropy() const
{
    float sum = 0;
    for (auto &p : domain)
    {
        sum += p->weight;
    }
    return sum;
}

bool Pattern::valid(const std::vector<Node *>& neighborhood) const {
    // matching problem (edges are between required values and neighbors with that value in domain)
    // map required values one-to-one (perfect matching) with available neighbors
    // if all requirements are satisfied, return true
    DenseGraph d{(int)neighborhood.size() + (int)required_values.size() + 2, true};

    auto source = std::make_unique<Node>("source", 1000);
    auto sink = std::make_unique<Node>("sink", 1001);

    // construct flow graph
    auto req_nodes = std::vector<std::unique_ptr<Node>>();
    std::unordered_multiset<Value> value_set{required_values.begin(), required_values.end()};
    int id = 0;
    for (auto& rv : required_values) {
        req_nodes.push_back(std::make_unique<Node>(std::to_string(cell_value.cell_id), id++));
        d.add_edge(source.get(), req_nodes[req_nodes.size() - 1].get(), 1.f);
    }
    
    for (const auto& n : neighborhood) {
        // if (n.do) // TODO
    }

    
    return false;
}

}