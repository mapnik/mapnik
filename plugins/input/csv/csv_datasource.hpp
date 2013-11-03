/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_CSV_DATASOURCE_HPP
#define MAPNIK_CSV_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/value_types.hpp>

// boost
#include <boost/optional.hpp>

// stl
#include <vector>
#include <deque>
#include <string>

class csv_datasource : public mapnik::datasource
{
public:
    csv_datasource(mapnik::parameters const& params);
    virtual ~csv_datasource ();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

    template <typename T>
    void parse_csv(T & stream,
                   std::string const& escape,
                   std::string const& separator,
                   std::string const& quote);

private:
    mapnik::layer_descriptor desc_;
    mapnik::box2d<double> extent_;
    std::string filename_;
    std::string inline_string_;
    unsigned file_length_;
    mapnik::value_integer row_limit_;
    std::deque<mapnik::feature_ptr> features_;
    std::string escape_;
    std::string separator_;
    std::string quote_;
    std::vector<std::string> headers_;
    std::string manual_headers_;
    bool strict_;
    double filesize_max_;
    mapnik::context_ptr ctx_;
    bool extent_initialized_;
};

#endif // MAPNIK_CSV_DATASOURCE_HPP
