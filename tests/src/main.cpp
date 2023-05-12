#include <memory>
#include <reference_counted.hpp>
#include <util.hpp>
#include <renderer/buffer.hpp>

#include "events/notifier.hpp"
#include "delegate.hpp"

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

class PrintListener : public ev2::Listener<const int&> {
public:
    void update(const int& message) override {
        std::cout << "Got message " << message << std::endl;
    }

    void unsubscribe(const ev2::INotifier<const int&>* notifier) override {
        ev2::Listener<const int&>::unsubscribe(notifier);
        std::cout << "unsubscribed" << std::endl;
    }
};

void test_events() {
    auto intNotifier = std::make_shared<ev2::Notifier<const int&>>();

    PrintListener listener{};
    listener.subscribe(intNotifier.get());

    intNotifier->notify(5);

    intNotifier.reset();

    std::cout << "End " << __FUNCTION__ << std::endl;
}

void test_delegate() {
    delegate<int(int, int)> del{};

    auto fn = [](int a, int b) -> int {
        return a + b;
    };

    del = decltype(del)::create(fn);

    std::cout << del(2, 3) << std::endl;

    struct S {
        int foo(int a, int b) {
            return a + b;
        }
    } s;

    del = decltype(del)::create<S, &S::foo>(&s);

    std::cout << del(2, 3) << std::endl;
}

int main() {
    ref_test0();

    no_copyable();

    test_events();

    test_delegate();

    return 0;
}