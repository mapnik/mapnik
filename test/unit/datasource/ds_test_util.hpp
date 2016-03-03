/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/


#ifndef MAPNIK_UNIT_DATSOURCE_UTIL
#define MAPNIK_UNIT_DATSOURCE_UTIL

#include "catch.hpp"

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/geometry_type.hpp>

namespace {

template <typename T>
std::string vector_to_string(T const& vec)
{
    std::stringstream s;
    for (auto const& item : vec)
    {
        s << "  " << item << "\n";
    }
    return s.str();
}

template <>
std::string vector_to_string(std::vector<mapnik::attribute_descriptor> const& vec)
{
    std::stringstream s;
    for (auto const& item : vec)
    {
        s << "  " << item.get_name() << "\n";
    }
    return s.str();
}

#define REQUIRE_FIELD_NAMES(fields, names) \
    INFO("fields:\n" + vector_to_string(fields) + "names:\n" +  vector_to_string(names)); \
    REQUIRE(fields.size() == names.size()); \
    auto itr_a = fields.begin(); \
    auto const end_a = fields.end(); \
    auto itr_b = names.begin(); \
    for (; itr_a != end_a; ++itr_a, ++itr_b) \
    { \
        CHECK(itr_a->get_name() == *itr_b); \
    } \

inline void require_field_names(std::vector<mapnik::attribute_descriptor> const &fields,
                         std::initializer_list<std::string> const &names)
{
    REQUIRE_FIELD_NAMES(fields,names);
}

#define REQUIRE_FIELD_TYPES(fields, types) \
    REQUIRE(fields.size() == types.size()); \
    auto itr_a = fields.begin(); \
    auto const end_a = fields.end(); \
    auto itr_b = types.begin(); \
    for (; itr_a != end_a; ++itr_a, ++itr_b) { \
        CHECK(itr_a->get_type() == *itr_b); \
    } \

inline void require_field_types(std::vector<mapnik::attribute_descriptor> const &fields,
                         std::initializer_list<mapnik::eAttributeType> const &types)
{
    REQUIRE_FIELD_TYPES(fields, types);
}

inline mapnik::featureset_ptr all_features(mapnik::datasource_ptr ds) {
    auto fields = ds->get_descriptor().get_descriptors();
    mapnik::query query(ds->envelope());
    for (auto const &field : fields) {
        query.add_property_name(field.get_name());
    }
    return ds->features(query);
}

inline std::size_t count_features(mapnik::featureset_ptr features) {
    std::size_t count = 0;
    while (features->next()) {
        ++count;
    }
    return count;
}

using attr = std::tuple<std::string, mapnik::value>;
inline void require_attributes(mapnik::feature_ptr feature,
                        std::initializer_list<attr> const &attrs) {
    REQUIRE(bool(feature));
    for (auto const &kv : attrs) {
        REQUIRE(feature->has_key(std::get<0>(kv)));
        CHECK(feature->get(std::get<0>(kv)) == std::get<1>(kv));
    }
}

namespace detail {
struct feature_count {
    template <typename T>
    std::size_t operator()(T const &geom) const {
        return mapnik::util::apply_visitor(*this, geom);
    }

    std::size_t operator()(mapnik::geometry::geometry_empty const &) const {
        return 0;
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::point<T> const &) const {
        return 1;
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::line_string<T> const &) const {
        return 1;
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::polygon<T> const &) const {
        return 1;
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::multi_point<T> const &mp) const {
        return mp.size();
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::multi_line_string<T> const &mls) const {
        return mls.size();
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::multi_polygon<T> const &mp) const {
        return mp.size();
    }

    template <typename T>
    std::size_t operator()(mapnik::geometry::geometry_collection<T> const &col) const {
        std::size_t sum = 0;
        for (auto const &geom : col) {
            sum += operator()(geom);
        }
        return sum;
    }
};
} // namespace detail

template <typename T>
inline std::size_t feature_count(mapnik::geometry::geometry<T> const &g) {
    return detail::feature_count()(g);
}

inline void require_geometry(mapnik::feature_ptr feature,
                      std::size_t num_parts,
                      mapnik::geometry::geometry_types type) {
    REQUIRE(bool(feature));
    CHECK(mapnik::geometry::geometry_type(feature->get_geometry()) == type);
    CHECK(feature_count(feature->get_geometry()) == num_parts);
}

inline int create_disk_index(std::string const& filename, bool silent = true)
{
    std::string cmd;
    if (std::getenv("DYLD_LIBRARY_PATH") != nullptr)
    {
        cmd += std::string("DYLD_LIBRARY_PATH=") + std::getenv("DYLD_LIBRARY_PATH") + " ";
    }
    cmd += "mapnik-index " + filename;
    if (silent)
    {
#ifndef _WINDOWS
        cmd += " 2>/dev/null";
#else
        cmd += " 2> nul";
#endif
    }
    return std::system(cmd.c_str());
}

}

#endif // MAPNIK_UNIT_DATSOURCE_UTIL
