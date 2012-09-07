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

#ifndef SHAPE_IO_HPP
#define SHAPE_IO_HPP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "dbfile.hpp"
#include "shapefile.hpp"
#include "shp_index.hpp"

struct shape_io : boost::noncopyable
{
public:
    enum shapeType
    {
        shape_null = 0,
        shape_point = 1,
        shape_polyline = 3,
        shape_polygon = 5,
        shape_multipoint = 8,
        shape_pointz = 11,
        shape_polylinez = 13,
        shape_polygonz = 15,
        shape_multipointz = 18,
        shape_pointm = 21,
        shape_polylinem = 23,
        shape_polygonm = 25,
        shape_multipointm = 28,
        shape_multipatch = 31
    };

    shape_io(std::string const& shape_name, bool open_index=true);
    ~shape_io();

    shape_file& shp();
    dbf_file& dbf();

    inline boost::shared_ptr<shape_file>& index()
    {
        return index_;
    }

    inline bool has_index() const
    {
        return (index_ && index_->is_open());
    }

    void move_to(int id);
    shapeType type() const;
    const box2d<double>& current_extent() const;
    void read_polyline(mapnik::geometry_container & geom);
    void read_polygon(mapnik::geometry_container & geom);
    shapeType type_;
    shape_file shp_;
    dbf_file   dbf_;
    boost::shared_ptr<shape_file> index_;
    unsigned reclength_;
    unsigned id_;
    box2d<double> cur_extent_;

    static const std::string SHP;
    static const std::string DBF;
    static const std::string INDEX;
};

#endif //SHAPE_IO_HPP
