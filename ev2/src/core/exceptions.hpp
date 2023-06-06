#ifndef EV_EXCEPTIONS_HPP
#define EV_EXCEPTIONS_HPP

namespace ev2 {

class engine_exception : public std::exception {
public:
    engine_exception(std::string description) noexcept : description{std::move(description)} {}
    virtual ~engine_exception() = default;

    const char* what() const noexcept override {
        return description.data();
    }

protected:
    std::string description;
};

}

#endif // EV_EXCEPTIONS_HPP