#include <memory>
#include <iostream>
#include <set>

#include <pcg/wfc.hpp>
#include <pcg/grid.hpp>

using namespace std;
using namespace pcg;

void sparse_test_empty() {
    std::cout << __FUNCTION__ << std::endl;
    // ensure empty graph behaves properly
    SparseGraph<Node> s{};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}


void dense_test_empty() {
    std::cout << __FUNCTION__ << std::endl;
    // ensure empty graph behaves properly
    DenseGraph<Node> s{100};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}

void adjacent_abc(Graph<Node>* s) {

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
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<Node> s{};

    adjacent_abc(&s);
}

void dense_test_add() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> s{4};

    adjacent_abc(&s);
}

void directed_adjacent_abc(Graph<Node>* s) {
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
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<Node> s{true};

    directed_adjacent_abc(&s);
}

void dense_directed_add() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> d{4, true};

    directed_adjacent_abc(&d);
}

void dense_bfs_no_soln() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    std::vector<Node *> path;
    assert(g.bfs(a, b, path) == false);
}

void dense_bfs_no_soln_directional() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    g.add_edge(b, a, 1.f);
    g.add_edge(a, c, 1.f);
    g.add_edge(b, c, 1.f);

    std::vector<Node *> path;
    assert(g.bfs(a, b, path) == false);
}

void dense_bfs_soln_directional0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    std::vector<Node *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 3);

    std::vector<Node *> path_correct {
        a, b, c
    };
    assert(path_correct == path);
}

void dense_bfs_soln_directional1() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);
    unique_ptr<Node> n_d = make_unique<Node>("D", 4);
    unique_ptr<Node> n_e = make_unique<Node>("E", 5);
    unique_ptr<Node> n_f = make_unique<Node>("F", 6);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();
    Node *d = n_d.get();
    Node *e = n_e.get();
    Node *f = n_f.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    g.add_edge(a, d, 1.f);
    g.add_edge(d, e, 1.f);
    g.add_edge(e, f, 1.f);

    std::vector<Node *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 3);

    std::vector<Node *> path_correct {
        a, b, c
    };
    assert(path_correct == path);
}

void dense_bfs_soln_0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, false};

    unique_ptr<Node> n_a = make_unique<Node>("A", 1);
    unique_ptr<Node> n_b = make_unique<Node>("B", 2);
    unique_ptr<Node> n_c = make_unique<Node>("C", 3);

    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    std::vector<Node *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 2);

    std::vector<Node *> path_correct {
        a, c
    };
    assert(path_correct == path);
}

void dense_flow_soln_directional0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};

    unique_ptr<Node> n_s = make_unique<Node>("source", 1);
    unique_ptr<Node> n_t = make_unique<Node>("sink", 2);
    unique_ptr<Node> n_a = make_unique<Node>("A", 3);
    unique_ptr<Node> n_b = make_unique<Node>("B", 4);
    unique_ptr<Node> n_c = make_unique<Node>("C", 5);
    unique_ptr<Node> n_d = make_unique<Node>("D", 6);

    Node *s = n_s.get();
    Node *t = n_t.get();
    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();
    Node *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 0.9f);
    g.add_edge(c, d, 1.f);
    g.add_edge(d, t, 1.f);

    std::vector<Node *> path;
    assert(g.bfs(s, t, path) == true);

    float max_flow = ford_fulkerson(g, s, t, &g);
    assert(max_flow == 0.9f);
}

void dense_flow_soln_directional1() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};
    DenseGraph<Node> r{10, true};

    unique_ptr<Node> n_s = make_unique<Node>("source", 1);
    unique_ptr<Node> n_t = make_unique<Node>("sink", 2);
    unique_ptr<Node> n_a = make_unique<Node>("A", 3);
    unique_ptr<Node> n_b = make_unique<Node>("B", 4);
    unique_ptr<Node> n_c = make_unique<Node>("C", 5);
    unique_ptr<Node> n_d = make_unique<Node>("D", 6);

    Node *s = n_s.get();
    Node *t = n_t.get();
    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();
    Node *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(s, b, 1.f);
    g.add_edge(s, c, 1.f);
    g.add_edge(s, d, 1.f);

    g.add_edge(a, t, 1.1f);
    g.add_edge(b, t, 1.1f);
    g.add_edge(c, t, 1.1f);
    g.add_edge(d, t, 1.1f);

    std::vector<Node *> path;
    assert(g.bfs(s, t, path) == true);

    std::cout << g << std::endl;

    float max_flow = ford_fulkerson(g, s, t, &r);
    assert(max_flow == 4.f);

    std::cout << r << std::endl;
}

void dense_flow_soln_directional2() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<Node> g{10, true};
    DenseGraph<Node> r{10, true};

    unique_ptr<Node> n_s = make_unique<Node>("source", 1);
    unique_ptr<Node> n_t = make_unique<Node>("sink", 2);
    unique_ptr<Node> n_a = make_unique<Node>("A", 3);
    unique_ptr<Node> n_b = make_unique<Node>("B", 4);
    unique_ptr<Node> n_c = make_unique<Node>("C", 5);
    unique_ptr<Node> n_d = make_unique<Node>("D", 6);

    Node *s = n_s.get();
    Node *t = n_t.get();
    Node *a = n_a.get();
    Node *b = n_b.get();
    Node *c = n_c.get();
    Node *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(s, b, 1.f);

    g.add_edge(a, c, 1.f);
    g.add_edge(a, d, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(b, d, 1.f);

    g.add_edge(c, t, 1.1f);
    g.add_edge(d, t, 1.1f);

    std::vector<Node *> path;
    assert(g.bfs(s, t, path) == true);

    std::cout << g << std::endl;

    float max_flow = ford_fulkerson(g, s, t, &r);
    assert(max_flow == 2.f);

    std::cout << r << std::endl;
}

void test_pattern_validity0() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    Value v0{3};
    Value v1{30};
    Value v2{6};
    Value v3{60};

    Pattern p_center{v0, {v1, v2, v3}};

    // neighbor patterns
    Pattern p1{v1, {}};
    Pattern p2{v2, {}};
    Pattern p3{v3, {}};

    unique_ptr<DNode> n_a = make_unique<DNode>("A", 10);
    unique_ptr<DNode> n_b = make_unique<DNode>("B", 20);
    unique_ptr<DNode> n_c = make_unique<DNode>("C", 30);
    unique_ptr<DNode> n_d = make_unique<DNode>("D", 40);

    DNode *a = n_a.get();
    a->domain.push_back(&p1);
    a->domain.push_back(&p2);
    a->domain.push_back(&p3);

    DNode *b = n_b.get();
    b->domain.push_back(&p1);
    b->domain.push_back(&p2);
    b->domain.push_back(&p3);

    DNode *c = n_c.get();
    c->domain.push_back(&p1);
    c->domain.push_back(&p2);
    c->domain.push_back(&p3);

    std::vector<DNode*> neighborhood{a, b, c};

    assert(p_center.valid(neighborhood));
}

void test_pattern_validity1() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    Value v0{3};
    Value v1{30};
    Value v2{6};
    Value v3{60};
    Value v4{7};

    Pattern p_center{v0, {v1, v2, v3}};

    // neighbor patterns
    Pattern p1{v1, {}};
    Pattern p2{v2, {}};
    Pattern p3{v3, {}};
    Pattern p4{v4, {}};

    unique_ptr<DNode> n_a = make_unique<DNode>("A", 10);
    unique_ptr<DNode> n_b = make_unique<DNode>("B", 20);
    unique_ptr<DNode> n_c = make_unique<DNode>("C", 30);
    unique_ptr<DNode> n_d = make_unique<DNode>("D", 40);

    DNode *a = n_a.get();
    a->domain.push_back(&p1);

    DNode *b = n_b.get();
    b->domain.push_back(&p2);

    DNode *c = n_c.get();
    c->domain.push_back(&p3);

    DNode *d = n_d.get();
    d->domain.push_back(&p4);
    d->domain.push_back(&p2);

    std::vector<DNode*> neighborhood{a, b, c, d};

    assert(p_center.valid(neighborhood));
}

void test_pattern_validity2() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    Value v0{10};
    Value v1{11};

    Pattern p_center{v0, {v1, v1}};
    Pattern PB{v1, {v0}};

    unique_ptr<DNode> n_a = make_unique<DNode>("A", 10);
    unique_ptr<DNode> n_b = make_unique<DNode>("B", 20);
    unique_ptr<DNode> n_c = make_unique<DNode>("C", 30);

    DNode *a = n_a.get();
    a->domain.push_back(&PB);

    DNode *b = n_b.get();
    b->domain.push_back(&p_center);
    b->domain.push_back(&PB);

    DNode *c = n_c.get();
    c->domain.push_back(&p_center);
    c->domain.push_back(&PB);

    std::vector<DNode*> neighborhood{a, b, c};

    assert(p_center.valid(neighborhood));
}

int main() {
    sparse_test_empty();
    sparse_test_add();
    sparse_directed_add();

    dense_test_empty();
    dense_test_add();
    dense_directed_add();

    // bfs tests
    dense_bfs_no_soln();
    dense_bfs_no_soln_directional();
    dense_bfs_soln_directional0();
    dense_bfs_soln_directional1();
    dense_bfs_soln_0();

    // ford_fulkerson algo tests
    dense_flow_soln_directional0();
    dense_flow_soln_directional1();
    dense_flow_soln_directional2();

    // patterns
    test_pattern_validity0();
    test_pattern_validity1();
    test_pattern_validity2();

    NodeGrid ngrid{3, 3};
    std::cout << ngrid.get_graph() << std::endl;

    std::cout << to_string(ngrid) << std::endl;

    Pattern PA{Value{10}, {Value{11}, Value{11}}};
    Pattern PB{Value{11}, {Value{10}}};

    std::vector<const Pattern*> patterns{&PA, &PB};

    ngrid.reset(patterns);

    std::cout << to_string(ngrid) << std::endl;

    // wfc tests

    WFCSolver solver{&ngrid.get_graph()};

    DNode* next = ngrid.at(0, 0);

    while(next) {
        std::cout << next->identifier << std::endl;

        solver.collapse(next);

        std::cout << to_string(ngrid) << std::endl;

        next = solver.propagate(next);

        std::cout << to_string(ngrid) << std::endl;
    }

    //

    cout << "Done" << endl;

    return 0;
}