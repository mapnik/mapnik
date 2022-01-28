/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef GEOJSON_DATASOURCE_HPP
#define GEOJSON_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/unicode.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#include <boost/version.hpp>
#include <boost/geometry/strategies/cartesian/disjoint_box_box.hpp>
#include <boost/geometry/index/rtree.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <functional>

template<std::size_t Max, std::size_t Min>
struct geojson_linear : boost::geometry::index::linear<Max, Min>
{};

namespace boost {
namespace geometry {
namespace index {
namespace detail {
namespace rtree {

template<std::size_t Max, std::size_t Min>
struct options_type<geojson_linear<Max, Min>>
{
    using type = options<geojson_linear<Max, Min>,
                         insert_default_tag,
                         choose_by_content_diff_tag,
                         split_default_tag,
                         linear_tag,
#if BOOST_VERSION >= 105700
                         node_variant_static_tag>;
#else
                         node_s_mem_static_tag>;

#endif
};

} // namespace rtree
} // namespace detail
} // namespace index
} // namespace geometry
} // namespace boost

class geojson_datasource : public mapnik::datasource
{
  public:
    using box_type = mapnik::box2d<double>;
    using item_type = std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>;
    using spatial_index_type = boost::geometry::index::rtree<item_type, geojson_linear<16, 4>>;
    // constructor
    geojson_datasource(mapnik::parameters const& params);
    virtual ~geojson_datasource();
    mapnik::datasource::datasource_t type() const;
    static const char* name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    mapnik::layer_descriptor get_descriptor() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    template<typename Iterator>
    void parse_geojson(Iterator start, Iterator end);
    template<typename Iterator>
    void initialise_index(Iterator start, Iterator end);
    void initialise_disk_index(std::string const& filename);

  private:
    void initialise_descriptor(mapnik::feature_ptr const&);
    mapnik::datasource::datasource_t type_;
    mapnik::layer_descriptor desc_;
    std::string filename_;
    bool from_inline_string_;
    mapnik::box2d<double> extent_;
    std::vector<mapnik::feature_ptr> features_;
    std::unique_ptr<spatial_index_type> tree_;
    bool cache_features_ = true;
    bool has_disk_index_ = false;
    const std::size_t num_features_to_query_;
};

#endif // GEOJSON_DATASOURCE_HPP
