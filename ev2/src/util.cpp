#include <util.h>

#include <cxxabi.h>
#include <random>

namespace ev2::util {

std::string name_demangle(const std::string& mangled_name) noexcept {
    int status;
    char* demangled_name = abi::__cxa_demangle(mangled_name.c_str(), 0, 0, &status);
    std::string name{demangled_name};
    if (demangled_name)
        std::free(demangled_name);
    return name;
}

// from https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
std::string get_unique_id() {
    using namespace std;
    static random_device dev;
    static mt19937 rng(dev());

    uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

}