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

// boost
#include <boost/optional.hpp>

// stl
#include <vector>
#include <string>

class csv_datasource : public mapnik::datasource
{
public:
    csv_datasource(mapnik::parameters const& params, bool bind=true);
    virtual ~csv_datasource ();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;
    void bind() const;

    template <typename T>
    void parse_csv(T & stream,
                   std::string const& escape,
                   std::string const& separator,
                   std::string const& quote) const;

private:
    mutable mapnik::layer_descriptor desc_;
    mutable mapnik::box2d<double> extent_;
    mutable std::string filename_;
    mutable std::string inline_string_;
    mutable unsigned file_length_;
    mutable int row_limit_;
    mutable std::vector<mapnik::feature_ptr> features_;
    mutable std::string escape_;
    mutable std::string separator_;
    mutable std::string quote_;
    mutable std::vector<std::string> headers_;
    mutable std::string manual_headers_;
    mutable bool strict_;
    mutable bool quiet_;
    mutable double filesize_max_;
    mutable mapnik::context_ptr ctx_;
};

#endif // MAPNIK_CSV_DATASOURCE_HPP
