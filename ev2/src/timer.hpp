/**
 * @file timer.cpp
 * @brief 
 * @date 2023-05-22
 * 
 * 
 */
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <ostream>
#include <iostream>
#include <iomanip>


class Timer {
public:
    Timer(std::string_view name, bool print = true) :
        m_name{name.data()},
        m_timer_start{std::chrono::system_clock::now()},
        m_print{print} {}

    ~Timer() {
        if (!m_stopped) {
            stop();
            if (m_print)
                std::cout << to_string(*this) << std::endl;
        }
    }

    void stop() noexcept {
        if (m_stopped)
            return;
        m_timer_stop = std::chrono::system_clock::now();
        m_stopped = true;
    }

    auto elapsed_ms() const noexcept {
        std::chrono::duration<double, std::milli> elapsed_ms = m_timer_stop - m_timer_start;
        return elapsed_ms.count();
    }

private:
    friend std::ostream& operator<<(std::ostream& os, const Timer& timer) {
        os << timer.m_name << " " << std::setprecision(3) << timer.elapsed_ms() << "ms";
        return os;
    }

    friend std::string to_string(const Timer& x) {
        std::ostringstream ss;
        ss << x;
        return ss.str();
    }

private:
    const char* m_name;
    std::chrono::time_point<std::chrono::system_clock> m_timer_start, m_timer_stop;
    bool m_stopped = false;
    const bool m_print = true;
};

#endif // TIMER_HPP