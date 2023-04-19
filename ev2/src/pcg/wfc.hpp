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
#include <iomanip>
#include <set>

#include <assert.h>

namespace pcg {

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
    bool operator<(const coord& other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        if (y < other.y) return true;
        return y > other.y;
    }

    bool operator==(const coord& other) const noexcept {
        return x == other.x && y == other.y;
    }

    int to_index(int row_count) const noexcept {
        return y * row_count + x;
    }
};

class Value
{
public:
    Value() = default;
    explicit Value(int cell_id): val{ cell_id } {}

    int val = 0; // cell tile identifier

private:

    friend bool operator==(const Value& a, const Value& b) noexcept {
        return a.val == b.val;
    }
};

} // namespace pcg

namespace std {

template<>
struct hash<pcg::coord>
{
    size_t operator()(const pcg::coord& k) const {
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

template<>
struct hash<pcg::Value>
{
    size_t operator()(const pcg::Value& k) const {
        using std::hash;

        return hash<int>()(k.val);
    }
};

} // namespace std

namespace pcg {

class Pattern;

class GraphNode {
public:
    GraphNode(const std::string& identifier, int node_id): node_id{ node_id }, identifier{ identifier } {
        assert(node_id > 0);
    }

    virtual ~GraphNode() = default;

    const int               node_id = -1;
    std::string             identifier = "";
};

/**
 * @brief Domain Node used in WFC for tracking valid domain on a cell
 * 
 */
class DGraphNode : public GraphNode {
public:
    DGraphNode(const std::string& identifier, int node_id) : GraphNode{identifier, node_id} {}

    float entropy() const;

    void set_value(const Pattern* p) noexcept;
    
    std::vector<const Pattern*> domain{};      // valid patterns for this node
    Value value{};
};

template<typename T,
    typename = std::enable_if_t<std::is_base_of_v<GraphNode, T>>>
class Graph {
public:
    virtual ~Graph() {}

    /**
     * @brief add edge
     *
     * @param a
     * @param b
     */
    virtual void add_edge(T* a, T* b, float v) = 0;

    /**
     * @brief check if nodes are adjacent, returns 0.f if not
     *
     * @param a
     * @param b
     * @return float
     */
    virtual float adjacent(T* a, T* b) const = 0;

    /**
     * @brief get adjacent nodes
     *
     * @param a
     * @return std::vector<Node*>
     */
    virtual std::vector<T*> adjacent_nodes(T* a) const = 0;

    virtual bool is_directed() const noexcept = 0;

    virtual int get_n_nodes() const noexcept = 0;

    /**
     * @brief make an unordered map of node ids to boolean flags
     *
     * @return std::unordered_map<int, bool>
     */
    virtual std::unordered_map<int, bool> make_visited_map() const = 0;
};

template<typename T>
class SparseGraph: public Graph<T>
{
private:

    struct internal_node {
        int mat_coord = -1;
        T* node = nullptr;
        std::vector<T*> adjacent_nodes{};
    };

    struct weight {
        float w = 0.f;
        // quick reference for finding adjacent nodes
        internal_node* i_nodeA = nullptr;
        internal_node* i_nodeB = nullptr;
    };

public:

    SparseGraph() = default;
    SparseGraph(bool is_directed): m_is_directed{ is_directed } {}

    void add_edge(T* a, T* b, float v) override {
        assert(a != nullptr && b != nullptr);
        assert(m_is_directed || a != b);
        internal_node* a_i = add_node(a);
        internal_node* b_i = add_node(b);

        // enforce populating only the upper triangular matrix
        if (!m_is_directed && a_i->mat_coord > b_i->mat_coord) {
            std::swap(a_i, b_i);
            std::swap(a, b);
        }

        coord c{ a_i->mat_coord, b_i->mat_coord };

        if (sparse_adjacency_map.find(c) != sparse_adjacency_map.end())
            return; // already added

        // build adjacency information
        a_i->adjacent_nodes.push_back(b);
        if (!m_is_directed)
            b_i->adjacent_nodes.push_back(a);

        if (!m_is_directed && c.x == c.y)
            return; // no self loops (diagonals)

        auto& sam = sparse_adjacency_map[c];
        sam.i_nodeA = a_i;
        sam.i_nodeB = b_i;
        sam.w = v;
    }

    float adjacent(T* a, T* b) const override {
        assert(a != nullptr && b != nullptr);
        const internal_node* a_i = get_node(a);
        const internal_node* b_i = get_node(b);

        if (a_i == nullptr || b_i == nullptr)
            return 0.f;

        if (!m_is_directed && a_i->mat_coord > b_i->mat_coord)
            std::swap(a_i, b_i);

        coord c{ a_i->mat_coord, b_i->mat_coord };

        if (!m_is_directed && c.x == c.y)
            return 0.f; // no self loops (diagonals)

        auto itr = sparse_adjacency_map.find(c);
        if (itr != sparse_adjacency_map.end())
            return itr->second.w;
        return 0.f;
    }

    std::vector<T*> adjacent_nodes(T* a) const override {
        assert(a != nullptr);
        std::vector<T*> output{};
        const internal_node* a_i = get_node(a);

        if (a_i == nullptr)
            return output;

        return a_i->adjacent_nodes;
    }

    bool is_directed() const noexcept override { return m_is_directed; }

    int get_n_nodes() const noexcept override { return node_map.size(); }

    std::unordered_map<int, bool> make_visited_map() const override {
        std::unordered_map<int, bool> out(get_n_nodes());
        for (const auto& [k, _] : node_map)
            out.insert({ k, false });
        return out;
    }

private:
    internal_node* add_node(T* node) {
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

    const internal_node* get_node(T* node) const {
        const internal_node* i_node = nullptr;
        if (node) {
            auto itr = node_map.find(node->node_id);
            if (itr != node_map.end())
                i_node = &itr->second;
        }
        return i_node;
    }

    int get_next_mat_coord() noexcept { return next_mat_coord++; }

    friend std::ostream& operator<< (std::ostream& out, const SparseGraph<T>& graph) {
        out << "Sparse Graph. " << (graph.is_directed() ? "Directed" : "Undirected") << "\n";
        if (graph.is_directed())
            out << "_From column_ \n";
        out << std::setw(10) << "_";

        std::vector<T*> node_order{};
        for (auto& [i, i_node] : graph.node_map) {
            out << std::setw(10) << i_node.node->identifier;
            node_order.push_back(i_node.node);
        }
        out << "\n" << std::setprecision(5);
        for (auto& [i, i_node] : graph.node_map) {
            out << std::setw(10) << i_node.node->identifier;
            
            // print row of adjacencies for this node
            for (auto j_node : node_order) {
                out << std::setw(10) << graph.adjacent(i_node.node, j_node);
            }
            out << "\n";
        }
        return out;
    }

private:
    int next_mat_coord = 0;
    bool m_is_directed = false;
    std::unordered_map<int, internal_node> node_map{}; // node id to internal adjacency
    std::unordered_map<coord, weight> sparse_adjacency_map{};
};

/**
 * @brief Dense Graph
 *
 */
template<typename T>
class DenseGraph: public Graph<T> {
public:
    explicit DenseGraph(int n_nodes, bool directed = false): m_n_nodes{ n_nodes }, m_is_directed{ directed }, m_adjacency_matrix(n_nodes* n_nodes) {}

    DenseGraph(const DenseGraph&) = default;
    DenseGraph(DenseGraph&&) = default;

    DenseGraph& operator=(const DenseGraph&) = default;
    DenseGraph& operator=(DenseGraph&&) = default;

    /**
     * @brief A to B
     *
     * @param a
     * @param b
     * @param v
     */
    void add_edge(T* a, T* b, float v) override {
        assert(a != nullptr && b != nullptr);

        if (!m_is_directed && a->node_id < b->node_id)
            std::swap(a, b);

        int ind_a = check_node_index(a);
        int ind_b = check_node_index(b);
        
        assert(!(ind_a < 0 || ind_b < 0));

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
    float adjacent(T* a, T* b) const override {
        assert(a != nullptr && b != nullptr);
        assert(a->node_id < m_n_nodes && b->node_id < m_n_nodes);

        int ind_a = get_node_index(a);
        int ind_b = get_node_index(b);

        if (ind_a < 0 || ind_b < 0)
            return 0.0f; // 

        return m_adjacency_matrix[ind(ind_a, ind_b)];
    }

    float adjacent(int a_ind, int b_ind) const {
        assert(a_ind >= 0 && b_ind >= 0);
        assert(a_ind < m_n_nodes && b_ind < m_n_nodes);

        return m_adjacency_matrix[ind(a_ind, b_ind)];
    }

    float& adjacent(int a_ind, int b_ind) {
        assert(a_ind >= 0 && b_ind >= 0);
        assert(a_ind < m_nodes.size() && b_ind < m_nodes.size()); // node should already be in graph

        return m_adjacency_matrix[ind(a_ind, b_ind)];
    }

    /**
     * @brief get adjacent nodes
     *
     * @param a
     * @return std::vector<Node*>
     */
    std::vector<T*> adjacent_nodes(T* a) const override {
        assert(a != nullptr);
        assert(a->node_id < m_n_nodes);

        std::vector<T*> nodes{};

        int ind_a = get_node_index(a);
        if (ind_a < 0)
            return nodes; // node not in graph

        for (int i = 0; i < m_n_nodes; ++i) {
            if (i == ind_a)
                continue;
            coord c{ ind_a, i };
            int ind = c.to_index(m_n_nodes);
            if (m_adjacency_matrix[ind] > 0.f)
                nodes.push_back(m_nodes[i]); // m_nodes indexing follows adjacency matrix
        }
        return nodes;
    }

    bool is_directed() const noexcept override { return m_is_directed; }

    int get_n_nodes() const noexcept override { return m_nodes.size(); }

    const auto& get_nodes() const noexcept { return m_nodes; }

    int get_max_node_id() const noexcept { return m_n_nodes; }

    /**
     * @brief Get the internal index for a node in the adjacency matrix
     *
     * @param node
     * @return int
     */
    int get_node_index(const T* node) const {
        auto itr = m_nodeid_to_nodeind.find(node->node_id);
        if (itr == m_nodeid_to_nodeind.end()) { // not found, none
            return -1;
        }
        return itr->second;
    }

    std::unordered_map<int, bool> make_visited_map() const override {
        std::unordered_map<int, bool> out(get_n_nodes());
        for (const auto& [k, _] : m_nodeid_to_nodeind)
            out.insert({ k, false });
        return out;
    }

    /**
     * @brief Attempt to find b starting from a through a BFS
     *
     * @param a
     * @param b
     * @param path Path array of nodes from a to b if found
     * @return true path found
     * @return false
     */
    bool bfs(const T* a, const T* b, std::vector<T*>& path) const {
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

    /**
     * @brief
     *
     * @param a
     * @param b
     * @param parent parent index of all visited nodes in the graph
     * @return true path found
     * @return false
     */
    bool bfs(int a_ind, int b_ind, std::vector<int>& parent) const {
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

private:
    int check_node_index(T* node) {
        auto itr = m_nodeid_to_nodeind.find(node->node_id);
        if (itr == m_nodeid_to_nodeind.end()) { // not found, add node and allocate it a row in matrix
            int ind = m_nodes.size();
            if (ind < m_n_nodes) {
                m_nodes.push_back(node);
                m_nodeid_to_nodeind.insert({ node->node_id, ind });
            } else
                ind = -1;
            return ind;
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

        coord c{ ind_a, ind_b };

        if (!m_is_directed && c.x == c.y)
            return -1; // no self loops (diagonals)

        return c.to_index(m_n_nodes);
    }

    friend std::ostream& operator<< (std::ostream& out, const DenseGraph<T>& graph) {
        out << "Dense Graph. " << (graph.is_directed() ? "Directed" : "Undirected") << "\n";
        if (graph.is_directed())
            out << "_From column_ \n";
        out << std::setw(10) << "_";
        for (int j = 0; j < graph.get_n_nodes(); ++j) {
            out << std::setw(10) << graph.m_nodes[j]->identifier;
        }
        out << "\n" << std::setprecision(5);
        for (int i = 0; i < graph.get_n_nodes(); ++i) {
            out << std::setw(10) << graph.m_nodes[i]->identifier;
            for (int j = 0; j < graph.get_n_nodes(); ++j)
                out << std::setw(10) << graph.adjacent(i, j);
            out << "\n";
        }

        return out;
    }

    int m_n_nodes = 0;
    bool m_is_directed = false;
    std::vector<float> m_adjacency_matrix{};
    std::vector<T*> m_nodes{}; // m_nodes indexing follows adjacency matrix, m_nodes at i corresponds to row and column i
    std::unordered_map<int, int> m_nodeid_to_nodeind{};
};

/**
 * @brief return maximum flow through graph from source to sink
 *
 * @param dg            dense graph of flow capacities
 * @param source        source node
 * @param sink          sink node
 * @param residual_graph optional residual graph output
 * @return int max flow
 */
float ford_fulkerson(const DenseGraph<GraphNode>& dg, const GraphNode* source, const GraphNode* sink, DenseGraph<GraphNode>* residual_graph);

/**
 * @brief Pattern is a valid configuration of cell values in the generated output
 *
 */
class Pattern
{
public:
    explicit Pattern(const Value& v) noexcept: cell_value{ v } {}

    Pattern(const Value& v, std::initializer_list<Value> l, float weight = 1.f) noexcept: required_values{ l }, cell_value{ v }, weight{weight} {}

    bool valid(const std::vector<DGraphNode*>& neighborhood) const;

    std::vector<Value> required_values{};
    Value cell_value{};
    float weight = 1.f; // relative probabilistic weight on this pattern
};

/**
 * @brief WFC has two stages. 
 *      1. collapse a node to force it to have a single value
 *      2. propagate the changes applied to that node
 * 
 */
class WFCSolver
{
public:
    WFCSolver(Graph<DGraphNode>* graph): graph{ graph } {
        assert(graph != nullptr);
    }

    void step_wfc() {
        collapse(next_node);
        next_node = propagate(next_node);
    }

    bool can_continue() noexcept {return next_node != nullptr;}

    /**
     * @brief Propagate the wave function collapse algorithm
     *
     * @param node
     */
    DGraphNode* propagate(DGraphNode* node) {
        assert(node != nullptr);
        std::queue<DGraphNode*> propagation_stack;
        propagation_stack.push(node);
        DGraphNode* min_e = nullptr;
        float entropy = 0.f;
        bool f = true; // force propagation on the first node
        while (!propagation_stack.empty()) {
            DGraphNode* n = propagation_stack.front();
            propagation_stack.pop();
            if (observe(n) || f) { // only update neighbors if the domain changed
                f = false;
                for (auto& neighbor_n : graph->adjacent_nodes(n)) {
                    DGraphNode* neighbor = static_cast<DGraphNode*>(neighbor_n);
                    // if (neighbor->domain.size() == 1)
                        propagation_stack.push(neighbor);
                }
            }
            if (float en = n->entropy(); n != node && n->domain.size() > 1 && (!min_e || en < entropy)) {
                min_e = n;
                entropy = en;
            }
        }
        return min_e;
    }

    /**
     * @brief 
     *
     * @param node
     */
    bool observe(DGraphNode* node) {
        assert(node != nullptr);
        bool changed = false;
        decltype(node->domain) new_domain{};
        for (auto& p : node->domain) {
            if (p->valid(graph->adjacent_nodes(node)))
                new_domain.push_back(p);
            else
                changed = true;
        }
        node->domain = new_domain;
        return changed;
    }

    /**
     * @brief Collapse the node to a single value. Performing a weighted random selection if multiple values are available.
     *
     * @param node
     */
    void collapse(DGraphNode* node) {
        assert(node != nullptr);
        assert(node->domain.size() >= 1); // TODO need to backtrack here, and not crash the program
        static std::random_device rd;
        static std::mt19937 gen(rd());
        if (node->domain.size() == 1) {
            node->set_value(node->domain[0]);
            return;
        } else {
            // weighted random selection of available domain values
            std::vector<float> weights{};
            for (auto& p : node->domain) {
                weights.push_back(p->weight);
            }
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            node->set_value(node->domain[dist(gen)]);
        }
    }

    Graph<DGraphNode>* graph = nullptr;
    DGraphNode* next_node = nullptr;
};

} // namespace pcg

#endif // WFC_H