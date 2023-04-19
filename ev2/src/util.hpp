/**
 * @file util.h
 * @brief 
 * @date 2022-04-11
 * 
 */
#ifndef EV2_UTIL_H
#define EV2_UTIL_H

#include <string>
#include <type_traits>
#include <typeinfo>

namespace ev2::util {

std::string name_demangle(std::string_view mangled_name) noexcept;

/**
 * @brief Attempt to demangle a compiler generated name
 * 
 * @tparam T 
 * @return std::string 
 */
template<typename T>
inline std::string type_name() noexcept {
    return name_demangle(typeid(T).name());
}

/**
 * @brief Get a unique id
 * 
 * @return std::string 
 */
std::string get_unique_id();

/**
 * @brief used for generating log file names and times
 * 
 * @return std::string 
 */
std::string formatted_current_time();

// variadic min and max functions from https://stackoverflow.com/questions/23815138/implementing-variadic-min-max-functions

template<typename T>
inline T&& vmin(T&&t) noexcept {
    return std::forward<T>(t);
}

/**
 * @brief Return the min of a set of values
 * 
 * @tparam T0 
 * @tparam T1 
 * @tparam Ts 
 * @param val1 
 * @param val2 
 * @param vs 
 * @return std::common_type<T0, T1, Ts...>::type 
 */
template<typename T0, typename T1, typename... Ts>
inline auto vmin(T0&& val1, T1&& val2, Ts&&... vs) noexcept {
    return val2 < val1 ? vmin(val2, std::forward<Ts>(vs)...) : 
        vmin(val1, std::forward<Ts>(vs)...);
}

template<typename T>
inline T&& vmax(T&& t) noexcept {
    return std::forward<T>(t);
}

/**
 * @brief Return the max of a set of comparable values
 * 
 * @tparam T0 
 * @tparam T1 
 * @tparam Ts 
 * @param val1 
 * @param val2 
 * @param vs 
 * @return std::common_type<T0, T1, Ts...>::type 
 */
template<typename T0, typename T1, typename... Ts>
inline auto vmax(T0&& val1, T1&& val2, Ts&&... vs) noexcept {
    return val2 > val1 ? vmax(val2, std::forward<Ts>(vs)...) : 
        vmax(val1, std::forward<Ts>(vs)...);
}

}

#endif // EV2_UTIL_H