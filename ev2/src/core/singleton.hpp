/**
 * @file singleton.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.2
 * @date 2019-10-02
 * 
 * 
 */
#ifndef EV2_SINGLETON_H
#define EV2_SINGLETON_H

#include "evpch.hpp"

#include "core/base.hpp"
#include "util.hpp"

namespace ev2 {

template<typename T>
class Singleton {
    static std::shared_ptr<T> m_singleton;

protected:
    // no virtual destructor, there should be no upcasting to this class, no polymorphic handling
    Singleton() = default;
    ~Singleton() = default;

public:
    template<typename... _Args>
    static void initialize(_Args&&... __args) {
        m_singleton = std::unique_ptr<T>(new T(std::forward<_Args>(__args)...));
    }

    static void shutdown() {
        // m_singleton->shutdown();
        m_singleton = nullptr;
    }

    inline static T& get_singleton() {
        EV_CORE_ASSERT(m_singleton, "Singleton " + util::type_name<T>() + " is not initialized, or has already been shut down");
        return *m_singleton.get();
    }

    static bool is_initialized() noexcept {return (bool)m_singleton;}
};

template<typename T>
std::shared_ptr<T> Singleton<T>::m_singleton;

} // namespace ev2

#endif // EV2_SINGLETON_H