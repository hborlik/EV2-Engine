#include <memory>
#include <iostream>
#include <random>
#include <set>

#include <pcg/wfc.hpp>
#include <pcg/grid.hpp>

using namespace std;
using namespace wfc;

void sparse_test_empty() {
    std::cout << __FUNCTION__ << std::endl;
    // ensure empty graph behaves properly
    SparseGraph<GraphNode> s{};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}


void dense_test_empty() {
    std::cout << __FUNCTION__ << std::endl;
    // ensure empty graph behaves properly
    DenseGraph<GraphNode> s{100};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);

    assert(s.adjacent(n_a.get(), n_b.get()) == 0.f);
}

void adjacent_abc(Graph<GraphNode>* s) {

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    s->add_edge(n_a.get(), n_b.get(), 1.1f);
    s->add_edge(n_b.get(), n_c.get(), 1.2f);

    assert(s->adjacent(a, b) == 1.1f);
    assert(s->adjacent(b, a) == 1.1f);
    assert(s->adjacent(b, c) == 1.2f);
    assert(s->adjacent(c, b) == 1.2f);
    assert(s->adjacent(a, c) == 0.f);
    assert(s->adjacent(c, a) == 0.f);
}

void add_remove_abc(Graph<GraphNode>* s) {
    assert(s->is_directed() == false);

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    s->add_edge(n_a.get(), n_b.get(), 1.1f);
    s->remove_edge(n_a.get(), n_b.get());

    s->add_edge(n_b.get(), n_c.get(), 1.2f);

    assert(s->adjacent(a, b) == 0.f);
    assert(s->adjacent(b, a) == 0.f);

    assert(s->adjacent(b, c) == 1.2f);
    assert(s->adjacent(c, b) == 1.2f);

    assert(s->adjacent(a, c) == 0.f);
    assert(s->adjacent(c, a) == 0.f);

    assert(s->adjacent_nodes(a).size() == 0);
    assert(s->adjacent_nodes(b) == std::vector{c});
}

void add_remove_abcd(SparseGraph<GraphNode>* s) {
    assert(s->is_directed() == false);

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 4);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();

    s->add_edge(a, b, 1.1f);
    s->add_edge(b, c, 1.2f);

    s->add_edge(d, a, 1.5f);
    s->add_edge(d, b, 1.5f);
    s->add_edge(d, c, 1.5f);

    s->remove_node(d);

    auto ajd_set = [s](GraphNode* n) -> std::set<GraphNode*> {
        auto adj = s->adjacent_nodes(n);
        auto adj_s = std::set<GraphNode*>{adj.begin(), adj.end()};
        return adj_s;
    };

    assert(ajd_set(a) == std::set{b});
    assert((ajd_set(b) == std::set{a, c}));
    assert((ajd_set(c) == std::set{b}));
    assert(ajd_set(d) == std::set<GraphNode*>{});

    assert(s->adjacent(d, a) == 0.f);
    assert(s->adjacent(a, d) == 0.f);

    std::cout << *s << std::endl;
}

void add_remove_abcd_2(SparseGraph<GraphNode>* s) {
    assert(s->is_directed() == false);

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 4);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();

    s->add_edge(a, b, 1.f);
    s->add_edge(a, c, 1.f);
    s->add_edge(a, d, 1.f);

    s->add_edge(b, c, 1.f);
    s->add_edge(b, d, 1.f);

    s->add_edge(c, d, 1.f);

    // std::cout << *s << std::endl;

    s->remove_node(a);

    std::cout << *s << std::endl;

    auto ajd_set = [s](GraphNode* n) -> std::set<GraphNode*> {
        auto adj = s->adjacent_nodes(n);
        auto adj_s = std::set<GraphNode*>{adj.begin(), adj.end()};
        return adj_s;
    };

    assert(ajd_set(a) == std::set<GraphNode*>{});
    assert((ajd_set(b) == std::set{d, c}));
    assert((ajd_set(c) == std::set{d, b}));
    assert((ajd_set(d) == std::set{c, b}));

    assert(s->adjacent(a, b) == 0.f);
    assert(s->adjacent(b, a) == 0.f);

    assert(s->adjacent(a, c) == 0.f);
    assert(s->adjacent(c, a) == 0.f);

    assert(s->adjacent(a, d) == 0.f);
    assert(s->adjacent(d, a) == 0.f);

    assert(s->adjacent(b, c) == 1.f);
    assert(s->adjacent(c, b) == 1.f);

    assert(s->adjacent(b, d) == 1.f);
    assert(s->adjacent(d, b) == 1.f);

    assert(s->adjacent(c, d) == 1.f);
    assert(s->adjacent(d, c) == 1.f);

    std::cout << *s << std::endl;
}

void sparse_test_add() {
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<GraphNode> s{};

    adjacent_abc(&s);
}

void sparse_test_remove() {
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<GraphNode> s{};

    add_remove_abc(&s);
}

void sparse_test_remove1() {
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<GraphNode> s{};

    add_remove_abcd(&s);
}

void sparse_test_remove2() {
    std::cout << __FUNCTION__ << std::endl;
    SparseGraph<GraphNode> s{};

    add_remove_abcd_2(&s);
}

void dense_test_add() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> s{4};

    adjacent_abc(&s);
}

void directed_adjacent_abc(Graph<GraphNode>* s) {
    assert(s->is_directed());

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

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
    SparseGraph<GraphNode> s{true};

    directed_adjacent_abc(&s);
}

void dense_directed_add() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> d{4, true};

    directed_adjacent_abc(&d);
}

void dense_bfs_no_soln() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    std::vector<GraphNode *> path;
    assert(g.bfs(a, b, path) == false);
}

void dense_bfs_no_soln_directional() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    g.add_edge(b, a, 1.f);
    g.add_edge(a, c, 1.f);
    g.add_edge(b, c, 1.f);

    std::vector<GraphNode *> path;
    assert(g.bfs(a, b, path) == false);
}

void dense_bfs_soln_directional0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    std::vector<GraphNode *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 3);

    std::vector<GraphNode *> path_correct {
        a, b, c
    };
    assert(path_correct == path);
}

void dense_bfs_soln_directional1() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 4);
    unique_ptr<GraphNode> n_e = make_unique<GraphNode>("E", 5);
    unique_ptr<GraphNode> n_f = make_unique<GraphNode>("F", 6);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();
    GraphNode *e = n_e.get();
    GraphNode *f = n_f.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    g.add_edge(a, d, 1.f);
    g.add_edge(d, e, 1.f);
    g.add_edge(e, f, 1.f);

    std::vector<GraphNode *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 3);

    std::vector<GraphNode *> path_correct {
        a, b, c
    };
    assert(path_correct == path);
}

void dense_bfs_soln_0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, false};

    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 1);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 2);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 3);

    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();

    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(c, a, 1.f);

    std::vector<GraphNode *> path;
    assert(g.bfs(a, c, path) == true);
    assert(path.size() == 2);

    std::vector<GraphNode *> path_correct {
        a, c
    };
    assert(path_correct == path);
}

void dense_flow_soln_directional0() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};

    unique_ptr<GraphNode> n_s = make_unique<GraphNode>("source", 1);
    unique_ptr<GraphNode> n_t = make_unique<GraphNode>("sink", 2);
    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 3);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 4);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 5);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 6);

    GraphNode *s = n_s.get();
    GraphNode *t = n_t.get();
    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(a, b, 1.f);
    g.add_edge(b, c, 0.9f);
    g.add_edge(c, d, 1.f);
    g.add_edge(d, t, 1.f);

    std::vector<GraphNode *> path;
    assert(g.bfs(s, t, path) == true);

    float max_flow = ford_fulkerson(g, s, t, &g);
    assert(max_flow == 0.9f);
}

void dense_flow_soln_directional1() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};
    DenseGraph<GraphNode> r{10, true};

    unique_ptr<GraphNode> n_s = make_unique<GraphNode>("source", 1);
    unique_ptr<GraphNode> n_t = make_unique<GraphNode>("sink", 2);
    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 3);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 4);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 5);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 6);

    GraphNode *s = n_s.get();
    GraphNode *t = n_t.get();
    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(s, b, 1.f);
    g.add_edge(s, c, 1.f);
    g.add_edge(s, d, 1.f);

    g.add_edge(a, t, 1.1f);
    g.add_edge(b, t, 1.1f);
    g.add_edge(c, t, 1.1f);
    g.add_edge(d, t, 1.1f);

    std::vector<GraphNode *> path;
    assert(g.bfs(s, t, path) == true);

    std::cout << g << std::endl;

    float max_flow = ford_fulkerson(g, s, t, &r);
    assert(max_flow == 4.f);

    std::cout << r << std::endl;
}

void dense_flow_soln_directional2() {
    std::cout << __FUNCTION__ << std::endl;
    DenseGraph<GraphNode> g{10, true};
    DenseGraph<GraphNode> r{10, true};

    unique_ptr<GraphNode> n_s = make_unique<GraphNode>("source", 1);
    unique_ptr<GraphNode> n_t = make_unique<GraphNode>("sink", 2);
    unique_ptr<GraphNode> n_a = make_unique<GraphNode>("A", 3);
    unique_ptr<GraphNode> n_b = make_unique<GraphNode>("B", 4);
    unique_ptr<GraphNode> n_c = make_unique<GraphNode>("C", 5);
    unique_ptr<GraphNode> n_d = make_unique<GraphNode>("D", 6);

    GraphNode *s = n_s.get();
    GraphNode *t = n_t.get();
    GraphNode *a = n_a.get();
    GraphNode *b = n_b.get();
    GraphNode *c = n_c.get();
    GraphNode *d = n_d.get();

    g.add_edge(s, a, 1.f);
    g.add_edge(s, b, 1.f);

    g.add_edge(a, c, 1.f);
    g.add_edge(a, d, 1.f);
    g.add_edge(b, c, 1.f);
    g.add_edge(b, d, 1.f);

    g.add_edge(c, t, 1.1f);
    g.add_edge(d, t, 1.1f);

    std::vector<GraphNode *> path;
    assert(g.bfs(s, t, path) == true);

    std::cout << g << std::endl;

    float max_flow = ford_fulkerson(g, s, t, &r);
    assert(max_flow == 2.f);

    std::cout << r << std::endl;
}

void test_pattern_validity0() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    int v0{3};
    int v1{30};
    int v2{6};
    int v3{60};

    Pattern p_center{v0, {v1, v2, v3}};

    // neighbor patterns
    Pattern p1{v1, {}};
    Pattern p2{v2, {}};
    Pattern p3{v3, {}};

    unique_ptr<DGraphNode> n_a = make_unique<DGraphNode>("A", 10);
    unique_ptr<DGraphNode> n_b = make_unique<DGraphNode>("B", 20);
    unique_ptr<DGraphNode> n_c = make_unique<DGraphNode>("C", 30);
    unique_ptr<DGraphNode> n_d = make_unique<DGraphNode>("D", 40);

    DGraphNode *a = n_a.get();
    a->domain.push_back(&p1);
    a->domain.push_back(&p2);
    a->domain.push_back(&p3);

    DGraphNode *b = n_b.get();
    b->domain.push_back(&p1);
    b->domain.push_back(&p2);
    b->domain.push_back(&p3);

    DGraphNode *c = n_c.get();
    c->domain.push_back(&p1);
    c->domain.push_back(&p2);
    c->domain.push_back(&p3);

    std::vector<DGraphNode*> neighborhood{a, b, c};

    assert(p_center.valid(neighborhood));
}

void test_pattern_validity1() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    int v0{3};
    int v1{30};
    int v2{6};
    int v3{60};
    int v4{7};

    Pattern p_center{v0, {v1, v2, v3}};

    // neighbor patterns
    Pattern p1{v1, {}};
    Pattern p2{v2, {}};
    Pattern p3{v3, {}};
    Pattern p4{v4, {}};

    unique_ptr<DGraphNode> n_a = make_unique<DGraphNode>("A", 10);
    unique_ptr<DGraphNode> n_b = make_unique<DGraphNode>("B", 20);
    unique_ptr<DGraphNode> n_c = make_unique<DGraphNode>("C", 30);
    unique_ptr<DGraphNode> n_d = make_unique<DGraphNode>("D", 40);

    DGraphNode *a = n_a.get();
    a->domain.push_back(&p1);

    DGraphNode *b = n_b.get();
    b->domain.push_back(&p2);

    DGraphNode *c = n_c.get();
    c->domain.push_back(&p3);

    DGraphNode *d = n_d.get();
    d->domain.push_back(&p4);
    d->domain.push_back(&p2);

    std::vector<DGraphNode*> neighborhood{a, b, c, d};

    assert(p_center.valid(neighborhood));
}

void test_pattern_validity2() {
    std::cout << __FUNCTION__ << std::endl;

    // values
    int v0{10};
    int v1{11};

    Pattern p_center{v0, {v1, v1}};
    Pattern PB{v1, {v0}};

    unique_ptr<DGraphNode> n_a = make_unique<DGraphNode>("A", 10);
    unique_ptr<DGraphNode> n_b = make_unique<DGraphNode>("B", 20);
    unique_ptr<DGraphNode> n_c = make_unique<DGraphNode>("C", 30);

    DGraphNode *a = n_a.get();
    a->domain.push_back(&PB);

    DGraphNode *b = n_b.get();
    b->domain.push_back(&p_center);
    b->domain.push_back(&PB);

    DGraphNode *c = n_c.get();
    c->domain.push_back(&p_center);
    c->domain.push_back(&PB);

    std::vector<DGraphNode*> neighborhood{a, b, c};

    assert(p_center.valid(neighborhood));
}

void wfc_solver_grid0() {
    ev2::pcg::NodeGrid ngrid{3, 3};
    std::cout << ngrid.get_graph() << std::endl;

    std::cout << to_string(ngrid) << std::endl;

    Pattern PA{10, {11, 11}};
    Pattern PB{11, {10}};

    std::vector<const Pattern*> patterns{&PA, &PB};

    ngrid.reset_domains(patterns);

    std::cout << to_string(ngrid) << std::endl;

    // wfc tests

    std::random_device rd{};
    std::mt19937 gen{rd()};

    WFCSolver solver{&ngrid.get_graph(), &gen};

    DGraphNode* next = ngrid.at(0, 0);

    while(next) {
        std::cout << next->identifier << std::endl;

        solver.collapse(next);

        std::cout << to_string(ngrid) << std::endl;

        next = solver.propagate(next);

        std::cout << to_string(ngrid) << std::endl;
    }
}

int main() {
    sparse_test_empty();
    sparse_test_add();
    sparse_test_remove();
    sparse_test_remove1();
    sparse_test_remove2();

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

    // wfc
    wfc_solver_grid0();

    cout << "Done" << endl;

    return 0;
}