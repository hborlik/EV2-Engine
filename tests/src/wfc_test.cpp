#include <memory>
#include <iostream>

#include <pcg/wfc.hpp>

using namespace std;
using namespace wfc;

int test_empty() {
    // ensure empty graph behaves properly
    UndirectedSparseGraph s{};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);

    return (s.adjacent(n_a.get(), n_b.get()) == 0.f) ? 0 : -1;
}

int test_add() {
    UndirectedSparseGraph s{};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    s.edge(n_a.get(), n_b.get(), 1.1f);
    s.edge(n_b.get(), n_c.get(), 1.2f);

    float v = true;
    v = v && (s.adjacent(n_a.get(), n_b.get()) == 1.1f);
    if (!v) return __LINE__;
    v = v && (s.adjacent(n_b.get(), n_a.get()) == 1.1f);
    if (!v) return __LINE__;

    v = v && (s.adjacent(n_b.get(), n_c.get()) == 1.2f);
    if (!v) return __LINE__;
    v = v && (s.adjacent(n_c.get(), n_b.get()) == 1.2f);
    if (!v) return __LINE__;

    v = v && (s.adjacent(n_a.get(), n_c.get()) == 0.f);
    if (!v) return __LINE__;
    v = v && (s.adjacent(n_c.get(), n_a.get()) == 0.f);
    if (!v) return __LINE__;

    return 0;
}

int main() {
    assert(test_empty() == 0);
    assert(test_add() == 0);

    cout << "Done" << endl;

    return 0;
}