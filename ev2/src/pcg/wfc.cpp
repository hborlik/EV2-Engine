#include <pcg/wfc.hpp>
#include <unordered_map>

namespace wfc {

bool Pattern::valid(const std::vector<DGraphNode*>& neighborhood) const {
    /* matching problem (edges are between required values and neighbors with that value in domain)
     * map required values one-to-one (perfect matching) with available neighbors
     * if all requirements are satisfied, return true
     * */
    if (required_types.size() == 0)
        return true;

    if (neighborhood.size() == 0)
        return false;

    // can't satisfy requirements
    if (required_types.size() > neighborhood.size())
        return false;

    using GN = Node;

    struct ValueAndNode {
        int v;
        GN* p;
    };
    std::vector<ValueAndNode> neigh_values{};   // values in neighborhood with associated node in flow graph
    std::vector<ValueAndNode> required{};       // required values and associated node in flow graph

    neigh_values.reserve(neighborhood.size());
    required.reserve(required_types.size());

    // extra two capacity for source and sink
    DenseGraph<GN> dg{ (int)neighborhood.size() + (int)required_types.size() + 2, true };

    /* construct flow graph */

    // node references
    auto source = std::make_unique<GN>(2);
    auto sink = std::make_unique<GN>(3);
    std::vector<std::unique_ptr<GN>> req_nodes{};
    std::vector<std::unique_ptr<GN>> neigh_nodes{};
    
    // for each required value, create a node in flow graph and connect it to source
    int id = 4;
    for (auto& rv : required_types) {
        auto req_node = std::make_unique<GN>(++id);
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
        auto domain_node = std::make_unique<GN>(++id);
        dg.add_edge(domain_node.get(), sink.get(), 1.f);
        for (auto val : node->domain) {
            neigh_values.emplace_back(ValueAndNode{
                .v = val.type, // requirements are only for type, not value
                .p = domain_node.get()
            });
        }

        // save reference for cleanup
        neigh_nodes.emplace_back(std::move(domain_node));
    }

    auto sort_vn = [](const ValueAndNode& a, const ValueAndNode& b) {
        // sorted high to low cell values
        return a.v < b.v;
    };

    // sort by the cell values
    std::sort(required.begin(), required.end(), sort_vn);
    std::sort(neigh_values.begin(), neigh_values.end(), sort_vn);

    // find the nodes associated with each required value by stepping through the sorted lists
    for (const auto& req : required) {
        auto vn_pos = neigh_values.begin();
        bool has_single_neighbor = false;
        while(vn_pos != neigh_values.end()) {
            if (vn_pos->v == req.v) {
                // connect required value node with neighbor node that
                // has required value in its domain
                dg.add_edge(req.p, vn_pos->p, 1.f);
                // there is at least one node satisfying the requirement
                has_single_neighbor = true;
            } else if (vn_pos->v > req.v) {
                if (has_single_neighbor == false)
                    return false;
                break; // did not match value
            }
            ++vn_pos;
        }
    }

    // std::cout << dg << std::endl;

    float flow = ford_fulkerson(dg, source.get(), sink.get(), &dg);

    // check that all requirements are met
    if (flow != required_types.size())
        return false;

    return true;
}


bool Pattern::valid_approx(const std::vector<DGraphNode*>& neighborhood) const {
    std::unordered_multiset<int> requirements{required_types.begin(), required_types.end()};
    std::vector<int> available_values{};

    for (const auto& n : neighborhood) {
        for (const auto& v : n->domain) {
            available_values.push_back(v.type);
        }
    }

    for (auto v : available_values) {
        auto itr = requirements.find(v);
        if (itr != requirements.end())
            requirements.erase(itr);
    }

    return requirements.empty();
}



}