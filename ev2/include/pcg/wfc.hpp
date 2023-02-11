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

    int cell_id = 0; // cell tile identifier
};

class Pattern;

class Node
{
public:
    Node(const std::string &identifier, int node_id) : node_id{node_id}, identifier{identifier} {
        assert(node_id > 0);
    }

    float entropy() const;

    const int               node_id = -1;
    Value                   value{};
    std::string             identifier = "";
    std::vector<Pattern*>   domain{};      // valid patterns for this node
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
    virtual float adjacent(Node *a, Node *b) const = 0;

    /**
     * @brief get adjacent nodes
     * 
     * @param a 
     * @return std::vector<Node*> 
     */
    virtual std::vector<Node*> adjacent_nodes(Node *a) const = 0;

    virtual bool is_directed() const noexcept = 0;

    virtual int get_n_nodes() const noexcept = 0;

    /**
     * @brief make an unordered map of node ids to boolean flags
     * 
     * @return std::unordered_map<int, bool> 
     */
    virtual std::unordered_map<int, bool> make_visited_map() const = 0;
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
    bool m_is_directed = false;
    std::unordered_map<int, internal_node> node_map{}; // node id to internal adjacency
    std::unordered_map<coord, weight> sparse_adjacency_map{};

public:

    SparseGraph() = default;
    SparseGraph(bool is_directed) : m_is_directed{is_directed} {}

    void add_edge(Node *a, Node *b, float v) override
    {
        assert(a != nullptr && b != nullptr);
        internal_node *a_i = add_node(a);
        internal_node *b_i = add_node(b);

        // enforce populating only the upper triangular matrix
        if (!m_is_directed && a_i->mat_coord > b_i->mat_coord)
            std::swap(a_i, b_i);

        // build adjacency information
        a_i->adjacent_nodes.push_back(b);
        if (!m_is_directed)
            b_i->adjacent_nodes.push_back(a);

        coord c{a_i->mat_coord, b_i->mat_coord};

        if (sparse_adjacency_map.find(c) != sparse_adjacency_map.end())
            return; // already added
        
        if (!m_is_directed && c.x == c.y)
            return; // no self loops (diagonals)

        auto& sam = sparse_adjacency_map[c];
        sam.i_nodeA = a_i;
        sam.i_nodeB = b_i;
        sam.w = v;
    }

    float adjacent(Node *a, Node *b) const override {
        assert(a != nullptr && b != nullptr);
        const internal_node *a_i = get_node(a);
        const internal_node *b_i = get_node(b);

        if (a_i == nullptr || b_i == nullptr)
            return 0.f;

        if (!m_is_directed && a_i->mat_coord > b_i->mat_coord)
            std::swap(a_i, b_i);

        coord c{a_i->mat_coord, b_i->mat_coord};

        if (!m_is_directed && c.x == c.y)
            return 0.f; // no self loops (diagonals)

        auto itr = sparse_adjacency_map.find(c);
        if (itr != sparse_adjacency_map.end())
            return itr->second.w;
        return 0.f;
    }

    std::vector<Node*> adjacent_nodes(Node *a) const override {
        assert(a != nullptr);
        std::vector<Node*> output{};
        const internal_node *a_i = get_node(a);

        if (a_i == nullptr)
            return output;

        return a_i->adjacent_nodes;
    }

    bool is_directed() const noexcept override {return m_is_directed;}

    int get_n_nodes() const noexcept override {return node_map.size();}

    std::unordered_map<int, bool> make_visited_map() const override {
        std::unordered_map<int, bool> out(get_n_nodes());
        for (const auto& [k, _] : node_map)
            out.insert({k, false});
        return out;
    }

private:
    internal_node* add_node(Node *node);

    const internal_node* get_node(Node* node) const;

    int get_next_mat_coord() noexcept {return next_mat_coord++;}

};

/**
 * @brief Dense Graph
 * 
 */
class DenseGraph : public Graph {
public:
    DenseGraph(int n_nodes, bool directed = false) : m_is_directed{directed}, m_n_nodes{n_nodes}, m_adjacency_matrix(n_nodes*n_nodes) {}

    /**
     * @brief A to B
     * 
     * @param a 
     * @param b 
     * @param v 
     */
    void add_edge(Node *a, Node *b, float v) override {
        assert(a != nullptr && b != nullptr);
        assert(a->node_id < m_n_nodes && b->node_id < m_n_nodes);

        if (!m_is_directed && a->node_id < b->node_id)
            std::swap(a, b);

        int ind_a = check_node_index(a);
        int ind_b = check_node_index(b);

        int index = ind(ind_a, ind_b);

        if (index < 0)
            return; // no self loops (diagonals)

        m_adjacency_matrix[index] = v;
    }

    /**
     * @brief A to B
     * 
     * @param a 
     * @param b 
     * @return float 
     */
    float adjacent(Node *a, Node *b) const override {
        assert(a != nullptr && b != nullptr);
        assert(a->node_id < m_n_nodes && b->node_id < m_n_nodes);

        int ind_a = check_node_index(a);
        int ind_b = check_node_index(b);

        if (ind_a < 0 || ind_b < 0)
            return 0.0f; // 

        return m_adjacency_matrix[ind(ind_a, ind_b)];
    }

    /**
     * @brief get adjacent nodes
     * 
     * @param a 
     * @return std::vector<Node*> 
     */
    std::vector<Node*> adjacent_nodes(Node *a) const override {
        assert(a != nullptr);
        assert(a->node_id < m_n_nodes);

        std::vector<Node*> nodes{};
        
        int ind_a = check_node_index(a);
        if (ind_a < 0)
            return nodes; // node not in graph

        for (int i = 0; i < m_n_nodes; ++i) {
            if (i == ind_a)
                continue;
            coord c{ind_a, i};
            int ind = c.to_index(m_n_nodes);
            if (m_adjacency_matrix[ind] > 0.f)
                nodes.push_back(m_nodes[i]); // m_nodes indexing follows adjacency matrix
        }
        return nodes;
    }

    bool is_directed() const noexcept override {return m_is_directed;}

    int get_n_nodes() const noexcept override {return m_nodes.size();}

    int get_max_node_id() const noexcept {return m_n_nodes;}

    std::unordered_map<int, bool> make_visited_map() const override {
        std::unordered_map<int, bool> out(get_n_nodes());
        for (const auto& [k, _] : m_nodeid_to_nodeind)
            out.insert({k, false});
        return out;
    }

    bool bfs(Node *a, Node *b, std::vector<Node*> path) const;

private:
    int check_node_index(Node* node) {
        auto itr = m_nodeid_to_nodeind.find(node->node_id);
        if (itr == m_nodeid_to_nodeind.end()) { // not found, add node and allocate it a row in matrix
            int ind = m_nodes.size();
            m_nodes.push_back(node);
            m_nodeid_to_nodeind.insert({node->node_id, ind});
            return ind;
        }
        return itr->second;
    }

    int check_node_index(Node* node) const {
        auto itr = m_nodeid_to_nodeind.find(node->node_id);
        if (itr == m_nodeid_to_nodeind.end()) { // not found, none
            return -1;
        }
        return itr->second;
    }

    /**
     * @brief adjacency matrix indices to array index, returns -1 when self loop in non-directed mode
     * 
     * @param ind_a 
     * @param ind_b 
     * @return int 
     */
    int ind(int ind_a, int ind_b) const noexcept {
        if (!m_is_directed && ind_a < ind_b)
            std::swap(ind_a, ind_b);

        coord c{ind_a, ind_b};

        if (!m_is_directed && c.x == c.y)
            return -1; // no self loops (diagonals)

        return c.to_index(m_n_nodes);
    }

    const int m_n_nodes = 0;
    const bool m_is_directed = false;
    std::vector<float> m_adjacency_matrix{};
    std::vector<Node*> m_nodes{}; // m_nodes indexing follows adjacency matrix, m_nodes at i corresponds to row and column i
    std::unordered_map<int, int> m_nodeid_to_nodeind{};
};

/**
 * @brief Pattern is a valid configuration of cell values in the generated output
 *
 */
class Pattern
{
public:
    bool valid(std::vector<Node *> neighborhood) const;

    std::vector<Value> required_values{};
    Value cell_value{};
    float weight = 0.f; // relative probabilistic weight on this pattern
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
        decltype(node->domain) new_domain{};
        for (auto &p : node->domain) {
            if(p->valid(graph->adjacent_nodes(node)))
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
        assert(node->domain.size() >= 1); // TODO need to backtrack here, and not crash the program
        static std::random_device rd;
        static std::mt19937 gen(rd());
        if (node->domain.size() == 1) {
            node->value = node->domain[0]->cell_value;
            return;
        } else {
            // weighted random selection of available domain values
            std::vector<float> weights{};
            for (auto &p : node->domain) {
                weights.push_back(p->weight);
            }
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            node->value = node->domain[dist(gen)]->cell_value;
        }
    }

    std::queue<Node *> propagation_stack;
    Graph *graph;
    std::vector<Pattern> patterns;
};

} // namespace wfc

#endif // WFC_H