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

//$Id$

#include "shape_io.hh"
#include "shape.hh"

const std::string shape_io::SHP = ".shp";
const std::string shape_io::DBF = ".dbf";

shape_io::shape_io(const std::string& shape_name)
    : type_(shape_null)
{
    bool ok = (shp_.open(shape_name + SHP) &&
	       dbf_.open(shape_name + DBF));
    if (!ok)
    { 
	throw datasource_exception("cannot read shape file");
    }
}

shape_io::~shape_io()
{
    shp_.close();
    dbf_.close();
}


void shape_io::move_to (int pos)
{
    shp_.seek(pos);
    int id = shp_.read_xdr_integer();
    int reclength_ = shp_.read_xdr_integer();
    type_ = shp_.read_ndr_integer();
    if (type_ != shape_point)
    {
        shp_.read_envelope(cur_extent_);
    }
}

int shape_io::type() const
{
    return type_;
}

const Envelope<double>& shape_io::current_extent() const
{
    return cur_extent_;
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

mapnik::geometry_ptr shape_io::read_polyline()
{
    int num_parts=shp_.read_ndr_integer();
    int num_points=shp_.read_ndr_integer();
    std::vector<int> parts(num_parts);;
    for (int i=0;i<num_parts;++i)
    {
        parts[i]=shp_.read_ndr_integer();
    }

    coord_array<coord<double,2> >  ar(num_points);
    shp_.read_coords(ar);

    geometry_ptr line(new line_string_impl(-1));

    for (int k=0;k<num_parts;++k)
    {
        int start=parts[k];
        int end;
        if (k==num_parts-1)
            end=num_points;
        else
            end=parts[k+1];
        line->move_to(ar[start].x,ar[start].y);
        for (int j=start+1;j<end;++j)
        {
             line->line_to(ar[j].x,ar[j].y);
        }
    }
    return line;
}

mapnik::geometry_ptr shape_io::read_polygon()
{
    int num_parts=shp_.read_ndr_integer();
    int num_points=shp_.read_ndr_integer();
    std::vector<int> parts(num_parts);
    geometry_ptr poly(new polygon_impl(-1));
    for (int i=0;i<num_parts;++i)
    {
        parts[i]=shp_.read_ndr_integer();
    }

    coord_array<coord<double,2> >  ar(num_points);
    shp_.read_coords(ar);

    for (int k=0;k<num_parts;++k)
    {
        int start=parts[k];
        int end;
        if (k==num_parts-1)
        {
            end=num_points;
        }
        else
        {
            end=parts[k+1];
        }
        poly->move_to(ar[start].x,ar[start].y);
   
        for (int j=start+1;j<end;j++)
        {
            poly->line_to(ar[j].x,ar[j].y);
        }

	poly->line_to(ar[start].x,ar[start].y);
    }
    return poly;
}
