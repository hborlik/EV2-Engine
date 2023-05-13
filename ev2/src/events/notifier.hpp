/**
 * @file notifier.hpp
 * @brief 
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_EVENT_HPP
#define EV2_EVENT_HPP

#include "evpch.hpp"

#include "delegate.hpp"

namespace ev2 {

template<typename T>
class INotifier;

template<typename T>
class IListener {
public:
    virtual ~IListener() = default;
    virtual void update(T message) = 0;
    virtual void unsubscribe(const INotifier<T>* notifier) = 0;
};

template<typename T>
class INotifier {
public:
    virtual ~INotifier() = default;
    virtual void attach(IListener<T>* listener) = 0;
    virtual void detach(IListener<T>* listener) = 0;

    virtual void notify(T message) = 0;
};

template<>
class INotifier<void> {
public:
    virtual ~INotifier(){};
    virtual void attach(IListener<void>* listener) = 0;
    virtual void detach(IListener<void>* listener) = 0;
    virtual void notify() = 0;
};

// Default implementations for Notifier and listener

template<typename T>
class Notifier : public INotifier<T> {
    using listener_set_t = std::unordered_set<IListener<T>*>;

public:
    Notifier() = default;
    ~Notifier() {
        for (auto itr = m_listeners.begin(); itr != m_listeners.end();) {
             // the iterator will become invalid once unsubscribe is called,
             // so post increment will handle this
            (*itr++)->unsubscribe(this);
        }
    }

    Notifier(const Notifier&) = delete;
    Notifier(Notifier&&) = delete;

    Notifier& operator=(const Notifier&) = delete;
    Notifier& operator=(Notifier&&) = delete;

    void attach(IListener<T>* listener) override {
        assert(listener);
        m_listeners.insert(listener);
    }

    void detach(IListener<T>* listener) override {
        assert(listener);
        m_listeners.erase(listener);
    }

    void notify(T message) override {
        for (auto listener : m_listeners) {
            listener->update(message);
        }
    }

private:
    listener_set_t m_listeners{};
};

template<typename T>
class Listener : public IListener<T> {
public:
    Listener() = default;
    ~Listener() {
        unsubscribe(m_subscribed);
    }

    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;

    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&) = delete;

    void update(T message) override {}

    void unsubscribe(const INotifier<T>* notifier) override {
        assert(notifier == m_subscribed);
        if (m_subscribed) m_subscribed->detach(this);
    }

    void subscribe(INotifier<T>* notifier) {
        unsubscribe(m_subscribed);
        m_subscribed = notifier;
        notifier->attach(this);
    }

    bool is_subscribed() const noexcept {return m_subscribed;}

private:
    INotifier<T>* m_subscribed{};
};

/**
 * @brief Listener designed to work with delegates. Calls delegate
 *  function when notified.
 * 
 * @tparam T 
 * @tparam _Fn 
 */
template<typename T, typename _Fn = void(T)>
class DelegateListener : public Listener<T> {
public:
    using delegate_t = delegate<_Fn>;
    
    DelegateListener() = default;
    DelegateListener(delegate_t fn) : m_delegate{std::move(fn)} {}

    void update(T message) override {
        if(m_delegate)
            m_delegate(message);
    }

private:
    delegate_t m_delegate{};
};

}

#endif  // EV2_EVENT_HPP