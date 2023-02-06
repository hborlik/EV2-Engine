#include <pcg/wfc.hpp>

namespace wfc {

SparseGraph::internal_node* SparseGraph::add_node(Node *node)
{
    internal_node *i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        // node does not exist
        if (itr == node_map.end()) {
            auto p = node_map.emplace(node->node_id, internal_node{get_next_mat_coord(), node, {}});

            // if false, failed to create, we probably already have the same id
            if (p.second)
                i_node = &p.first->second;
        } else
            i_node = &itr->second;
    }
    return i_node;
}

SparseGraph::internal_node* SparseGraph::get_node(Node *node)
{
    internal_node *i_node = nullptr;
    if (node) {
        auto itr = node_map.find(node->node_id);
        if (itr != node_map.end())
            i_node = &itr->second;
    }
    return i_node;
}

}