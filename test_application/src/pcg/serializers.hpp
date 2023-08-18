/**
 * @file serializers.hpp
 * @brief 
 * @date 2023-04-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef WFC_SERIALIZERS_HPP
#define WFC_SERIALIZERS_HPP

#include "evpch.hpp"

#include "geometry.hpp"
#include "sc_wfc.hpp"
#include "wfc.hpp"
#include "object_database.hpp"

#include <json.hpp>


namespace ev2::pcg {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectData::XYZ, v)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectData, name, asset_path, properties, propagation_patterns, extent, axis_settings)

} // namespace pcg


namespace wfc {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pattern, required_types, pattern_type, weight)

} // namespace wfc


#endif // WFC_SERIALIZERS_HPP