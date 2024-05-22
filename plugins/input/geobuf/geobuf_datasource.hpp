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

#ifndef GEOBUF_DATASOURCE_HPP
#define GEOBUF_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_plugin.hpp>
// boost
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
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

using mapnik::datasource;

template<std::size_t Max, std::size_t Min>
struct geobuf_linear : boost::geometry::index::linear<Max, Min>
{};

namespace boost {
namespace geometry {
namespace index {
namespace detail {
namespace rtree {

template<std::size_t Max, std::size_t Min>
struct options_type<geobuf_linear<Max, Min>>
{
    using type = options<geobuf_linear<Max, Min>,
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

DATASOURCE_PLUGIN_DEF(geobuf_datasource_plugin, geobuf);
class geobuf_datasource : public mapnik::datasource
{
  public:
    using box_type = mapnik::box2d<double>;
    using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;
    using spatial_index_type = boost::geometry::index::rtree<item_type, geobuf_linear<16, 4>>;

    // constructor
    geobuf_datasource(mapnik::parameters const& params);
    virtual ~geobuf_datasource();
    mapnik::datasource::datasource_t type() const override;
    static const char* name();
    mapnik::featureset_ptr features(mapnik::query const& q) const override;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const override;
    mapnik::box2d<double> envelope() const override;
    mapnik::layer_descriptor get_descriptor() const override;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const override;
    void parse_geobuf(char const* buffer, std::size_t size);

  private:
    mapnik::datasource::datasource_t type_;
    mapnik::layer_descriptor desc_;
    std::string filename_;
    mapnik::box2d<double> extent_;
    std::vector<mapnik::feature_ptr> features_;
    std::unique_ptr<spatial_index_type> tree_;
};

#endif // GEOBUF_DATASOURCE_HPP
