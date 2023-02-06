/**
 * @file wfc.hpp
 * @brief
 * @date 2023-01-28
 *
 */
#ifndef WFC_H
#define WFC_H

#include <vector>
#include <random>
#include <list>
#include <queue>
#include <string>
#include <algorithm>
#include <unordered_map>

#include <assert.h>

namespace wfc {

struct coord {
    int x = 0, y = 0;

    coord() = default;

    /**
     * @brief enable use in maps
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator<(const coord &other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        if (y < other.y) return true;
        return y > other.y;
    }

    bool operator==(const coord &other) const noexcept {
        return x == other.x && y == other.y;
    }

    int to_index(int row_count) const noexcept {
        return y * row_count + x;
    }
};

}

template<>
struct std::hash<wfc::coord>
{
    std::size_t operator()(const wfc::coord& k) const
    {
        // from https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        using std::size_t;
        using std::hash;
        using std::string;

        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:

        return ((hash<int>()(k.x)
                ^ (hash<int>()(k.y) << 1)) >> 1);
                // ^ (hash<int>()(k.third) << 1);
    }
};

namespace wfc
{

class Value
{
public:
    Value() = default;
    explicit Value(int cell_id) : cell_id{cell_id} {}

    float weight = 0.f;
    int cell_id = 0; // cell tile identifier
};

class Pattern;

class Node
{
public:
    Node(const std::string &identifier, int node_id) : node_id{node_id}, identifier{identifier} {}

    float entropy() const
    {
        float sum = 0;
        for (auto &p : domain)
        {
            // TODO
        }
        return sum;
    }

    const int               node_id = -1;
    Value                   value{};
    std::string             identifier = "";
    std::vector<Pattern>    domain{};      // valid patterns for this node
};

class Graph {
public:
    virtual ~Graph() {}

    /**
     * @brief add edge
     * 
     * @param a 
     * @param b 
     */
    virtual void add_edge(Node *a, Node *b, float v) = 0;

    /**
     * @brief check if nodes are adjacent, returns 0.f if not
     * 
     * @param a 
     * @param b 
     * @return float 
     */
    virtual float adjacent(Node *a, Node *b) = 0;

    /**
     * @brief get adjacent nodes
     * 
     * @param a 
     * @return std::vector<Node*> 
     */
    virtual std::vector<Node*> adjacent_nodes(Node *a) = 0;
};

class SparseGraph : public Graph
{
private:

    struct internal_node {
        int mat_coord = -1;
        Node* node = nullptr;
        std::vector<Node*> adjacent_nodes{};
    };

    struct weight {
        float w = 0.f;
        // quick reference for finding adjacent nodes
        internal_node* i_nodeA = nullptr;
        internal_node* i_nodeB = nullptr;
    };

    int next_mat_coord = 0;
    bool is_directed = false;
    std::unordered_map<int, internal_node> node_map{};
    std::unordered_map<coord, weight> sparse_adjacency_map{};

public:

    SparseGraph() = default;
    SparseGraph(bool is_directed) : is_directed{is_directed} {}

    void add_edge(Node *a, Node *b, float v) override
    {
        assert(a != nullptr && b != nullptr);
        assert(a != b);
        internal_node *a_i = add_node(a);
        internal_node *b_i = add_node(b);

        // enforce populating only the upper triangular matrix
        if (!is_directed && a_i->mat_coord > b_i->mat_coord)
            std::swap(a_i, b_i);

        // build adjacency information
        a_i->adjacent_nodes.push_back(b);
        if (!is_directed)
            b_i->adjacent_nodes.push_back(a);

        coord c{a_i->mat_coord, b_i->mat_coord};

        if (sparse_adjacency_map.find(c) != sparse_adjacency_map.end())
            return; // already added
        
        if (c.x == c.y)
            return; // no self loops (diagonals)

        auto& sam = sparse_adjacency_map[c];
        sam.i_nodeA = a_i;
        sam.i_nodeB = b_i;
        sam.w = v;
    }

    float adjacent(Node *a, Node *b) override {
        assert(a != nullptr && b != nullptr);
        assert(a != b);
        internal_node *a_i = get_node(a);
        internal_node *b_i = get_node(b);

        if (a_i == nullptr || b_i == nullptr)
            return 0.f;

        if (a_i->mat_coord > b_i->mat_coord)
            std::swap(a_i, b_i);

        coord c{a_i->mat_coord, b_i->mat_coord};

        assert(c.x != c.y); // sanity check, no diagonals

        auto itr = sparse_adjacency_map.find(c);
        if (itr != sparse_adjacency_map.end())
            return itr->second.w;
        return 0.f;
    }

private:
    internal_node* add_node(Node *node);

    internal_node* get_node(Node* node);

    int get_next_mat_coord() noexcept {return next_mat_coord++;}

};

/**
 * @brief Dense Graph
 * 
 */
class DirectedDenseGraph : public Graph {
public:
    DirectedDenseGraph(int n_nodes) : m_n_nodes{n_nodes}, m_adjacency_matrix(n_nodes*n_nodes) {}

    /**
     * @brief A to B
     * 
     * @param a 
     * @param b 
     * @param v 
     */
    void add_edge(Node *a, Node *b, float v) override {
        assert(a != nullptr && b != nullptr);
        assert(a != b);
        assert(a->node_id < m_n_nodes && b->node_id < m_n_nodes);

        coord c{a->node_id, b->node_id};

        int ind = c.to_index(m_n_nodes);
        m_adjacency_matrix[ind] = v;
    }

    /**
     * @brief A to B
     * 
     * @param a 
     * @param b 
     * @return float 
     */
    float adjacent(Node *a, Node *b) override {
        assert(a != nullptr && b != nullptr);
        assert(a != b);
        assert(a->node_id < m_n_nodes && b->node_id < m_n_nodes);

        coord c{a->node_id, b->node_id};

        int ind = c.to_index(m_n_nodes);
        return m_adjacency_matrix[ind];
    }

private:
    const int m_n_nodes = 0;
    std::vector<float> m_adjacency_matrix{};
};

/**
 * @brief Pattern is a valid configuration of nodes and maintains adjacency
 *
 */
class Pattern
{
public:
    bool valid(std::vector<Node *> neighborhood) {
        // TODO matching problem (edges are between required values and neighbors with that value in domain)
        // map required values one-to-one (perfect matching) with available neighbors
        // if all satisfied, return true
        return false;
    }

    std::vector<Value> required_values{};
    Value cell_value{};
};

class WFCSolver
{
public:
    WFCSolver(Graph *graph, std::vector<Pattern> patterns) : graph{graph}, patterns{patterns} {
        assert(graph != nullptr);
    }

    /**
     * @brief Propogate the wave function collapse algorithm
     * 
     * @param node 
     */
    void propogate(Node *node)
    {
        assert(node != nullptr);
        propagation_stack.push(node);
        while (!propagation_stack.empty())
        {
            Node *n = propagation_stack.front();
            propagation_stack.pop();
            observe(n);
            for (auto &neighbor : graph->adjacent_nodes(n))
            {
                if (neighbor->domain.size() == 1)
                    propagation_stack.push(neighbor);
            }
        }
    }

    /**
     * @brief Observe the node and update its domain
     * 
     * @param node 
     */
    void observe(Node *node)
    {
        assert(node != nullptr);
        std::vector<Pattern> new_domain{};
        for (auto &p : node->domain) {
            if(p.valid(graph->adjacent_nodes(node)))
                new_domain.push_back(p);
        }
        node->domain = new_domain;
    }

    /**
     * @brief Collapse the node to a single value. Performing a weighted random selection if multiple values are available.
     * 
     * @param node 
     */
    void collapse(Node *node) {
        assert(node != nullptr);
        assert(node->domain.size() >= 1);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        if (node->domain.size() == 1) {
            node->value = node->domain[0].cell_value;
            return;
        } else {
            // weighted random selection of available domain values
            std::vector<float> weights{};
            for (auto &p : node->domain) {
                weights.push_back(p.cell_value.weight);
            }
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            node->value = node->domain[dist(gen)].cell_value;
        }
    }

    std::queue<Node *> propagation_stack;
    Graph *graph;
    std::vector<Pattern> patterns;
};

} // namespace wfc

#endif // WFC_H