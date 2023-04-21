#include <reference_counted.hpp>
#include <util.hpp>
#include <renderer/buffer.hpp>

#include <iostream>

struct Foo : public ev2::ReferenceCounted<Foo> {

};

struct Glue : public Foo {};

void ref_test0() {
    ev2::Ref<Glue> glue = ev2::make_referenced<Glue>();
    ev2::Ref<Foo> f = glue;
    ev2::Ref<Foo> g(f);

    std::cout << f._ref->count << std::endl;
}


struct X {
    ev2::util::non_copyable<int> nocopy{0};
};

void no_copyable() {
    std::cout << __FUNCTION__ << std::endl;

    X x{1};

    // X b = x; // should cause compile error
    // X b{x}; // should cause compile error

    X b{std::move(x)};

    std::cout << (int)x.nocopy << std::endl;
    std::cout << (int)b.nocopy << std::endl;

    assert((int)x.nocopy == 0);
    assert((int)b.nocopy == 1);

    X d{2};

    d = std::move(b);

    assert((int)d.nocopy == 1);
    assert((int)b.nocopy == 2);

    // ev2::renderer::Buffer buf{ev2::gl::BindingTarget::SHADER_STORAGE, ev2::gl::Usage::DYNAMIC_DRAW};
    // ev2::renderer::Buffer cbuf{std::move(buf)};
}

int main() {
    ref_test0();

    no_copyable();

    return 0;
}