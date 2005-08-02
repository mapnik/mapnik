/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef SHAPE_IO_HH
#define SHAPE_IO_HH

#include "dbffile.hpp"
#include "shapefile.hpp"
#include "shp_index.hpp"

using mapnik::geometry_ptr;

struct shape_io
{
    static const std::string SHP;
    static const std::string SHX;
    static const std::string DBF;

    shape_file shp_;
    shape_file shx_;
    dbf_file   dbf_;
    unsigned type_;
    unsigned reclength_;
    unsigned id_;
    Envelope<double> cur_extent_;

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

    shape_io(const std::string& shape_name);
    ~shape_io();
    shape_file& shp();
    shape_file& shx();
    dbf_file& dbf();
    void move_to(int id);
    int type() const;
    const Envelope<double>& current_extent() const;

    geometry_ptr read_polyline();
    geometry_ptr read_polylinem();
    geometry_ptr read_polylinez();
    geometry_ptr read_polygon();
    geometry_ptr read_polygonm();
    geometry_ptr read_polygonz();
private:
    //void read_record(const shape_record& record);
    // no copying
    shape_io(const shape_io&);
    shape_io& operator=(const shape_io&);
};
#endif                                            //SHAPE_IO_HH
