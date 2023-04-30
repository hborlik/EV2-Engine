/**
 * @file serializers.hpp
 * @brief 
 * @date 2023-04-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef EV2_IO_SERIALIZERS_HPP
#define EV2_IO_SERIALIZERS_HPP

#include <vector>
#include <fstream>
#include <unordered_map>

#include "geometry.hpp"
#include "pcg/sc_wfc.hpp"
#include "pcg/wfc.hpp"
#include "pcg/object_database.hpp"

#include <json.hpp>

namespace ev2::io {

std::string read_file(std::string_view path);

} // namespace ev2::io


namespace ev2 {

void to_json(nlohmann::json& j, const OBB& p);

void from_json(const nlohmann::json& j, OBB& p);

} // namespace ev2


namespace ev2::pcg {

void to_json(nlohmann::json& j, const ObjectData& p);

void from_json(const nlohmann::json& j, ObjectData& p);

} // namespace pcg


namespace wfc {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Value, val)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pattern, required_classes, pattern_class, weight)

} // namespace wfc

// // partial specialization (full specialization works too)
// namespace nlohmann {
//     template <typename Key, typename Tp>
//     struct adl_serializer<std::unordered_multimap<Key, Tp>> {
//         static void to_json(json& j, const std::unordered_multimap<Key, Tp>& opt) {
//             if (opt == boost::none) {
//                 j = nullptr;
//             } else {
//                 j = *opt;   // this will call adl_serializer<T>::to_json which will
//                             // find the free function to_json in T's namespace!
//             }
//         }

//         static void from_json(const json& j, std::unordered_multimap<Key, Tp>& opt) {
//             if (j.is_null()) {
//                 opt = boost::none;
//             } else {
//                 opt = j.get<T>(); // same as above, but with
//                                   // adl_serializer<T>::from_json
//             }
//         }
//     };
// }


#endif // EV2_PCG_SERIALIZERS_HPP