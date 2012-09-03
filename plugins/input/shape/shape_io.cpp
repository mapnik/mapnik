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

#include "shape_io.hpp"

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>

using mapnik::datasource_exception;
using mapnik::geometry_type;

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
            index_= boost::make_shared<shape_file>(shape_name + INDEX);
        }
        catch (...)
        {
            MAPNIK_LOG_WARN(shape) << "shape_io: Could not open index=" << shape_name << INDEX;
        }
    }
}

shape_io::~shape_io() {}

void shape_io::move_to(int pos)
{
    shp_.seek(pos);
    id_ = shp_.read_xdr_integer();
    reclength_ = shp_.read_xdr_integer();
    type_ = static_cast<shape_io::shapeType>(shp_.read_ndr_integer());

    if (type_ != shape_null && type_ != shape_point && type_ != shape_pointm && type_ != shape_pointz)
    {
        shp_.read_envelope(cur_extent_);
    }
}

shape_io::shapeType shape_io::type() const
{
    return type_;
}

const box2d<double>& shape_io::current_extent() const
{
    return cur_extent_;
}

shape_file& shape_io::shp()
{
    return shp_;
}

dbf_file& shape_io::dbf()
{
    return dbf_;
}

void shape_io::read_polyline(mapnik::geometry_container & geom)
{
    shape_file::record_type record(reclength_ * 2 - 36);
    shp_.read_record(record);

    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();
    if (num_parts == 1)
    {
        std::auto_ptr<geometry_type> line(new geometry_type(mapnik::LineString));
        record.skip(4);
        double x = record.read_double();
        double y = record.read_double();
        line->move_to(x, y);
        for (int i = 1; i < num_points; ++i)
        {
            x = record.read_double();
            y = record.read_double();
            line->line_to(x, y);
        }
        geom.push_back(line);
    }
    else
    {
        std::vector<int> parts(num_parts);
        for (int i = 0; i < num_parts; ++i)
        {
            parts[i] = record.read_ndr_integer();
        }

        int start, end;
        for (int k = 0; k < num_parts; ++k)
        {
            std::auto_ptr<geometry_type> line(new geometry_type(mapnik::LineString));
            start = parts[k];
            if (k == num_parts - 1)
            {
                end = num_points;
            }
            else
            {
                end = parts[k + 1];
            }

            double x = record.read_double();
            double y = record.read_double();
            line->move_to(x, y);

            for (int j = start + 1; j < end; ++j)
            {
                x = record.read_double();
                y = record.read_double();
                line->line_to(x, y);
            }
            geom.push_back(line);
        }
    }
    // z-range
    //double z0=record.read_double();
    //double z1=record.read_double();
    //for (int i=0;i<num_points;++i)
    // {
    //  double z=record.read_double();
    // }

    // m-range
    //double m0=record.read_double();
    //double m1=record.read_double();

    //for (int i=0;i<num_points;++i)
    //{
    //   double m=record.read_double();
    //}

}

void shape_io::read_polygon(mapnik::geometry_container & geom)
{
    shape_file::record_type record(reclength_ * 2 - 36);
    shp_.read_record(record);

    int num_parts = record.read_ndr_integer();
    int num_points = record.read_ndr_integer();
    std::vector<int> parts(num_parts);

    for (int i = 0; i < num_parts; ++i)
    {
        parts[i] = record.read_ndr_integer();
    }

    for (int k = 0; k < num_parts; ++k)
    {
        std::auto_ptr<geometry_type> poly(new geometry_type(mapnik::Polygon));
        int start = parts[k];
        int end;
        if (k == num_parts - 1)
        {
            end = num_points;
        }
        else
        {
            end = parts[k + 1];
        }

        double x = record.read_double();
        double y = record.read_double();
        poly->move_to(x, y);

        for (int j=start+1;j<end-1;j++)
        {
            x = record.read_double();
            y = record.read_double();
            poly->line_to(x, y);
        }
        x = record.read_double();
        y = record.read_double();
        poly->close(x, y);

        geom.push_back(poly);
    }
    // z-range
    //double z0=record.read_double();
    //double z1=record.read_double();
    //for (int i=0;i<num_points;++i)
    //{
    //  double z=record.read_double();
    //}

    // m-range
    //double m0=record.read_double();
    //double m1=record.read_double();

    //for (int i=0;i<num_points;++i)
    //{
    //   double m=record.read_double();
    //}
}
