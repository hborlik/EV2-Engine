#include <pcg/wfc.hpp>

#include <memory>

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

SparseGraph::internal_node* SparseGraph::get_node(Node *node) const 
{
    internal_node *i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        if (itr != node_map.end())
            i_node = &itr->second;
    }
    return i_node;
}

bool DenseGraph::bfs(Node *a, Node *b, std::vector<Node*> path) const {
    assert(a && b);

    path.clear();

    std::queue<Node*> q{};
    q.push(a);

    auto visited = std::vector<bool>(m_nodes.size(), false);
    auto parent = std::vector<Node*>(m_nodes.size());

    while (!q.empty()) {
        Node *n = q.front();
        q.pop();

        int n_ind = m_nodeid_to_nodeind.at(n->node_id);

        for (int v = 0; v < m_nodes.size(); ++v) {
            Node *ne = m_nodes[v];
            
            if (!visited[v] && m_adjacency_matrix[ind(n_ind, v)] > 0.f) { // if this node has not been visited
                q.push(ne);

                path.push_back(ne);
                
                if (ne == b) {
                    // found target

                    return true;
                }
            }
        }
    }
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

bool Pattern::valid(std::vector<Node *> neighborhood) const {
    // matching problem (edges are between required values and neighbors with that value in domain)
    // map required values one-to-one (perfect matching) with available neighbors
    // if all requirements are satisfied, return true
    DenseGraph d{neighborhood.size() + required_values.size() + 2};

    auto source = std::make_unique<Node>();
    auto sink = std::make_unique<Node>();

    // construct flow graph


    

    DenseGraph r = d;
    return false;
}

}