/**
 * @file wfc.hpp
 * @brief 
 * @date 2023-01-28
 * 
 */
#ifndef WFC_H
#define WFC_H

#include <vector>
#include <list>
#include <queue>
#include <string>

#include <assert.h>

namespace wfc {

class Value {
    float weight = 0.f;
    int id = 0; // cell tile identifier
};

/**
 * @brief Pattern is a valid configuration of nodes
 * 
 */
class Pattern {
public:
    bool valid(std::vector<Node*> neighborhood);

    std::vector<Value> required_values{};
    Value cell_value{};
};

class Node {
public:
    Node(const std::string& identifier) : identifier{identifier} {}

    std::list<Node*> neighborhood{}; // dependent nodes
    std::list<Pattern*> domain{}; // list of valid patterns for this node
    std::string identifier = "";
};

class SparseGraph {
public:
    void edge(Node* a, Node* b) {
        assert(a != nullptr && b != nullptr);
        a->neighborhood.push_back(b);
        b->neighborhood.push_back(a);
    }

    void add_node(Node* node) {
        nodes.push_back(node);
    }

    std::vector<Node*> nodes;
};

class WFCSolver {
public:
    void propogate(Node* node) {
        assert(node != nullptr);
        for (auto& node : node->neighborhood) {
            
        }
    }

    void observe(Node* node) {

    }

    std::queue<Node*> propagation_stack;
};

}

#endif // WFC_H