#include <memory>
#include <iostream>
#include <set>

#include <pcg/wfc.hpp>

using namespace std;
using namespace wfc;

void sparse_test_empty() {
    // ensure empty graph behaves properly
    SparseGraph s{};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}


void dense_test_empty() {
    // ensure empty graph behaves properly
    DenseGraph s{100};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}

void adjacent_abc(Graph* s) {

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    s->add_edge(n_a.get(), n_b.get(), 1.1f);
    s->add_edge(n_b.get(), n_c.get(), 1.2f);

    assert(s->adjacent(a, b) == 1.1f);
    assert(s->adjacent(b, a) == 1.1f);
    assert(s->adjacent(b, c) == 1.2f);
    assert(s->adjacent(c, b) == 1.2f);
    assert(s->adjacent(a, c) == 0.f);
    assert(s->adjacent(c, a) == 0.f);
}

void sparse_test_add() {
    SparseGraph s{};

    adjacent_abc(&s);
}

void dense_test_add() {
    DenseGraph s{4};

    adjacent_abc(&s);
}

void directed_adjacent_abc(Graph* s) {
    assert(s->is_directed());

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    s->add_edge(a, b, 1.1f);
    s->add_edge(b, a, 2.2f);
    s->add_edge(b, c, 1.2f);
    s->add_edge(c, c, 5.f);

    assert(s->adjacent(a, b) == 1.1f);
    assert(s->adjacent(b, a) == 2.2f);
    assert(s->adjacent(b, c) == 1.2f);
    assert(s->adjacent(c, b) == 0.f);
    assert(s->adjacent(b, a) == 2.2f);
    assert(s->adjacent(c, c) == 5.f);

    assert(s->adjacent(a, c) == 0.f);
    assert(s->adjacent(c, a) == 0.f);
}

void sparse_directed_add() {
    SparseGraph s{true};

    directed_adjacent_abc(&s);
}

void dense_directed_add() {
    DenseGraph d{4, true};

    directed_adjacent_abc(&d);
}

int main() {
    sparse_test_empty();
    sparse_test_add();
    sparse_directed_add();

    dense_test_empty();
    dense_test_add();
    dense_directed_add();

    cout << "Done" << endl;

    return 0;
}