/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include "shape_io.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/geom_util.hpp>
// boost

using mapnik::datasource_exception;
using mapnik::geometry_type;
using mapnik::hit_test_first;
const std::string shape_io::SHP = ".shp";
const std::string shape_io::DBF = ".dbf";
const std::string shape_io::INDEX = ".index";

shape_io::shape_io(std::string const& shape_name, bool open_index)
    : type_(shape_null),
      shp_(shape_name + SHP),
      dbf_(shape_name + DBF),
      reclength_(0),
      id_(0)
{
    bool ok = (shp_.is_open() && dbf_.is_open());
    if (! ok)
    {
        throw datasource_exception("Shape Plugin: cannot read shape file '" + shape_name + "'");
    }

    if (open_index)
    {
        try
        {
            index_ = std::make_unique<shape_file>(shape_name + INDEX);
        }
        catch (...)
        {
            MAPNIK_LOG_WARN(shape) << "shape_io: Could not open index=" << shape_name << INDEX;
        }
    }
}

shape_io::~shape_io() {}

void shape_io::move_to(std::streampos pos)
{
    shp_.seek(pos);
    id_ = shp_.read_xdr_integer();
    reclength_ = shp_.read_xdr_integer();
}

shape_file& shape_io::shp()
{
    return shp_;
}

dbf_file& shape_io::dbf()
{
    return dbf_;
}

void shape_io::read_bbox(shape_file::record_type & record, mapnik::box2d<double> & bbox)
{
    double lox = record.read_double();
    double loy = record.read_double();
    double hix = record.read_double();
    double hiy = record.read_double();
    bbox.init(lox, loy, hix, hiy);
}

mapnik::new_geometry::geometry shape_io::read_polyline(shape_file::record_type & record)
{
    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();

    if (num_parts == 1)
    {

        mapnik::new_geometry::line_string line;
        line.reserve(num_points);
        record.skip(4);
        for (int i = 0; i < num_points; ++i)
        {
            double x = record.read_double();
            double y = record.read_double();
            line.add_coord(x, y);
        }
        return std::move(mapnik::new_geometry::geometry(std::move(line)));
    }
    else
    {
        std::vector<int> parts(num_parts);
        for (int i = 0; i < num_parts; ++i)
        {
            parts[i] = record.read_ndr_integer();
        }

        int start, end;
        mapnik::new_geometry::multi_line_string multi_line;
        for (int k = 0; k < num_parts; ++k)
        {
            start = parts[k];
            if (k == num_parts - 1)
            {
                end = num_points;
            }
            else
            {
                end = parts[k + 1];
            }

            mapnik::new_geometry::line_string line;
            line.reserve(end - start);
            for (int j = start; j < end; ++j)
            {
                double x = record.read_double();
                double y = record.read_double();
                line.add_coord(x, y);
            }
            multi_line.push_back(std::move(line));
        }
        return std::move(mapnik::new_geometry::geometry(std::move(multi_line)));
    }
}

template <typename T>
bool is_clockwise(T const& points, int start, int end)
{
    int num_points = end - start;
    double area = 0.0;
    for (int i = 0; i < num_points; ++i)
    {
        auto const& p0 = points[start + i];
        auto const& p1 = points[start + (i + 1) % num_points];
        area += std::get<0>(p0) * std::get<1>(p1) - std::get<1>(p0) * std::get<0>(p1);
    }
    return ( area < 0.0) ? true : false;
}

mapnik::new_geometry::geometry shape_io::read_polygon(shape_file::record_type & record)
{
// FIXME
    return std::move(mapnik::new_geometry::geometry(mapnik::new_geometry::polygon3()));
#if 0
    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();
    std::vector<int> parts(num_parts);

    using points_cont = std::vector<std::tuple<double,double> >;
    points_cont points;
    points.reserve(num_points);

    for (int i = 0; i < num_parts; ++i)
    {
        parts[i] = record.read_ndr_integer();
    }

    for (int k = 0; k < num_points; ++k)
    {
        double x = record.read_double();
        double y = record.read_double();
        points.emplace_back(x,y);
    }

    mapnik::new_geometry::polygon3 poly;
    for (int k = 0; k < num_parts; ++k)
    {
        int start = parts[k];
        int end;
        if (k == num_parts - 1) end = num_points;
        else end = parts[k + 1];
        auto const& pt = points[start];
        double x = std::get<0>(pt);
        double y = std::get<1>(pt);
        if ( k > 0 && is_clockwise(points, start, end))
        {
            geom.push_back(poly.release());
            poly.reset(new geometry_type(mapnik::geometry_type::types::Polygon));
        }
        poly->move_to(x, y);
        for (int j = start + 1; j < end; ++j)
        {
            auto const& pt = points[j];
            x = std::get<0>(pt);
            y = std::get<1>(pt);
            poly->line_to(x, y);
        }
        poly->close_path();
    }
    geom.push_back(poly.release());
#endif
}
