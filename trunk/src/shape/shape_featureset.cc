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

#include "shape_featureset.hh"
#include <iostream>

template <typename filterT>
ShapeFeatureset<filterT>::ShapeFeatureset(const filterT& filter, const std::string& shape_file, long file_length, int srid)
    : srid_(srid),
      filter_(filter),
      shape_type_(shape_io::shape_null),
      shape_(shape_file),
      query_ext_(),
      file_length_(file_length),
      count_(0)
{
    shape_.shp().skip(100);
}


template <typename filterT>
Feature* ShapeFeatureset<filterT>::next()
{
    VectorFeature* feature=0;
    unsigned long pos=shape_.shp().pos(); // should be pos_type (or streampos)!!!
    
    if (pos < file_length_ * 2)
    {
	int id = shape_.shp().read_xdr_integer();
	int record_len = shape_.shp().read_xdr_integer();
	int type = shape_.shp().read_ndr_integer();
        
	if (type == shape_io::shape_point)
	{
	    coord<double,2> c;
            shape_.shp().read_coord(c);
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(c.x,c.y);
            feature=new VectorFeature(pos,point);
	    ++count_;
	}
	else if (type == shape_io::shape_pointz)
	{
	    coord<double,3> c;
	    shape_.shp().read_coord(c);
	    geometry_ptr point(new point_impl(-1));
	    point->move_to(c.x,c.y);
            feature=new VectorFeature(pos,point);
	    ++count_;
	}
	else
	{
	    Envelope<double> extent;
	    shape_.shp().read_envelope(extent);
	    while (!filter_.pass(extent))
	    {	
		shape_.shp().skip(2 * record_len + 8 - (4 * 3 + 8 * 4));
		if (shape_.shp().pos() >= file_length_ * 2)
		    return 0;
		id = shape_.shp().read_xdr_integer();
		record_len = shape_.shp().read_xdr_integer();
		type = shape_.shp().read_ndr_integer();
		shape_.shp().read_envelope(extent);
	    }
	    
	    switch (type)
            {
	    case shape_io::shape_polyline:
                {
                    geometry_ptr line = shape_.read_polyline();
                    feature=new VectorFeature(pos,line);
		    ++count_;
                    break;
                }
	    case shape_io::shape_polygon:
                {
                    geometry_ptr poly = shape_.read_polygon();
                    feature=new VectorFeature(pos,poly);
		    ++count_;
                    break;
                }
	    default:
		return 0;//TODO
            }
	    
            if (0)
            {
                shape_.dbf().move_to(id);
                for (int j=0;j<shape_.dbf().num_fields();++j) {
                  shape_.dbf().add_attribute(j,feature); //TODO optimize!!!
                }
            }
	}
    }
    if (!feature)
	std::cout<<" total shapes read="<<count_<<"\n";
    return feature;
}


template <typename filterT>
ShapeFeatureset<filterT>::~ShapeFeatureset() {}

template class ShapeFeatureset<filter_in_box>;
template class ShapeFeatureset<filter_at_point>;
