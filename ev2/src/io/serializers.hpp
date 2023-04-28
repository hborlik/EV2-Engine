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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pattern, required_values, cell_value, weight)

} // namespace wfc




#endif // EV2_PCG_SERIALIZERS_HPP