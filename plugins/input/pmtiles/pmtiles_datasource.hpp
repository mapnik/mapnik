/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef PMTILES_DATASOURCE_HPP_
#define PMTILES_DATASOURCE_HPP_

#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/datasource_plugin.hpp>

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

DATASOURCE_PLUGIN_DEF(pmtiles_datasource_plugin, pmtiles);

namespace mapnik {

using zxy_type = std::tuple<std::uint8_t, std::uint32_t, std::uint32_t>;

class pmtiles_file; //fwd decl
}

class pmtiles_datasource : public mapnik::datasource
{
public:
    pmtiles_datasource(mapnik::parameters const& params);
    virtual ~pmtiles_datasource ();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    void init(mapnik::parameters const& params);
    mapnik::context_ptr get_context_with_attributes() const;
    mapnik::context_ptr get_query_context(mapnik::query const& q) const;
    std::string database_path_;
    std::shared_ptr<mapnik::pmtiles_file> file_ptr_;
    static std::unordered_map<std::string, std::string> & tile_cache();
public:
    mapnik::box2d<double> extent_;
    std::int64_t minzoom_ = 0;
    std::int64_t maxzoom_ = 14;
    std::int64_t zoom_ = 14;
    std::string json_;
    std::string layer_;
    mapnik::layer_descriptor desc_;
};



#endif // PMTILES_DATASOURCE_HPP_
