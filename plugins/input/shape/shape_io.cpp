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

#include "shape_io.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/util/is_clockwise.hpp>
#include <mapnik/geometry/correct.hpp>

using mapnik::datasource_exception;
const std::string shape_io::SHP = ".shp";
const std::string shape_io::SHX = ".shx";
const std::string shape_io::DBF = ".dbf";
const std::string shape_io::INDEX = ".index";

shape_io::shape_io(std::string const& shape_name, bool open_index)
    : type_(shape_null),
      shp_(shape_name + SHP),
      shx_(shape_name + SHX),
      dbf_(shape_name + DBF),
      reclength_(0),
      id_(0)
{
    bool ok = (shp_.is_open() && dbf_.is_open());
    if (!ok)
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
    if  (!index_ && !shx_.is_open())
    {
        throw datasource_exception("Shape Plugin: cannot read shape index file '" + shape_name + ".shx'");
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

shape_file& shape_io::shx()
{
    return shx_;
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

mapnik::geometry::geometry<double> shape_io::read_polyline(shape_file::record_type & record)
{
    mapnik::geometry::geometry<double> geom; // default empty
    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();

    if (num_parts == 1)
    {
        mapnik::geometry::line_string<double> line;
        line.reserve(num_points);
        record.skip(4);
        for (int i = 0; i < num_points; ++i)
        {
            double x = record.read_double();
            double y = record.read_double();
            line.emplace_back(x, y);
        }
        geom = std::move(line);
    }
    else
    {
        std::vector<int> parts;
        parts.resize(num_parts);
        std::for_each(parts.begin(), parts.end(), [&](int & part) { part = record.read_ndr_integer();});
        int start, end;
        mapnik::geometry::multi_line_string<double> multi_line;
        multi_line.reserve(num_parts);
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

            mapnik::geometry::line_string<double> line;
            line.reserve(end - start);
            for (int j = start; j < end; ++j)
            {
                double x = record.read_double();
                double y = record.read_double();
                line.emplace_back(x, y);
            }
            multi_line.push_back(std::move(line));
        }
        geom = std::move(multi_line);
    }
    return geom;
}

mapnik::geometry::geometry<double> shape_io::read_polyline_parts(shape_file::record_type & record, std::vector<std::pair<int, int>> const& parts)
{
    mapnik::geometry::geometry<double> geom; // default empty
    int total_num_parts = record.read_ndr_integer();
    int num_parts = parts.size();
    mapnik::geometry::multi_line_string<double> multi_line;
    multi_line.reserve(num_parts);
    for (int k = 0; k < num_parts; ++k)
    {
        int start = parts[k].first;
        int end = parts[k].second;
        unsigned pos = 4 + 32 + 8 + 4 * total_num_parts + start * 16;
        record.set_pos(pos);

        mapnik::geometry::line_string<double> line;
        line.reserve(end - start);
        for (int j = start; j < end; ++j)
        {
            double x = record.read_double();
            double y = record.read_double();
            line.emplace_back(x, y);
        }
        multi_line.push_back(std::move(line));
    }
    geom = std::move(multi_line);
    return geom;
}


mapnik::geometry::geometry<double> shape_io::read_polygon(shape_file::record_type & record)
{
    mapnik::geometry::geometry<double> geom; // default empty
    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();

    std::vector<int> parts;
    parts.resize(num_parts);
    std::for_each(parts.begin(), parts.end(), [&](int & part) { part = record.read_ndr_integer();});
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::multi_polygon<double> multi_poly;
    for (int k = 0; k < num_parts; ++k)
    {
        int start = parts[k];
        int end;
        if (k == num_parts - 1) end = num_points;
        else end = parts[k + 1];

        mapnik::geometry::linear_ring<double> ring;
        ring.reserve(end - start);
        for (int j = start; j < end; ++j)
        {
            double x = record.read_double();
            double y = record.read_double();
            ring.emplace_back(x, y);
        }
        if (k == 0)
        {
            poly.push_back(std::move(ring));
        }
        else if (mapnik::util::is_clockwise(ring))
        {
            multi_poly.emplace_back(std::move(poly));
            poly.clear();
            poly.push_back(std::move(ring));
        }
        else
        {
            poly.push_back(std::move(ring));
        }
    }

    if (multi_poly.size() > 0) // multi
    {
        multi_poly.emplace_back(std::move(poly));
        geom = std::move(multi_poly);
    }
    else
    {
        geom = std::move(poly);
    }
    mapnik::geometry::correct(geom);
    return geom;
}

mapnik::geometry::geometry<double> shape_io::read_polygon_parts(shape_file::record_type & record, std::vector<std::pair<int,int>> const& parts)
{
    mapnik::geometry::geometry<double> geom; // default empty
    int total_num_parts = record.read_ndr_integer();
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::multi_polygon<double> multi_poly;
    int num_parts = parts.size();
    for (int k = 0; k < num_parts; ++k)
    {
        int start = parts[k].first;
        int end = parts[k].second;
        unsigned pos = 4 + 32 + 8 + 4 * total_num_parts + start * 16;
        record.set_pos(pos);
        mapnik::geometry::linear_ring<double> ring;
        ring.reserve(end - start);
        for (int j = start; j < end; ++j)
        {
            double x = record.read_double();
            double y = record.read_double();
            ring.emplace_back(x, y);
        }
        if (k == 0)
        {
            poly.push_back(std::move(ring));
        }
        else if (mapnik::util::is_clockwise(ring))
        {
            multi_poly.emplace_back(std::move(poly));
            poly.clear();
            poly.push_back(std::move(ring));
        }
        else
        {
            poly.push_back(std::move(ring));
        }
    }

    if (multi_poly.size() > 0) // multi
    {
        multi_poly.emplace_back(std::move(poly));
        geom = std::move(multi_poly);
    }
    else
    {
        geom = std::move(poly);
    }
    mapnik::geometry::correct(geom);
    return geom;
}
